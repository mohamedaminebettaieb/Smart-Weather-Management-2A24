#include "connection.h"
#include <QDebug>

// Constructor
Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

// Destructor
Connection::~Connection()
{
    if (db.isOpen())
        db.close();
}

// Singleton instance
Connection& Connection::createInstance()
{
    static Connection instance;
    return instance;
}

// Establish Oracle connection
bool Connection::createConnection()
{
    if (db.isOpen())
        return true;

    db.setDatabaseName("projet2A24");
    db.setUserName("khadija");
    db.setPassword("1234");

    if (db.open()) {
        qDebug() << "Connexion Success";
        return true;
    }

    qDebug() << "Echec de la connexion :"
             << db.lastError().text();

    return false;
}
