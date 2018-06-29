#pragma once

#include <string>
#include <memory>
#include <cassert>
#include <fstream>
#include <iterator>
#include <algorithm>
#include <functional>
#include "sformat.h"

#ifdef _DEBUG

//  条件判断, 抛出异常
#define DEBUG_CHECK(exp, type, ...) if (!exp) { throw type(__VA_ARGS__); }

//  异常说明, 显示Json错误位置往后20个字符
#define DEBUG_EXPINFO(exp, string) SFormat("{0}: {1}", exp, std::string(string).substr(0, 20))

#endif

class JValue {
public:
    using JValuePtr = std::shared_ptr<JValue>;

    struct Parser {
        //  解析异常
        class Exception : public std::exception {
        public:
            Exception(const std::string & what): _what(what), std::exception(what.c_str())
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

        static const char * ParseList(const char * string, std::vector<JValuePtr> * output);
        static const char * ParseHash(const char * string, std::vector<JValuePtr> * output);
        static const char * ParseString(const char * string, std::string * output);
        static const char * ParseNumber(const char * string, double * output);
        static const char * ParseFalse(const char * string, double * output);
        static const char * ParseTrue(const char * string, double * output);
        static const char * Parse(const char * string, JValue * output);
    };

public:
    enum JType {
        kNUMBER,
        kSTRING,
        kHASH,
        kLIST,
        kBOOL,
        kNULL,
    };

    template <class T>
    struct IsString: public std::false_type {};
    template <>
    struct IsString<char *>: public std::true_type {};
    template <int N>
    struct IsString<char[N]>: public std::true_type {};
    template <>
    struct IsString<std::string>: public std::true_type {};

    template <class T>
    struct IsJValue: public std::false_type {};
    template <>
    struct IsJValue<JValue>: public std::true_type {};

    template <class T>
    struct IsInteger: public std::false_type {};
    template <>
    struct IsInteger<std::int8_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int16_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int32_t>: public std::true_type {};
    template <>
    struct IsInteger<std::int64_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint8_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint16_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint32_t>: public std::true_type {};
    template <>
    struct IsInteger<std::uint64_t>: public std::true_type {};

    template <class T>
    struct IsNumber: public std::false_type {};
    template <>
    struct IsNumber<float>: public std::true_type {};
    template <>
    struct IsNumber<double>: public std::true_type {};

    template <class T>
    struct IsBool: public std::false_type {};
    template <>
    struct IsBool<bool>: public std::true_type {};

    static JValue FromFile(const std::string & fname)
    {
        std::ifstream ifile(fname);
        std::string buffer;
        std::copy(std::istream_iterator<char>(ifile), 
                std::istream_iterator<char>(), 
                std::back_inserter(buffer));
        return std::move(FromString(buffer));
    }

    static JValue FromString(const std::string & string)
    {
        JValue jvalue;
        try
        {
            Parser::Parse(string.c_str(), &jvalue);
        }
        catch (const Parser::Exception & error)
        {
            jvalue.Set(nullptr);
        }
        return std::move(jvalue);
    }

    JValue(): _type(JType::kNULL)
    { }

    template <class T>
    JValue(T && value)
    {
        Set(std::forward(value));
    }

    JValue(const JValue & json)
    {
        *this = json;
    }

    JValue(JValue && json)
    {
        *this = std::move(json);
    }

    JValue & operator =(const JValue & json)
    {
        _elements = json._elements;
        _number = json._number;
        _string = json._string;
        _type = json._type;
        _key = json._key;
        return *this;
    }

    JValue & operator =(JValue && json)
    {
        _elements = std::move(json._elements);
        _number = std::move(json._number);
        _string = std::move(json._string);
        _type = std::move(json._type);
        _key = std::move(json._key);
        return *this;
    }

    JType Type() const
    {
        return _type;
    }

    const std::string & Key() const
    {
        return _key;
    }

    int ToInt() const
    {
        return static_cast<int>(_number);
    }

    float ToFloat() const
    {
        return static_cast<float>(_number);
    }

    double ToDouble() const
    {
        return static_cast<double>(_number);
    }

    bool ToBool() const
    {
        return _number != 0;
    }

    const std::string & ToString() const
    {
        return _string;
    }

    const std::string & ToPrint() const
    {
        return Print();
    }

    template <class ...Keys>
    JValuePtr Get(const Keys & ...keys)
    {
        return GetImpl(keys...);
    }

    template <class T, class ...Keys>
    bool Set(const T & val, const Keys & ...keys)
    {
        return SetImpl(std::make_shared<JValue>(val), keys...);
    }

