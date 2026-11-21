
#ifndef EQUIPEMENT_H
#define EQUIPEMENT_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

class Equipement
{
private:
    QString id;
    QString nom;
    QString localisation;
    QString statut;
    QDate dateInstallation;
    QString type;
    QString idStation;      // lien vers la table STATIONS (vide = aucune station)

public:
    // Constructeur par defaut
    Equipement();

    // Constructeur parametre
    Equipement(const QString &id,
               const QString &nom,
               const QString &localisation,
               const QString &statut,
               const QDate &dateInstallation,
               const QString &type,
               const QString &idStation = QString());

    // Getters
    QString getId() const;
    QString getNom() const;
    QString getLocalisation() const;
    QString getStatut() const;
    QDate getDateInstallation() const;
    QString getType() const;
    QString getIdStation() const;

    // Setters
    void setId(const QString &id);
    void setNom(const QString &nom);
    void setLocalisation(const QString &localisation);
    void setStatut(const QString &statut);
    void setDateInstallation(const QDate &dateInstallation);
    void setType(const QString &type);
    void setIdStation(const QString &idStation);

    // Affichage
    void afficher() const;
};

class EquipementDAO
{
public:
    // Dernier message d'erreur
    QString derniereErreur() const { return erreur; }

    // Ajouter
    bool ajouter(const QString &id,
                 const QString &nom,
                 const QString &localisation,
                 const QString &statut,
                 const QDate &dateInstallation,
                 const QString &type,
                 const QString &idStation = QString());

    // Afficher
    QSqlQueryModel *afficher();

    // Modifier
    bool modifier(const QString &id,
                  const QString &nom,
                  const QString &localisation,
                  const QString &statut,
                  const QDate &dateInstallation,
                  const QString &type,
                  const QString &idStation = QString());

    // Supprimer
    bool supprimer(const QString &id);

    // Recherche
    QSqlQueryModel *rechercher(const QString &texte);

    // Tri
    QSqlQueryModel *trier(const QString &champTri);

    // Liste des stations
    // colonne 0 = ID_STATION, colonne 1 = NOM_STATION
    QSqlQueryModel *stations();

    // Statistiques Oracle par statut
    QSqlQueryModel *statistiquesParStatut();

    // Donnees Oracle pour l'exportation PDF
    QSqlQueryModel *donneesPDF();

private:
    QString erreur;

    // Vérification des longueurs des colonnes
    QString verifierLongueurs(const QString &id, const QString &nom,
                              const QString &localisation, const QString &statut,
                              const QString &type) const;

    // Vérification de l'ID station
    QString verifierStation(const QString &idStation) const;
};

#endif // EQUIPEMENT_H
