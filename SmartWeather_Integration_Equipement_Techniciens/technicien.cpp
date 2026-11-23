#include "technicien.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>
#include <QHash>
#include <algorithm>
#include "connection.h"

// ============================================================
// CONSTRUCTEURS
// ============================================================

Technicien::Technicien()
    : id(0), experience(0)
{
}

Technicien::Technicien(int id, const QString &nom, const QString &prenom,
                       const QString &telephone, const QString &email,
                       const QString &formation, int experience,
                       const QString &specialite, const QString &competences,
                       const QString &statut, const QDate &dateIntegration)
    : id(id), nom(nom), prenom(prenom), telephone(telephone), email(email),
    formation(formation), experience(experience), specialite(specialite),
    competences(competences), statut(statut), dateIntegration(dateIntegration)
{
}

// ============================================================
// GETTERS
// ============================================================

int      Technicien::getId() const              { return id; }
QString  Technicien::getNom() const             { return nom; }
QString  Technicien::getPrenom() const          { return prenom; }
QString  Technicien::getTelephone() const       { return telephone; }
QString  Technicien::getEmail() const           { return email; }
QString  Technicien::getFormation() const       { return formation; }
int      Technicien::getExperience() const      { return experience; }
QString  Technicien::getSpecialite() const      { return specialite; }
QString  Technicien::getCompetences() const     { return competences; }
QString  Technicien::getStatut() const          { return statut; }
QDate    Technicien::getDateIntegration() const { return dateIntegration; }
QString  Technicien::derniereErreur() const     { return erreur; }

// ============================================================
// SETTERS
// ============================================================

void Technicien::setId(int v)                      { id = v; }
void Technicien::setNom(const QString &v)          { nom = v; }
void Technicien::setPrenom(const QString &v)       { prenom = v; }
void Technicien::setTelephone(const QString &v)    { telephone = v; }
void Technicien::setEmail(const QString &v)        { email = v; }
void Technicien::setFormation(const QString &v)    { formation = v; }
void Technicien::setExperience(int v)              { experience = v; }
void Technicien::setSpecialite(const QString &v)   { specialite = v; }
void Technicien::setCompetences(const QString &v)  { competences = v; }
void Technicien::setStatut(const QString &v)       { statut = v; }
void Technicien::setDateIntegration(const QDate &v){ dateIntegration = v; }

// ============================================================
// CONTROLE DE SAISIE (basé sur la structure réelle de la table Oracle)
// ============================================================

int Technicien::longueurMax(const QString &colonne)
{
    return Connection::longueurColonne("TECHNICIENS", colonne);
}

bool Technicien::existe(int id)
{
    QSqlQuery q;
    q.prepare("SELECT COUNT(*) FROM TECHNICIENS WHERE ID_TECHNICIEN = :id");
    q.bindValue(":id", QString::number(id));   // ID_TECHNICIEN est VARCHAR2
    return q.exec() && q.next() && q.value(0).toInt() > 0;
}

QString Technicien::valider() const
{
    struct Champ { const char *colonne; const char *libelle; QString valeur; };
    const Champ champs[] = {
        { "NOM_TECHNICIEN",    "Le nom",          nom },
        { "PRENOM_TECHNICIEN", "Le prénom",       prenom },
        { "TEL_TECHNICIEN",    "Le téléphone",    telephone },
        { "EMAIL_TECHNICIEN",  "L'email",         email },
        { "FORMATION",         "La formation",    formation },
        { "SPECIALITE",        "La spécialité",   specialite },
        { "COMPETENCES",       "Les compétences", competences },
        { "STATUT_TECHNICIEN", "Le statut",       statut }
    };

    for (const Champ &c : champs)
    {
        const int max = longueurMax(c.colonne);
        const int octets = c.valeur.toUtf8().size();
        if (max > 0 && octets > max)
            return QString::fromUtf8("%1 dépasse la limite Oracle de %2 octets (%3 octets saisis).")
                .arg(QString::fromUtf8(c.libelle)).arg(max).arg(octets);
    }
    return QString();
}


// ============================================================
// CRUD
// ============================================================

bool Technicien::ajouter()
{
    erreur = valider();
    if (!erreur.isEmpty())
        return false;

    QSqlQuery q;
    q.prepare("INSERT INTO TECHNICIENS (ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, "
              "TEL_TECHNICIEN, EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, SPECIALITE, "
              "COMPETENCES, STATUT_TECHNICIEN, DATE_INTEGRATION) "
              "VALUES (:id, :nom, :prenom, :tel, :email, :formation, :exp, :spec, "
              ":comp, :statut, TO_DATE(:dt, 'YYYY-MM-DD'))");
    q.bindValue(":id", QString::number(id));
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", telephone.toInt());
    q.bindValue(":email", email);
    q.bindValue(":formation", formation);
    q.bindValue(":exp", experience);
    q.bindValue(":spec", specialite);
    q.bindValue(":comp", competences);
    q.bindValue(":statut", statut);
    q.bindValue(":dt", dateIntegration.toString("yyyy-MM-dd"));

    if (!q.exec())
    {
        erreur = q.lastError().text();
        if (erreur.contains("ORA-00001"))
            erreur = QString::fromUtf8("Cet ID existe déjà dans la base. Choisissez-en un autre.");
        return false;
    }
    return true;
}

