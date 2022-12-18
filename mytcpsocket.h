#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();
signals:
    void offine(MyTcpSocket *mySocket);

public slots:
    void recvMsg();
    void clientOffine();  // 客户端下线
private:
    QString m_strName;
};

#endif // MYTCPSOCKET_H
