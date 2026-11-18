#ifndef BUREAU_H
#define BUREAU_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

// Classe purement C++ (couche Modèle) : toutes les requêtes SQL
// sur la table BUREAUX sont ici, aucune derrière un bouton.
class bureau
{
public:
    int id;
    QString localisation;
    QDate dateOuverture;
    QString statut;        // "Actif" ou "Inactif"
    QString utilisateur;   // "Administrateur" ou "Technicien"
    QString erreurSql;     // dernier message d'erreur (validation ou Oracle)

    bureau() : id(0), statut("Actif") {}
    bureau(int, QString, QDate, QString, QString);

    // Contrôle de saisie (obligatoire dans le code C++)
    bool valider(QString *message = nullptr) const;

    // CRUD
    bool ajouter();                                    // Create
    QSqlQueryModel *afficher(QObject *parent = nullptr); // Read
    bool modifier();                                   // Update
    bool supprimer(int);                               // Delete
};

#endif // BUREAU_H