#ifndef USER_H
#define USER_H

#include <string>
using std::string;


//匹配user表的ORM类
class User
{
protected:
    int id;
    string name;
    string password;
    string state;

public:
    User(int id=-1, string name="", string pwd="", string state="offline"){
        this->id=id;
        this->name=name;
        this->state=state;
        this->password=pwd; 
    };

    void setId(int id){this->id = id;}
    void setName(string name){this->name = name;}
    void setPwd(string pwd){this->password = pwd;}
    void setState(string state){this->state = state;}

    int getId(){return this->id;}
    string getName(){return this->name;}
    string getPwd(){return this->password;}
    string getState(){return this->state;}
};

#endif