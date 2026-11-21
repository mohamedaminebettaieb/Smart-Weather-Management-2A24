#include "Connection.h"
#include <QHash>
#include <QSet>
#include <QSqlQuery>

// Ctor
// Initialise l'attribut `db` avec le driver ODBC.
// On le fait dans le constructeur pour que `db` soit stocké dans le Singleton.
// Cela signifie que la même instance de `QSqlDatabase` sera utilisée partout dans l'application
// via le Singleton, au lieu de créer une nouvelle connexion à chaque fois.
Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}

//Dtor
// Ferme la connexion si elle est encore ouverte lorsque l'objet est détruit
Connection::~Connection() {
    if (db.isOpen())
        db.close(); // Libère proprement les ressources
}

// --- Accès à l'instance unique du Singleton ---
// Utilise la technique "Meyers Singleton" : instance statique locale
// Elle est créée la première fois qu'on appelle createInstance()
// et reste vivante pendant toute la durée du programme (thread-safe en C++11+)
Connection& Connection::createInstance() {
    static Connection instance;  // Instance unique du Singleton
    return instance; // Retourne une référence à cette instance
}


bool Connection::createConnection() {
    db.setDatabaseName("Source_Projet2A");// saisissez le nom de votre source de données ici
    db.setUserName("amine");// saisissez votre nom d'utilisateur ici
    db.setPassword("0000");// saisissez votre mot de passe utilisateur ici

    if (db.open()) {
        qDebug() << "Connexion établie";
        return true;
    }

    qDebug() << "Échec de la connexion :" << db.lastError().text();
    return false;
}

int Connection::longueurColonne(const QString &table, const QString &colonne)
{
    static QHash<QString, int> cache;
    static QSet<QString> tablesChargees;
    const QString nomTable = table.toUpper();

    if (!tablesChargees.contains(nomTable)) {
        QSqlQuery query;
        query.prepare("SELECT COLUMN_NAME, CHAR_LENGTH FROM USER_TAB_COLUMNS WHERE TABLE_NAME = :table");
        query.bindValue(":table", nomTable);
        if (query.exec()) {
            while (query.next())
                cache.insert(nomTable + "." + query.value(0).toString().toUpper(), query.value(1).toInt());
            tablesChargees.insert(nomTable);
        } else {
            qDebug() << "Connection::longueurColonne:" << query.lastError().text();
        }
    }

    return cache.value(nomTable + "." + colonne.toUpper(), 0);
}

