#ifndef CHATSERVICE_H
#define CHATSERVICE_H

#include <unordered_map>
#include <functional>
#include <mutex>
#include "json.hpp"

#include "redis.hpp"
#include "offlinemessagemodel.hpp"
#include "usermodel.hpp"
#include "friendmodel.hpp"
#include "groupmodel.hpp"
#include <muduo/net/TcpConnection.h>
using std::unordered_map;
using json = nlohmann::json;
namespace plhd = std::placeholders;
using namespace muduo; using namespace muduo::net;

//表示处理事件消息的回调函数类型
using MsgHandler = std::function<void(const TcpConnectionPtr& conn, json &js, Timestamp)>;


//聊天服务器业务类
class ChatService
{
public:
    //获取单例对象的接口函数
    static ChatService* instance();
    // 处理登陆业务
    void login(const TcpConnectionPtr& conn, json &js, Timestamp time);

    // 处理用户下线业务
    void loginout(const TcpConnectionPtr& conn, json &js, Timestamp time);
    
    //处理注册业务
    void reg(const TcpConnectionPtr& conn, json &js, Timestamp time);
    
    //一对一聊天业务
    void oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time);
    
    // 添加好友业务
    void addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time);

    // 创建群聊业务
    void createGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);

    // 加入群聊业务
    void addGroup(const TcpConnectionPtr& conn, json &js, Timestamp time);
    
    // 群聊消息业务
    void groupChat(const TcpConnectionPtr& conn, json &js, Timestamp time);

    //服务器异常，业务重置方法
    void reset();

    //获取消息对应的处理器
    MsgHandler getHandler(int msgid);

    // 从redis消息队列中获取订阅的信息
    void handleRedisSubscribeMessage(int userid, string msg);

    //处理客户端异常退出
    void clientCloseException(const TcpConnectionPtr& conn);

    
private:
    
    ChatService();
    //存储消息id和对应时间的业务处理方法
    unordered_map<int, MsgHandler> _msgHandlerMap;

    //定义互斥锁，保证_userConnMap的安全
    std::mutex _connMutex;
    // 存储在线用户的通信连接(会不断改变，会在不同线程中回调，要注意线程安全)
    unordered_map<int, TcpConnectionPtr> _userConnMap;
    
    //数据操作类对象
    UserModel _userModel;

    //离线消息对象
    OfflineMsgModel _offlineMsgModel;

    //好友对象
    FriendModel _friendModel;

    //群组对象
    GroupModel _groupModel;

    //redis连接对象
    Redis _redis;
    
};

#endif

