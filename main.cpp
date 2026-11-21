#include <QApplication>
#include <QDate>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include "smartweatherwindow.h"
#include "connection.h"
#include "equipement.h"
#include "technicien.h"

static bool ensureDemoData(QString *error)
{
    struct EquipementDemo {
        const char *id; const char *nom; const char *lieu;
        const char *statut; const char *type; QDate date; const char *ancienNom;
    } equipements[] = {
        {"E001", "Station meteo princ", "Tunis", "Actif", "Station meteo", QDate(2018, 3, 12), "Station meteo"},
        {"E002", "Thermometre centr", "Sfax", "Actif", "Thermometre", QDate(2019, 7, 25), "Thermometre"},
        {"E003", "Baromètre Sousse", "Sousse", "Actif", "Baromètre", QDate(2020, 11, 10), "Barometre"},
        {"E004", "Anemometre Kairouan", "Kairouan", "Inactif", "Anemometre", QDate(2021, 5, 3), "Anemometre"},
        {"E005", "Pluviomètre Gabès", "Gabès", "Actif", "Pluviomètre", QDate(2022, 9, 17), nullptr},
        {"E006", "Hygromètre Tozeur", "Tozeur", "Actif", "Hygromètre", QDate(2023, 2, 28), nullptr},
        {"E007", "Capteur Kasserine", "Kasserine", "Inactif", "Capteur météo", QDate(2023, 6, 14), nullptr},
        {"E008", "Station Bizerte", "Bizerte", "Actif", "Station meteo", QDate(2023, 12, 22), nullptr},
        {"E009", "Capteur Gafsa", "Gafsa", "En maintenance", "Capteur météo", QDate(2024, 4, 8), nullptr},
        {"E010", "Station Médenine", "Médenine", "Actif", "Station meteo", QDate(2024, 10, 16), nullptr}
    };
    struct TechnicienDemo {
        int id; const char *nom; const char *prenom; const char *tel;
        const char *email; const char *formation; int experience;
        const char *specialite; const char *competences; const char *statut;
        QDate date; const char *ancienEmail;
    } techniciens[] = {
        {1, "Ben Ali", "Sami", "20123456", "sami.benali@sw.tn", "Licence electronique", 5,
         "Capteurs", "Etalonnage, maint.", "Actif", QDate(2021, 3, 15), "sami@sw.tn"},
        {2, "Trabelsi", "Amira", "98234567", "amira.trabelsi@sw.tn", "Master telecom", 8,
         "Reseaux", "Transm. donnees", "Actif", QDate(2018, 9, 1), "amira@sw.tn"},
        {3, "Gharbi", "Walid", "55345678", "walid.gharbi@sw.tn", "Technicien superieur", 3,
         "Electricite", "Cablage, solaire", "Inactif", QDate(2022, 1, 20), "walid@sw.tn"},
        {4, "Mansouri", "Rim", "22456789", "rim.mansouri@sw.tn", "Ingenieur mecatro", 6,
         "Automatisme", "Diagnostic, prog.", "Actif", QDate(2020, 6, 10), "rim@sw.tn"}
    };

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.transaction()) {
        if (error) *error = db.lastError().text();
        return false;
    }

    for (const EquipementDemo &e : equipements) {
        const QString id = QString::fromLatin1(e.id);
        const QString nom = QString::fromUtf8(e.nom);
        const QString lieu = QString::fromUtf8(e.lieu);
        const QString statut = QString::fromLatin1(e.statut);
        const QString type = QString::fromUtf8(e.type);
        QSqlQuery existing;
        existing.prepare("SELECT NOM_EQUIPEMENT FROM EQUIPEMENTS WHERE ID_EQUIPEMENT = :id");
        existing.bindValue(":id", id);
        if (!existing.exec()) {
            if (error) *error = existing.lastError().text();
            db.rollback();
            return false;
        }
        const bool found = existing.next();
        const bool isOldDemo = found && e.ancienNom
            && existing.value(0).toString() == QString::fromLatin1(e.ancienNom);
        existing.finish();

        EquipementDAO dao;
        const bool saved = found
            ? (!isOldDemo || dao.modifier(id, nom, lieu, statut, e.date, type))
            : dao.ajouter(id, nom, lieu, statut, e.date, type);
        if (!saved) {
            if (error) *error = dao.derniereErreur();
            db.rollback();
            return false;
        }
    }

    for (const TechnicienDemo &d : techniciens) {
        Technicien t(d.id, QString::fromLatin1(d.nom), QString::fromLatin1(d.prenom),
                     QString::fromLatin1(d.tel), QString::fromLatin1(d.email),
                     QString::fromLatin1(d.formation), d.experience,
                     QString::fromUtf8(d.specialite), QString::fromLatin1(d.competences),
                     QString::fromLatin1(d.statut), d.date);
        QSqlQuery existing;
        existing.prepare("SELECT EMAIL_TECHNICIEN FROM TECHNICIENS WHERE ID_TECHNICIEN = :id");
        existing.bindValue(":id", QString::number(d.id));
        if (!existing.exec()) {
            if (error) *error = existing.lastError().text();
            db.rollback();
            return false;
        }
        const bool found = existing.next();
        const bool isOldDemo = found && existing.value(0).toString()
            == QString::fromLatin1(d.ancienEmail);
        existing.finish();

        const bool saved = found
            ? (!isOldDemo || t.modifier(d.id))
            : t.ajouter();
        if (!saved) {
            if (error) *error = QString("Technicien %1 (ID %2) : %3")
                    .arg(QString::fromLatin1(d.prenom))
                    .arg(QString::number(d.id))
                    .arg(t.derniereErreur());
            db.rollback();
            return false;
        }
    }

    if (!db.commit()) {
        if (error) *error = db.lastError().text();
        db.rollback();
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    app.setApplicationName("SmartWeather");
    app.setApplicationDisplayName("SmartWeather");
    Connection& connection = Connection::createInstance();
    if (!connection.createConnection())
    {
        QMessageBox::critical(nullptr, "Erreur de connexion",
                              "Impossible de se connecter à la base de données.\n"
                              "Vérifiez la source ODBC, le nom d'utilisateur et le mot de passe.");
        return -1;
    }

    QString seedError;
    if (!ensureDemoData(&seedError))
        QMessageBox::warning(nullptr, "Données équipements/techniciens",
                             "Les données de démonstration n'ont pas pu être enregistrées :\n"
                                 + seedError);
    QMessageBox::information(nullptr, "Connexion", "Connexion à la base de données réussie.");

    SmartWeatherWindow window;
    window.show();

    return app.exec();
}
