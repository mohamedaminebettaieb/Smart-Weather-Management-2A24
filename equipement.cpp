
#include "equipement.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QDebug>
#include "connection.h"

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

    erreur = verifierLongueurs(id, nom, localisation, statut, type);
    if (!erreur.isEmpty())
        return false;

    QSqlQuery query;

    query.prepare(
        "INSERT INTO EQUIPEMENTS "
        "(ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT) "
        "VALUES (:id, :nom, :localisation, :statut, "
        "TO_DATE(:dateInstallation, 'YYYY-MM-DD'), :type)"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation.toString("yyyy-MM-dd"));
    query.bindValue(":type", type);

    if (!query.exec())
    {
        erreur = query.lastError().text();
        qDebug() << "Erreur ajout équipement:" << erreur;
        return false;
    }

    qDebug() << "Equipement insere avec succes dans Oracle!";

    return true;
}


// Contrôle des longueurs par rapport aux colonnes réelles d'Oracle
QString EquipementDAO::verifierLongueurs(const QString &id, const QString &nom,
                                         const QString &localisation, const QString &statut,
                                         const QString &type) const
{
    struct Champ { const char *colonne; const char *libelle; QString valeur; };
    const Champ champs[] = {
        { "ID_EQUIPEMENT",           "L'ID",           id },
        { "NOM_EQUIPEMENT",          "Le nom",         nom },
        { "LOCALISATION_EQUIPEMENT", "La localisation", localisation },
        { "STATUT_EQUIPEMENT",       "Le statut",      statut },
        { "TYPE_EQUIPEMENT",         "Le type",        type }
    };
    for (const Champ &c : champs)
    {
        const int max = Connection::longueurColonne("EQUIPEMENTS", c.colonne);
        if (max > 0 && c.valeur.size() > max)
            return QString::fromUtf8("%1 ne doit pas dépasser %2 caractères (%3 saisis).")
                .arg(QString::fromUtf8(c.libelle)).arg(max).arg(c.valeur.size());
    }
    return QString();
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
    erreur = verifierLongueurs(id, nom, localisation, statut, type);
    if (!erreur.isEmpty())
        return false;

    QSqlQuery query;

    query.prepare(
        "UPDATE EQUIPEMENTS SET "
        "NOM_EQUIPEMENT = :nom, "
        "LOCALISATION_EQUIPEMENT = :localisation, "
        "STATUT_EQUIPEMENT = :statut, "
        "DATE_INSTALLATION_EQUIPEMENT = TO_DATE(:dateInstallation, 'YYYY-MM-DD'), "
        "TYPE_EQUIPEMENT = :type "
        "WHERE ID_EQUIPEMENT = :id"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation.toString("yyyy-MM-dd"));
    query.bindValue(":type", type);

    if (!query.exec())
    {
        erreur = query.lastError().text();
        qDebug() << "Erreur modification équipement:" << erreur;
        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        erreur = QString::fromUtf8("Aucun équipement avec l'ID %1 dans la base.").arg(id);
        return false;
    }
    return true;
}

// Supprimer un equipement
bool EquipementDAO::supprimer(const QString &id)
{
    erreur.clear();
    QSqlQuery query;

    query.prepare(
        "DELETE FROM EQUIPEMENTS WHERE ID_EQUIPEMENT = :id"
        );

    query.bindValue(":id", id);

    if (!query.exec())
    {
        erreur = query.lastError().text();
        qDebug() << "Erreur suppression équipement:" << erreur;
        if (erreur.contains("ORA-02292"))   // clé étrangère MAINTENIR -> EQUIPEMENTS
            erreur = QString::fromUtf8("Cet équipement est lié à des techniciens (table MAINTENIR) : "
                                       "retirez d'abord ces liens.");
        return false;
    }

    if (query.numRowsAffected() <= 0)
    {
        erreur = QString::fromUtf8("Aucun équipement avec l'ID %1 dans la base.").arg(id);
        return false;
    }
    return true;
}


// ==========================================
// RECHERCHER UN EQUIPEMENT
// ==========================================

QSqlQueryModel *EquipementDAO::rechercher(const QString &texte)
{
    QSqlQuery query;

    query.prepare(
        "SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, "
        "LOCALISATION_EQUIPEMENT, STATUT_EQUIPEMENT, "
        "DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT "
        "FROM EQUIPEMENTS "
        "WHERE TO_CHAR(ID_EQUIPEMENT) LIKE :id "
        "OR UPPER(NOM_EQUIPEMENT) LIKE UPPER(:nom) "
        "OR UPPER(LOCALISATION_EQUIPEMENT) LIKE UPPER(:localisation) "
        "OR UPPER(STATUT_EQUIPEMENT) LIKE UPPER(:statut) "
        "OR TO_CHAR(DATE_INSTALLATION_EQUIPEMENT, 'DD/MM/YYYY') LIKE :date "
        "OR UPPER(TYPE_EQUIPEMENT) LIKE UPPER(:type)"
        );

    QString recherche = "%" + texte.trimmed() + "%";

    query.bindValue(":id", recherche);
    query.bindValue(":nom", recherche);
    query.bindValue(":localisation", recherche);
    query.bindValue(":statut", recherche);
    query.bindValue(":date", recherche);
    query.bindValue(":type", recherche);

    if (!query.exec())
    {
        qDebug() << "Erreur recherche :" << query.lastError().text();
    }

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(query);

    return model;
}


// ==========================================
// TRIER LES EQUIPEMENTS
// ==========================================

QSqlQueryModel *EquipementDAO::trier(const QString &champTri)
{
    QString colonneTri;

    if (champTri == "Nom")
        colonneTri = "NOM_EQUIPEMENT";
    else if (champTri == "Localisation")
        colonneTri = "LOCALISATION_EQUIPEMENT";
    else if (champTri == "Statut")
        colonneTri = "STATUT_EQUIPEMENT";
    else if (champTri == "Type")
        colonneTri = "TYPE_EQUIPEMENT";
    else
        colonneTri = "ID_EQUIPEMENT";

    QString sql =
        "SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, "
        "LOCALISATION_EQUIPEMENT, STATUT_EQUIPEMENT, "
        "DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT "
        "FROM EQUIPEMENTS ORDER BY " + colonneTri + " ASC";

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(sql);

    if (model->lastError().isValid())
    {
        qDebug() << "Erreur tri :" << model->lastError().text();
    }

    return model;
}