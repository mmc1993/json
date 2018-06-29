#include "json.h"
#include "sformat.h"
#include <utility>

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
    else if (string[0] == 't' && string[1] == 'u' && string[2] == 'r' && string[3] == 'e')
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


const std::string & JValue::Print() const
{
    // TODO: 在此处插入 return 语句
    return std::string();
}