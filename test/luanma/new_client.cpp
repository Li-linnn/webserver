#include<iostream>
#include<socket_package/socket.cpp>

int main()
{  
    //创建socket
    Socket client;
    //连接服务端
    client.connect("127.0.0.1", 8080);
    //一旦connect成功，客户端的sockfd就自动和服务端的connfd绑定
    
    //向服务端发送数据
    std::string data = "hello world!";
    client.send(data.c_str(), data.size());

    //接收服务端数据
    char buf[1024] = {0};
    client.recv(buf, sizeof(buf));

    //关闭
    client.close();

    return 0;
}