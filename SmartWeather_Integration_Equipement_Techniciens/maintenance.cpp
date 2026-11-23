#include "maintenance.h"

#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QVariant>

const QString MaintenanceDAO::STATUT_MAINTENANCE = QStringLiteral("En maintenance");
const QString MaintenanceDAO::STATUT_ACTIF = QStringLiteral("Actif");

QList<EquipementMaintenance> MaintenanceDAO::equipementsEnMaintenance(QString *erreur)
{
    QList<EquipementMaintenance> liste;

    QSqlQuery q;
    q.prepare("SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, TYPE_EQUIPEMENT, "
              "LOCALISATION_EQUIPEMENT, STATUT_EQUIPEMENT "
              "FROM EQUIPEMENTS "
              "WHERE UPPER(TRIM(STATUT_EQUIPEMENT)) = UPPER(:statut) "
              "ORDER BY LENGTH(ID_EQUIPEMENT), ID_EQUIPEMENT");
    q.bindValue(":statut", STATUT_MAINTENANCE);

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        qDebug() << "equipementsEnMaintenance :" << q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        EquipementMaintenance e;
        e.id = q.value(0).toString();
        e.nom = q.value(1).toString();
        e.type = q.value(2).toString();
        e.localisation = q.value(3).toString();
        e.statut = q.value(4).toString().trimmed();
        liste.append(e);
    }
    return liste;
}

QList<EquipementMaintenance> MaintenanceDAO::tousLesEquipements(QString *erreur)
{
    QList<EquipementMaintenance> liste;

    QSqlQuery q;
    q.prepare("SELECT ID_EQUIPEMENT, NOM_EQUIPEMENT, TYPE_EQUIPEMENT, "
              "LOCALISATION_EQUIPEMENT, STATUT_EQUIPEMENT "
              "FROM EQUIPEMENTS ORDER BY LENGTH(ID_EQUIPEMENT), ID_EQUIPEMENT");

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        EquipementMaintenance e;
        e.id = q.value(0).toString();
        e.nom = q.value(1).toString();
        e.type = q.value(2).toString();
        e.localisation = q.value(3).toString();
        e.statut = q.value(4).toString().trimmed();
        liste.append(e);
    }
    return liste;
}

bool MaintenanceDAO::signalerMaintenance(const QString &idEquipement, QString &message)
{
    message.clear();

    QSqlQuery q;
    q.prepare("UPDATE EQUIPEMENTS SET STATUT_EQUIPEMENT = :nouveau "
              "WHERE ID_EQUIPEMENT = :id AND UPPER(TRIM(STATUT_EQUIPEMENT)) = UPPER(:ancien)");
    q.bindValue(":nouveau", STATUT_MAINTENANCE);
    q.bindValue(":id", idEquipement);
    q.bindValue(":ancien", STATUT_ACTIF);

    if (!q.exec())
    {
        message = QString::fromUtf8("Mise à jour impossible :\n") + q.lastError().text();
        return false;
    }
    if (q.numRowsAffected() != 1)
    {
        message = QString::fromUtf8("Seul un équipement « Actif » peut être passé en maintenance.");
        return false;
    }

    message = QString::fromUtf8("L'équipement %1 est maintenant « En maintenance ».").arg(idEquipement);
    return true;
}

QString MaintenanceDAO::statutReel(const QString &idEquipement, QString *erreur)
{
    QSqlQuery q;
    q.prepare("SELECT STATUT_EQUIPEMENT FROM EQUIPEMENTS WHERE ID_EQUIPEMENT = :id");
    q.bindValue(":id", idEquipement);

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        return QString();
    }
    if (q.next())
        return q.value(0).toString().trimmed();
    return QString();
}

bool MaintenanceDAO::associationExiste(const QString &idEquipement, int idTechnicien)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM MAINTENIR "
              "WHERE ID_EQUIPEMENT = :eq AND ID_TECHNICIEN = :tech");
    q.bindValue(":eq", idEquipement);
    q.bindValue(":tech", QString::number(idTechnicien));
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

