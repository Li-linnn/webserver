#include"chatserver.hpp"
#include"chatservice.hpp"
#include<iostream>
#include<signal.h>

void resetHandler(int)
{
    ChatService::instance()->reset();
    exit(0);
}
int main(int argc, char** argv)
{
    if(argc < 3)
    {
        std::cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << std::endl;
        //cerr标准错误流输出
        exit(-1);
    }
    //解析ip和port端口
    char* ip = argv[1];//argv:char**, argv[1]:char*
    uint16_t port = atoi(argv[2]);
    signal(SIGINT, resetHandler);
    // SIGINT（Signal Interrupt）：由操作系统发送给进程的中断信号。

    EventLoop loop;//创建主EventLoop
    InetAddress addr(ip, port);
    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}