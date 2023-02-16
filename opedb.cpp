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
//    QString path = QDir::currentPath()+"/cloud.db";
//    m_db.setDatabaseName(":/cloud.db");   // 相对路径不好使
//    m_db.setDatabaseName("D:\\front\\code\\QQQQ\\NetdiskTcpServer\\cloud.db");   // 绝对路径测试下
    m_db.setDatabaseName("D:\\Code\\QT\\2.netDisk\\NetdiskTcpServer\\cloud.db");   // 绝对路径测试下
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

bool OpeDB::handleRegist(const char *name, const char *pwd)
{
    if (NULL == name || NULL ==pwd){
        return false;
    }
    QString data = QString("insert into userInfo(name,pwd) values(\'%1\',\'%2\')").arg(name).arg(pwd);
    qDebug() << data ;
    QSqlQuery query;
    return query.exec(data);
}

bool OpeDB::handleLogin(const char *name, const char *pwd)
{
    if (NULL == name || NULL ==pwd){
        return false;
    }
    QString data = QString("select * from userInfo where name = \'%1\' and pwd = \'%2\' and online = 0").arg(name).arg(pwd);
    qDebug() << data ;
    QSqlQuery query;
    query.exec(data);
    if(query.next()){
        data = QString("update userInfo set online = 1 where name = \'%1\' ").arg(name);
        qDebug() << data ;
        QSqlQuery query;
        query.exec(data);
        return true;
    }else{
        return false;
    }
}

void OpeDB::handleOffine(const char *name)
{
    if (NULL == name){
        return ;
    }
    QString data = QString("update userInfo set online = 0 where name = \'%1\' ").arg(name);
    qDebug() << data ;
    QSqlQuery query;
    query.exec(data);
}

QStringList OpeDB::handleAllOnline()
{
    QString data = QString("select name from userInfo  where online = 1");
    qDebug() << data ;
    QSqlQuery query;
    query.exec(data);
    QStringList result;
    result.clear();
    while(query.next()){
        result.append(query.value(0).toString());
    }
    return result;
}