bool Technicien::modifier(int ancienId)
{
    erreur = valider();
    if (!erreur.isEmpty())
        return false;

    QSqlQuery q;
    q.prepare("UPDATE TECHNICIENS SET ID_TECHNICIEN = :id, NOM_TECHNICIEN = :nom, "
              "PRENOM_TECHNICIEN = :prenom, TEL_TECHNICIEN = :tel, "
              "EMAIL_TECHNICIEN = :email, FORMATION = :formation, EXPERIENCE = :exp, "
              "SPECIALITE = :spec, COMPETENCES = :comp, STATUT_TECHNICIEN = :statut, "
              "DATE_INTEGRATION = TO_DATE(:dt, 'YYYY-MM-DD') "
              "WHERE ID_TECHNICIEN = :ancien");
    q.bindValue(":id", QString::number(id));
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", telephone.toInt());
    q.bindValue(":email", email);
    q.bindValue(":formation", formation);
    q.bindValue(":exp", experience);
    q.bindValue(":spec", specialite);
    q.bindValue(":comp", competences);
    q.bindValue(":statut", statut);
    q.bindValue(":dt", dateIntegration.toString("yyyy-MM-dd"));
    q.bindValue(":ancien", QString::number(ancienId));

    if (!q.exec())
    {
        erreur = q.lastError().text();
        if (erreur.contains("ORA-00001"))
            erreur = QString::fromUtf8("Cet ID existe déjà dans la base. Choisissez-en un autre.");
        if (erreur.contains("ORA-02292"))
            erreur = QString::fromUtf8("Ce technicien est lié à des équipements (table MAINTENIR) : "
                                       "retirez d'abord ces liens.");
        return false;
    }
    if (q.numRowsAffected() <= 0)
    {
        erreur = QString::fromUtf8("Aucun technicien avec l'ID %1 dans la base.").arg(ancienId);
        return false;
    }
    return true;
}

bool Technicien::supprimer(int idASupprimer)
{
    erreur.clear();

    QSqlQuery q;
    q.prepare("DELETE FROM TECHNICIENS WHERE ID_TECHNICIEN = :id");
    q.bindValue(":id", QString::number(idASupprimer));

    if (!q.exec())
    {
        erreur = q.lastError().text();
        if (erreur.contains("ORA-02292"))
            erreur = QString::fromUtf8("Ce technicien est lié à des équipements (table MAINTENIR) : "
                                       "retirez d'abord ces liens.");
        return false;
    }
    if (q.numRowsAffected() <= 0)
    {
        erreur = QString::fromUtf8("Aucun technicien avec l'ID %1 dans la base.").arg(idASupprimer);
        return false;
    }
    return true;
}

QSqlQueryModel *Technicien::afficher()
{
    QSqlQueryModel *model = new QSqlQueryModel();
    model->setQuery("SELECT ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, "
                    "TEL_TECHNICIEN, EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, "
                    "SPECIALITE, COMPETENCES, STATUT_TECHNICIEN, DATE_INTEGRATION "
                    "FROM TECHNICIENS ORDER BY LENGTH(ID_TECHNICIEN), ID_TECHNICIEN");
    return model;
}

QList<Technicien> Technicien::lister()
{
    erreur.clear();
    QList<Technicien> liste;

    QSqlQuery q;
    if (!q.exec("SELECT ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, TEL_TECHNICIEN, "
                "EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, SPECIALITE, COMPETENCES, "
                "STATUT_TECHNICIEN, DATE_INTEGRATION FROM TECHNICIENS ORDER BY LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"))
    {
        erreur = q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        liste.append(Technicien(q.value(0).toInt(),
                                q.value(1).toString(),
                                q.value(2).toString(),
                                q.value(3).toString().rightJustified(8, '0'),
                                q.value(4).toString(),
                                q.value(5).toString(),
                                q.value(6).toInt(),
                                q.value(7).toString(),
                                q.value(8).toString(),
                                q.value(9).toString().trimmed(),
                                q.value(10).toDate()));
    }
    return liste;
}

