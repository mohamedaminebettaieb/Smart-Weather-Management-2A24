#ifndef UTILISATEUR_H
#define UTILISATEUR_H

#include <QString>
#include <QList>

class Utilisateur
{
private:
    int id;
    QString nom, prenom, tel, role, mdp, email;

public:
    Utilisateur();

    int getId() const;
    QString getNom() const;
    QString getPrenom() const;
    QString getTel() const;
    QString getRole() const;
    QString getMdp() const;
    QString getEmail() const;

    void setId(int id);
    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setTel(const QString &tel);
    void setRole(const QString &role);
    void setMdp(const QString &mdp);
    void setEmail(const QString &email);

    // ----- CRUD -----
    bool ajouter(QString *erreur = nullptr);
    bool modifier(QString *erreur = nullptr);
    static bool supprimer(int id, QString *erreur = nullptr);
    static QList<Utilisateur> afficher(QString *erreur = nullptr);
};

#endif