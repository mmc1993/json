#include "json.h"

#include <iostream>
#include <chrono>
#include <vector>

int main()
{
	Json jsonArray("[]");
	jsonArray
		.InsertArray(1)
		.InsertArray(1.5)
		.InsertArray(1.5f)
		.InsertArray(true)
		.InsertArray(false)
		.InsertArray("abc");

	Json jsonHash("{}");
	jsonHash
		.InsertHash(1, "key1")
		.InsertHash(1.5, "key2")
		.InsertHash(1.5f, "key3")
		.InsertHash(true, "key4")
		.InsertHash(false, "key5")
		.InsertHash("abc", "key6")
		.InsertHash(jsonArray, "key9")
		.InsertHash(Json("[1, 2, 3, 4]"), "key7")
		.InsertHash(Json("{\"key\": \"hash\"}"), "key8");

	STD cout << jsonHash << STD endl;
	return 0;
}