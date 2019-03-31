#pragma once

#include <string>
#include <memory>
#include <vector>
#include <cassert>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include <type_traits>
#include "string_tool.h"
#include "sformat.h"

//  条件判断, 抛出异常
#define DEBUG_CHECK(exp, type, ...) if (!exp) { throw type(__VA_ARGS__); }

//  异常说明, 显示Json错误位置往后20个字符
#define DEBUG_ERROR(exp, string) SFormat("{0}: {1}", exp, std::string(string).substr(0, 20))

namespace mmc {
    class JsonValue: public std::enable_shared_from_this<JsonValue> {
    public:
        using Entiy = std::shared_ptr<JsonValue>;
		using Value = std::weak_ptr<JsonValue>;
		struct Child {
			Child() {}
			Child(const Value & value) : mValue(value) {}
            Child(const Entiy & entiy) : mValue(entiy) {}
			Child(const Value & value, const std::string & key)
				: mValue(value), mKey(key) 
			{ }
            Child(const Entiy & entiy, const std::string & key)
                : mValue(entiy), mKey(key)
            { }
			bool operator==(const Value & other) const
			{
				return Lock() == other.lock();
			}
			bool operator==(const Entiy & other) const
			{
				return Lock() == other;
			}
			bool operator==(const std::string & key) const 
			{
				return mKey == key; 
			}
			Entiy Lock() const { return mValue.lock(); }
			//	key 不保存在节点中
			std::string mKey;
			//	weak_ptr
			Value mValue;
		};

        enum Type {
            kNUMBER,
            kSTRING,
            kHASH,
            kLIST,
            kBOOL,
            kNULL,
        };

    private:
		static Value s_nullptr;
		
		static std::vector<Entiy> s_entiys;

	public:
        static void GC()
        {
			auto it = std::remove_if(s_entiys.begin(), s_entiys.end(), [](const Entiy & entiy) {
				for (const auto & val : s_entiys)
				{
					return entiy.use_count() == 1 
                        && std::find(val->GetElements().begin(), 
                                     val->GetElements().end(), entiy) == val->GetElements().end();
				}
				return true;
			});
			s_entiys.erase(it, s_entiys.end());
        }

    private:
        struct Parser {
            class Exception : public std::exception {
            public:
                Exception(const std::string & what) : _what(what), std::exception(what.c_str())
                {}

            private:
                std::string _what;
            };

            static const char * SkipSpace(const char * string)
            {
                for (; *string != '\0' && *string <= 32; ++string)
                    ;
                return string;
            }

            static const char * ParseList(const char * string, std::vector<Child> * childs)
            {
                childs->clear();
                while (*string != ']')
                {
					auto value = FromValue();
                    string = Parser::Parse(string, &value);
                    string = SkipSpace(string);
                    *string == ',' && ++string;
                    string = SkipSpace(string);
                    childs->push_back(value);
                }
                return ++string;
            }

            static const char * ParseHash(const char * string, std::vector<Child> * childs)
            {
                childs->clear();
                while (*string != '}')
                {
                    DEBUG_CHECK((*string == '\"'), Parser::Exception, DEBUG_ERROR("Parse Hash Error: ", string));
					std::string key; auto value = FromValue();
                    string = SkipSpace(Parser::ParseString(string + 1, &key));
                    DEBUG_CHECK((*string == ':'), Parser::Exception, DEBUG_ERROR("Parse Hash Error: ", string));
                    string = SkipSpace(Parser::Parse(string + 1, &value));
                    *string == ',' && ++string;
                    string = SkipSpace(string);
                    childs->emplace_back(value, key);
                }
                return ++string;
            }

            static const char * ParseString(const char * string, std::string * output)
            {
                output->clear();
                for (; *string != '\"'; ++string)
                {
                    DEBUG_CHECK(*string != '\0', Parser::Exception, DEBUG_ERROR("Parse String Error", string));
                    output->append(1, *string);
                }
                return string + 1;
            }

            static const char * ParseNumber(const char * string, double * output)
            {
                char value[64] = { 0 };
                for (auto i = 0; 
					*string >= '0' &&
                    *string <= '9' ||
                    *string == '.' ||
                    *string == '-'; ++i, ++string)
                {
                    value[i] = *string;
                }
                *output = std::strtod(value, nullptr);
                return string;
            }

            static const char * ParseFalse(const char * string, double * output)
            {
                *output = 0; return string + 5;
            }

            static const char * ParseTrue(const char * string, double * output)
            {
                *output = 1; return string + 4;
            }

