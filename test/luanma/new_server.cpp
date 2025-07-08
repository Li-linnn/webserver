#include<socket_package/socket.cpp>

int main()
{
    Socket server;
    server.bind("127.0.0.1", 8080);
    server.listen(1024);
    while(true)
    {
        int connfd = server.accept();
        Socket client(connfd);
        /*
            这里新创建的client个人理解不是真正意义上的客户端socket，
            而是server端已经与client端连接成功的socket，为了用新socket继承旧socket
        */
        char buf[1024] = {0};
        client.recv(buf, sizeof(buf));
        client.send(buf, sizeof(buf));
    }
    server.close();
    return 0;
}