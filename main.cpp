#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QPushButton>
#include <QList>
#include <QString>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStackedWidget>

#include "technicienne.h"
#include "stationwuindow.h"
#include "gbureaux.h"
#include "weatherwisetest.h"
#include "gutilisateur.h"
#include "connection.h"

<<<<<<< HEAD
// Navigate between modules inside ONE main application window
static void connecterNavigation(
    QMainWindow *from,
    QStackedWidget *pages,
    QWidget *toPage,
    const QString &motCle)
=======
// Switch from one module window to another
static void basculerFenetre(QMainWindow *from, QMainWindow *to)
>>>>>>> origin/crudbureau
{
    if (from == nullptr || pages == nullptr || toPage == nullptr)
        return;

    const QList<QPushButton *> boutons =
        from->findChildren<QPushButton *>();

    for (QPushButton *bouton : boutons) {
        if (bouton->text().contains(motCle, Qt::CaseInsensitive)) {
            QObject::connect(
                bouton,
                &QPushButton::clicked,
                pages,
                [pages, toPage]() {
                    pages->setCurrentWidget(toPage);
                });
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle("Fusion");
    QApplication::setApplicationName("Smart Weather Management");
    Connection& connection = Connection::createInstance();
    if(!connection.createConnection())
    {
        //QMessageBox::critical(nullptr, "Erreur de connexion", "Impossible de se connecter à la base de données.\n", "Vérifiez la source ODBC, le nom d'utilisateur et le mot de passe.", QMessageBox::Ok);
        return -1;
    }
    QMessageBox::information(nullptr, "Information", "Connexion à la base de données réussie.", QMessageBox::Ok);

    // Oracle connection using the existing Singleton
    Connection& connexion = Connection::createInstance();

    if (connexion.createConnection()) {
        QMessageBox::information(
            nullptr,
            "Connexion Oracle",
            "Connexion reussie !"
            );
    } else {
        QMessageBox::critical(
            nullptr,
            "Connexion Oracle",
            QString("Echec de la connexion !\n\nErreur : %1")
                .arg(QSqlDatabase::database().lastError().text())
            );
    }

    // Create the five existing modules.
    // They are allocated dynamically because the shared
    // QStackedWidget will become their parent.
    Technicienne *fenetreTechniciens = new Technicienne();
    StationWindow *fenetreStations = new StationWindow();
    GBureaux *fenetreBureaux = new GBureaux();
    WeatherWiseTest *fenetreEquipements = new WeatherWiseTest();
    Gutilisateur *fenetreUtilisateurs = new Gutilisateur();

    // Create ONE application window
    QMainWindow fenetrePrincipale;
    fenetrePrincipale.setWindowTitle("Smart Weather Management");
    fenetrePrincipale.resize(1500, 850);

    QStackedWidget *pages = new QStackedWidget(&fenetrePrincipale);
    fenetrePrincipale.setCentralWidget(pages);

    // Put the complete module windows into the shared stack.
    // No centralWidget() or takeCentralWidget() calls are needed.
    pages->addWidget(fenetreTechniciens);
    pages->addWidget(fenetreStations);
    pages->addWidget(fenetreBureaux);
    pages->addWidget(fenetreEquipements);
    pages->addWidget(fenetreUtilisateurs);

    // TECHNICIENS -> other modules
    connecterNavigation(fenetreTechniciens, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // STATIONS -> other modules
    connecterNavigation(fenetreStations, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreStations, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreStations, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreStations, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // BUREAUX -> other modules
    connecterNavigation(fenetreBureaux, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // EQUIPEMENTS -> other modules
    connecterNavigation(fenetreEquipements, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // UTILISATEURS -> other modules
    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreTechniciens, "Techniciens");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreStations, "Stations");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreEquipements, "Équipements");

<<<<<<< HEAD
    // Start with the existing login screen
    pages->setCurrentWidget(fenetreUtilisateurs);

    // Display the single application window
    fenetrePrincipale.show();
=======
    // Start with the Techniciens module
    fenetreUtilisateurs.show();

    if (!Connection::createInstance().createConnection()) {
        QMessageBox::critical(nullptr, "Erreur", "Connexion à la base impossible.");
        return 1;
    }
    GBureaux w;
>>>>>>> origin/crudbureau

    return app.exec();
}