            static const char * Parse(const char * string, Entiy * value)
            {
                string = Parser::SkipSpace(string);
                if (*string == '[')
                {
                    string = ParseList(SkipSpace(string + 1), &(*value)->_elems);
                    (*value)->_type = mmc::JsonValue::Type::kLIST;
                }
                else if (*string == '{')
                {
                    string = ParseHash(SkipSpace(string + 1), &(*value)->_elems);
                    (*value)->_type = mmc::JsonValue::Type::kHASH;
                }
                else if (*string == '\"')
                {
                    string = ParseString(string + 1, &(*value)->_string);
                    (*value)->_type = mmc::JsonValue::Type::kSTRING;
                }
                else if (*string >= '0' && *string <= '9')
                {
                    string = ParseNumber(string, &(*value)->_number);
                    (*value)->_type = mmc::JsonValue::Type::kNUMBER;
                }
                else if (string[0] == 't' &&
                         string[1] == 'r' &&
                         string[2] == 'u' &&
                         string[3] == 'e')
                {
                    string = ParseTrue(string, &(*value)->_number);
                    (*value)->_type = mmc::JsonValue::Type::kBOOL;
                }
                else if (string[0] == 'f' &&
                         string[1] == 'a' &&
                         string[2] == 'l' &&
                         string[3] == 's' &&
                         string[4] == 'e')
                {
                    string = ParseFalse(string, &(*value)->_number);
					(*value)->_type = mmc::JsonValue::Type::kBOOL;
                }
                else
                {
                    DEBUG_CHECK(false, Parser::Exception, DEBUG_ERROR("Parse Json Error", string));
                }
                return string;
            }
        };

	public:
		JsonValue() : _type(Type::kNULL), _number(0) { }

		template <class T>
		static Entiy FromValue(const T & val)
		{
			auto value = FromValue();
			value->Set(val);
			return value;
		}

		static Entiy FromValue(const std::string & val)
		{
			return FromValue(val.c_str(), val.size());
		}

		static Entiy FromValue(const char * val, size_t len = 0)
		{
			auto value =	FromValue();
			value->Set(val, len);
			return value;
		}
		
		static Entiy FromValue()
		{
			auto value = std::make_shared<JsonValue>();
			s_entiys.push_back(value);
			return value;
		}

		static Entiy FromHash()
		{
			auto value = FromValue();
			value->SetHash();
			return value;
		}

		static Entiy FromList()
		{
			auto value = FromValue();
			value->SetList();
			return value;
		}

		static Entiy FromFile(const std::string & fname)
		{
			std::string buffer;
			std::ifstream ifile(fname);
			std::copy(std::istream_iterator<char>(ifile),
				std::istream_iterator<char>(),
				std::back_inserter(buffer));
			return FromValue(buffer);
		}

		static Entiy FromBuffer(const std::string & buffer)
		{
			return FromBuffer(buffer.c_str());
		}

		static Entiy FromBuffer(const char * buffer)
		{
			auto value = FromValue();
			try
			{
				Parser::Parse(buffer, &value);
			}
			catch (const Parser::Exception &)
			{
				value.reset();
			}
			return value;
		}

        const std::vector<Child> & GetElements() const
        {
            return _elems;
        }

        std::vector<Child> & GetElements()
        {
            return _elems;
        }

        Type GetType() const
        {
            return _type;
        }

        int ToInt() const
        {
            return static_cast<int>(_number);
        }

		bool ToBool() const
		{
			return _number != 0;
		}

        float ToFloat() const
        {
            return static_cast<float>(_number);
        }

        double ToDouble() const
        {
            return static_cast<double>(_number);
        }

		const std::string & ToString() const
		{
			return _string;
		}

		void Set(const char * val, size_t len = 0) 
		{ 
			_elems.clear();
			_string = 0 == len ? val
			: std::string(val, len);
			_type = Type::kSTRING; 
		}
		
		void Set(const std::double_t val) 
		{ 
			_number = val;
			_elems.clear();
			_type = Type::kNUMBER; 
		}

		void Set(const bool  val) 
		{ 
			_number = val ? 1 : 0;
			_elems.clear();
			_type = Type::kBOOL; 
		}

		void Set(const std::uint16_t val) { Set(static_cast<double>(val)); }
		void Set(const std::uint32_t val) { Set(static_cast<double>(val)); }
		void Set(const std::uint64_t val) { Set(static_cast<double>(val)); }
		void Set(const std::string & val) { Set(val.c_str(), val.size()); }
		void Set(const std::int16_t val) { Set(static_cast<double>(val)); }
		void Set(const std::int32_t val) { Set(static_cast<double>(val)); }
		void Set(const std::int64_t val) { Set(static_cast<double>(val)); }
		void Set(const std::float_t val) { Set(static_cast<double>(val)); }
        void SetHash() { _elems.clear(); _type = Type::kHASH; }
        void SetList() { _elems.clear(); _type = Type::kLIST; }
        void SetNull() { _elems.clear(); _type = Type::kNULL; }

		template <class Key, class ...Keys>
		Entiy Insert(const Value & value, const Key & key, Keys && ... keys)
		{
			return At(key).lock()->Insert(value, std::forward<Keys...>(keys...));
		}

		template <class Key>
		Entiy Insert(const Value & value, const Key & key)
		{
			return InsertImpl(value, key);
		}

        template <class Key, class ...Keys>
        Value & At(const Key & key, Keys && ... keys)
        {
            return At(key).lock()->At(std::forward<Keys...>(keys...));
        }

