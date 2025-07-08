#include<iostream>
#include<thread>
#include<string>
#include<chrono>
#include<vector>
#include<ctime>
#include"json.hpp"
using json = nlohmann::json;
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include"group.hpp"
#include"user.hpp"
#include"public.hpp"
#include <c++/11/iomanip>

//记录已登录的用户信息
User g_currentUser;
//记录已登录用户的好友列表信息
std::vector<User> g_currentUserFriendList;
//记录已登录用户的群组列表信息
std::vector<Group> g_currentUserGroupList;
//登录成功用户的基本信息
void showCurrentUserData();

//接受信息线程
void readTaskHandler(int clientfd);
//获取时间信息
std::string getCurrentTime();
//主聊天页面程序
void mainMenu(int clientfd);
//控制聊天页面程序
bool isMainMenuRunning = false;

//聊天客户端实现，main线程用作发送线程，子线程用作接收线程
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatClient 127.0.0.1 6000" << std::endl;
        //cerr标准错误流输出
        exit(-1);
    }

    char* ip = argv[1];//argv:char**, argv[1]:char*
    uint16_t port = atoi(argv[2]);

    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd == -1)
    {
        std::cerr << "socket create error!" << std::endl;
        exit(-1);//0：正常退出，非0：异常退出
    }

    sockaddr_in server;//sockaddr_in:用来封装IP地址、端口号和地址类型的结构体
    memset(&server, 0, sizeof(sockaddr_in));

    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    server.sin_addr.s_addr = inet_addr(ip);

    //client和server进行连接
    if(connect(clientfd, (sockaddr*)&server, sizeof(sockaddr_in)) == -1)
    {
        std::cerr << "connect server error!" << std::endl;
        close(clientfd);
        exit(-1);
    }
    
    //main线程，用于接收用户输入，负责发送数据
    for(;;)
    {
        //显示菜单
        std::cout << "1.login" << std::endl;
        std::cout << "2.register" << std::endl;
        std::cout << "3.quit" << std::endl;
        std::cout << "choice:";
        int choice = 0;
        std::cin >> choice;
        std::cin.get();//读掉缓冲区残留的回车

        switch(choice)
        {
        case 1://登录业务
        {
            int id = 0;
            char pwd[50] = {0};
            std::cout << "userid:";
            std::cin >> id;
            std::cin.get();
            std::cout << "password:";
            std::cin.getline(pwd, 50);

           json js;
           js["id"] = id;
           js["password"] = pwd;
           js["msgid"] = LOGIN_MSG;
           std::string request = js.dump();
           
           //发送给服务端
           int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
           if(len == -1)
           {
            //发送请求失败
                std::cerr << "send login msg error!" << request << std::endl;
           }
           else
           {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if(len == -1)
                {
                    std::cerr << "recv login response error!" << std::endl;
                }
                else
                {
                    json responsejs;
                    responsejs = json::parse(buffer);
                    if(responsejs["errno"].get<int>() != 0)
                    {
                        std::cerr << "login error!" << std::endl;
                       // std::cerr << responsejs["errmsg"] << std::endl;
                    }
                    else
                    {
                        //用户消息
                        g_currentUser.setId(responsejs["id"].get<int>());
                        g_currentUser.setName(responsejs["name"]);
                        //好友列表
                        if(responsejs.contains("friend"))
                        {
                            std::vector<std::string> friendVec = responsejs["friend"];
                            for(std::string& friendMsg : friendVec)
                            {
                                json friendjs = json::parse(friendMsg);
                                User user;
                                user.setId(friendjs["friendid"].get<int>());
                                user.setName(friendjs["friendname"]);
                                user.setState(friendjs["friendstate"]);
                                g_currentUserFriendList.push_back(user);
                            }   
                        }
                        //群组列表
                        if(responsejs.contains("group"))
                        {
                            std::vector<std::string> groupVec = responsejs["group"];
                            for(std::string& groupMsg : groupVec)
                            {
                                json groupjs = json::parse(groupMsg);
                                Group group;
                                group.setID(groupjs["groupid"]);
                                group.setName(groupjs["groupname"]);
                                group.setDesc(groupjs["groupdesc"]);
                                
                                std::vector<std::string> groupuserVec = groupjs["groupuser"];
                                for(std::string& groupuserMsg : groupuserVec)
                                {
                                    json groupUserjs = json::parse(groupuserMsg);
                                    GroupUser groupuser;
                                    groupuser.setId(groupUserjs["userid"]);
                                    groupuser.setName(groupUserjs["username"]);
                                    groupuser.setState(groupUserjs["userstate"]);
                                    groupuser.setRole(groupUserjs["userrole"]);
                                    group.getUsers().push_back(groupuser);
                                }
                                g_currentUserGroupList.push_back(group);
                            }
                        }
                        //显示登录用户的基本信息
                        showCurrentUserData();

                        //显示当前用户离线消息
                        if(responsejs.contains("offline message"))
                        {
                            std::vector<std::string> offlineMsgVec = responsejs["offline message"];
                            for(std::string& offlineMsg : offlineMsgVec)
                            {
                                json offlineMsgjs = json::parse(offlineMsg);
                                if(offlineMsgjs.contains("groupid"))
                                {
                                    std::cout << "from: group" << offlineMsgjs["groupid"] <<" "
                                              << "[" << offlineMsgjs["id"]<< "]" 
                                              << offlineMsgjs["name"].get<std::string>()
                                              << offlineMsgjs["msg"].get<std::string>()
                                              << std::endl;
                                }
                                else
                                {
                                    std::cout << offlineMsgjs["time"] << "[" << offlineMsgjs["id"]
                                    << "]" << offlineMsgjs["name"] << "said::" << offlineMsgjs["msg"]
                                    << std::endl;
                                }
                            }
                        }

                        //登陆成功，启动接受线程接受数据
                        isMainMenuRunning = true;
                        std::thread readTask(readTaskHandler, clientfd);
                        readTask.detach();
                        //进入聊天主菜单页面
                        mainMenu(clientfd);
                        std::this_thread::sleep_for(std::chrono::seconds(1));
                    }
                }
           }
        }
        break;
        case 2://注册业务
        {
            char name[50] = {0};
            char pwd[50] = {0};
            std::cout << "user name:";
            std::cin.getline(name, 50);
            std::cout << "user password:";
            std::cin.getline(pwd,50);
        
            json js;
            js["msgid"] = REG_MSG;
            js["name"] = name;
            js["password"] = pwd;
            std::string request = js.dump();
        
            int len = send(clientfd, request.c_str(), strlen(request.c_str()) + 1, 0);
            if(len == -1)
            {
                std::cerr << "send register request error!" << std::endl;
            }
            else
            {
                char buffer[1024] = {0};
                len = recv(clientfd, buffer, 1024, 0);
                if(len == -1)
                {
                    std::cerr << "recv register response error!" << std::endl;
                }
                else
                {
                    json responsejs = json::parse(buffer);
                    if(responsejs["errno"].get<int>() != 0)
                    {
                        std::cerr << "register error!" << std::endl;
                    }
                    else
                    {
                        std::cout << name << " register success, userid is " << responsejs["id"] << std::endl;
                    }
                }
            }
        }
        break;
        case 3:
        {
            close(clientfd);
            exit(-1);
        }
        break;
        }
    }
    return 0;
}

