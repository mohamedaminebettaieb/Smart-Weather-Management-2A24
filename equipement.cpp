
#include "equipement.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include <QVariant>
#include <QMetaType>
#include <QDebug>
#include "connection.h"


// ID station vide -> NULL en base, sinon nombre entier
static QVariant valeurStation(const QString &idStation)
{
    const QString s = idStation.trimmed();

    if (s.isEmpty())
        return QVariant(QMetaType::fromType<int>());   // NULL

    return QVariant(s.toInt());
}


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
    idStation = "";
}


// Constructeur parametre
Equipement::Equipement(const QString &id,
                       const QString &nom,
                       const QString &localisation,
                       const QString &statut,
                       const QDate &dateInstallation,
                       const QString &type,
                       const QString &idStation)
{
    this->id = id;
    this->nom = nom;
    this->localisation = localisation;
    this->statut = statut;
    this->dateInstallation = dateInstallation;
    this->type = type;
    this->idStation = idStation;
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

QString Equipement::getIdStation() const
{
    return idStation;
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

void Equipement::setIdStation(const QString &idStation)
{
    this->idStation = idStation;
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
    qDebug() << "ID Station :" << idStation;
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
                            const QString &type,
                            const QString &idStation)
{
    qDebug() << "AJOUTER() CALLED";

    erreur = verifierLongueurs(id, nom, localisation, statut, type);
    if (!erreur.isEmpty())
        return false;

    erreur = verifierStation(idStation);
    if (!erreur.isEmpty())
        return false;

    QSqlQuery query;

    query.prepare(
        "INSERT INTO EQUIPEMENTS "
        "(ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT, ID_STATION) "
        "VALUES (:id, :nom, :localisation, :statut, "
        "TO_DATE(:dateInstallation, 'YYYY-MM-DD'), :type, :idStation)"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation.toString("yyyy-MM-dd"));
    query.bindValue(":type", type);
    query.bindValue(":idStation", valeurStation(idStation));

    if (!query.exec())
    {
        erreur = query.lastError().text();
        qDebug() << "Erreur ajout équipement:" << erreur;

        if (erreur.contains("ORA-02291"))
            erreur = QString::fromUtf8("Cette station n'existe pas dans la base.");

        return false;
    }

    qDebug() << "Equipement insere avec succes dans Oracle!";

    return true;
}


// ==========================================
// VERIFICATION DES LONGUEURS
// ==========================================

QString EquipementDAO::verifierLongueurs(const QString &id,
                                         const QString &nom,
                                         const QString &localisation,
                                         const QString &statut,
                                         const QString &type) const
{
    struct Champ {
        const char *colonne;
        const char *libelle;
        QString valeur;
    };

    const Champ champs[] = {
        { "ID_EQUIPEMENT",           "L'ID",            id },
        { "NOM_EQUIPEMENT",          "Le nom",          nom },
        { "LOCALISATION_EQUIPEMENT", "La localisation", localisation },
        { "STATUT_EQUIPEMENT",       "Le statut",       statut },
        { "TYPE_EQUIPEMENT",         "Le type",         type }
    };

    for (const Champ &c : champs)
    {
        const int max = Connection::longueurColonne("EQUIPEMENTS", c.colonne);

        if (max > 0 && c.valeur.size() > max)
        {
            return QString::fromUtf8("%1 ne doit pas dépasser %2 caractères (%3 saisis).")
                .arg(QString::fromUtf8(c.libelle))
                .arg(max)
                .arg(c.valeur.size());
        }
    }

    return QString();
}


// ==========================================
// VERIFICATION DE LA STATION
// ==========================================

QString EquipementDAO::verifierStation(const QString &idStation) const
{
    const QString s = idStation.trimmed();

    if (s.isEmpty())
        return QString();

    bool ok = false;
    const int valeur = s.toInt(&ok);

    if (!ok || valeur < 1)
        return QString::fromUtf8("La station choisie n'est pas valide.");

    return QString();
}


// ==========================================
// AFFICHER LES EQUIPEMENTS
// Colonnes :
// 0 ID, 1 Nom, 2 Localisation, 3 Statut,
// 4 Date, 5 Type, 6 ID Station
// ==========================================

QSqlQueryModel *EquipementDAO::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT, ID_STATION "
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
    model->setHeaderData(6, Qt::Horizontal, "ID Station");

    return model;
}


// ==========================================
// MODIFIER UN EQUIPEMENT
// ==========================================

bool EquipementDAO::modifier(const QString &id,
                             const QString &nom,
                             const QString &localisation,
                             const QString &statut,
                             const QDate &dateInstallation,
                             const QString &type,
                             const QString &idStation)
{
    erreur = verifierLongueurs(id, nom, localisation, statut, type);
    if (!erreur.isEmpty())
        return false;

    erreur = verifierStation(idStation);
    if (!erreur.isEmpty())
        return false;

    QSqlQuery query;

    query.prepare(
        "UPDATE EQUIPEMENTS SET "
        "NOM_EQUIPEMENT = :nom, "
        "LOCALISATION_EQUIPEMENT = :localisation, "
        "STATUT_EQUIPEMENT = :statut, "
        "DATE_INSTALLATION_EQUIPEMENT = TO_DATE(:dateInstallation, 'YYYY-MM-DD'), "
        "TYPE_EQUIPEMENT = :type, "
        "ID_STATION = :idStation "
        "WHERE ID_EQUIPEMENT = :id"
        );

    query.bindValue(":id", id);
    query.bindValue(":nom", nom);
    query.bindValue(":localisation", localisation);
    query.bindValue(":statut", statut);
    query.bindValue(":dateInstallation", dateInstallation.toString("yyyy-MM-dd"));
    query.bindValue(":type", type);
    query.bindValue(":idStation", valeurStation(idStation));

    if (!query.exec())
    {
        erreur = query.lastError().text();
        qDebug() << "Erreur modification équipement:" << erreur;

        if (erreur.contains("ORA-02291"))
            erreur = QString::fromUtf8("Cette station n'existe pas dans la base.");

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
// SUPPRIMER UN EQUIPEMENT
// ==========================================

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

        if (erreur.contains("ORA-02292"))
        {
            erreur = QString::fromUtf8(
                "Cet équipement est lié à des techniciens (table MAINTENIR) : "
                "retirez d'abord ces liens."
                );
        }

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
        "DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT, ID_STATION "
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
        "DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT, ID_STATION "
        "FROM EQUIPEMENTS ORDER BY " + colonneTri + " ASC";

    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery(sql);

    if (model->lastError().isValid())
    {
        qDebug() << "Erreur tri :" << model->lastError().text();
    }

    return model;
}


// ==========================================
// LISTE DES STATIONS (LECTURE SEULE)
// ==========================================

QSqlQueryModel *EquipementDAO::stations()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_STATION, NOM_STATION FROM STATIONS ORDER BY NOM_STATION"
        );

    if (model->lastError().isValid())
    {
        qDebug() << "Erreur lecture stations :" << model->lastError().text();
    }

    return model;
}


