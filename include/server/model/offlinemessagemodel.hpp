#ifndef OFFLINEMESSAGEMODEL_H
#define OFFLINEMESSAGEMODEL_H

#include <string>

#include <vector>
using std::string; using std::vector;
class OfflineMsgModel
{
private:
    /* data */
public:
    //存储用户的离线消息
    void insert(int userid,string msg);

    //删除用户的离线消息
    void remove(int userid);

    //查询用户的离线消息(使用字符串数组来实现存储)
    vector<string> query(int userid);

};






#endif
