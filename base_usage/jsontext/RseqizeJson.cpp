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


std::string func2()
{
    json js;
    js["id"] = {1,2,3,4,5};
    js["name"] = "zhang san";
    
    js["msg"]["zhang san"] = "hello world";
    js["msg"]["luo shuo"]  = "hello! china";
    //上下的作用相同
    js["msg"] = {{"zhang san", "hello world"}, {"liu shuo", "hello! china"}};
    std::string sendBuf = js.dump();
    // std::cout << js << std::endl;

    return sendBuf;
}


std::string func3()
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
    // std::cout << sendBuf.c_str() << std::endl;

    return sendBuf;
}

int main()
{
    // func1();
    // func2();
    std::string recvBuf = func3();
    //数据的反序列化

    json buf = json::parse(recvBuf);

    std::cout << buf["list"] << std::endl;
    std::cout << buf["path"] << std::endl;

    std::vector<int> vec = buf["list"];
    for(auto &c:vec)
    {
        std::cout << c << std::endl;
    }

    std::map<int, std::string> m = buf["path"];

    for(auto &p: m)
    {
        std::cout << p.first <<"," << p.second << std::endl; 
    }


    // std::string recvBuf = func2();
    // //数据的反序列化

    // json jsbuf = json::parse(recvBuf);

    // auto msgjs = jsbuf["msg"];

    // std::cout << msgjs["zhang san"] << std::endl;
    // std::cout << msgjs["liu shuo"] << std::endl;
    return 0;
}