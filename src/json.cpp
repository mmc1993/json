#include "json.h"

const char * JValue::Parser::ParseList(const char * string, std::vector<JValuePtr> * output)
{
    output->clear();
    while (*string != ']')
    {
        auto element = std::make_shared<JValue>();
        string = Parser::Parse(string, element.get());
        string = SkipSpace(string);
        *string == ',' && ++string;
        string = SkipSpace(string);
        output->push_back(element);
    }
    return ++string;
}

const char * JValue::Parser::ParseHash(const char * string, std::vector<JValuePtr> * output)
{
    output->clear();
    while (*string != '}')
    {
        auto element = std::make_shared<JValue>();
        DEBUG_CHECK((*string == '\"'), Parser::Exception, DEBUG_EXPINFO("Parse Hash Error: ", string));
        string = Parser::ParseString(string + 1, &element->_key);
        string = SkipSpace(string);
        DEBUG_CHECK((*string == ':'), Parser::Exception, DEBUG_EXPINFO("Parse Hash Error: ", string));
        string = Parser::Parse(string + 1, element.get());
        string = SkipSpace(string);
        *string == ',' && ++string;
        string = SkipSpace(string);
        output->push_back(element);
    }
    return ++string;
}

const char * JValue::Parser::ParseString(const char * string, std::string * output)
{
    output->clear();
    for (; *string != '\"'; ++string)
    {
        DEBUG_CHECK(*string != '\0', Parser::Exception, DEBUG_EXPINFO("Parse String Error", string));
        output->append(1, *string);
    }
    return string + 1;
}

const char * JValue::Parser::ParseNumber(const char * string, double * output)
{
    char value[64] = { 0 };
    for (auto i = 0; *string >= '0' && 
                     *string <= '9' || 
                     *string == '.'; ++i, ++string)
    {
        value[i] = *string;
    }
    *output = std::strtod(value, nullptr);
    return string;
}

const char * JValue::Parser::ParseFalse(const char * string, double * output)
{
    *output = 0;
    return string + 5;
}

const char * JValue::Parser::ParseTrue(const char * string, double * output)
{
    *output = 1;
    return string + 4;
}

const char * JValue::Parser::Parse(const char * string, JValue * output)
{
    string = Parser::SkipSpace(string);
    if (*string == '[')
    {
        string = ParseList(SkipSpace(string + 1), &output->_elements);
        output->_type = kLIST;
    }
    else if (*string == '{')
    {
        string = ParseHash(SkipSpace(string + 1), &output->_elements);
        output->_type = kHASH;
    }
    else if (*string == '\"')
    {
        string = ParseString(string + 1, &output->_string);
        output->_type = kSTRING;
    }
    else if (*string >= '0' && *string <= '9')
    {
        string = ParseNumber(string, &output->_number);
        output->_type = kNUMBER;
    }
    else if (string[0] == 't' && string[1] == 'r' && string[2] == 'u' && string[3] == 'e')
    {
        string = ParseTrue(string, &output->_number);
        output->_type = kBOOL;
    }
    else if (string[0] == 'f' && string[1] == 'a' && string[2] == 'l' && string[3] == 's' && string[4] == 'e')
    {
        string = ParseFalse(string, &output->_number);
        output->_type = kBOOL;
    }
    else
    {
        DEBUG_CHECK(false, Parser::Exception, DEBUG_EXPINFO("Parse Json Error", string));
    }
    return string;
}

std::string JValue::Print(std::string & space) const
{
    switch (Type())
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
            std::string resule("{\n");
            space.append("\t");
            resule.append(space);
            for (const auto & element : _elements)
            {
                strings.push_back(SFormat("\"{0}\": {1}", 
                    element->Key(), element->Print(space)));
            }
            resule.append(PrintJoin(strings, ",\n" + space));
            space.pop_back();
            resule.append("\n");
            resule.append(space);
            resule.append("}");
            return std::move(resule);
        }
        break;
    case kLIST:
        {
            std::vector<std::string> strings;
            for (const auto & element : _elements)
            {
                strings.push_back(element->Print(space));
            }
            return "[" + PrintJoin(strings, ", ") + "]";
        }
        break;
    case kBOOL:
        {
            return ToBool() ? "\"true\"" : "\"false\"";
        }
        break;
    }
    return "null";
}

std::string JValue::PrintJoin(const std::vector<std::string>& strings, const std::string & join) const
{
    size_t count = 0;
    for (const auto & string : strings)
    {
        count += string.size();
    }

    std::string resule;
    resule.reserve(count);

    if (!strings.empty())
    {
        resule.append(strings.at(0));

        for (auto i = 1; i != strings.size(); ++i)
        {
            resule.append(join);
            resule.append(strings.at(i));
        }
    }

    return resule;
}
