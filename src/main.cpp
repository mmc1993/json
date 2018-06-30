#include "json.h"
#include "sformat.h"
#include <iostream>

int main()
{
    const std::string s;
    JValue json = JValue::FromString("[]");
    json.Set(JValue::Hash(), 0);
    json.Set(JValue::List(), 1);
    for (auto & jptr : json)
    {
        std::cout << jptr->Key() << std::endl;
    }
    //json.Set(0, "hash", "key1");
    //json.Set(1, "hash", "key2");
    std::cout << json.ToPrint() << std::endl;
    std::cin.get();

	//AppWindow app;
	//app.Create("Render");
	//app.Move(100, 100);
	//app.Size(800, 800);
	//app.Loop();
	return 0;
}