//接收线程
void readTaskHandler(int clientfd)
{
    for(;;)
    {
        char buf[1024] = {0};
        int len = recv(clientfd, buf, 1024, 0);
        /*
        当服务端调用 conn->send(msg)：
            数据通过 TCP 协议发送到客户端。
            客户端内核的接收缓冲区被填入数据。
            阻塞在 recv() 的接收线程被唤醒，读取缓冲区数据。
        */
        if(len == -1 || len == 0)
        {
            std::cerr << "recv response error!";
            close(clientfd);
            exit(-1);
        }
        else
        {
            json response = json::parse(buf);
            if(response["msgid"].get<int>() == ONE_CHAT_MSG_ACK)
            {
                std::cout << "from " << "[" << response["id"] << "]" 
                          << response["name"].get<std::string>() << ": " 
                          << response["msg"].get<std::string>()<< "  "
                          << "time: " << response["time"].get<std::string>()
                          << std::endl;
                //continue;
            }
            else if(response["msgid"].get<int>() == GROUP_CHAT_MSG_ACK)
            {
                std::cout << "from: group: " << response["groupid"] << " "
                          << "[" << response["id"] << "]"
                          << response["name"].get<std::string>() << ": "
                          << response["msg"].get<std::string>()<< "  "
                          << "time: " << response["time"].get<std::string>()
                          << std::endl;
            }
            else if(response["msgid"].get<int>() == LOGINOUT_MSG_ACK)
            {
                if(response["errno"].get<int>() != 0)
                {
                    std::cerr << "logingout error!" << std::endl;
                }
                else
                {
                    std::cout << "[" << response["id"] << "]"
                              << response["name"].get<std::string>()
                              << " loginout success!" << std::endl; 
                }
            }
            else
            {
                
            }
        }
    }
}

//显示当前登录用户基本信息
void showCurrentUserData()
{
    std::cout << "user: " << "[" << g_currentUser.getId() << "]" << g_currentUser.getName()
    << " is online" << std::endl;
    std::cout << g_currentUser.getName() << " friends:" << std::endl;
    for(User& user : g_currentUserFriendList)
    {
        std::cout << std::left << std::setw(10) << user.getName() 
                  << std::setw(10) << user.getState() << std::endl;
    }
}

//系统支持的客户端命令列表
std::unordered_map<std::string, std::string> commandMap = {
    {"help", "显示所有支持的命令，格式help"},
    {"chat", "一对一聊天，格式chat:(friendid):(message)"},
    {"addfriend", "添加好友，格式addfriend:(friendid)"},
    {"creategroup", "建群，格式creategroup:(groupname):(groupdesc)"},
    {"addgroup", "加群，格式addgroup:(groupid)"},
    {"groupchat", "群组聊天，格式groupchat:(groupid):(msg)"},
    {"loginout", "注销，格式loginout"}};

