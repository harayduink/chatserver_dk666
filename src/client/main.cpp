#include "json.hpp"
#include <iostream>
#include <thread>
#include <string>
#include <vector>
#include <chrono>
#include <ctime>
#include <unordered_map>
#include <functional>

using std::vector; using std::string; using std::thread;
using std::ctime; using std::cout; using std::cin; using std::endl;
using json = nlohmann::json; using std::cerr; using std::unordered_map;
using std::function;

#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <semaphore.h>
#include <atomic>

#include "group.hpp"
#include "user.hpp"
#include "public.hpp"

// 记录当前系统登陆的用户信息
User g_currentUser;
// 记录当前登录用户的好友列表信息
vector<User> g_currentUserFriendList;
// 记录当前登陆用户的群组列表信息
vector<Group> g_currentUserGroupList;

// 控制聊天页面程序
bool isMainMenuRunning = false;
// 用于读写线程之间的通信
sem_t rwsem;
// 记录登录状态
std::atomic_bool g_isLoginSuccess;
std::atomic_bool g_isResSuccess;


// 显示当前登陆成功用户的基本信息
void showCurrentUserData();
// 

//接收线程
void readTaskHandler(int clientfd);
//获取系统时间（聊天消息需要添加时间信息）
string getCurrentTime();
//主聊天页面程序
void mainMenu(int clientfd);
void doLoginResponse(json &responsejs);

void doRegResponse(json &responsejs);

//聊天客户端程序实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char **argv)
{
    if(argc < 3)
    {
        cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << endl;
        exit(-1);
    }

    //解析通过命令行参数传递的ip和port

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);

    //创建client端的socket
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);

    if(-1==clientfd)
    {
        cerr << "socket create error" << endl;
        exit(-1);
    }

    // 填写client需要连接的server信息ip+port
    sockaddr_in server;
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    if(-1 == connect(clientfd, (sockaddr *) &server, sizeof(sockaddr_in)))
    {
        cerr << "connect server error" << endl;
        close(clientfd);
        exit(-1);
    }

    // 初始化读写信号量
    sem_init(&rwsem, 0, 0);

    // 链接服务器成功，启动接收子线程
    std::thread readTask(readTaskHandler, clientfd); //pthread_create
    readTask.detach();                              //pthread_detach
    //main线程用于接收用户输入，负责发送数据
    for(;;) //定义一个持续循环
    {
        //显示首页面菜单 登陆注册退出
        cout << "====================" << '\n';
        cout << "1. login" << '\n';
        cout << "2. register" << '\n';
        cout << "3. quit" << '\n';
        cout << "====================" << '\n';
        cout << "choice:";
        int choice = 0; 
        string tempstr;

        cin >> choice;
        cin.get(); //读掉缓冲区残余的回车

        switch (choice)
        {
            case 1: //login 业务
            {
                int id = 0;
                char pwd[50] = {0};
                cout << "userid:";
                cin >> id;
                cin.get(); // 读掉缓冲区残留的回车
                cout << "userpassword:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = LOGIN_MSG;
                js["id"] = id;
                js["password"] = pwd;
                string request = js.dump();

                g_isLoginSuccess = false;

                int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
                
                if(len == -1)
                {
                    cerr << "send login msg error:" << request << endl;
                }
                
                sem_wait(&rwsem); // 等待信号量，由子线程处理完登录的响应消息后，通知到这里
                // 在子线程发回消息之前，主程序一直会在当前位置阻塞
                if(g_isLoginSuccess){
                    isMainMenuRunning = true;
                    mainMenu(clientfd);
                }
                // 进入聊天主菜单页面
            }
            break;
            case 2: // register 业务
            {
                char name[50] = {0};
                char pwd[50] = {0};

                cout << "username:";
                cin.getline(name, 50);
                cout << "userpassword:";
                cin.getline(pwd, 50);

                json js;
                js["msgid"] = REG_MSG;
                js["name"] = name;
                js["password"] = pwd;
                string request = js.dump();
                
                int len = send(clientfd, request.c_str(), strlen(request.c_str())+1, 0);
                if(len==-1)
                {
                    cerr << "send reg msg error" << endl;
                    exit(-1);
                }
                sem_wait(&rwsem); // 等待信号量，由子线程处理完注册的响应消息后，通知到这里
            } 
            break;
            case 3: //quit 业务
                close(clientfd);
                sem_destroy(&rwsem); //释放需要取址操作
                exit(0);
            default:
                cerr << "invalid input!" << endl;
                break;
        }

    }
}

