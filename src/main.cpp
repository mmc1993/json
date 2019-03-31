#include <iostream>
#include "mmcjson.h"

int main()
{
	auto json = mmc::JsonValue::FromBuffer(R"({
		"a": 0,
		"b": 1
	})");
	std::cout << std::to_string(json) << std::endl;

    mmc::JsonValue::GC();

    json->Insert(mmc::JsonValue::FromValue(2), "c")
        ->Insert(mmc::JsonValue::FromValue(3), "d")
        ->Insert(mmc::JsonValue::FromValue("a"), "e")
        ->Insert(mmc::JsonValue::FromHash(), "f")
        ->Insert(mmc::JsonValue::FromValue("hash"), "f", "0");

    std::cout << std::to_string(json) << std::endl;

    json->At("f", "0") = mmc::JsonValue::FromValue("mmc");

    std::cout << std::to_string(json) << std::endl;

    mmc::JsonValue::GC();

	json->Set(1);
	json->Set(0.1f);
	json->Set(0.01);
	json->Set(true);
	json->Set(false);
	json->Set("char");
	json->Set(std::string("string"));

	std::cout << std::to_string(json) << std::endl;
	for (const auto & val : json)
	{
		std::cout << "key: " << val.mKey << std::endl;
	}
    return 0;
}