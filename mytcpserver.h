#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H
#include<QTcpServer>
#include<QList>
#include"mytcpsocket.h"


class MyTcpServer : public QTcpServer
{
    Q_OBJECT
public:
    MyTcpServer();

    static MyTcpServer &getInstance();

    void incomingConnection(qintptr socketDescriptor);
private:
    QList<MyTcpSocket*> m_tcpSocketList;


};

#endif // MYTCPSERVER_H
