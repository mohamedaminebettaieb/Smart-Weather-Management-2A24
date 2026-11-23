#include "maintenancedialog.h"

#include "maintenance.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

MaintenanceDialog::MaintenanceDialog(int idTechnicien, const QString &nomTechnicien, QWidget *parent)
    : QDialog(parent),
      m_idTechnicien(idTechnicien),
      m_nomTechnicien(nomTechnicien),
      m_table(nullptr),
      m_info(nullptr),
      m_btnOk(nullptr),
      m_btnSignaler(nullptr),
      m_chkTous(nullptr),
      m_btnActualiser(nullptr),
      m_btnFermer(nullptr)
{
    setWindowTitle(QString::fromUtf8("Équipements en maintenance"));
    setModal(true);
    resize(860, 520);

    setStyleSheet(
        "QDialog { background: #F4F8FD; }"
        "QLabel { color: #173B6C; }"
        "QTableWidget { background: white; border: 1px solid #C7D7E9; border-radius: 6px;"
        "  gridline-color: #E3ECF6; selection-background-color: #287ACB; selection-color: white; }"
        "QHeaderView::section { background: #173B6C; color: white; padding: 7px; border: none; font-weight: bold; }"
        "QPushButton { background: white; color: #173B6C; border: 1px solid #C7D7E9;"
        "  border-radius: 6px; padding: 9px 18px; min-height: 20px; }"
        "QPushButton:hover { background: #EEF6FF; border-color: #287ACB; }"
        "QPushButton#okButton { background: #1F9D55; color: white; border: none; font-weight: bold; }"
        "QPushButton#okButton:hover { background: #17824A; }");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 16);
    layout->setSpacing(10);

    QLabel *titre = new QLabel(QString::fromUtf8("🔧  Équipements en maintenance"));
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #173B6C;");
    layout->addWidget(titre);

    QLabel *tech = new QLabel(QString::fromUtf8("Technicien sélectionné : <b>#%1 — %2</b>")
                                  .arg(m_idTechnicien).arg(m_nomTechnicien.toHtmlEscaped()));
    layout->addWidget(tech);

    QLabel *aide = new QLabel(QString::fromUtf8(
        "Sélectionnez un équipement pour que ce technicien le prenne en charge. "
        "Cochez « Afficher tous les équipements » pour tester aussi un équipement qui n'est pas en maintenance."));
    aide->setWordWrap(true);
    aide->setStyleSheet("color: #7B8CA5; font-size: 11px;");
    layout->addWidget(aide);

    m_chkTous = new QCheckBox(QString::fromUtf8("Afficher tous les équipements (et pas seulement ceux en maintenance)"));
    layout->addWidget(m_chkTous);

    m_table = new QTableWidget;
    m_table->setColumnCount(5);
    m_table->setHorizontalHeaderLabels(QStringList()
                                       << "ID" << QString::fromUtf8("Nom de l'équipement") << "Type"
                                       << "Localisation" << "Statut");
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->setAlternatingRowColors(true);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setStretchLastSection(true);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Stretch);
    layout->addWidget(m_table, 1);

    m_info = new QLabel;
    m_info->setWordWrap(true);
    m_info->setMinimumHeight(44);
    layout->addWidget(m_info);

    QHBoxLayout *boutons = new QHBoxLayout;
    m_btnActualiser = new QPushButton(QString::fromUtf8("↻  Actualiser"));
    m_btnSignaler = new QPushButton(QString::fromUtf8("⚠  Mettre en maintenance"));
    m_btnSignaler->setVisible(false);                 // visible uniquement pour un équipement « Actif »
    m_btnOk = new QPushButton("OK");
    m_btnOk->setObjectName("okButton");
    m_btnOk->setVisible(false);                       // visible uniquement si l'équipement est en maintenance
    m_btnFermer = new QPushButton("Fermer");

    boutons->addWidget(m_btnActualiser);
    boutons->addWidget(m_btnSignaler);
    boutons->addStretch();
    boutons->addWidget(m_btnOk);
    boutons->addWidget(m_btnFermer);
    layout->addLayout(boutons);

    connect(m_table, &QTableWidget::itemSelectionChanged, this, &MaintenanceDialog::selectionChangee);
    connect(m_btnActualiser, &QPushButton::clicked, this, &MaintenanceDialog::chargerListe);
    connect(m_btnOk, &QPushButton::clicked, this, &MaintenanceDialog::confirmerPriseEnCharge);
    connect(m_btnSignaler, &QPushButton::clicked, this, &MaintenanceDialog::signalerMaintenance);
    connect(m_chkTous, &QCheckBox::toggled, this, &MaintenanceDialog::chargerListe);
    connect(m_btnFermer, &QPushButton::clicked, this, &QDialog::accept);

    chargerListe();
}

