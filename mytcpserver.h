#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include<QTcpServer>


class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();
};

#endif // MYTCPSERVER_H
