#pragma once

#include<string>
using std::string;
#include<sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include<unistd.h>
#include<cstring>
#include<utility/logger.h>
using namespace utility;
namespace yazi
{
    namespace socket
    //命名空间：避免命名冲突，且更好的实现模块化管理，例如实现其他功能的模块还能再创建一个命名空间
    {
        class Socket
        {
        public:
            Socket();
            Socket(int sockfd);
            bool bind(const string& ip, int port);
            bool listen(int backlog);//感觉listen不传参也行？
            bool connect(const string& ip, int port);
            int accept();
            int send(const char* buf, int len);
            int recv(char* buf, int len);
            void close();
            ~Socket();
        protected:
            string m_ip;
            int m_port;
            int m_sockfd;
        };
    }
}