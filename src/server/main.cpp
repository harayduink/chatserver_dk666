#include "chatserver.hpp"
#include "chatservice.hpp"
#include <iostream>
#include <signal.h>

using std::cout; using std::endl;
using std::cerr;

//处理服务器ctrl+c结束后，重置user的状态信息
void resetHandler(int a)
{   
    ChatService::instance()->reset();
    exit(0);
}

int main(int argc, char **argv)
{

    if(argc < 3)
    {
        cerr << "command invalid! example: ./ChatServer 127.0.0.1 6000" << endl;
        exit(-1);
    }

    char *ip = argv[1];
    uint16_t port = atoi(argv[2]);
    signal(SIGINT, resetHandler);
    net::EventLoop loop;
    net::InetAddress addr(ip, port);

    ChatServer server(&loop, addr, "ChatServer");

    server.start();
    loop.loop();

    return 0;
}