// ==========================================
// STATISTIQUES PAR STATUT (ORACLE)
// ==========================================

QSqlQueryModel *EquipementDAO::statistiquesParStatut()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT STATUT_EQUIPEMENT, COUNT(*) "
        "FROM EQUIPEMENTS "
        "GROUP BY STATUT_EQUIPEMENT "
        "ORDER BY STATUT_EQUIPEMENT"
        );

    if (model->lastError().isValid())
    {
        erreur = model->lastError().text();
        qDebug() << "Erreur statistiques Oracle :" << erreur;
    }
    else
    {
        erreur.clear();
    }

    return model;
}


// ==========================================
// DONNEES POUR EXPORTATION PDF (ORACLE)
// Colonnes :
// 0 ID, 1 Nom, 2 Localisation,
// 3 Date, 4 Statut, 5 Type
// ==========================================

QSqlQueryModel *EquipementDAO::donneesPDF()
{
    QSqlQueryModel *model = new QSqlQueryModel();

    model->setQuery(
        "SELECT ID_EQUIPEMENT, "
        "NOM_EQUIPEMENT, "
        "LOCALISATION_EQUIPEMENT, "
        "DATE_INSTALLATION_EQUIPEMENT, "
        "STATUT_EQUIPEMENT, "
        "TYPE_EQUIPEMENT "
        "FROM EQUIPEMENTS "
        "ORDER BY ID_EQUIPEMENT"
        );

    if (model->lastError().isValid())
    {
        erreur = model->lastError().text();
        qDebug() << "Erreur lecture PDF Oracle :" << erreur;
    }
    else
    {
        erreur.clear();
    }

    return model;
}