// 处理登录的响应逻辑
void doLoginResponse(json &responsejs)
{
    if(0 != responsejs["errno"].get<int>()) //登录失败
    {
        cerr << responsejs["errmsg"] << endl;

        cout << "Login failed! Please check id and password!" << endl;
        g_isLoginSuccess = false;
    }
    else // 登录成功
    {
        //记录当前用户的id和name
        g_currentUser.setId(responsejs["id"].get<int>());
        g_currentUser.setName(responsejs["name"]);
        
        //记录当前用户的好友列表信息
        if(responsejs.contains("friends"))//有的用户可能没有好友，需要判断是否包含friends键
        {
            g_currentUserFriendList.clear();
            vector<string> vec = responsejs["friends"];
            for(string &str : vec)
            {
                json js = json::parse(str);
                User user;
                user.setId(js["id"].get<int>());
                user.setName(js["name"]);
                user.setState(js["state"]);
                g_currentUserFriendList.push_back(user);
            }
        }

        if (responsejs.contains("groups"))
        {
            g_currentUserGroupList.clear();
            vector<string> vec1 = responsejs["groups"];
            for(string &groupstr : vec1)
            {
                json grpjs = json::parse(groupstr);  //json反序列化操作,整理群组信息
                Group group;
                group.setId(grpjs["id"].get<int>());
                group.setName(grpjs["groupname"]);
                group.setDesc(grpjs["groupdesc"]);

                vector<string> vec2 = grpjs["users"];
                for(string &userstr: vec2)
                {
                    GroupUser user;
                    json ujs = json::parse(userstr);

                    //添加群成员的信息
                    user.setId((ujs["id"]).get<int>());
                    user.setName(ujs["name"]);
                    user.setState(ujs["state"]);
                    user.setRole(ujs["role"]);

                    group.getUser().push_back(user);
                }
                g_currentUserGroupList.push_back(group);
            }
            
        } 
        showCurrentUserData();
        //接收离线消息
        if(responsejs.contains("offlinemsg"))
        {
            vector<string> vec = responsejs["offlinemsg"];
            for( string &str:vec)
            {
                json js = json::parse(str);
                int msgtype = js["msgid"].get<int>();
                if (ONE_CHAT_MSG == msgtype)
                {
                    cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                        << " said:" << js["msg"].get<string>() << endl;
                }
                else
                {   
                    cout << "群消息[" << js["groupid"] << "]" << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                        << " said:" << js["msg"].get<string>() << endl;
                }

            }
        }
        // 登录成功，启动接收线程负责接收数据，该线程只启动一次
        g_isLoginSuccess = true;
    }
}

// 处理注册的相应逻辑
void doRegResponse(json &responsejs)
{

    if (0 != responsejs["errno"].get<int>()) // 注册失败
    {
        cerr << "name is already exist, register error!" << endl;
    }
    else // 注册成功
    {
        cout << "name register success, userid is " << responsejs["id"]
                << ", do not forget it!" << endl;
    }
}

//接收线程
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buffer[1024] = {0};
        int len = recv(clientfd, buffer, 1024, 0);
        if(-1==len || 0==len) //接收异常
        {
            close(clientfd);
            exit(-1);
        }
        // 接收chatserver转发的数据，反序列化产生json对象
        json js = json::parse(buffer);
        int msgtype = js["msgid"].get<int>();
        // 根据接受的js序列化消息，判断是好友消息还是群发消息，分别进行处理
        if (ONE_CHAT_MSG == msgtype)
        {
            cout << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                 << " said:" << js["msg"].get<string>() << endl;
                 continue;
        }
        // 处理群聊消息
        if(GROUP_CHAT_MSG == msgtype)
        {
            cout << "群消息[" << js["groupid"] << "]" << js["time"].get<string>() << "[" << js["id"] << "]" << js["name"].get<string>()
                 << " said:" << js["msg"].get<string>() << endl;
                 continue;
        }
        // 处理登录响应消息
        if(LOGIN_MSG_ACK == msgtype)
        {
            doLoginResponse(js); // 处理登录响应的业务逻辑
            sem_post(&rwsem);   // 通知主线程，登陆结果处理完成
            continue;
        }
        // 处理注册响应信息
        if(REG_MSG_ACK == msgtype)
        {
            doRegResponse(js);
            sem_post(&rwsem);
            continue;
        }
    }
}



//帮助 "help" command handler
void help(int fd=0, string msg="");
//聊天 "chat" command handler
void chat(int, string);
//添加好友  "addfriend" command handler
void addfriend(int ,string);
//创建群组  "creategroup" command handler
void creategroup(int ,string);
//加入群组 "addgroup" command handler
void addgroup(int, string);
//群聊 "groupchat" command handler
void groupchat(int, string);
//注销 "loginout" command handler
void loginout(int, string);

//int 用于传入发送数据的clientfd，string用于传入发送的数据
unordered_map<string, string> commandMap = {
    {"help","显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:friendid:message"},
    {"addfriend", "添加好友，格式addfriend:friendid"},
    {"creategroup", "创建群组，格式creategroup:groupname:groupdesc"},
    {"addgroup", "加入群组，格式addgroup:groupid"},
    {"groupchat","群聊，格式groupchat:groupid:message"},
    {"loginout", "注销，格式loginout"},
};


unordered_map<string, function<void(int ,string)>> commandHandlerMap = {
    { "help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup",creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout},
};


