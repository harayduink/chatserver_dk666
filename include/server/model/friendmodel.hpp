#ifndef FRIENDMODEL_H
#define FRIENDMODEL_H

#include <vector>

#include "user.hpp"
using std::vector;
//维护好友信息的接口操作方法
class FriendModel
{
private:
    /* data */
public:
    //添加好友关系，客户端登录成功以后，
    
    void insert(int userid, int friendid);
    
    //从服务返回好友列表 调用列表时，还需要查询信息，使用两个表的联合查询
    //实际业务当中，为了降低服务器的io负担，好友列表存储在客户本地
    vector<User> query(int userid);



};

#endif