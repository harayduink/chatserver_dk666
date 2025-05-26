#include "friendmodel.hpp"
#include "db.h"
// #include

void FriendModel::insert(int userid, int friendid)
{
    //1 组装sql语句
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "insert into Friend values('%d','%d')", userid, friendid);

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}
    
//从服务返回好友列表 调用列表时，还需要查询信息，使用两个表的联合查询
//实际业务当中，为了降低服务器的io负担，好友列表存储在客户本地
vector<User> FriendModel::query(int userid)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "select a.id, a.name, a.state from User a inner join Friend b on b.friendid = a.id where b.userid=%d", userid);

    vector<User> vec; //
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            //吧userid用户的所有离线消息放入vec中返回
            MYSQL_ROW row; 
            //row = mysql_fetch_row(res) 调用时就会读取一行条目，因此需要先默认构造，防止第一条条目被覆盖
            while((row = mysql_fetch_row(res))!=nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                vec.push_back(user);
            }
            mysql_free_result(res);
            return vec;
        }
    }
    return vec;
}