bool MaintenanceDAO::prendreEnCharge(const QString &idEquipement, int idTechnicien, QString &message)
{
    message.clear();

    QSqlDatabase db = QSqlDatabase::database();   // connexion Oracle ouverte par Connection
    if (!db.isOpen())
    {
        message = QString::fromUtf8("La connexion à la base Oracle n'est pas ouverte.");
        return false;
    }

    const QString idTech = QString::number(idTechnicien);

    if (!db.transaction())
    {
        message = QString::fromUtf8("Impossible de démarrer la transaction : ") + db.lastError().text();
        return false;
    }

    // Toute erreur : ROLLBACK + message
    auto echec = [&](const QString &texte, const QSqlError &err = QSqlError()) -> bool
    {
        message = texte;
        if (err.isValid() && !err.text().trimmed().isEmpty())
            message += "\n" + err.text();
        db.rollback();
        return false;
    };

    // 1) l'équipement existe ? statut réel (ligne verrouillée jusqu'au COMMIT / ROLLBACK)
    QSqlQuery q(db);
    q.prepare("SELECT STATUT_EQUIPEMENT FROM EQUIPEMENTS WHERE ID_EQUIPEMENT = :id FOR UPDATE");
    q.bindValue(":id", idEquipement);
    if (!q.exec())
        return echec(QString::fromUtf8("Lecture de l'équipement impossible."), q.lastError());
    if (!q.next())
        return echec(QString::fromUtf8("Cet équipement n'existe plus dans la base de données."));

    const QString statut = q.value(0).toString().trimmed();

    // Contrôle métier réalisé dans le code (pas seulement dans l'interface)
    if (statut.compare(STATUT_MAINTENANCE, Qt::CaseInsensitive) != 0)
        return echec(QString::fromUtf8("Cet équipement n'est pas actuellement en maintenance. "
                                       "Vous ne pouvez pas le prendre en charge."));

    // 2) le technicien existe ?
    QSqlQuery qt(db);
    qt.prepare("SELECT COUNT(*) FROM TECHNICIENS WHERE ID_TECHNICIEN = :id");
    qt.bindValue(":id", idTech);
    if (!qt.exec() || !qt.next())
        return echec(QString::fromUtf8("Lecture du technicien impossible."), qt.lastError());
    if (qt.value(0).toInt() == 0)
        return echec(QString::fromUtf8("Ce technicien n'existe plus dans la base de données."));

    // 3) relation MAINTENIR : jamais de doublon
    QSqlQuery qa(db);
    qa.prepare("SELECT COUNT(*) FROM MAINTENIR WHERE ID_EQUIPEMENT = :eq AND ID_TECHNICIEN = :tech");
    qa.bindValue(":eq", idEquipement);
    qa.bindValue(":tech", idTech);
    if (!qa.exec() || !qa.next())
        return echec(QString::fromUtf8("Vérification de la relation MAINTENIR impossible."), qa.lastError());

    const bool dejaAssocie = qa.value(0).toInt() > 0;

    if (!dejaAssocie)
    {
        QSqlQuery qi(db);
        qi.prepare("INSERT INTO MAINTENIR (ID_EQUIPEMENT, ID_TECHNICIEN) VALUES (:eq, :tech)");
        qi.bindValue(":eq", idEquipement);
        qi.bindValue(":tech", idTech);
        if (!qi.exec())
            return echec(QString::fromUtf8("Enregistrement de la relation MAINTENIR impossible."), qi.lastError());
    }

    // 4) l'équipement redevient "Actif" (même transaction)
    QSqlQuery qu(db);
    qu.prepare("UPDATE EQUIPEMENTS SET STATUT_EQUIPEMENT = :nouveau "
               "WHERE ID_EQUIPEMENT = :id AND UPPER(TRIM(STATUT_EQUIPEMENT)) = UPPER(:ancien)");
    qu.bindValue(":nouveau", STATUT_ACTIF);
    qu.bindValue(":id", idEquipement);
    qu.bindValue(":ancien", STATUT_MAINTENANCE);
    if (!qu.exec())
        return echec(QString::fromUtf8("Mise à jour du statut de l'équipement impossible."), qu.lastError());
    if (qu.numRowsAffected() != 1)
        return echec(QString::fromUtf8("Le statut de l'équipement n'a pas pu être modifié "
                                       "(il a peut-être été changé par un autre utilisateur)."));

    if (!db.commit())
        return echec(QString::fromUtf8("Validation de la transaction impossible."), db.lastError());

    message = dejaAssocie
        ? QString::fromUtf8("Ce technicien était déjà associé à cet équipement (aucun doublon créé dans MAINTENIR).\n"
                            "L'équipement est de nouveau « Actif ».")
        : QString::fromUtf8("Prise en charge enregistrée dans MAINTENIR.\nL'équipement est de nouveau « Actif ».");
    return true;
}
