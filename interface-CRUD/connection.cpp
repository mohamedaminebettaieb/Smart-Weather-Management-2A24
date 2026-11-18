#include "connection.h"
#include <QDebug>
#include <QHash>
#include <QSet>
#include <QSqlQuery>

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

    db.setDatabaseName("Source_projet2A");
    db.setUserName("donia");
    db.setPassword("12345");

    if (db.open()) {
        qDebug() << "Connexion Success";
        return true;
    }

    qDebug() << "Echec de la connexion :"
             << db.lastError().text();

    return false;
}

// Longueur maximale d'une colonne texte (cache : une requête par table)
int Connection::longueurColonne(const QString &table, const QString &colonne)
{
    static QHash<QString, int> cache;      // clé "TABLE.COLONNE"
    static QSet<QString> tablesChargees;

    const QString t = table.toUpper();

    if (!tablesChargees.contains(t))
    {
        QSqlQuery q;
        q.prepare("SELECT COLUMN_NAME, CHAR_LENGTH FROM USER_TAB_COLUMNS "
                  "WHERE TABLE_NAME = :t");
        q.bindValue(":t", t);

        if (q.exec())
        {
            while (q.next())
                cache.insert(t + "." + q.value(0).toString().toUpper(),
                             q.value(1).toInt());
            tablesChargees.insert(t);
        }
        else
        {
            qDebug() << "Connection::longueurColonne :" << q.lastError().text();
        }
    }
    return cache.value(t + "." + colonne.toUpper(), 0);
}
