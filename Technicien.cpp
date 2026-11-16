#include "technicien.h"

#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

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
// CRUD
// ============================================================

bool Technicien::ajouter()
{
    erreur.clear();

    QSqlQuery q;
    q.prepare("INSERT INTO TECHNICIENS (ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, "
              "TEL_TECHNICIEN, EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, SPECIALITE, "
              "COMPETENCES, STATUT_TECHNICIEN, DATE_INTEGRATION) "
              "VALUES (:id, :nom, :prenom, :tel, :email, :formation, :exp, :spec, "
              ":comp, :statut, TO_DATE(:dt, 'YYYY-MM-DD'))");
    q.bindValue(":id", id);
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", telephone);
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
        return false;
    }
    return true;
}

bool Technicien::modifier(int ancienId)
{
    erreur.clear();

    QSqlQuery q;
    q.prepare("UPDATE TECHNICIENS SET ID_TECHNICIEN = :id, NOM_TECHNICIEN = :nom, "
              "PRENOM_TECHNICIEN = :prenom, TEL_TECHNICIEN = :tel, "
              "EMAIL_TECHNICIEN = :email, FORMATION = :formation, EXPERIENCE = :exp, "
              "SPECIALITE = :spec, COMPETENCES = :comp, STATUT_TECHNICIEN = :statut, "
              "DATE_INTEGRATION = TO_DATE(:dt, 'YYYY-MM-DD') "
              "WHERE ID_TECHNICIEN = :ancien");
    q.bindValue(":id", id);
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", telephone);
    q.bindValue(":email", email);
    q.bindValue(":formation", formation);
    q.bindValue(":exp", experience);
    q.bindValue(":spec", specialite);
    q.bindValue(":comp", competences);
    q.bindValue(":statut", statut);
    q.bindValue(":dt", dateIntegration.toString("yyyy-MM-dd"));
    q.bindValue(":ancien", ancienId);

    if (!q.exec())
    {
        erreur = q.lastError().text();
        return false;
    }
    return true;
}

bool Technicien::supprimer(int idASupprimer)
{
    erreur.clear();

    QSqlQuery q;
    q.prepare("DELETE FROM TECHNICIENS WHERE ID_TECHNICIEN = :id");
    q.bindValue(":id", idASupprimer);

    if (!q.exec())
    {
        erreur = q.lastError().text();
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
                    "FROM TECHNICIENS ORDER BY ID_TECHNICIEN");
    return model;
}

QList<Technicien> Technicien::lister()
{
    erreur.clear();
    QList<Technicien> liste;

    QSqlQuery q;
    if (!q.exec("SELECT ID_TECHNICIEN, NOM_TECHNICIEN, PRENOM_TECHNICIEN, TEL_TECHNICIEN, "
                "EMAIL_TECHNICIEN, FORMATION, EXPERIENCE, SPECIALITE, COMPETENCES, "
                "STATUT_TECHNICIEN, DATE_INTEGRATION FROM TECHNICIENS ORDER BY ID_TECHNICIEN"))
    {
        erreur = q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        liste.append(Technicien(q.value(0).toInt(),
                                q.value(1).toString(),
                                q.value(2).toString(),
                                q.value(3).toString(),
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