
#include <QApplication>
#include <QMessageBox>
#include <QMainWindow>
#include <QPushButton>
#include <QList>
#include <QString>
#include <QSqlDatabase>
#include <QSqlError>
#include <QStackedWidget>
#include <QWidget>

#include "technicienne.h"
#include "stationwuindow.h"
#include "gbureaux.h"
#include "weatherwisetest.h"
#include "gutilisateur.h"
#include "connection.h"

// ==========================================
// NAVIGATION BETWEEN MODULES
// ==========================================

static void connecterNavigation(
    QMainWindow *from,
    QStackedWidget *pages,
    QWidget *toPage,
    const QString &motCle)
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

// ==========================================
// MAIN
// ==========================================

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QApplication::setStyle("Fusion");
    QApplication::setApplicationName("Smart Weather Management");

    // ==========================================
    // DATABASE CONNECTION
    // ==========================================

    Connection& connection = Connection::createInstance();

    if (!connection.createConnection()) {

        QMessageBox::critical(
            nullptr,
            "Erreur de connexion",
            QString("Impossible de se connecter à la base de données.\n\nErreur : %1")
                .arg(QSqlDatabase::database().lastError().text())
            );

        return -1;
    }

    QMessageBox::information(
        nullptr,
        "Connexion Oracle",
        "Connexion à la base de données réussie."
        );

    // ==========================================
    // CREATE THE FIVE MODULES
    // ==========================================

    Technicienne *fenetreTechniciens = new Technicienne();

    StationWindow *fenetreStations = new StationWindow();

    GBureaux *fenetreBureaux = new GBureaux();

    WeatherWiseTest *fenetreEquipements = new WeatherWiseTest();

    Gutilisateur *fenetreUtilisateurs = new Gutilisateur();

    // ==========================================
    // MAIN APPLICATION WINDOW
    // ==========================================

    QMainWindow fenetrePrincipale;

    fenetrePrincipale.setWindowTitle("Smart Weather Management");

    fenetrePrincipale.resize(1500, 850);

    QStackedWidget *pages = new QStackedWidget(&fenetrePrincipale);

    fenetrePrincipale.setCentralWidget(pages);

    // Add all modules to the same QStackedWidget

    pages->addWidget(fenetreTechniciens);

    pages->addWidget(fenetreStations);

    pages->addWidget(fenetreBureaux);

    pages->addWidget(fenetreEquipements);

    pages->addWidget(fenetreUtilisateurs);

    // ==========================================
    // TECHNICIENS -> OTHER MODULES
    // ==========================================

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreTechniciens, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // ==========================================
    // STATIONS -> OTHER MODULES
    // ==========================================

    connecterNavigation(fenetreStations, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreStations, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreStations, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreStations, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // ==========================================
    // BUREAUX -> OTHER MODULES
    // ==========================================

    connecterNavigation(fenetreBureaux, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreEquipements, "Gestion des équipements");

    connecterNavigation(fenetreBureaux, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // ==========================================
    // EQUIPEMENTS -> OTHER MODULES
    // ==========================================

    connecterNavigation(fenetreEquipements, pages,
                        fenetreTechniciens, "Gestion des techniciens");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreStations, "Gestion des stations");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreEquipements, pages,
                        fenetreUtilisateurs, "Gestion des utilisateurs");

    // ==========================================
    // UTILISATEURS -> OTHER MODULES
    // ==========================================

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreTechniciens, "Techniciens");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreStations, "Stations");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreBureaux, "bureau");

    connecterNavigation(fenetreUtilisateurs, pages,
                        fenetreEquipements, "Équipements");

    // ==========================================
    // START APPLICATION
    // ==========================================

    // Start with the existing login screen

    pages->setCurrentWidget(fenetreUtilisateurs);

    // Display the single application window

    fenetrePrincipale.show();

    return app.exec();
}