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

    //  ���Դ��ַ������л�������
    {
        std::ifstream ifile("a.json");
        std::string buffer;
        std::copy(std::istream_iterator<char>(ifile),
            std::istream_iterator<char>(),
            std::back_inserter(buffer));

        time.Begin();
        auto json = std::move(JValue::FromString(buffer));
        std::cout << "���Դ��ַ������л�������: " << time.Time() << std::endl;
    }

    //  ���Դ��ļ����л�������
    {
        time.Begin();
        auto json = std::move(JValue::FromFile("a.json"));
        std::cout << "���Դ��ļ����л�������: " << time.Time() << std::endl;
    }

    //  ���ԴӶ������л����ַ���
    {
        auto json = std::move(JValue::FromFile("a.json"));

        time.Begin();
        std::string buffer = json.ToPrint();
        std::cout << "���ԴӶ������л����ַ���: " << time.Time() << std::endl;
    }

    //  ���ļ�����
    auto jfile = JValue::FromFile("a.json");
    //  ���ַ�������
    auto jstring = JValue::FromString("{}");
    //  ������ֵ
    JValue json;
    json.Set(0);
    json.Set(true);
    json.Set("123");
    json.Set(JValue::Hash());
    json.Set(JValue::List());
    json.Set(JValue::FromString("{}"));

    //  ������ԭ�� JValue::Get(key1, key2, key3...)
    //  �� json["hash"]["k1"]
    json.Get("hash", "k1")->ToInt();
    //  �� json["list"][0]
    json.Get("list", 0)->ToInt();

    //  д����ԭ�� JValue::Set(val, key1, key2, key3...);
    //  д json["list"] = []
    json.Set(JValue::List(), "list");
    //  д json["list"][0] = 0
    json.Set(0, "list", 0);
    //  д json["hash"] = {}
    json.Set(JValue::Hash(), "hash");
    //  д json["hash"]["k"] = 0
    json.Set(0, "hash", "k");

    //  ɾ����ԭ�� JValue::Del(key1, key2, key3...)
    //  ɾ json["hash"]["k"] = undefine
    json.Del("hash", "k");

    //  ��ֵ���
    {
        JValue json1, json2;
        json1 = json2;
    }
    
    //  ��ֵ����
    {
        JValue json1, json2;
        json1 = std::move(json2);
    }

    //  ���л����ַ���
    std::string str = json.ToPrint();

    //  ��Χ����
    for (auto & jptr : json)
    { }

    //  ��������
    for (auto i = 0; i != json.GetCount(); ++i)
    {
        json.Get(i);
    }

    //  �ɽ��C++��׼�㷨
    auto it = std::find(std::begin(json), std::end(json), "key");

    std::cin.get();
	return 0;
}