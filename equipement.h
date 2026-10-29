#ifndef EQUIPEMENT_H
#define EQUIPEMENT_H

#include <QString>
#include <QDate>
#include <QSqlQueryModel>

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