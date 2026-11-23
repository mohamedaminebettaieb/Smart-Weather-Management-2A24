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

    // Recherche + tri faits par Oracle (WHERE ... LIKE / ORDER BY)
    //   texte : cherche dans toutes les colonnes (vide = tout)
    //   tri   : 0 ID, 1 Nom, 2 Prenom, 3 Experience, 4 Statut, 5 Date d'integration
    QList<Technicien> rechercherTrier(const QString &texte, int tri);

    // Nombre de techniciens par statut (SELECT ... GROUP BY)
    static void compterParStatut(int &actifs, int &inactifs);

    // Prochain ID libre (MAX + 1) lu dans Oracle
    static int prochainId();

    // Retourne un message si un technicien portant le même email (ou le même nom ET prénom)
    // existe déjà dans la table TECHNICIENS ; chaîne vide sinon.
    static QString doublonExistant(const QString &email, const QString &nom, const QString &prenom);

    // Dernier message d'erreur SQL (vide si tout s'est bien passé)
    QString derniereErreur() const;

    // Taille maximale (en caractères) d'une colonne de TECHNICIENS, lue dans
    // le dictionnaire Oracle (USER_TAB_COLUMNS). Retourne 0 si inconnue.
    static int longueurMax(const QString &colonne);

    // Vrai si un technicien avec cet ID existe déjà dans la base
    static bool existe(int id);

    // Contrôle de saisie : retourne "" si tout est correct, sinon le message
    // d'erreur (champ trop long par rapport à la base, etc.)
    QString valider() const;

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