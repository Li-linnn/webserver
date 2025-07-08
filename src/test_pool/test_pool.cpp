#include<iostream>
#include<Connection.hpp>
#include<CommonConnectionPool.hpp>
#include<mysql/mysql.h>
#include<string>
using namespace std;

int main()
{
    ConnectionPool *cp = ConnectionPool::getConnectionPool();
    auto conn = cp->getConnection();
    string name = "lijing", pwd = "123456", state = "online";
    
    char sql[1024] = {0};
    sprintf(sql, "insert into users(username, password, state) values('%s', '%s', '%s')",
        name.c_str(), pwd.c_str(), state.c_str());
        
    conn->update(sql);
    return 0;
}