//执行命令业务的处理器函数
void help(int = -1, std::string = " ");
void chat(int, std::string);
void addfriend(int, std::string);
void creategroup(int, std::string);
void addgroup(int, std::string);
void groupchat(int, std::string);
void loginout(int = -1, std::string = " ");

//定义系统支持的客户端命令处理器
std::unordered_map<std::string, std::function<void(int, std::string)>> commandHandlerMap = {
    {"help", help},
    {"chat", chat},
    {"addfriend", addfriend},
    {"creategroup", creategroup},
    {"addgroup", addgroup},
    {"groupchat", groupchat},
    {"loginout", loginout}
};


//主聊天页面程序
void mainMenu(int clientfd)
{
    help();//显示功能
    char buf[1024] = {0};
    while(isMainMenuRunning)
    {
        std::cin.getline(buf, 1024);
        std::string commandbuf(buf);//直接用buf没有find功能
        int idx = commandbuf.find(":");
        std::string command;
        if(idx == -1)
        {
            command = commandbuf;
        }
        else
        {
            command = commandbuf.substr(0, idx);//从索引0开始取idx个长度
        }

        auto it = commandHandlerMap.find(command);
        if(it == commandHandlerMap.end())
        {
            std::cerr << "invalid command input!" << std::endl;
            continue;
        }
        //调用处理器函数
        it->second(clientfd, commandbuf.substr(idx + 1, commandbuf.size() - (idx + 1)));
    }
}

void help(int fd, std::string str)
{

}

void chat(int clientfd, std::string command)
{
    int idx = command.find(":");
    if(idx == -1)
    {
        std::cerr << "chat command invalid!" << std::endl;
        return;
    }
    int friendid = atoi(command.substr(0, idx).c_str());
    std::string msg = command.substr(idx + 1, command.size() - (idx + 1));

    json chatjs;
    chatjs["msgid"] = ONE_CHAT_MSG;
    chatjs["id"] = g_currentUser.getId();
    chatjs["name"] = g_currentUser.getName();
    chatjs["toid"] = friendid;
    chatjs["msg"] = msg;
    chatjs["time"] = getCurrentTime();
    std::string request = chatjs.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "chat request error!" << std::endl;
        return;
    }
}

void addfriend(int clientfd, std::string command)
{
    int friendid = atoi(command.c_str());
    json js;
    js["msgid"] = ADD_FRIEND_MSG;
    js["id"] = g_currentUser.getId();
    js["friendid"] = friendid;
    std::string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "addfriend request error!" << std::endl;
        return;
    }
}

void creategroup(int clientfd, std::string command)
{
    int idx = command.find(":");
    if(idx == -1)
    {
        std::cerr << "creategroup command error!" << std::endl;
        return;
    }
    std::string groupname = command.substr(0, idx);
    std::string groupdesc = command.substr(idx + 1, command.size() - (idx + 1));
    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupname"] = groupname;
    js["groupdesc"] = groupdesc;
    std::string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "addfriend request error!" << std::endl;
        return;
    }
}

void addgroup(int clientfd, std::string command)
{
    int groupid = atoi(command.c_str());
    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = g_currentUser.getId();
    js["groupid"] = groupid;
    std::string request = js.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "addgroup request error!" << std::endl;
        return;
    }
}

void groupchat(int clientfd, std::string command)
{
    int idx = command.find(":");
    if(idx == -1)
    {
        std::cerr << "groupchat command invalid!" << std::endl;
        return;
    }
    int groupid = atoi(command.substr(0, idx).c_str());
    std::string msg = command.substr(idx + 1, command.size() - (idx + 1));

    json chatjs;
    chatjs["msgid"] = GROUP_CHAT_MSG;
    chatjs["id"] = g_currentUser.getId();
    chatjs["name"] = g_currentUser.getName();
    chatjs["groupid"] = groupid;
    chatjs["msg"] = msg;
    chatjs["time"] = getCurrentTime();
    std::string request = chatjs.dump();
    int len = send(clientfd, request.c_str(), strlen(request.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "groupchat request error!" << std::endl;
        return;
    }
}

void loginout(int clientfd, std::string command)
{
    json js;
    js["id"] = g_currentUser.getId();
    js["msgid"] = LOGINOUT_MSG;
    std::string str = js.dump();
    int len = send(clientfd, str.c_str(), strlen(str.c_str()), 0);
    if(len == -1)
    {
        std::cerr << "loginout request error!" << std::endl;
        return;
    }
    else
    {
        isMainMenuRunning = false;
    }
}

std::string getCurrentTime()
{
    auto tt = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());
    struct tm *ptm = localtime(&tt);
    char date[60] = {0};
    sprintf(date, "%d-%02d-%02d %02d:%02d:%02d",
            (int)ptm->tm_year + 1900, (int)ptm->tm_mon + 1, (int)ptm->tm_mday,
            (int)ptm->tm_hour, (int)ptm->tm_min, (int)ptm->tm_sec);
    return std::string(date);
}
