#include "offlinemessagemodel.hpp"
#include "db.h"



//存储用户的离线消息
void OfflineMsgModel::insert(int userid,string msg)
{
     //1 组装sql语句
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "insert into OfflineMessage values('%d','%s')", userid, msg.c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

//删除用户的离线消息
void OfflineMsgModel::remove(int userid)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "delete from OfflineMessage where userid=%d", userid);

    MySQL mysql;
    if(mysql.connect())
    {
        mysql.update(sql);
    }
}

//查询用户的离线消息(使用字符串数组来实现存储)
vector<string> OfflineMsgModel::query(int userid)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "select message from OfflineMessage where userid=%d", userid);

    vector<string> vec; //
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
                vec.push_back(row[0]);
            }
            mysql_free_result(res);
            return vec;
        }
        
    }
    return vec;
}


