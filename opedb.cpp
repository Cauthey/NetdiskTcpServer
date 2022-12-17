#include "opedb.h"
#include<QMessageBox>
#include<QDebug>
#include<QDir>


OpeDB::OpeDB(QObject *parent) : QObject(parent)
{
    m_db = QSqlDatabase::addDatabase("QSQLITE");

}

OpeDB &OpeDB::getInstance()
{
    static OpeDB instance;
    return instance;
}

void OpeDB::init()
{
    m_db.setHostName("localhost");
    QString path = QDir::currentPath()+"/cloud.db";
//    m_db.setDatabaseName(":/cloud.db");   // 相对路径不好使
    m_db.setDatabaseName("D:\\front\\code\\QQQQ\\NetdiskTcpServer\\cloud.db");   // 绝对路径测试下
    if (m_db.open()){
        QSqlQuery query;
        query.exec("select * from userInfo");
        while(query.next()){
            QString data = QString("%1,%2,%3").arg(query.value(0).toString()).arg(query.value(1).toString().arg(query.value(2).toString()));
            qDebug() << data;
        }
    }else{
        QMessageBox::critical(NULL,"打开数据库","打开数据库失败");
    }

}

OpeDB::~OpeDB()
{
    m_db.close();

}