QList<Technicien> Technicien::rechercherTrier(const QString &texte, int tri)
{
    erreur.clear();
    QList<Technicien> liste;

    QString sql = "SELECT ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, TEL_TECHNICIEN, "
                  "EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, SPECIALITE, COMPETENCES, "
                  "STATUT_TECHNICIEN, DATE_INTEGRATION FROM TECHNICIENS ";

    const QString t = texte.trimmed();
    if (!t.isEmpty())
        sql += "WHERE UPPER(ID_TECHNICIEN) LIKE UPPER(:p1) "
               "OR UPPER(NOM_TECHNICIEN) LIKE UPPER(:p2) "
               "OR UPPER(PRENOM_TECHNICIEN) LIKE UPPER(:p3) "
               "OR UPPER(EMAIL_TECHNICIEN) LIKE UPPER(:p4) "
               "OR TO_CHAR(TEL_TECHNICIEN) LIKE :p5 "
               "OR UPPER(FORMATION) LIKE UPPER(:p6) "
               "OR TO_CHAR(EXPERIENCE) LIKE :p7 "
               "OR UPPER(SPECIALITE) LIKE UPPER(:p8) "
               "OR UPPER(COMPETENCES) LIKE UPPER(:p9) "
               "OR UPPER(STATUT_TECHNICIEN) LIKE UPPER(:p10) "
               "OR TO_CHAR(DATE_INTEGRATION, 'DD/MM/YYYY') LIKE :p11 ";

    // ORDER BY choisi selon le critere (liste blanche : jamais de texte saisi)
    switch (tri)
    {
    case 1:  sql += "ORDER BY UPPER(NOM_TECHNICIEN), LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    case 2:  sql += "ORDER BY UPPER(PRENOM_TECHNICIEN), LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    case 3:  sql += "ORDER BY EXPERIENCE, LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    case 4:  sql += "ORDER BY STATUT_TECHNICIEN, LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    case 5:  sql += "ORDER BY DATE_INTEGRATION, LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    default: sql += "ORDER BY LENGTH(ID_TECHNICIEN), ID_TECHNICIEN"; break;
    }

    QSqlQuery q;
    q.prepare(sql);
    if (!t.isEmpty())
    {
        const QString motif = "%" + t + "%";
        for (int i = 1; i <= 11; ++i)
            q.bindValue(":p" + QString::number(i), motif);
    }

    if (!q.exec())
    {
        erreur = q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        liste.append(Technicien(q.value(0).toInt(),
                                q.value(1).toString(),
                                q.value(2).toString(),
                                q.value(3).toString().rightJustified(8, '0'),
                                q.value(4).toString(),
                                q.value(5).toString(),
                                q.value(6).toInt(),
                                q.value(7).toString(),
                                q.value(8).toString(),
                                q.value(9).toString().trimmed(),
                                q.value(10).toDate()));
    }
    return liste;
}

void Technicien::compterParStatut(int &actifs, int &inactifs)
{
    actifs = 0;
    inactifs = 0;

    QSqlQuery q;
    if (!q.exec("SELECT STATUT_TECHNICIEN, COUNT(*) FROM TECHNICIENS GROUP BY STATUT_TECHNICIEN"))
        return;

    while (q.next())
    {
        if (q.value(0).toString().trimmed() == "Actif")
            actifs += q.value(1).toInt();
        else
            inactifs += q.value(1).toInt();
    }
}

int Technicien::prochainId()
{
    int max = 0;
    QSqlQuery q;
    if (q.exec("SELECT ID_TECHNICIEN FROM TECHNICIENS"))
    {
        while (q.next())
        {
            bool ok = false;
            const int v = q.value(0).toString().trimmed().toInt(&ok);
            if (ok && v > max)
                max = v;
        }
    }
    return max + 1;
}

QString Technicien::doublonExistant(const QString &email, const QString &nom, const QString &prenom)
{
    if (!email.trimmed().isEmpty())
    {
        QSqlQuery q;
        q.prepare("SELECT ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN FROM TECHNICIENS "
                  "WHERE UPPER(EMAIL_TECHNICIEN) = UPPER(:email)");
        q.bindValue(":email", email.trimmed());
        if (q.exec() && q.next())
            return QString::fromUtf8("Un technicien avec cet email existe déjà : #%1 %2 %3.")
                .arg(q.value(0).toString(), q.value(2).toString(), q.value(1).toString());
    }
    if (!nom.trimmed().isEmpty() && !prenom.trimmed().isEmpty())
    {
        QSqlQuery q;
        q.prepare("SELECT ID_TECHNICIEN FROM TECHNICIENS "
                  "WHERE UPPER(NOM_TECHNICIEN) = UPPER(:nom) AND UPPER(PRENOM_TECHNICIEN) = UPPER(:prenom)");
        q.bindValue(":nom", nom.trimmed());
        q.bindValue(":prenom", prenom.trimmed());
        if (q.exec() && q.next())
            return QString::fromUtf8("Un technicien nommé %1 %2 existe déjà (ID #%3).")
                .arg(prenom.trimmed(), nom.trimmed(), q.value(0).toString());
    }
    return QString();
}
