#ifndef GROUPMODEL_H
#define GROUPMODEL_H

#include "group.hpp"

//维护群组信息的操作接口方法
class GroupModel
{
private:
    /* data */
public:
    //创建群组
    bool createGroup(Group &group);

    //加入一个群组(已经在mysql记录的)
    void addGroup(int userid, int groupid, string role);

    //查询用户所在群组信息
    vector<Group> queryGroup(int userid);

    //根据指定的groupid查询群组用户id列表，除了userid自己，主要用于用户群聊业务给群组其他成员群发消息
    vector<int> queryGroupUsers(int userid, int groupid);
};



#endif