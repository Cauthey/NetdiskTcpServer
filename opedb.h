#ifndef OPEDB_H
#define OPEDB_H

#include <QObject>
#include<Qtsql/QSqlDatabase>
#include<Qtsql/QSqlQuery>


class OpeDB : public QObject
{
    Q_OBJECT
public:
    explicit OpeDB(QObject *parent = nullptr);
    static OpeDB& getInstance();
    void init();
    ~OpeDB();

signals:
private:
    QSqlDatabase m_db;

};

#endif // OPEDB_H
