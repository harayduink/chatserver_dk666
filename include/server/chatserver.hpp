#ifndef CHATSERVER_H
#define CHATSERVER_H

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <vector>
#include <string>


using std::string;
namespace net=muduo::net ;


//聊天服务器主类
class ChatServer
{
    public:
        //创建聊天对象
        ChatServer(net::EventLoop* loop,
            const net::InetAddress& listenAddr,
            const string& nameArg);
        //启动服务
        void start();
    private:
        //上报链接相关信息的回调函数
        void onConnection(const net::TcpConnectionPtr&);

        //上报读写事件信息的回调函数
        void onMessage(const net::TcpConnectionPtr&,
                            net::Buffer*,
                            muduo::Timestamp );
        net::TcpServer _server; //组合muduo库，实现服务器相关的类对象
        net::EventLoop *_loop; //指向事件循环对象的指针
};
#endif

