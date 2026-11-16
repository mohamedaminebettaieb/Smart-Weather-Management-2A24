#ifndef TECHNICIEN_H
#define TECHNICIEN_H

#include <QString>
#include <QDate>
#include <QList>
#include <QSqlQueryModel>

/*
 * Classe Technicien : représente un technicien et gère sa persistance
 * dans la table TECHNICIENS (Oracle) via les méthodes CRUD.
 * L'interface graphique (classe Technicienne) ne contient plus aucun SQL :
 * elle crée un objet Technicien et appelle ses méthodes.
 */
class Technicien
{
public:
    // ----- Constructeurs -----
    Technicien();
    Technicien(int id, const QString &nom, const QString &prenom,
               const QString &telephone, const QString &email,
               const QString &formation, int experience,
               const QString &specialite, const QString &competences,
               const QString &statut, const QDate &dateIntegration);

    // ----- Getters -----
    int      getId() const;
    QString  getNom() const;
    QString  getPrenom() const;
    QString  getTelephone() const;
    QString  getEmail() const;
    QString  getFormation() const;
    int      getExperience() const;
    QString  getSpecialite() const;
    QString  getCompetences() const;
    QString  getStatut() const;
    QDate    getDateIntegration() const;

    // ----- Setters -----
    void setId(int id);
    void setNom(const QString &nom);
    void setPrenom(const QString &prenom);
    void setTelephone(const QString &telephone);
    void setEmail(const QString &email);
    void setFormation(const QString &formation);
    void setExperience(int experience);
    void setSpecialite(const QString &specialite);
    void setCompetences(const QString &competences);
    void setStatut(const QString &statut);
    void setDateIntegration(const QDate &date);

    // ----- Méthodes CRUD (base de données) -----
    bool ajouter();                       // INSERT
    bool modifier(int ancienId);          // UPDATE (ancienId = id avant modification)
    bool supprimer(int id);               // DELETE
    QSqlQueryModel *afficher();           // SELECT -> modèle prêt pour un QTableView
    QList<Technicien> lister();           // SELECT -> liste d'objets Technicien

    // Dernier message d'erreur SQL (vide si tout s'est bien passé)
    QString derniereErreur() const;

private:
    int     id;
    QString nom;
    QString prenom;
    QString telephone;
    QString email;
    QString formation;
    int     experience;
    QString specialite;
    QString competences;
    QString statut;
    QDate   dateIntegration;

    QString erreur;
};

#endif // TECHNICIEN_H