#ifndef BUREAU_H
#define BUREAU_H

#include <QString>
#include <QDate>
#include <QSqlQuery>
#include <QSqlQueryModel>

class bureau
{
public:
    int id;
    QString localisation;
    QDate dateOuverture;
    QString statut;
    QString utilisateur;   // "Administrateur" ou "Technicien"
    QString erreurSql;   // dernier message d'erreur (validation ou Oracle)
    bureau() : id(0), statut("Actif") {}
    bureau(int, QString, QDate, QString, QString = QString());

    bool valider(QString *message = 0) const;

    bool ajouter();
    QSqlQueryModel *afficher(QObject *parent = 0);
    bool modifier();
    bool supprimer(int);
};

#endif // BUREAU_H