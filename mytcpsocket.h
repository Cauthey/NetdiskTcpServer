#ifndef MYTCPSOCKET_H
#define MYTCPSOCKET_H

#include <QTcpSocket>
#include "protocol.h"
#include "opedb.h"
#include<QDir>
#include<QFile>
#include<QTimer>

class MyTcpSocket : public QTcpSocket
{
    Q_OBJECT
public:
    MyTcpSocket();
    QString getName();
    void copyDir(QString strSrcDir,QString strDesDir);
signals:
    void offine(MyTcpSocket *mySocket);

public slots:
    void recvMsg();
    void clientOffine();  // 客户端下线

    void sendFileToClient();

private:
    QString m_strName;

    QFile m_file;
    qint64 m_iTotal;
    qint64 m_iRecived;
    bool m_bUpload;

    QTimer *m_pTimer;
};

#endif // MYTCPSOCKET_H