        template <class Key>
        Value & At(const Key & key)
        {
            return AtImpl(key);
        }

        size_t GetCount() const
        {
            return _elems.size();
        }

        std::string Print(size_t space = 0) const
        {
            switch (GetType())
            {
            case kNUMBER:
            {
                return std::to_string(_number);
            }
            break;
            case kSTRING:
            {
                return "\"" + _string + "\"";
            }
            break;
            case kHASH:
            {
                std::vector<std::string> strings;
                std::string resule("{\n" + std::string(++space, '\t'));
                for (const auto & ele : _elems)
                {
                    strings.push_back(SFormat("\"{0}\": {1}",
                        ele.mKey, ele.Lock()->Print(space)));
                }
                resule.append(string_tool::Join(strings, ",\n" + std::string(space, '\t')));
                resule.append("\n"); resule.append(--space, '\t'); resule.append("}");
                return std::move(resule);
            }
            break;
            case kLIST:
            {
                std::vector<std::string> strings;
                for (const auto & ele : _elems)
                {
                    strings.push_back(ele.Lock()->Print(space));
                }
                return "[" + string_tool::Join(strings, ", ") + "]";
            }
            break;
            case kBOOL:
            {
                return ToBool() ? "true" : "false";
            }
            break;
            }
            return "null";
        }

    private:
        Entiy InsertImpl(const Value & value, const char * key)
        {
            assert(_type == Type::kHASH);
            return InsertImpl(value, std::string(key));
        }

        Entiy InsertImpl(const Value & value, const std::string & key)
        {
            assert(_type == Type::kHASH);
            if (Find(key) == _elems.end())
            {
                _elems.emplace_back(value, key);
            }
            return shared_from_this();
        }

        Entiy InsertImpl(const Value & value, size_t idx = (size_t)-1)
        {
            assert(_type == Type::kLIST);
            auto insert = idx >= _elems.size()
                ? _elems.insert(_elems.end(), value)
                : std::next(_elems.begin(), idx);
            _elems.insert(insert, value);
            return shared_from_this();
        }

        Value & AtImpl(size_t idx)
        {
            assert(_type == Type::kLIST);
            return _elems.at(idx).mValue;
        }

        Value & AtImpl(const char * key)
        {
            assert(_type == Type::kHASH);
            return AtImpl(std::string(key));
        }

        Value & AtImpl(const std::string & key)
        {
            auto it = Find(key);
            if (it == _elems.end()) { return s_nullptr; }
            return const_cast<Value &>(it->mValue);
        }

		std::vector<Child>::const_iterator Find(const std::string & key) const
		{
			return std::find(_elems.begin(), _elems.end(), key);
		}

    private:
        std::vector<Child> _elems;
        std::string _string;
        double _number;
        Type _type;

        friend struct Parser;
    };
}

namespace std {
    inline vector<mmc::JsonValue::Child>::iterator begin(mmc::JsonValue & json)
    {
        return json.GetElements().begin();
    }

    inline vector<mmc::JsonValue::Child>::iterator end(mmc::JsonValue & json)
    {
        return json.GetElements().end();
    }

    inline vector<mmc::JsonValue::Child>::iterator begin(mmc::JsonValue::Value & json)
    {
        return begin(*json.lock());
    }

    inline vector<mmc::JsonValue::Child>::iterator end(mmc::JsonValue::Value & json)
    {
        return end(*json.lock());
    }

    inline vector<mmc::JsonValue::Child>::iterator begin(mmc::JsonValue::Entiy & json)
    {
        return begin(*json);
    }

    inline vector<mmc::JsonValue::Child>::iterator end(mmc::JsonValue::Entiy & json)
    {
        return end(*json);
    }

    inline vector<mmc::JsonValue::Child>::const_iterator begin(const mmc::JsonValue & json)
    {
        return json.GetElements().begin();
    }

    inline vector<mmc::JsonValue::Child>::const_iterator end(const mmc::JsonValue & json)
    {
        return json.GetElements().end();
    }

    inline vector<mmc::JsonValue::Child>::const_iterator begin(const mmc::JsonValue::Value & json)
    {
        return begin(*json.lock());
    }

    inline vector<mmc::JsonValue::Child>::const_iterator end(const mmc::JsonValue::Value & json)
    {
        return end(*json.lock());
    }

    inline vector<mmc::JsonValue::Child>::const_iterator begin(const mmc::JsonValue::Entiy & json)
    {
        return begin(*json);
    }

    inline vector<mmc::JsonValue::Child>::const_iterator end(const mmc::JsonValue::Entiy & json)
    {
        return end(*json);
    }

    inline std::string to_string(const mmc::JsonValue & json)
    {
        return json.Print(0);
    }

    inline std::string to_string(const mmc::JsonValue::Entiy & json)
    {
        return to_string(*json);
    }

    inline std::string to_string(const mmc::JsonValue::Value & json)
    {
        return to_string(json.lock());
    }
}