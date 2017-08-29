#include "json.h"

#include <iostream>
#include <chrono>

int main()
{
	Json json("[]");
	json.InsertArray(0);
	json.InsertArray(1);
	json.InsertArray(2);
	json.InsertArray(3);

	Json json2("{\"key1\": \"abc\"}, {\"key2\": \"abc\"}}");

	json.InsertHash(json2, "hash");

	STD cout << json.Print() << STD endl;
	return 0;
}