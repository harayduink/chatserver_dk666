#include "db.h"

#include <muduo/base/Logging.h>
// #include <string>
// using std::string;
static string server = "127.0.0.1";
static string user = "root";
static string password = "332400349";
static string dbname = "server_talk";

// 初始化数据库连接
MySQL::MySQL()
{
    _conn = mysql_init(nullptr);
}
// 释放数据库连接资源
MySQL::~MySQL()
{
    if (_conn != nullptr)
        mysql_close(_conn);
}
// 连接数据库
bool MySQL::connect()
{
    MYSQL *p = mysql_real_connect(_conn, server.c_str(), user.c_str(),
    password.c_str(), dbname.c_str(), 3306, nullptr, 0);
    if (p != nullptr)
    {
        //c和c++代码默认的编码字符是ASCII，如果不设置，从mysql上拉下来的中文显示？
        mysql_query(_conn, "set names gbk");
        LOG_INFO << "connet mysql success!";;
    }else
    {
        LOG_INFO << "connect mysql fail!";
    }
    return p;
}
// 更新操作
bool MySQL::update(string sql)
{
    //执行sql语句，执行失败返回错误日志
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "更新失败!\n";
        LOG_INFO << "ERROR_INFO IS:" << mysql_error(_conn) << '\n';
        return false;
    }
        return true;
}
// 查询操作
MYSQL_RES* MySQL::query(string sql)
{
    if (mysql_query(_conn, sql.c_str()))
    {
        LOG_INFO << __FILE__ << ":" << __LINE__ << ":"
        << sql << "查询失败!";
        return nullptr;
    }
    return mysql_use_result(_conn);
}

//获取连接
MYSQL* MySQL::getConnection()
{
    return _conn;
}