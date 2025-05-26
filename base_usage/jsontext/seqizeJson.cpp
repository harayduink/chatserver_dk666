#include "json.hpp"



#include <iostream>
#include <vector>
#include <map>
#include <string>

using json = nlohmann::json;


void func1(){
    json js;
    js["msg_type"] = 2;
    js["from"] = "zhang san";
    js["to"] = "li si";
    js["msg"] = "hello! what are u doing";

    std::string sendBuf = js.dump();
    std::cout << sendBuf.c_str() << std::endl;
}


void func2()
{
    json js;
    js["id"] = {1,2,3,4,5};
    js["name"] = "zhang san";
    
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["luo shuo"]  = "hello! china";
    //上下的作用相同
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello! china"}};

    std::cout << js << std::endl;
}


void func3()
{
    json js;

    //直接序列化一个vector容器
    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(5);

    js["list"] = vec;

    //直接序列化一个map容器

    std::map<int, std::string> m;
    m.insert({1,"黄山"});
    m.insert({2,"华山"});
    m.insert({5,"泰山"});

    js["path"] = m;

    std::string sendBuf = js.dump();
    std::cout << sendBuf.c_str() << std::endl;
}

int main()
{
    // func1();
    // func2();
    func3();
    return 0;
}