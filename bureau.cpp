#include "bureau.h"

#include <QDebug>
#include <QObject>
#include <QSqlError>

// Toutes les requêtes utilisent des requêtes préparées :
// prepare() + bindValue() + exec()  ->  protection contre l'injection SQL.
//
// La date est liée sous forme de texte et convertie côté Oracle avec
// TO_DATE : c'est plus fiable avec le pilote ODBC qu'un QDate lié directement.

bureau::bureau(int id, QString localisation, QDate dateOuverture, QString statut, QString utilisateur)
{
    this->id = id;
    this->localisation = localisation;
    this->dateOuverture = dateOuverture;
    this->statut = statut;
    this->utilisateur = utilisateur;
}


bool bureau::valider(QString *message) const
{
    QString erreur;

    if (id <= 0)
        erreur = QString::fromUtf8("L'id doit être un entier positif.");
    else if (localisation.trimmed().isEmpty())
        erreur = QString::fromUtf8("La localisation est obligatoire.");
    else if (localisation.trimmed().size() > 50)
        erreur = QString::fromUtf8("La localisation ne doit pas dépasser 50 caractères.");
    else if (!dateOuverture.isValid())
        erreur = QString::fromUtf8("La date d'ouverture est invalide.");
    else if (dateOuverture > QDate::currentDate())
        erreur = QString::fromUtf8("La date d'ouverture ne peut pas être dans le futur.");
    else if (statut != "Actif" && statut != "Inactif")
        erreur = QString::fromUtf8("Le statut doit être « Actif » ou « Inactif ».");
    else if (utilisateur != "Administrateur" && utilisateur != "Technicien")
        erreur = QString::fromUtf8("L'utilisateur doit être « Administrateur » ou « Technicien ».");

    if (message)
        *message = erreur;
    return erreur.isEmpty();
}


// ---------------------------------------------------------------- CREATE
bool bureau::ajouter()
{
    erreurSql.clear();
    if (!valider(&erreurSql))
        return false;

    QSqlQuery query;
    query.prepare("insert into bureaux (ID_BUREAU, LOCALISATION_BUREAU, DATE_OUVERTURE, "
                  "STATUT_BUREAU, UTILISATEUR_BUREAU) "
                  "values (:id, :loc, TO_DATE(:date, 'YYYY-MM-DD'), :statut, :user)");
    query.bindValue(":id", id);
    query.bindValue(":loc", localisation.trimmed());
    query.bindValue(":date", dateOuverture.toString("yyyy-MM-dd"));
    query.bindValue(":statut", statut);
    query.bindValue(":user", utilisateur);

    if (!query.exec()) {
        erreurSql = query.lastError().text();
        qDebug() << "bureau::ajouter :" << erreurSql;
        return false;
    }
    return true;
}


// ---------------------------------------------------------------- READ
QSqlQueryModel *bureau::afficher(QObject *parent)
{
    QSqlQueryModel *model = new QSqlQueryModel(parent);

    model->setQuery("select ID_BUREAU, LOCALISATION_BUREAU, DATE_OUVERTURE, "
                    "STATUT_BUREAU, UTILISATEUR_BUREAU "
                    "from bureaux order by ID_BUREAU");

    if (model->lastError().isValid())
        qDebug() << "bureau::afficher :" << model->lastError().text();

    model->setHeaderData(0, Qt::Horizontal, QObject::tr("ID"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Localisation"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date d'ouverture"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Utilisateur"));

    return model;
}


// ---------------------------------------------------------------- UPDATE
bool bureau::modifier()
{
    erreurSql.clear();
    if (!valider(&erreurSql))
        return false;

    QSqlQuery query;
    query.prepare("update bureaux set LOCALISATION_BUREAU = :loc, "
                  "DATE_OUVERTURE = TO_DATE(:date, 'YYYY-MM-DD'), "
                  "STATUT_BUREAU = :statut, "
                  "UTILISATEUR_BUREAU = :user "
                  "where ID_BUREAU = :id");
    query.bindValue(":loc", localisation.trimmed());
    query.bindValue(":date", dateOuverture.toString("yyyy-MM-dd"));
    query.bindValue(":statut", statut);
    query.bindValue(":user", utilisateur);
    query.bindValue(":id", id);

    if (!query.exec()) {
        erreurSql = query.lastError().text();
        qDebug() << "bureau::modifier :" << erreurSql;
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        erreurSql = QString::fromUtf8("Aucun bureau avec l'ID %1.").arg(id);
        return false;
    }
    return true;
}


// ---------------------------------------------------------------- DELETE
bool bureau::supprimer(int id)
{
    erreurSql.clear();
    if (id <= 0) {
        erreurSql = QString::fromUtf8("L'id doit être un entier positif.");
        return false;
    }

    QSqlQuery query;
    query.prepare("delete from bureaux where ID_BUREAU = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        erreurSql = query.lastError().text();
        qDebug() << "bureau::supprimer :" << erreurSql;
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        erreurSql = QString::fromUtf8("Aucun bureau avec l'ID %1.").arg(id);
        return false;
    }
    return true;
}