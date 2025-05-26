#include "chatserver.hpp"
#include "json.hpp"
#include "chatservice.hpp"
#include <functional>
using std::bind;
using json = nlohmann::json;
namespace plhd =  std::placeholders;

ChatServer::ChatServer(net::EventLoop* loop,
            const net::InetAddress& listenAddr,
            const string& nameArg)
            :_server(loop, listenAddr, nameArg)
            ,_loop(loop)
{
    //注册连接回调
    _server.setConnectionCallback(bind(&ChatServer::onConnection, this, plhd::_1));

    //注册消息回调
    _server.setMessageCallback(bind(&ChatServer::onMessage, this, plhd::_1, plhd::_2, plhd::_3));

    //设置线程数量
    _server.setThreadNum(4);
}

//启动服务
void ChatServer::start()
{
    _server.start();
}

//上报连接相关信息的回调函数
void ChatServer::onConnection(const net::TcpConnectionPtr& conn)
{
    //客户端断开连接
    if(!conn->connected())
    {
        ChatService::instance()->clientCloseException(conn);
        conn->shutdown();
        //用户下线
    }
}

//上报读写事件信息的回调函数
void ChatServer::onMessage(const net::TcpConnectionPtr& conn,
                net::Buffer* buffer,
                muduo::Timestamp time)
{
    string buf = buffer->retrieveAllAsString();
    //数据反序列化
    json js = json::parse(buf);
    //目的 完全解耦网络模块的代码和业务模块的代码
    //通过绑定id js['msgid]'对应操作获取handler =》conn js time

    //获取到相应id的事件处理器
    auto msgHandler = ChatService::instance()->getHandler(js["msgid"].get<int>());
    
    //回调消息绑定好的事件处理器，来执行相应的业务处理
    msgHandler(conn, js, time);
    

}