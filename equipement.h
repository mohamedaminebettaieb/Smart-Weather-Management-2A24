
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

public:
    // Constructeur par defaut
    Equipement();

    // Constructeur parametre
    Equipement(const QString &id,
               const QString &nom,
               const QString &localisation,
               const QString &statut,
               const QDate &dateInstallation,
               const QString &type);

    // Getters
    QString getId() const;
    QString getNom() const;
    QString getLocalisation() const;
    QString getStatut() const;
    QDate getDateInstallation() const;
    QString getType() const;

    // Setters
    void setId(const QString &id);
    void setNom(const QString &nom);
    void setLocalisation(const QString &localisation);
    void setStatut(const QString &statut);
    void setDateInstallation(const QDate &dateInstallation);
    void setType(const QString &type);

    // Affichage
    void afficher() const;
};

class EquipementDAO
{
public:
    bool ajouter(const QString &id,
                 const QString &nom,
                 const QString &localisation,
                 const QString &statut,
                 const QDate &dateInstallation,
                 const QString &type);

    QSqlQueryModel *afficher();

    bool modifier(const QString &id,
                  const QString &nom,
                  const QString &localisation,
                  const QString &statut,
                  const QDate &dateInstallation,
                  const QString &type);

    bool supprimer(const QString &id);
};

#endif // EQUIPEMENT_H
