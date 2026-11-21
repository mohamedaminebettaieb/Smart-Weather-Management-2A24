
#include "utilisateur.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QVariant>

Utilisateur::Utilisateur()
    : id(0)
{
}

int Utilisateur::getId() const { return id; }
QString Utilisateur::getNom() const { return nom; }
QString Utilisateur::getPrenom() const { return prenom; }
QString Utilisateur::getTel() const { return tel; }
QString Utilisateur::getRole() const { return role; }
QString Utilisateur::getMdp() const { return mdp; }
QString Utilisateur::getEmail() const { return email; }

void Utilisateur::setId(int nouvelId) { id = nouvelId; }
void Utilisateur::setNom(const QString &nouveauNom) { nom = nouveauNom; }
void Utilisateur::setPrenom(const QString &nouveauPrenom) { prenom = nouveauPrenom; }
void Utilisateur::setTel(const QString &nouveauTel) { tel = nouveauTel; }
void Utilisateur::setRole(const QString &nouveauRole) { role = nouveauRole; }
void Utilisateur::setMdp(const QString &nouveauMdp) { mdp = nouveauMdp; }
void Utilisateur::setEmail(const QString &nouvelEmail) { email = nouvelEmail; }

// AJOUTER
bool Utilisateur::ajouter(QString *erreur)
{
    QSqlQuery q;

    q.prepare("INSERT INTO UTILISATEURS "
              "(ID_UTILISATEUR, NOM_UTILISATEUR, PRENOM_UTILISATEUR, "
              "TEL_UTILISATEUR, MDP, EMAIL_UTILISATEUR) "
              "VALUES (:id, :nom, :prenom, :tel, :mdp, :email)");

    q.bindValue(":id", id);
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", tel);
    q.bindValue(":mdp", mdp);
    q.bindValue(":email", email);

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        return false;
    }

    return true;
}

// MODIFIER
bool Utilisateur::modifier(QString *erreur)
{
    QSqlQuery q;

    q.prepare("UPDATE UTILISATEURS SET "
              "NOM_UTILISATEUR=:nom, "
              "PRENOM_UTILISATEUR=:prenom, "
              "TEL_UTILISATEUR=:tel, "
              "MDP=:mdp, "
              "EMAIL_UTILISATEUR=:email "
              "WHERE ID_UTILISATEUR=:id");

    q.bindValue(":id", id);
    q.bindValue(":nom", nom);
    q.bindValue(":prenom", prenom);
    q.bindValue(":tel", tel);
    q.bindValue(":mdp", mdp);
    q.bindValue(":email", email);

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        return false;
    }

    return true;
}

// SUPPRIMER
bool Utilisateur::supprimer(int id, QString *erreur)
{
    QSqlQuery q;

    q.prepare("DELETE FROM UTILISATEURS "
              "WHERE ID_UTILISATEUR=:id");

    q.bindValue(":id", id);

    if (!q.exec())
    {
        if (erreur) *erreur = q.lastError().text();
        return false;
    }

    return true;
}

// AFFICHER
QList<Utilisateur> Utilisateur::afficher(QString *erreur)
{
    QList<Utilisateur> liste;
    QSqlQuery q;

    if (!q.exec("SELECT ID_UTILISATEUR, NOM_UTILISATEUR, "
                "PRENOM_UTILISATEUR, TEL_UTILISATEUR, "
                "MDP, EMAIL_UTILISATEUR "
                "FROM UTILISATEURS "
                "ORDER BY ID_UTILISATEUR"))
    {
        if (erreur) *erreur = q.lastError().text();
        return liste;
    }

    while (q.next())
    {
        Utilisateur u;

        u.setId(q.value(0).toInt());
        u.setNom(q.value(1).toString());
        u.setPrenom(q.value(2).toString());
        u.setTel(q.value(3).toString());
        // The supplied Oracle schema has no role column. Use least privilege
        // for display/access checks until roles are represented in the schema.
        u.setRole(QStringLiteral("Technicien"));
        u.setMdp(q.value(4).toString());
        u.setEmail(q.value(5).toString());

        liste.append(u);
    }

    return liste;
}
