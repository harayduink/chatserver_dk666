/*
muduo网络库两个主要类
TcpServer：编写服务器端程序
TcpClient：编写客户端程序
epoll+线程池
网络I/O代码和业务代码分开
            用户的连接和断开，用户的可读写时间
*/

#include <muduo/net/TcpServer.h>
#include <muduo/net/EventLoop.h>
#include <muduo/base/Timestamp.h>
#include <iostream>
#include <functional>
#include <string>


using std::cout; using std::endl;
using namespace std::placeholders;
namespace net = muduo::net;
using Timestamp = muduo::Timestamp;
//基于muduou网络库开发服务器程序
/*
组合TcpServer对象
创建EventLoop时间循环对象的指针
明确TcpServer构造函数的输入需要什么参数，输出ChatServer的构造函数
在当前服务类的构造函数当中，注册处理连接的回调函数和读写时间的回调函数
设置合适的服务端线程数量，muduo自动分配IO线程和worker线程
*/

class ChatServer
{
    public:
        ChatServer(net::EventLoop* loop, //事件循环
            const net::InetAddress& listenAddr, //IP+Port
            const std::string& nameArg ) //服务器名称
            :_server(loop, listenAddr, nameArg), _loop(loop)
            {
                // 
                //给服务器注册用户连接的创建和断开回调
                _server.setConnectionCallback(std::bind(&ChatServer::onConnection, this, _1)); //一个占位符
                //给服务器注册用户读写 事件回调
                _server.setMessageCallback(std::bind(&ChatServer::onMessage, this, _1,_2,_3 )); //3个占位符

                //设置服务器端的线程数量 1个I/O线程， 3个worker线程
                _server.setThreadNum(4);
            }
        void start()
        {
            _server.start();
        }
    private:
        //专门处理用户的连接创建和断开 epoll listenfd accept
        void onConnection(const net::TcpConnectionPtr & conn ){
            if(conn->connected())
            {
                std::cout << conn->peerAddress().toIpPort() << "->" << \
                conn->localAddress().toIpPort() << "state:online" << std::endl;
            }else
            {
                std::cout << conn->peerAddress().toIpPort() << "->" << \
                conn->localAddress().toIpPort() << " state:offline" << std::endl;

                conn->shutdown(); //close(fd)
                // _loop->quit();
            }
            
        }
        void onMessage(const net::TcpConnectionPtr & conn, //链接
                        net::Buffer *buffer, //缓冲区
                        Timestamp time //接收到数据的时间信息
                        )
        {
            std::string buf = buffer->retrieveAllAsString();
            cout << "recv data" << ',' << buf << " time:" << time.toString() << endl;
            conn->send(buf);
        }
        net::TcpServer _server; //#1 必须要进行指定的构造
        net::EventLoop *_loop;  //# epoll
};

int main()
{
    net::EventLoop loop; //epoll
    net::InetAddress addr("127.0.0.1", 6000);

    ChatServer server(&loop, addr, "ChatServer");
    server.start(); //listenfd epoll_ctl=>epoll
    loop.loop(); // 调用epoll_wait 以阻塞方式等待新用户链接， 已连接用户的读写事件等
    return 0;
}