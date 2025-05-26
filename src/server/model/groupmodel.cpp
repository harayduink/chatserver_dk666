#include "groupmodel.hpp"
#include "db.h"


//创建群组
bool GroupModel::createGroup(Group &group)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "insert into AllGroup(groupname, groupdesc) values('%s','%s')",\
         group.getName().c_str(), group.getDesc().c_str());

    MySQL mysql;
    if(mysql.connect())
    {
        if(mysql.update(sql))
        {
            group.setId(mysql_insert_id(mysql.getConnection()));
            return true;
        }
    }
    return false;
}

//加入一个群组(已经在mysql记录的)
void GroupModel::addGroup(int userid, int groupid, string role)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    sprintf(sql, "insert into GroupUser values('%d','%d', '%s')",\
         groupid, userid, role.c_str());

    MySQL mysql;
    if(mysql.connect()){
        mysql.update(sql);
    }
}

//查询用户所在群组信息
vector<Group> GroupModel::queryGroup(int userid)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    //多表格查询，需要使用联合查询操作，减少访问数据库的次数
    sprintf(sql, "select a.id, a.groupname, a.groupdesc from AllGroup a inner join \
        GroupUser b on a.id=groupid where b.userid=%d", userid);

    vector<Group> groupVec;

    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while ((row = mysql_fetch_row(res))!=nullptr)   
            {
                Group group;
                group.setId(atoi(row[0]));
                group.setName(row[1]);
                group.setDesc(row[2]);
                groupVec.push_back(group);
            }
            mysql_free_result(res);
        }
    }
    //查询到的group构建了以后，其中的群员信息还是空的，需要查询每个群组
    //将群员信息进行添加到对应的群组当中
    for (Group &group: groupVec)
    {
        sprintf(sql, "select a.id, a.name, a.state, b.grouprole from User a inner join \
        GroupUser b on b.userid=a.id where b.groupid=%d", group.getId());

        MYSQL_RES *res = mysql.query(sql);

        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while ((row=mysql_fetch_row(res))!=nullptr) 
            {
                GroupUser user;
                user.setId(atoi(row[0]));
                user.setName(row[1]);
                user.setState(row[2]);
                user.setRole(row[3]);
                group.getUser().push_back(user);
            }
            mysql_free_result(res);
        }
    }
    return groupVec;
}

//根据指定的groupid查询群组用户id列表，除了userid自己，主要用户群聊业务给群组其他成员群发消息
vector<int> GroupModel::queryGroupUsers(int userid, int groupid)
{
    char sql[1024] = {0};
    //sql语句书写一定要正确，一般主要会出问题在这里
    //多表格查询，需要使用联合查询操作，减少访问数据库的次数
    sprintf(sql, "select userid from GroupUser where groupid=%d and userid!=%d", groupid, userid);


    vector<int> idVec;
    MySQL mysql;
    if(mysql.connect())
    {
        MYSQL_RES *res = mysql.query(sql);
        if(res!=nullptr)
        {
            MYSQL_ROW row;
            while((row=mysql_fetch_row(res) )!= nullptr)
            {
                idVec.push_back(atoi(row[0]));
            }
            mysql_free_result(res);
        }
    }
    return idVec;
}