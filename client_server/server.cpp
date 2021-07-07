#include "server.h"

int main(int argc, char* argv[]) {
    MyRpcServer rpcServer;
    myserver::EchoServer* dfsService = new MyServiceImpl();

    rpcServer.RegisterService(dfsService);
    rpcServer.Start("127.0.0.1",1234);

    return 0;
}