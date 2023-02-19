#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include<Qtsql/QSqlDatabase>
#include<Qtsql/QSqlQuery>
#include<QStringList>


class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

    bool handleRegist(const char *name,const char *pwd);
    bool handleLogin(const char *name,const char *pwd);
    void handleOffine(const char *name);
    QStringList handleAllOnline();

    int handleSearchUser(const char *name);

signals:
private:
    QSqlDatabase m_db;

};

#endif // OPEDB_H
