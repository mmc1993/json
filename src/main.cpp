#include "json.h"
#include "sformat.h"
#include <iostream>

#include <ctime>

class Clock {
public:
    void Begin()
    {
        _time = clock();
    }

    clock_t Time()
    {
        return clock() - _time;
    }

    clock_t _time;
};

int main()
{
    Clock time;

    //  测试从字符串序列化到对象
    {
        std::ifstream ifile("a.json");
        std::string buffer;
        std::copy(std::istream_iterator<char>(ifile),
            std::istream_iterator<char>(),
            std::back_inserter(buffer));

        time.Begin();
        auto json = std::move(JValue::FromString(buffer));
        std::cout << "测试从字符串序列化到对象: " << time.Time() << std::endl;
    }

    //  测试从文件序列化到对象
    {
        time.Begin();
        auto json = std::move(JValue::FromFile("a.json"));
        std::cout << "测试从文件序列化到对象: " << time.Time() << std::endl;
    }

    //  测试从对象序列化到字符串
    {
        auto json = std::move(JValue::FromFile("a.json"));

        time.Begin();
        std::string buffer = json.ToPrint();
        std::cout << "测试从对象序列化到字符串: " << time.Time() << std::endl;
    }

    //  从文件解析
    auto jfile = JValue::FromFile("a.json");
    //  从字符串解析
    auto jstring = JValue::FromString("{}");
    //  单独设值
    JValue json;
    json.Set(0);
    json.Set(true);
    json.Set("123");
    json.Set(JValue::Hash());
    json.Set(JValue::List());
    json.Set(JValue::FromString("{}"));

    //  读函数原型 JValue::Get(key1, key2, key3...)
    //  读 json["hash"]["k1"]
    json.Get("hash", "k1")->ToInt();
    //  读 json["list"][0]
    json.Get("list", 0)->ToInt();

    //  写函数原型 JValue::Set(val, key1, key2, key3...);
    //  写 json["list"] = []
    json.Set(JValue::List(), "list");
    //  写 json["list"][0] = 0
    json.Set(0, "list", 0);
    //  写 json["hash"] = {}
    json.Set(JValue::Hash(), "hash");
    //  写 json["hash"]["k"] = 0
    json.Set(0, "hash", "k");

    //  删函数原型 JValue::Del(key1, key2, key3...)
    //  删 json["hash"]["k"] = undefine
    json.Del("hash", "k");

    //  左值深拷贝
    {
        JValue json1, json2;
        json1 = json2;
    }
    
    //  右值拷贝
    {
        JValue json1, json2;
        json1 = std::move(json2);
    }

    //  序列化到字符串
    std::string str = json.ToPrint();

    //  范围遍历
    for (auto & jptr : json)
    { }

    //  索引遍历
    for (auto i = 0; i != json.GetCount(); ++i)
    {
        json.Get(i);
    }

    //  可结合C++标准算法
    auto it = std::find(std::begin(json), std::end(json), "key");

    std::cin.get();
	return 0;
}