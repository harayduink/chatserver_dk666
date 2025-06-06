#ifndef GROUP_H
#define GROUP_H

#include "groupuser.hpp"
#include <string>
#include <vector>
using std::string; using std::vector;
//User表的ORM类
class Group
{
    public:
        Group(int id=-1, string name="", string desc="")
        {
            this->id = id;
            this->name = name;
            this->desc = desc;
        }

        void setId(int id) {this->id = id;}
        void setName(string name) {this->name = name;}
        void setDesc(string desc) {this->desc = desc;}

        int getId() {return this->id;}
        string getName() {return this->name;}
        string getDesc() {return this->desc;}
        vector<GroupUser> &getUser() { return this->users;}

    private:
        int id;
        string name;
        string desc;
        vector<GroupUser> users; //有一个保存组员信息的vec
};


#endif