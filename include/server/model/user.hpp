#ifndef USER_H
#define USER_H
#include<string>

//创建一个User类，数据库内表的字段有哪些，这里就怎样设计
class User
{
public:
    User(int id = -1, std::string name = " ", 
        std::string password = " ", std::string state = "offline")
    {
        this->id = id;
        this->name = name;
        this->password = password;
        this->state = state;
    }
    
    void setId(int id){this->id = id;}
    void setName(std::string name){this->name = name;}
    void setPassword(std::string password){this->password = password;}
    void setState(std::string state){this->state = state;}
    /*
    类似于：
        id=___ , name=___ ...
        设计一个能够对应表中字段的类，为了让用户设置字段信息和我们获取字段信息
        
        User user;
        user.setName(name);
        user.setPassword(password);
        创建时先利用set设置好值

        bool state = _userModel.insert(user);再insert

        bool UserModel::insert(User& user)
        {
            //组装sql语句
            char sql[1024] = {0};
            sprintf(sql, "insert into User(name, password, state) values('%s', '%s', '%s')",
                user.getName(), user.getPassword(), user.getState());
    */

    int getId(){return this->id;}
    std::string getName(){return this->name;}
    std::string getPassword(){return this->password;}
    std::string getState(){return this->state;}    
private:
    int id;
    std::string name;
    std::string password;
    std::string state;
};

#endif