

// #include <jsoncpp/json/json.h>
// #include <iostream>

// int main() {
//     Json::Value root;
//     Json::FastWriter writer;

//     root["name"] = "John";
//     root["age"] = 30;

//     std::string jsonString = writer.write(root);
//     std::cout << jsonString << std::endl;

//     return 0;
// }



#include<jsoncpp/json/json.h>
#include<iostream>

using namespace std;
int main(int argc, char** argv) 
{
    Json::Value root;
    Json::FastWriter fast;
    root["ModuleType"]= Json::Value("你好");
    root["ModuleCode"]= Json::Value("22");
    root["ModuleDesc"]= Json::Value("33");
    root["DateTime"]= Json::Value("44");
    root["LogType"]= Json::Value("55");
    cout<<fast.write(root)<<endl;
    return 0;
}