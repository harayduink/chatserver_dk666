#include "usermodel.hpp"
#include "db/db.h"

#include <iostream>

using std::string;
 
bool UserModel::insert(User& user )
{
    //1 组装sql语句
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "insert into User(name, password, state) values('%s','%s','%s')"
    , user.getName().c_str(), user.getPwd().c_str(), user.getState().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            //获取插入成功的用户数据生成的主键id
            user.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

User UserModel::query(int id)
{
    //1 组装sql语句
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "select * from User where id = '%d'"
    , id);

    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES* res = mysql.query(sql);
        //读数据 
        if(res != nullptr)
        {
            MYSQL_ROW row = mysql_fetch_row(res);
            if(row != nullptr)
            {
                User user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setPwd(row[2]);
                user.setState(row[3]);

                mysql_free_result(res);
                return user;
            }
        }
    }
    return false;
}

bool UserModel::updateState(User user)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    // update user set state = '%s' where id=%d
    sprintf(sql, "update User set state = '%s' where id=%d"
    , user.getState().c_str(), user.getId());

    MySQL mysql;

    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            return true;
        }
    }
    return false;
}

void UserModel::resetState()
{
    //1 组装sql语句
    char sql[1024] = "update User set state = 'offline' where state = 'online'";
    //sql语句书写一定要正确，一般主要会出问题在这里

    MySQL mysql;

    if(mysql.connect())
    {
        mysql.update(sql);
    }

}