QString MaintenanceDialog::idSelectionne() const
{
    const int row = m_table->currentRow();
    if (row < 0 || !m_table->item(row, 0) || m_table->selectedItems().isEmpty())
        return QString();
    return m_table->item(row, 0)->text();
}

void MaintenanceDialog::afficherMessage(const QString &texte, bool erreur)
{
    m_info->setStyleSheet(erreur
        ? "background: #FDECEA; color: #A12622; border: 1px solid #F3B8B4; border-radius: 6px; padding: 8px;"
        : "background: #E8F5EC; color: #1B6B3A; border: 1px solid #B5DEC4; border-radius: 6px; padding: 8px;");
    m_info->setText(texte);
}

void MaintenanceDialog::chargerListe()
{
    QString erreur;
    const bool tous = m_chkTous && m_chkTous->isChecked();
    const QList<EquipementMaintenance> liste = tous ? MaintenanceDAO::tousLesEquipements(&erreur)
                                                    : MaintenanceDAO::equipementsEnMaintenance(&erreur);

    m_table->blockSignals(true);
    m_table->setRowCount(0);
    for (const EquipementMaintenance &e : liste)
    {
        const int row = m_table->rowCount();
        m_table->insertRow(row);
        m_table->setItem(row, 0, new QTableWidgetItem(e.id));
        m_table->setItem(row, 1, new QTableWidgetItem(e.nom));
        m_table->setItem(row, 2, new QTableWidgetItem(e.type));
        m_table->setItem(row, 3, new QTableWidgetItem(e.localisation));
        m_table->setItem(row, 4, new QTableWidgetItem(e.statut));
    }
    m_table->clearSelection();
    m_table->setCurrentCell(-1, -1);
    m_table->blockSignals(false);

    m_btnOk->setVisible(false);
    m_btnSignaler->setVisible(false);

    if (!erreur.isEmpty())
        afficherMessage(QString::fromUtf8("Lecture des équipements impossible :\n") + erreur, true);
    else if (liste.isEmpty())
        afficherMessage(QString::fromUtf8("Aucun équipement n'est actuellement en maintenance. "
                                          "Cochez « Afficher tous les équipements » pour en passer un en maintenance."), false);
    else
    {
        m_info->clear();
        m_info->setStyleSheet("");
    }
}