    template <class ...Keys>
    bool Set(const char * val, const Keys & ...keys)
    {
        return SetImpl(std::make_shared<JValue>(val), keys...);
    }

private:
    JValue Clone(const JValue jvalue) const
    {
        JValue newval;
        //switch (jvalue.Type())
        //{
        //case kNUMBER:
        //    {
        //        newval.Set(jvalue.ToDouble()); 
        //        newval.Key = jvalue.Key;
        //    }
        //    break;
        //case kSTRING:
        //    {
        //        newval.Set(jvalue.ToString()); 
        //        newval.Key = jvalue.Key;
        //    }
        //    break;
        //case kHASH:
        //case kLIST:
        //    {
        //        std::transform(std::cbegin(_elements),
        //                       std::cend(_elements),
        //                       std::back_inserter(newval._elements),
        //                       std::bind(&JValue::ClonePtr, this, std::placeholders::_1));
        //    }
        //    break;
        //case kBOOL:
        //    newval.Set(jvalue.ToBool()); break;
        //}
        return std::move(newval);
    }

    JValuePtr ClonePtr(const JValuePtr & ptr) const
    {
        return std::make_shared<JValue>(std::move(Clone(*ptr)));
    }

    template <class Key, class ...Keys>
    JValuePtr GetImpl(const Key & key, const Keys & ...keys)
    {
        auto jptr = GetImpl(key);
        if (jptr != nullptr)
        {
            return jptr->GetImpl(keys...);
        }
        return nullptr;
    }

    template <class Key>
    JValuePtr GetImpl(const Key & key)
    {
        if constexpr (IsJValue<Key>::value)
        {
            return key._cjson;
        }
        if constexpr (IsInteger<Key>::value)
        {
            return _elements.at(key);
        }
        if constexpr (IsString<Key>::value)
        {
            return *std::find(std::begin(_elements), std::end(_elements), key);
        }
        return nullptr;
    }

    template <class Key1, class Key2, class ...Keys>
    bool SetImpl(cJSON * root, cJSON * val, const Key1 & key1, const Key2 & key2, const Keys & ...keys)
    {
        if constexpr (IsJValue<Key1>::value)
        {
            return SetImpl(key1._cjson, val, key2, keys...);
        }
        if constexpr (!IsJValue<Key1>::value)
        {
            return SetImpl(GetImpl(root, key1), val, key2, keys...);
        }
    }

    template <class Value, class Key>
    bool SetImpl(const Value & val, const Key & key)
    {
        auto jptr = GetImpl(key);
        if (jptr == nullptr)
        {
            if constexpr (IsString<Key>::value)
            {
                auto newval = std::make_shared<JValue>(val);
                newval->_key = key;
                _elements.push(newval);
            }
            if constexpr (IsInteger<Key>::value)
            {
                _elements.push(std::make_shared<JValue>(val));
            }
        }
        else
        {
            jptr* = val;
        }

        if constexpr (IsInteger<Key>::value)
        {
        }
        if constexpr (IsString<Key>::value)
        {

        }
        root = GetImpl(root, key1);
        if constexpr (IsInteger<Key2>::value)
        {
            if (!cJSON_IsArray(root)) { return false; }
            if (key2 >= cJSON_GetArraySize(root))
            {
                cJSON_AddItemToArray(root, val);
            }
            else
            {
                cJSON_DeleteItemFromArray(root, key2);
                cJSON_InsertItemInArray(root, key2, val);
            }
        }
        if constexpr (IsString<Key2>::value)
        {
            if (!cJSON_IsObject(root)) { return false; }
            cJSON_DeleteItemFromObject(root, key2.c_str());
            cJSON_AddItemToObject(root, key2.c_str(), val);
        }
        if constexpr (IsCharArr<Key2>::value)
        {
            if (!cJSON_IsObject(root)) { return false; }
            cJSON_DeleteItemFromObject(root, key2);
            cJSON_AddItemToObject(root, key2, val);
        }
        return true;
    }

    bool SetImpl(const JValuePtr & value)
    {

        //  TODO
        //if constexpr (IsInteger<Value>::value || IsNumber<Value>::value)
        //{
        //    _type = kNUMBER;
        //    _number = value;
        //}
        //if constexpr (IsString<Value>::value)
        //{
        //    _type = kSTRING;
        //    _string = value;
        //}
        //if constexpr (IsBool<Value>::value)
        //{
        //    _type = kBOOL;
        //    _number = value != 0;
        //}
        //if constexpr (IsJValue<Value>::value)
        //{
        //    *this = value;
        //}
        return true;
    }

    const std::string & Print() const;

private:
    std::vector<JValuePtr> _elements;
    std::string _string;
    std::string _key;
    double _number;
    JType _type;

    friend struct Parser;
};

inline bool operator==(const JValue::JValuePtr & ptr, const std::string & key)
{
    return ptr->Key() == key;
}

inline bool operator!=(const JValue::JValuePtr & ptr, const std::string & key)
{
    return ptr->Key() != key;
}

inline bool operator==(const JValue::JValuePtr & ptr, const char * key)
{
    return ptr->Key() == key;
}

inline bool operator!=(const JValue::JValuePtr & ptr, const char * key)
{
    return ptr->Key() != key;
}