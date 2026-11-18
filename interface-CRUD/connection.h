#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QString>

/* Classe Connection
 * Implémentation du pattern Singleton pour gérer une connexion unique à la base de données.
 * Utilise QSqlDatabase pour établir et fermer la connexion.
 * L'instance est accessible via la méthode statique getInstance().
 * La méthode createConnection() configure et ouvre la base de données.
 */

class Connection {
public:


    /******Méthodes publiques*****/
    /****************************/

    static Connection& createInstance(); // Méthode qui retourne une référence unique
    bool createConnection();  // Méthode utilisée pour établir la connexion à la base

    // Taille maximale (en caractères) d'une colonne texte, lue dans le dictionnaire
    // Oracle (USER_TAB_COLUMNS). Retourne 0 si la colonne est inconnue ou non texte.
    // Permet d'aligner les contrôles de saisie C++ sur la vraie structure de la base.
    static int longueurColonne(const QString &table, const QString &colonne);

private:

    // attribut Connexion persistante: attribut d’une instance qui vit longtemps (le Singleton)
    QSqlDatabase db;

    /******Méthodes privées*****/
    /**************************/

    Connection(); //Constructeur privé pour eviter la création d'autres instances
    ~Connection(); //Destructeur privé pour fermer proprement la connexion et pour empêcher toute destruction non contrôlée de l’instance Singleton

    // Interdire la copie & l'affectation ==> Suppression pour garantir l’unicité de l'instance de connexion
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H


