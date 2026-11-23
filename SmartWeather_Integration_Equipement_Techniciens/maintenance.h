#ifndef MAINTENANCE_H
#define MAINTENANCE_H

#include <QList>
#include <QString>

/*
 * Métier avancé 1 : gestion intelligente de la maintenance des équipements.
 *
 * Tables Oracle utilisées (structure réelle de la base) :
 *   EQUIPEMENTS(ID_EQUIPEMENT, NOM_EQUIPEMENT, LOCALISATION_EQUIPEMENT,
 *               STATUT_EQUIPEMENT, DATE_INSTALLATION_EQUIPEMENT, TYPE_EQUIPEMENT)
 *   TECHNICIENS(ID_TECHNICIEN, ...)
 *   MAINTENIR(ID_EQUIPEMENT, ID_TECHNICIEN)   -- clé primaire composée,
 *             clés étrangères vers EQUIPEMENTS et TECHNICIENS
 *
 * Valeurs de STATUT_EQUIPEMENT : "Actif", "Inactif", "En maintenance".
 */

struct EquipementMaintenance
{
    QString id;
    QString nom;
    QString type;
    QString localisation;
    QString statut;
};

class MaintenanceDAO
{
public:
    static const QString STATUT_MAINTENANCE;   // "En maintenance"
    static const QString STATUT_ACTIF;         // "Actif"

    // Équipements dont le statut (lu dans Oracle) est "En maintenance"
    static QList<EquipementMaintenance> equipementsEnMaintenance(QString *erreur = nullptr);

    // Tous les équipements (pour pouvoir choisir lequel signaler / tester)
    static QList<EquipementMaintenance> tousLesEquipements(QString *erreur = nullptr);

    // Passe un équipement « Actif » à « En maintenance » (UPDATE dans EQUIPEMENTS).
    // Refusé si l'équipement n'est pas « Actif ».
    static bool signalerMaintenance(const QString &idEquipement, QString &message);

    // Statut réel d'un équipement dans Oracle ("" s'il n'existe pas)
    static QString statutReel(const QString &idEquipement, QString *erreur = nullptr);

    // Vrai si la relation MAINTENIR (équipement, technicien) existe déjà
    static bool associationExiste(const QString &idEquipement, int idTechnicien);

    // Prise en charge, en UNE transaction SQL :
    //   1. verrouille l'équipement et relit son statut réel (doit être "En maintenance")
    //   2. vérifie que le technicien et l'équipement existent
    //   3. insère la relation dans MAINTENIR (sans doublon)
    //   4. passe le statut de l'équipement à "Actif"
    // En cas d'erreur : ROLLBACK, la base reste inchangée et message explique pourquoi.
    // Retourne true si tout a réussi (message = information à afficher).
    static bool prendreEnCharge(const QString &idEquipement, int idTechnicien, QString &message);
};

#endif // MAINTENANCE_H
