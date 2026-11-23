#ifndef MAINTENANCEDIALOG_H
#define MAINTENANCEDIALOG_H

#include <QDialog>
#include <QString>

class QCheckBox;
class QLabel;
class QPushButton;
class QTableWidget;

/*
 * Fenêtre « Équipements en maintenance » (métier avancé 1).
 * Affiche uniquement les équipements dont le statut Oracle est « En maintenance »
 * et permet au technicien sélectionné de les prendre en charge.
 * Le bouton « OK » n'apparaît que si le statut RÉEL (relu dans Oracle) de
 * l'équipement sélectionné est « En maintenance ».
 */
class MaintenanceDialog : public QDialog
{
    Q_OBJECT

public:
    MaintenanceDialog(int idTechnicien, const QString &nomTechnicien, QWidget *parent = nullptr);

signals:
    // Émis après une prise en charge réussie (les autres fenêtres doivent se rafraîchir)
    void equipementsModifies();

private slots:
    void chargerListe();
    void selectionChangee();
    void confirmerPriseEnCharge();
    void signalerMaintenance();

private:
    QString idSelectionne() const;
    void afficherMessage(const QString &texte, bool erreur);

    int m_idTechnicien;
    QString m_nomTechnicien;

    QTableWidget *m_table;
    QLabel *m_info;
    QPushButton *m_btnOk;
    QPushButton *m_btnSignaler;
    QCheckBox *m_chkTous;
    QPushButton *m_btnActualiser;
    QPushButton *m_btnFermer;
};

#endif // MAINTENANCEDIALOG_H
