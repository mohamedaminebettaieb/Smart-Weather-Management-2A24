
#include "equipement.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>

// ==========================================
// CLASSE EQUIPEMENT
// ==========================================

// Constructeur par defaut
Equipement::Equipement()
{
    id = "";
    nom = "";
    localisation = "";
    statut = "";
    dateInstallation = QDate();
    type = "";
}

// Constructeur parametre
Equipement::Equipement(const QString &id,
                       const QString &nom,
                       const QString &localisation,
                       const QString &statut,
                       const QDate &dateInstallation,
                       const QString &type)
{
    this->id = id;
    this->nom = nom;
    this->localisation = localisation;
    this->statut = statut;
    this->dateInstallation = dateInstallation;
    this->type = type;
}

// Getters
QString Equipement::getId() const
{
    return id;
}

QString Equipement::getNom() const
{
    return nom;
}

QString Equipement::getLocalisation() const
{
    return localisation;
}

QString Equipement::getStatut() const
{
    return statut;
}

QDate Equipement::getDateInstallation() const
{
    return dateInstallation;
}

QString Equipement::getType() const
{
    return type;
}

// Setters
void Equipement::setId(const QString &id)
{
    this->id = id;
}

void Equipement::setNom(const QString &nom)
{
    this->nom = nom;
}

void Equipement::setLocalisation(const QString &localisation)
{
    this->localisation = localisation;
}

void Equipement::setStatut(const QString &statut)
{
    this->statut = statut;
}

void Equipement::setDateInstallation(const QDate &dateInstallation)
{
    this->dateInstallation = dateInstallation;
}

void Equipement::setType(const QString &type)
{
    this->type = type;
}

// Affichage
void Equipement::afficher() const
{
    qDebug() << "ID :" << id;
    qDebug() << "Nom :" << nom;
    qDebug() << "Localisation :" << localisation;
    qDebug() << "Statut :" << statut;
    qDebug() << "Date d'installation :" << dateInstallation;
    qDebug() << "Type :" << type;
}


// ==========================================
// CLASSE EQUIPEMENTDAO
// ==========================================

// Ajouter un equipement
bool EquipementDAO::ajouter(const QString &id,
                            const QString &nom,
                            const QString &localisation,
                            const QString &statut,
                            const QDate &dateInstallation,
                            const QString &type)
{
    qDebug() << "AJOUTER() CALLED";

    QSqlQuery query;

    query.prepare(
        "INSERT INTO EQUIPEMENTS "
        "(ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT) "
        "VALUES (:id, :nom, :localisation, :statut, :dateInstallation, :type)"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation);
    query.bindValue(":type", type);

    if (!query.exec())
    {
        qDebug() << "Erreur ajout équipement:"
                 << query.lastError().text();
        return false;
    }

    qDebug() << "Equipement insere avec succes dans Oracle!";

    return true;
}


// Afficher les equipements
QSqlQueryModel *EquipementDAO::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT "
        "FROM EQUIPEMENTS"
        );

    if (model->lastError().isValid())
    {
        qDebug() << "Erreur affichage équipements:"
                 << model->lastError().text();
    }

    model->setHeaderData(0, Qt::Horizontal, "ID");
    model->setHeaderData(1, Qt::Horizontal, "Nom");
    model->setHeaderData(2, Qt::Horizontal, "Localisation");
    model->setHeaderData(3, Qt::Horizontal, "Statut");
    model->setHeaderData(4, Qt::Horizontal, "Date d'installation");
    model->setHeaderData(5, Qt::Horizontal, "Type");

    return model;
}


// Modifier un equipement
bool EquipementDAO::modifier(const QString &id,
                             const QString &nom,
                             const QString &localisation,
                             const QString &statut,
                             const QDate &dateInstallation,
                             const QString &type)
{
    QSqlQuery query;

    query.prepare(
        "UPDATE EQUIPEMENTS SET "
        "NOM_EQUIPEMENT = :nom, "
        "LOCALISATION_EQUIPEMENT = :localisation, "
        "STATUT_EQUIPEMENT = :statut, "
        "DATE_INSTALLATION_EQUIPEMENT = :dateInstallation, "
        "TYPE_EQUIPEMENT = :type "
        "WHERE ID_EQUIPEMENT = :id"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation);
    query.bindValue(":type", type);

    if (!query.exec())
    {
        qDebug() << "Erreur modification équipement:"
                 << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}


// Supprimer un equipement
bool EquipementDAO::supprimer(const QString &id)
{
    QSqlQuery query;

    query.prepare(
        "DELETE FROM EQUIPEMENTS WHERE ID_EQUIPEMENT = :id"
        );

    query.bindValue(":id", id);

    if (!query.exec())
    {
        qDebug() << "Erreur suppression équipement:"
                 << query.lastError().text();
        return false;
    }

    return query.numRowsAffected() > 0;
}