void MaintenanceDialog::selectionChangee()
{
    const QString id = idSelectionne();
    m_btnOk->setVisible(false);
    m_btnSignaler->setVisible(false);

    if (id.isEmpty())
    {
        m_info->clear();
        m_info->setStyleSheet("");
        return;
    }

    // Statut RÉEL relu dans Oracle (la liste affichée peut être périmée)
    QString erreur;
    const QString statut = MaintenanceDAO::statutReel(id, &erreur);

    if (!erreur.isEmpty())
    {
        afficherMessage(QString::fromUtf8("Lecture du statut impossible :\n") + erreur, true);
        return;
    }

    // Cas 1 : en maintenance -> le bouton OK apparaît
    if (statut.compare(MaintenanceDAO::STATUT_MAINTENANCE, Qt::CaseInsensitive) == 0)
    {
        m_btnOk->setVisible(true);
        afficherMessage(QString::fromUtf8("L'équipement <b>%1</b> est en maintenance : "
                                          "cliquez sur « OK » pour confirmer sa prise en charge par %2.")
                            .arg(id.toHtmlEscaped(), m_nomTechnicien.toHtmlEscaped()), false);
        return;
    }

    // Cas 2 : pas en maintenance -> pas de bouton OK, message explicatif
    const bool tous = m_chkTous->isChecked();
    if (!tous)
        chargerListe();     // la liste affichée n'était plus à jour (le message est affiché après)

    QString texte = QString::fromUtf8("Cet équipement n'est pas actuellement en maintenance. "
                                      "Vous ne pouvez pas le prendre en charge.");
    if (tous && statut.compare(MaintenanceDAO::STATUT_ACTIF, Qt::CaseInsensitive) == 0)
    {
        m_btnSignaler->setVisible(true);
        texte += QString::fromUtf8("\nVous pouvez le passer en maintenance avec le bouton « Mettre en maintenance ».");
    }
    afficherMessage(texte, true);
}


void MaintenanceDialog::signalerMaintenance()
{
    const QString id = idSelectionne();
    if (id.isEmpty())
    {
        QMessageBox::warning(this, "Maintenance", QString::fromUtf8("Veuillez sélectionner un équipement."));
        return;
    }

    const QMessageBox::StandardButton rep = QMessageBox::question(
        this, "Confirmation",
        QString::fromUtf8("Passer l'équipement %1 au statut « En maintenance » ?").arg(id),
        QMessageBox::Yes | QMessageBox::No);
    if (rep != QMessageBox::Yes)
        return;

    QString message;
    if (!MaintenanceDAO::signalerMaintenance(id, message))
    {
        QMessageBox::critical(this, "Maintenance", message);
        chargerListe();
        return;
    }

    emit equipementsModifies();
    chargerListe();
    afficherMessage(message + QString::fromUtf8(" Sélectionnez-le pour le prendre en charge."), false);
}


void MaintenanceDialog::confirmerPriseEnCharge()
{
    const QString id = idSelectionne();
    if (id.isEmpty())
    {
        QMessageBox::warning(this, "Prise en charge", QString::fromUtf8("Veuillez sélectionner un équipement."));
        return;
    }

    // Deuxième contrôle, dans le code, juste avant l'action
    QString erreur;
    const QString statut = MaintenanceDAO::statutReel(id, &erreur);
    if (statut.compare(MaintenanceDAO::STATUT_MAINTENANCE, Qt::CaseInsensitive) != 0)
    {
        m_btnOk->setVisible(false);
        QMessageBox::warning(this, "Prise en charge",
                             QString::fromUtf8("Cet équipement n'est pas actuellement en maintenance. "
                                               "Vous ne pouvez pas le prendre en charge."));
        chargerListe();
        return;
    }

    const QMessageBox::StandardButton rep = QMessageBox::question(
        this, "Confirmation",
        QString::fromUtf8("Confirmer la prise en charge de l'équipement %1 par le technicien #%2 (%3) ?\n\n"
                          "L'équipement repassera au statut « Actif ».")
            .arg(id).arg(m_idTechnicien).arg(m_nomTechnicien),
        QMessageBox::Yes | QMessageBox::No);
    if (rep != QMessageBox::Yes)
        return;

    QString message;
    if (!MaintenanceDAO::prendreEnCharge(id, m_idTechnicien, message))
    {
        QMessageBox::critical(this, "Prise en charge impossible", message);
        chargerListe();
        return;
    }

    QMessageBox::information(this, QString::fromUtf8("Succès"), message);
    emit equipementsModifies();
    chargerListe();     // l'équipement n'est plus « En maintenance » : il disparaît de la liste
}
