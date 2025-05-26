#include "chatservice.hpp"
#include "public.hpp"
#include <string>
#include <muduo/base/Logging.h>
#include <vector>
#include <map>




using std::vector;


ChatService * ChatService::instance()
{
    static ChatService service;
    return &service;
}


//注册消息以及对应的回调操作 (每次添加新的业务都需要在这里绑定事件派发消息的操作)
ChatService::ChatService()
{
    _msgHandlerMap.insert({LOGIN_MSG, std::bind(&ChatService::login, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({LOGINOUT_MSG, std::bind(&ChatService::loginout, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({REG_MSG, std::bind(&ChatService::reg,this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({ONE_CHAT_MSG, std::bind(&ChatService::oneChat, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({ADD_FRIEND_MSG, std::bind(&ChatService::addFriend, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({CREATE_GROUP_MSG, std::bind(&ChatService::createGroup, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({ADD_GROUP_MSG, std::bind(&ChatService::addGroup, this, plhd::_1, plhd::_2, plhd::_3)});
    _msgHandlerMap.insert({GROUP_CHAT_MSG, std::bind(&ChatService::groupChat, this, plhd::_1, plhd::_2, plhd::_3)});

    //连接Redis服务器
    if(_redis.connect())
    {
        //设置上报消息的回调
        _redis.init_notify_handler(std::bind(&ChatService::handleRedisSubscribeMessage, this, plhd::_1, plhd::_2));
    }
}


MsgHandler ChatService::getHandler(int msgid)
{
    //记录错误日志，msgid没有对应的事件处理回调
    auto it = _msgHandlerMap.find(msgid);
    if(it == _msgHandlerMap.end())
    {
        // std::string errstr = "msgid:" + 
        //返回一个默认的处理器

        return [=](const TcpConnectionPtr& conn, json &js, Timestamp time) {
            LOG_ERROR << "msgid:" << msgid << "can not find handler!";
        };
    } 
    else
    {
        return _msgHandlerMap[msgid];
    }
}

//服务器异常，业务重置方法
void ChatService::reset()
{
    //把online状态的用户，设置成offline
    _userModel.resetState();
}

 // 处理登陆业务 id pwd == spwd
void ChatService::login(const TcpConnectionPtr& conn, json &js, Timestamp time){
    int id = js["id"].get<int>();
    string pwd = js["password"];

    //传入id值，得到
    User user = _userModel.query(id);

    if(user.getId()==id && user.getPwd() == pwd)
    {

        
        if(user.getState() == "online")
        {
            //表示用户已登录，不允许重复登陆
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 2; //重复登陆的消息
            response["errmsg"] = "this account is using, input another!";
            conn->send(response.dump());
        }
        else
        {
            //登录成功，记录用户连接信息
            {
                std::lock_guard<std::mutex>  lock(_connMutex);
                _userConnMap.insert({id, conn});
            }
            //创建锁保护
            
            //id用户登录成功后，向redis订阅channel(id)
            _redis.subscribe(id);

            //登录成功，更新用户状态信息 state offline=>online
            user.setState("online"); //设置用户为在线
            _userModel.updateState(user); //刷新用户信息
            json response;
            response["msgid"] = LOGIN_MSG_ACK;
            response["errno"] = 0; //登录成功的消息
            response["id"] = user.getId(); 
            response["name"] = user.getName();
            //查询用户是否有离线消息
            vector<string> vec = _offlineMsgModel.query(id);
            if(!vec.empty())
            {
                response["offlinemsg"] = vec;
                //读取用户的离线消息后，吧该用户的所有离线消息清除
                _offlineMsgModel.remove(id);
            }
            //查询用户的好友信息并返回
            vector<User> userVec = _friendModel.query(id);
            if(!userVec.empty())
            {
                vector<string> vec2;
                for(User &user:userVec)
                {
                    json js; //临时调整
                    js["id"] = user.getId();
                    js["name"] = user.getName();
                    js["state"] = user.getState();
                    vec2.push_back(js.dump());
                }
                response["friends"] = vec2;
            }
            //
            vector<Group> groupVec = _groupModel.queryGroup(id);
            // response['groups'] = groupVec;

            if(!groupVec.empty())
            {
                //将用户保存为一个jsdump的形式
                vector<string> vec3;
                for(Group &group:groupVec)
                {
                    json gjs; //存储群组的json格式化信息
                    gjs["id"] = group.getId();
                    gjs["groupname"] = group.getName();
                    gjs["groupdesc"] = group.getDesc();

                    vector<string> vec_Users;
                    for(GroupUser guser:group.getUser())
                    {
                        json ujs; //存储群组用户的json格式话信息
                        ujs["id"] = guser.getId();
                        ujs["name"] = guser.getName();
                        ujs["state"] = guser.getState();
                        ujs["role"] = guser.getRole();

                        vec_Users.push_back(ujs.dump());
                    }

                    gjs["users"] = vec_Users;

                    vec3.push_back(gjs.dump());
                }
                response["groups"] = vec3;
            }
            conn->send(response.dump());
        }
       
    }
    else
    {
        //登录失败 用户不存在或者用户存在密码错误
        json response;
        response["msgid"] = LOGIN_MSG_ACK;
        response["errno"] = 1; //业务失败的消息 
        response["errmsg"] = "id or password is invalid";
        conn->send(response.dump());
    }
}

//处理注册业务   name password 
void ChatService::reg(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    string name = js["name"];
    string pwd = js["password"];

    User user;
    user.setName(name);
    user.setPwd(pwd);

    bool state = _userModel.insert(user);

    if(state)
    {
        //注册成功
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 0; //业务成功的消息
        response["id"] = user.getId(); 
        conn->send(response.dump());

    }else
    {
        json response;
        response["msgid"] = REG_MSG_ACK;
        response["errno"] = 1; //业务失败的消息
        
        conn->send(response.dump());
        
        //注册失败
    }
}

//处理用户下线业务
void ChatService::loginout(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();

    {
        std::lock_guard<std::mutex> lock(_connMutex);
        auto it = _userConnMap.find(userid);
        if(it != _userConnMap.end())
        {
            //
            _userConnMap.erase(it);
            
        }
    }
    //更新用户的状态信息
    User user(userid, "", "", "offline");
    _userModel.updateState(user);
}


//处理客户端异常退出
void ChatService::clientCloseException(const TcpConnectionPtr& conn)
{
    User user;
    {
        std::lock_guard<std::mutex>  lock(_connMutex);
        
        for(auto it=_userConnMap.begin(); it != _userConnMap.end(); it ++)
        {
            if(it->second == conn)
            {
                //从map表删除用户的连接信息
                user.setId(it->first);
                _userConnMap.erase(it);
                break;
            }
        }
    }
    
    //用户注销，相当于下线，去掉redis订阅
    _redis.unsubscribe(user.getId());
    //更新用户的状态信息
    if(user.getId() != -1)
    {
        user.setState("offline");
        _userModel.updateState(user);
    }
}

//一对一聊天业务
void ChatService::oneChat(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int toid = js["toid"].get<int>();
    //访问链接信息表，需要进行mutex阻塞其他线程
    bool userState = false;
    {
        std::lock_guard<std::mutex> lock(_connMutex);

        auto it = _userConnMap.find(toid);
        if(it != _userConnMap.end())
        {
            //toid 在线，转发消息在锁内，服务器主动推送消息给用户
            it->second->send(js.dump());
            return;
        }
    }

    //toid在线但不在本服务器，向redis发布消息
    User user = _userModel.query(toid);

    if(user.getState()== "online")
    {
        _redis.publish(toid, js.dump());
        return;
    }

    // toid不在线，存储离线消息
    _offlineMsgModel.insert(toid, js.dump());
}

// 添加好友业务
void ChatService::addFriend(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["id"].get<int>();
    int friendid = js["friendid"].get<int>();

    // 存储好友信息

    _friendModel.insert(userid, friendid);

}

// 创建群聊业务
void ChatService::createGroup(const TcpConnectionPtr& conn, json &js, Timestamp time)
{

    int userid = js["id"].get<int>();
    string name = js["groupname"];
    string desc = js["groupdesc"];
    
    // 存储新创建的群组信息
    Group group(-1,name, desc);

    if(_groupModel.createGroup(group))
    {
        //创建群组成功，存入数据表中,并将当前用户添加进群组当中，设置为管理员权限
        // 实际要用常量来定义role属性要更合适一些
        _groupModel.addGroup(userid, group.getId(), "creator");
    }
}

void ChatService::addGroup(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    int userid = js["userid"].get<int>();
    int groupid = js["groupid"].get<int>();
    _groupModel.addGroup(userid, groupid, "normal");
}

void ChatService::groupChat(const TcpConnectionPtr& conn, json &js, Timestamp time)
{
    //需要获取所有群员的id，根据js里的groupid进行联合查找（要获取state）
    //接收js中的msg，自身地userid也要获取
    int userid = js["id"].get<int>();
    int groupid = js["groupid"].get<int>();
    vector<int> useridvec = _groupModel.queryGroupUsers(userid, groupid);
    
    std::lock_guard<std::mutex> lock(_connMutex);
    
    for(int id:useridvec)
    {
        auto it = _userConnMap.find(id);
        //发送消息时，根据用户在线和离线状态选择发送在线或是离线消息
        if(it != _userConnMap.end())
        {
            //转发群消息
            it->second->send(js.dump());
        }
        else
        {
            //查询toid是否在线
            User user = _userModel.query(id);
            if(user.getState()=="online")
            {
                _redis.publish(id, js.dump());
            }
            else
            {
                //借助离线消息接口转发离线群消息
                _offlineMsgModel.insert(id, js.dump());
            }
            
        }
    }
}


void ChatService::handleRedisSubscribeMessage(int userid, string msg)
{
    lock_guard<mutex> lock(_connMutex);
    auto it = _userConnMap.find(userid);
    if(it != _userConnMap.end())
    {
        it->second->send(msg);
        return;
    }

    //存储该用户的离线消息
    _offlineMsgModel.insert(userid, msg);

}