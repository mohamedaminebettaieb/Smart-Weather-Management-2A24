#include "bureau.h"

#include <QDebug>
#include <QObject>
#include <QSqlError>

// Le pilote ODBC Microsoft pour Oracle échoue avec les paramètres liés
// (erreur S1010). On construit donc des requêtes SQL complètes, avec
// échappement des apostrophes dans les chaînes.
static QString sqlTexte(const QString &t)
{
    QString r = t;
    r.replace("'", "''");
    return "'" + r + "'";
}

static QString sqlDate(const QDate &d)
{
    return "TO_DATE('" + d.toString("yyyy-MM-dd") + "', 'YYYY-MM-DD')";
}

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
    else if (statut != "Actif" && statut != "Inactif")
        erreur = QString::fromUtf8("Le statut doit être « Actif » ou « Inactif ».");
    else if (utilisateur != "Administrateur" && utilisateur != "Technicien")
        erreur = QString::fromUtf8("L'utilisateur doit être « Administrateur » ou « Technicien ».");

    if (message)
        *message = erreur;
    return erreur.isEmpty();
}


bool bureau::ajouter()
{
    if (!valider(&erreurSql))
        return false;

    QSqlQuery query;
    const QString sql = QString("insert into bureaux (ID_BUREAU, LOCALISATION_BUREAU, DATE_OUVERTURE, STATUT_BUREAU, UTILISATEUR_BUREAU) "
                                "values (%1, %2, %3, %4, %5)")
                            .arg(id)
                            .arg(sqlTexte(localisation.trimmed()))
                            .arg(sqlDate(dateOuverture))
                            .arg(sqlTexte(statut))
                            .arg(sqlTexte(utilisateur));

    const bool ok = query.exec(sql);
    if (!ok) {
        erreurSql = query.lastError().text();
        qDebug() << "Bureau::ajouter :" << erreurSql << sql;
    }
    return ok;
}


QSqlQueryModel *bureau::afficher(QObject *parent)
{
    QSqlQueryModel *model = new QSqlQueryModel(parent);

    model->setQuery("select ID_BUREAU, LOCALISATION_BUREAU, DATE_OUVERTURE, STATUT_BUREAU, UTILISATEUR_BUREAU from bureaux order by ID_BUREAU");
    model->setHeaderData(0, Qt::Horizontal, QObject::tr("id"));
    model->setHeaderData(1, Qt::Horizontal, QObject::tr("Localisation"));
    model->setHeaderData(2, Qt::Horizontal, QObject::tr("Date d'ouverture"));
    model->setHeaderData(3, Qt::Horizontal, QObject::tr("Statut"));
    model->setHeaderData(4, Qt::Horizontal, QObject::tr("Utilisateur"));

    return model;
}


bool bureau::modifier()
{
    if (!valider(&erreurSql))
        return false;

    QSqlQuery query;
    const QString sql = QString("update bureaux set LOCALISATION_BUREAU = %1, "
                                "DATE_OUVERTURE = %2, STATUT_BUREAU = %3, "
                                "UTILISATEUR_BUREAU = %4 "
                                "where ID_BUREAU = %5")
                            .arg(sqlTexte(localisation.trimmed()))
                            .arg(sqlDate(dateOuverture))
                            .arg(sqlTexte(statut))
                            .arg(sqlTexte(utilisateur))
                            .arg(id);

    if (!query.exec(sql)) {
        erreurSql = query.lastError().text();
        qDebug() << "Bureau::modifier :" << erreurSql << sql;
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        erreurSql = QString::fromUtf8("Aucun bureau avec l'ID %1 dans Oracle.").arg(id);
        return false;
    }
    return true;
}


bool bureau::supprimer(int id)
{
    QSqlQuery query;
    const QString sql = QString("delete from bureaux where ID_BUREAU = %1").arg(id);

    if (!query.exec(sql)) {
        erreurSql = query.lastError().text();
        qDebug() << "Bureau::supprimer :" << erreurSql << sql;
        return false;
    }
    if (query.numRowsAffected() <= 0) {
        erreurSql = QString::fromUtf8("Aucun bureau avec l'ID %1 dans Oracle.").arg(id);
        return false;
    }
    return true;
}