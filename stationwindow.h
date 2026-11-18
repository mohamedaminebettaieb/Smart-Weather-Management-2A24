#ifndef STATIONWINDOW_H
#define STATIONWINDOW_H

#include <QMainWindow>
#include <QWidget>
#include <QString>
#include <QVector>
#include <QDate>
#include <QTableWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QDateEdit>
#include <QDoubleSpinBox>
#include <QSpinBox>
#include <QLabel>
#include <QGroupBox>
#include <QPushButton>

/*
 * Classe représentant une station météo
 */
class Station
{
    int id;
    QString nom;
    QString localisation;
    double latitude;
    double longitude;
    QDate dateInstallation;
    QString type;
    QString etat;

public:
    Station()
        : id(0), latitude(0.0), longitude(0.0)
    {
    }

    int getId() const { return id; }
    void setId(int value) { id = value; }

    QString getNom() const { return nom; }
    void setNom(const QString &value) { nom = value; }

    QString getLocalisation() const { return localisation; }
    void setLocalisation(const QString &value) { localisation = value; }

    double getLatitude() const { return latitude; }
    void setLatitude(double value) { latitude = value; }

    double getLongitude() const { return longitude; }
    void setLongitude(double value) { longitude = value; }

    QDate getDateInstallation() const { return dateInstallation; }
    void setDateInstallation(const QDate &value) { dateInstallation = value; }

    QString getType() const { return type; }
    void setType(const QString &value) { type = value; }

    QString getEtat() const { return etat; }
    void setEtat(const QString &value) { etat = value; }

};


/*
 * Widget personnalisé pour afficher
 * les statistiques sous forme de diagramme circulaire.
 */
class StatisticsWidget : public QWidget
{
    int m_actifs;
    int m_maintenance;
    int m_horsService;

public:
    explicit StatisticsWidget(QWidget *parent = nullptr);

    void setStatistics(int actifs,
                       int maintenance,
                       int horsService);

protected:
    void paintEvent(QPaintEvent *event) override;
};


/*
 * Fenêtre principale SmartWeather
 */
class StationWindow : public QMainWindow
{
    // Attributs privés : interface

    QWidget *centralWidget;

    // Champs du formulaire
    QSpinBox *idEdit;
    QLineEdit *nomEdit;
    QLineEdit *localisationEdit;

    QDoubleSpinBox *latitudeEdit;
    QDoubleSpinBox *longitudeEdit;

    QDateEdit *dateInstallationEdit;

    QComboBox *typeCombo;
    QComboBox *etatCombo;

    // Recherche / tri
    QLineEdit *searchEdit;
    QComboBox *sortCombo;

    // Tableau
    QTableWidget *table;

    // Statistiques
    StatisticsWidget *statisticsWidget;

    QLabel *totalLabel;

    // Données
    QVector<Station> stations;

public:
    explicit StationWindow(QWidget *parent = nullptr);

private:

    // Méthodes privées : création de l'interface

    void setupUI();
    void setupStyle();

    QWidget *createSidebar();
    QWidget *createHeader();

    QGroupBox *createStationForm();
    QGroupBox *createActionBar();

    QGroupBox *createStationList();
    QGroupBox *createStatistics();
    QGroupBox *createLocationBox();

    // Méthodes privées : données

    void loadDemoData();
    void loadStationsFromDatabase();

    // Méthodes privées : tableau et formulaire

    void refreshTable();

    void updateStatistics();

    void clearForm();

    void fillFormFromSelectedRow();

    // Méthodes privées : actions

    void addStation();
    void modifyStation();
    void deleteStation();

    void searchStations();
    void sortStations();

    void exportStations();

    // Méthodes privées : utilitaires

    int selectedRow() const;

    QString statusText(const QString &etat) const;

    void updateLocationList();
};

#endif // STATIONWINDOW_H