void mainMenu(int clientfd)
{
    help();

    char buffer[1024];
    while (isMainMenuRunning)
    {
        cin.getline(buffer, 1024);
        string commandbuf(buffer);
        string command;
        int idx = commandbuf.find(":"); //找到返回索引位置，找不到返回-1

        if(-1==idx)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0,idx);
        }

        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            cerr << "invalid command!" << endl;
            continue;
        }
        //调用相应命令的事件处理回调，mainMenu对修改封闭，添加新功能不需要修改该函数
        // cout << commandbuf << "," << commandbuf.substr(idx+1,commandbuf.size()-idx) << endl;
        it->second(clientfd, commandbuf.substr(idx+1,commandbuf.size()-idx));
        //调用命令处理方法，命令后面的具体要求传入回调函数当中
    }
}

void showCurrentUserData()
{
    cout << "=========================login user=========================" << endl;
    cout << "Current login user => id:" << g_currentUser.getId() \
         << "name:" << g_currentUser.getName() << endl;
    
    cout << "------------------------friend list------------------------" << endl;
    if(!g_currentUserFriendList.empty())
    {
        for(User &user:g_currentUserFriendList)
        {
            cout << user.getId() << " " << user.getName() << " " << user.getState() << endl;
        }
    }

    cout << "------------------------group list------------------------" << endl;
    if(!g_currentUserGroupList.empty())
    {
        
        for (Group & group:g_currentUserGroupList)
        {
            cout << "Group" << ":" <<  group.getId() << "\n";
            cout << group.getName() << ":" << group.getDesc() << endl;
            cout << "Users" << "\n";
            for( GroupUser &user : group.getUser())
            {
                cout << '\t' << user.getId() << ":" << user.getName() << ":" << user.getRole() << ":" << user.getState() << "\n";
            }
        }

        
    }
}

//帮助 "help" command handler
void help(int ,string )
{
    cout << "show command list" << endl;
    for(auto &p:commandMap)
    {
        cout << p.first << ":" << p.second << endl;
    }
    cout << endl;
}


//聊天 "chat" command handler
void chat(int clientfd, string str)
{
    int idx = str.find(":");
    if(idx==-1)
    {
        cout << "error command:" << str << endl;
        cerr << "chat command invalid!" << endl;
        return;
    }

    int friendid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx + 1, str.size()-idx);
    json js;
    js["msgid"] = ONE_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["toid"] = friendid;
    js["msg"] = message;
    js["time"] = getCurrentTime();

    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(-1==len)
    {
        cerr << "send chat msg error ->" << buffer << endl;
    }

}

//添加好友  "addfriend" command handler
void addfriend(int clientfd, string str)
{
    int friendid = atoi(str.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);

    if(-1 == len)
    {
        cerr << "send addfriend msg error ->" << buffer << endl;
    }
}


//创建群组  "creategroup" command handler
void creategroup(int clientfd, string str)
{
    json js;

    int idx = str.find(":");

    if(idx == -1)
    {
        cerr << "creategroup command invalid!" << endl;
        return;
    }
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    
    string groupname = str.substr(0, idx);
    string groupdesc = str.substr(idx+1, str.size()-idx);
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;

    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(-1==len)
    {
        cerr << "send creategroup msg error ->" << buffer << endl;
    }

}

//加入群组 "addgroup" command handler
void addgroup(int clientfd, string str)
{
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["userid"] = g_currentUser.getId();
    js["groupid"] = atoi(str.c_str());
    string buffer = js.dump();
    cout << buffer << endl;
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(-1==len)
    {
        cerr << "send addgroup msg error ->" << buffer << endl;
    }

}

//群聊 "groupchat" command handler
void groupchat(int clientfd, string str)
{
    int idx = str.find(":");
    if(idx == -1)
    {
        cerr << "groupchat command invalid!" << endl;
        return;
    }
    int groupid = atoi(str.substr(0,idx).c_str());
    string message = str.substr(idx+1, str.size()-idx);
    // cout << message << endl;
    json js;

    js["msgid"] = GROUP_CHAT_MSG;
    js["id"] = g_currentUser.getId();
    js["name"] = g_currentUser.getName();
    js["groupid"] = groupid;
    js["msg"] = message;
    js["time"] = getCurrentTime();
    
    string buffer = js.dump();

    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);
    if(-1==len)
    {
        cerr << "send groupchat msg error ->" << buffer << endl;
    }
}

void loginout(int clientfd, string str)
{
    json js;
    js["msgid"] = LOGINOUT_MSG;
    js["id"] = g_currentUser.getId();
    string buffer = js.dump();

    isMainMenuRunning = false;
    int len = send(clientfd, buffer.c_str(), strlen(buffer.c_str())+1, 0);

    if(-1 == len)
    {
        cerr << "send loginout msg error ->" << buffer << endl;
    }
}

string getCurrentTime()
{
    std::time_t now = time(nullptr);
    string stime = std::ctime(&now);

    if(!stime.empty() && stime.back() =='\n')
    {
        stime.pop_back();
    }
    
    return stime;
}