#include "weatherwisetest.h"
#include "equipement.h"
#include "qrcodegen.hpp"

#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include <QAbstractItemView>
#include <QComboBox>
#include <QDateTime>
#include <QDateEdit>
#include <QFileDialog>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPageLayout>
#include <QPageSize>
#include <QPdfWriter>
#include <QPushButton>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QTextDocument>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <QBrush>
#include <QColor>
#include <QStringList>
#include <QImage>
#include <QPixmap>
#include <QByteArray>
#include <QFont>
#include <QDate>
#include <QList>
#include <QWidget>
#include <QPainter>

#include <exception>

// ==========================================
// UTILITAIRES
// ==========================================

static QFrame *makeCard()
{
    QFrame *f = new QFrame;
    f->setObjectName("card");
    return f;
}

static QLabel *makeCardTitle(const QString &t)
{
    QLabel *l = new QLabel(t);
    l->setObjectName("cardTitle");
    return l;
}

// Conversion d'une valeur Oracle en QDate
static QDate convertirDate(const QVariant &dateValue)
{
    QDate date = dateValue.toDate();

    if (!date.isValid())
    {
        const QString dateText = dateValue.toString();

        date = QDate::fromString(dateText, "yyyy-MM-dd");

        if (!date.isValid())
        {
            date = QDate::fromString(dateText, "dd/MM/yyyy");
        }

        if (!date.isValid())
        {
            date = QDate::fromString(
                dateText.left(10), "yyyy-MM-dd"
                );
        }
    }

    return date;
}

// ==========================================
// INTERVALLE DE MAINTENANCE PAR TYPE
// ==========================================

static int intervalMaintenanceMois(const QString &type)
{
    if (type == "Station météo")
    {
        return 12;
    }
    else if (type == "Thermomètre")
    {
        return 6;
    }
    else if (type == "Baromètre")
    {
        return 12;
    }
    else if (type == "Anémomètre")
    {
        return 6;
    }
    else if (type == "Pluviomètre")
    {
        return 6;
    }
    else if (type == "Hygromètre")
    {
        return 6;
    }
    else if (type == "Capteur météo")
    {
        return 6;
    }

    return 6;
}

// ==========================================
// AJOUTER UN CHAMP DANS LA GRILLE
// ==========================================

static void addField(
    QGridLayout *g,
    int row,
    int col,
    const QString &label,
    QWidget *w)
{
    QLabel *l = new QLabel(label);
    l->setMinimumWidth(115);

    g->addWidget(l, row, col * 2);
    g->addWidget(w, row, col * 2 + 1);
}

// ==========================================
// CONSTRUCTEUR
// ==========================================

WeatherWiseTest::WeatherWiseTest(QWidget *parent)
    : QMainWindow(parent),
    m_sortMode(-1),
    m_table(0),
    m_search(0),
    m_sort(0),
    m_donut(0),
    m_actifsLabel(0),
    m_inactifsLabel(0),
    m_maintenanceLabel(0),
    m_dateLabel(0),
    m_timeLabel(0),
    m_maintenanceTable(0),
    m_maintenanceCount(0),
    m_qrPreview(0),
    m_qrInfo(0),
    m_id(0),
    m_nom(0),
    m_loc(0),
    m_station(0),
    m_date(0),
    m_statut(0),
    m_type(0)
{
    setWindowTitle(
        "Smart Weather Management - Gestion des équipements"
        );

    setStyleSheet(styleSheet());

    // Taille initiale de la fenêtre
    resize(1500, 900);

    QWidget *central = new QWidget;
    QHBoxLayout *root = new QHBoxLayout(central);

    root->setContentsMargins(0, 0, 0, 0);
    root->setSpacing(0);

    root->addWidget(buildSidebar());

    QWidget *right = new QWidget;
    QVBoxLayout *rv = new QVBoxLayout(right);

    rv->setContentsMargins(0, 0, 0, 0);
    rv->setSpacing(0);

    rv->addWidget(buildHeader());

    // Contenu principal sans zone de défilement
    QWidget *content = new QWidget;
    content->setObjectName("content");

    QGridLayout *g = new QGridLayout(content);

    g->setContentsMargins(16, 8, 16, 8);
    g->setSpacing(8);

    // Formulaire sur toute la largeur
    g->addWidget(buildFormCard(), 0, 0, 1, 2);

    // Barre des boutons sur toute la largeur
    g->addWidget(buildToolbar(), 1, 0, 1, 2);

    // Tableau à gauche
    g->addWidget(buildTableCard(), 2, 0);

    // QR Code à droite du tableau
    g->addWidget(buildQrCard(), 2, 1);

    // Maintenance en bas à gauche
    g->addWidget(buildMaintenanceCard(), 3, 0);

    // Statistiques en bas à droite
    g->addWidget(buildStatsCard(), 3, 1);

    g->setColumnStretch(0, 3);
    g->setColumnStretch(1, 2);

    g->setRowStretch(0, 0);
    g->setRowStretch(1, 0);
    g->setRowStretch(2, 3);
    g->setRowStretch(3, 2);

    // Affichage direct : aucun scroll vertical
    rv->addWidget(content, 1);

    root->addWidget(right, 1);

    setCentralWidget(central);

    // ==========================================
    // CHARGER LES EQUIPEMENTS DEPUIS ORACLE
    // ==========================================

    m_equipements.clear();

    EquipementDAO dao;
    QSqlQueryModel *model = dao.afficher();

    if (model && !model->lastError().isValid())
    {
        for (int row = 0; row < model->rowCount(); ++row)
        {
            Equipement e;

            e.setId(
                model->data(model->index(row, 0)).toString()
                );

            e.setNom(
                model->data(model->index(row, 1)).toString()
                );

            e.setLocalisation(
                model->data(model->index(row, 2)).toString()
                );

            e.setStatut(
                model->data(model->index(row, 3)).toString()
                );

            const QVariant dateValue =
                model->data(model->index(row, 4));

            e.setDateInstallation(convertirDate(dateValue));

            e.setType(
                model->data(model->index(row, 5)).toString()
                );

            // ID_STATION : 7eme colonne du resultat Oracle
            e.setIdStation(
                model->data(model->index(row, 6)).toString()
                );

            m_equipements.append(e);
        }
    }
    else if (model)
    {
        qDebug() << "Erreur chargement équipements Oracle:"
                 << model->lastError().text();

        QMessageBox::warning(
            this,
            "Erreur Oracle",
            QString::fromUtf8(
                "Impossible de charger les équipements depuis Oracle :\n"
                ) + model->lastError().text()
            );
    }

    delete model;

    onClear();
    refreshTable();

    // Horloge
    QTimer *clock = new QTimer(this);

    connect(
        clock,
        SIGNAL(timeout()),
        this,
        SLOT(updateClock())
        );

    clock->start(1000);
    updateClock();
}

// ==========================================
// RECHARGER DEPUIS ORACLE (apres une prise en charge par un technicien)
// ==========================================

void WeatherWiseTest::rechargerDepuisOracle()
{
    EquipementDAO dao;
    QSqlQueryModel *model = dao.afficher();

    if (!model || model->lastError().isValid())
    {
        qDebug() << "Erreur rechargement équipements Oracle:"
                 << (model ? model->lastError().text() : QString());
        delete model;
        return;
    }

    m_equipements.clear();

    for (int row = 0; row < model->rowCount(); ++row)
    {
        Equipement e;
        e.setId(model->data(model->index(row, 0)).toString());
        e.setNom(model->data(model->index(row, 1)).toString());
        e.setLocalisation(model->data(model->index(row, 2)).toString());
        e.setStatut(model->data(model->index(row, 3)).toString());
        e.setDateInstallation(convertirDate(model->data(model->index(row, 4))));
        e.setType(model->data(model->index(row, 5)).toString());
        e.setIdStation(model->data(model->index(row, 6)).toString());

        m_equipements.append(e);
    }

    delete model;

    onClear();
    refreshTable();
}

// ==========================================
// DONNEES EXEMPLES
// ==========================================

void WeatherWiseTest::loadSampleData()
{
    m_equipements.clear();

    const char *noms[] = {
        "Station météo principale",
        "Thermomètre central",
        "Baromètre Sousse",
        "Anémomètre Kairouan",
        "Pluviomètre Gabès",
        "Hygromètre Tozeur",
        "Capteur Kasserine",
        "Station Bizerte",
        "Capteur Gafsa",
        "Station Médenine"
    };

    const char *villes[] = {
        "Tunis", "Sfax", "Sousse", "Kairouan", "Gabès",
        "Tozeur", "Kasserine", "Bizerte", "Gafsa", "Médenine"
    };

    const char *statuts[] = {
        "Actif", "Actif", "Actif", "Inactif", "Actif",
        "Actif", "Inactif", "Actif", "En maintenance", "Actif"
    };

    const char *types[] = {
        "Station météo", "Thermomètre", "Baromètre",
        "Anémomètre", "Pluviomètre", "Hygromètre",
        "Capteur météo", "Station météo", "Capteur météo",
        "Station météo"
    };

    const int annees[] = {
        2018, 2019, 2020, 2021, 2022,
        2023, 2023, 2023, 2024, 2024
    };

    const int mois[] = {
        3, 7, 11, 5, 9, 2, 6, 12, 4, 10
    };

    const int jours[] = {
        12, 25, 10, 3, 17, 28, 14, 22, 8, 16
    };

    const int count = 10;

    for (int i = 0; i < count; ++i)
    {
        Equipement e;

        e.setId(
            QString("E%1").arg(i + 1, 3, 10, QChar('0'))
            );

        e.setNom(QString::fromUtf8(noms[i]));
        e.setLocalisation(QString::fromUtf8(villes[i]));
        e.setStatut(QString::fromUtf8(statuts[i]));
        e.setType(QString::fromUtf8(types[i]));

        e.setDateInstallation(
            QDate(annees[i], mois[i], jours[i])
            );

        m_equipements.append(e);
    }
}

// ==========================================
// GENERER UN NOUVEL ID
// ==========================================

QString WeatherWiseTest::nextId() const
{
    int mx = 0;

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        QString id = m_equipements.at(i).getId();

        if (id.length() > 1)
        {
            mx = qMax(mx, id.mid(1).toInt());
        }
    }

    return QString("E%1").arg(
        mx + 1, 3, 10, QChar('0')
        );
}

// ==========================================
// SIDEBAR
// ==========================================

QWidget *WeatherWiseTest::buildSidebar()
{
    QFrame *side = new QFrame;

    side->setObjectName("sidebar");
    side->setFixedWidth(300);

    QVBoxLayout *v = new QVBoxLayout(side);

    v->setContentsMargins(0, 14, 0, 14);
    v->setSpacing(6);

    QLabel *sun = new QLabel(QString::fromUtf8("☀"));

    sun->setObjectName("sunIcon");
    sun->setAlignment(Qt::AlignCenter);

    v->addWidget(sun);

    QLabel *brand = new QLabel(
        "SMART WEATHER\nMANAGEMENT"
        );

    brand->setObjectName("brand");
    brand->setAlignment(Qt::AlignCenter);

    v->addWidget(brand);
    v->addSpacing(26);

    const char *items[] = {
        "⌂   Accueil",
        "🏢   Gestion de bureau",
        "⚙   Gestion des équipements",
        "👷   Gestion des techniciens",
        "📡   Gestion des stations",
        "👥   Gestion des utilisateurs"
    };

    const bool active[] = {
        false, false, true, false, false, false
    };

    for (int i = 0; i < 6; ++i)
    {
        QPushButton *b = new QPushButton(
            QString::fromUtf8(items[i])
            );

        b->setObjectName("nav");
        b->setProperty("active", active[i]);
        b->setCursor(Qt::PointingHandCursor);

        v->addWidget(b);
    }

    v->addStretch();

    return side;
}

// ==========================================
// HEADER
// ==========================================

QWidget *WeatherWiseTest::buildHeader()
{
    QFrame *h = new QFrame;

    h->setObjectName("header");
    h->setFixedHeight(72);

    QHBoxLayout *l = new QHBoxLayout(h);

    l->setContentsMargins(34, 6, 34, 6);

    QVBoxLayout *tv = new QVBoxLayout;
    tv->setSpacing(0);

    QLabel *t = new QLabel("Gestion des équipements");

    t->setObjectName("headerTitle");

    QLabel *s = new QLabel(
        QString::fromUtf8(
            "Gérez les équipements météorologiques et leurs informations"
            )
        );

    s->setObjectName("headerSub");

    tv->addStretch();
    tv->addWidget(t);
    tv->addWidget(s);
    tv->addStretch();

    l->addLayout(tv);
    l->addStretch();

    m_dateLabel = new QLabel;
    m_dateLabel->setObjectName("headerInfo");

    m_timeLabel = new QLabel;
    m_timeLabel->setObjectName("headerInfo");

    QLabel *dot = new QLabel("•");
    dot->setObjectName("headerInfo");

    QLabel *admin = new QLabel(
        QString::fromUtf8("Administrateur  ▼")
        );

    admin->setObjectName("headerInfo");

    l->addWidget(m_dateLabel);
    l->addSpacing(22);
    l->addWidget(m_timeLabel);
    l->addSpacing(22);
    l->addWidget(dot);
    l->addSpacing(8);
    l->addWidget(admin);

    return h;
}

// ==========================================
// HORLOGE
// ==========================================

void WeatherWiseTest::updateClock()
{
    const QDateTime now = QDateTime::currentDateTime();

    m_dateLabel->setText(now.toString("dd/MM/yyyy"));
    m_timeLabel->setText(now.toString("HH:mm"));
}

// ==========================================
// FORMULAIRE
// ==========================================

QWidget *WeatherWiseTest::buildFormCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    // Formulaire plus compact
    v->setContentsMargins(16, 7, 16, 8);
    v->setSpacing(5);

    v->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "⚙  Informations de l'équipement"
                )
            )
        );

    m_id = new QLineEdit;

    m_id->setPlaceholderText(
        QString::fromUtf8(
            "Généré automatiquement si vide (ex. E011)"
            )
        );

    m_nom = new QLineEdit;

    m_loc = new QComboBox;

    QStringList villes;

    villes << "Tunis"
           << "Sfax"
           << "Sousse"
           << "Kairouan"
           << "Gabès"
           << "Tozeur"
           << "Kasserine"
           << "Bizerte"
           << "Gafsa"
           << "Médenine"
           << "Tataouine"
           << "Nabeul"
           << "Monastir"
           << "Mahdia"
           << "Béja"
           << "Jendouba"
           << "Le Kef"
           << "Siliana"
           << "Zaghouan"
           << "Sidi Bouzid"
           << "Kébili";

    m_loc->addItems(villes);

    // ==========================================
    // CHAMP : STATION
    // ==========================================

    m_station = new QComboBox;

    m_station->addItem(
        "Aucune station",
        QString()
        );

    EquipementDAO dao;

    QSqlQueryModel *stationsModel = dao.stations();

    if (stationsModel && !stationsModel->lastError().isValid())
    {
        for (int i = 0; i < stationsModel->rowCount(); ++i)
        {
            QString idStation =
                stationsModel->data(
                                 stationsModel->index(i, 0)
                                 ).toString();

            QString nomStation =
                stationsModel->data(
                                 stationsModel->index(i, 1)
                                 ).toString();

            QString affichage =
                idStation + " - " + nomStation;

            m_station->addItem(
                affichage,
                idStation
                );
        }
    }
    else
    {
        qDebug() << "Erreur chargement stations Oracle:"
                 << (stationsModel
                         ? stationsModel->lastError().text()
                         : "Model null");

        QMessageBox::warning(
            this,
            "Stations Oracle",
            QString::fromUtf8(
                "Impossible de charger la liste des stations depuis Oracle.\n"
                "Vérifiez la connexion et la table STATIONS."
                )
            );
    }

    delete stationsModel;

    m_date = new QDateEdit(QDate::currentDate());

    m_date->setCalendarPopup(true);
    m_date->setDisplayFormat("dd/MM/yyyy");

    m_statut = new QComboBox;

    m_statut->addItem("Actif");
    m_statut->addItem("Inactif");
    m_statut->addItem("En maintenance");

    m_type = new QComboBox;

    m_type->addItem("Station météo");
    m_type->addItem("Thermomètre");
    m_type->addItem("Baromètre");
    m_type->addItem("Anémomètre");
    m_type->addItem("Pluviomètre");
    m_type->addItem("Hygromètre");
    m_type->addItem("Capteur météo");

    QGridLayout *g = new QGridLayout;

    g->setHorizontalSpacing(14);
    g->setVerticalSpacing(5);

    g->setColumnStretch(1, 1);
    g->setColumnStretch(3, 1);

    addField(g, 0, 0, "ID Équipement :", m_id);
    addField(g, 0, 1, "Nom :", m_nom);

    addField(g, 1, 0, "Localisation :", m_loc);
    addField(g, 1, 1, "Station :", m_station);

    addField(g, 2, 0, "Date d'installation :", m_date);
    addField(g, 2, 1, "Statut :", m_statut);

    addField(g, 3, 0, "Type d'équipement :", m_type);

    v->addLayout(g);

    return card;
}

// ==========================================
// TOOLBAR
// ==========================================

QWidget *WeatherWiseTest::buildToolbar()
{
    QFrame *card = makeCard();

    QHBoxLayout *l = new QHBoxLayout(card);

    l->setContentsMargins(14, 6, 14, 6);
    l->setSpacing(8);

    QPushButton *bAdd = new QPushButton(
        QString::fromUtf8("＋  Ajouter")
        );

    bAdd->setObjectName("primary");

    QPushButton *bEdit = new QPushButton(
        QString::fromUtf8("✎  Modifier")
        );

    QPushButton *bDel = new QPushButton(
        QString::fromUtf8("▣  Supprimer")
        );

    bDel->setObjectName("danger");

    QPushButton *bClear = new QPushButton(
        QString::fromUtf8("↻  Vider")
        );

    l->addWidget(bAdd);
    l->addWidget(bEdit);
    l->addWidget(bDel);
    l->addWidget(bClear);

    l->addSpacing(10);

    l->addWidget(
        new QLabel(QString::fromUtf8("↕  Trier par"))
        );

    m_sort = new QComboBox;

    m_sort->addItem("Nom");
    m_sort->addItem("Localisation");
    m_sort->addItem("Statut");
    m_sort->addItem("Type");

    m_sort->setMinimumWidth(180);

    l->addWidget(m_sort);

    QPushButton *bSort = new QPushButton("Trier");

    l->addWidget(bSort);

    m_search = new QLineEdit;

    m_search->setPlaceholderText(
        QString::fromUtf8(
            "Rechercher par ID, nom, localisation, statut, date ou type..."
            )
        );

    l->addWidget(m_search, 1);

    QPushButton *bFind = new QPushButton(
        QString::fromUtf8("🔍")
        );

    l->addWidget(bFind);

    QPushButton *bPdf = new QPushButton(
        QString::fromUtf8("▤  Exporter en PDF")
        );

    l->addWidget(bPdf);

    connect(bAdd, SIGNAL(clicked()),
            this, SLOT(onAdd()));

    connect(bEdit, SIGNAL(clicked()),
            this, SLOT(onEdit()));

    connect(bDel, SIGNAL(clicked()),
            this, SLOT(onDelete()));

    connect(bClear, SIGNAL(clicked()),
            this, SLOT(onClear()));

    connect(bSort, SIGNAL(clicked()),
            this, SLOT(onSort()));

    connect(bFind, SIGNAL(clicked()),
            this, SLOT(onSearch()));

    connect(m_search, SIGNAL(returnPressed()),
            this, SLOT(onSearch()));

    connect(bPdf, SIGNAL(clicked()),
            this, SLOT(exportPdf()));

    return card;
}

// ==========================================
// TABLEAU PRINCIPAL
// ==========================================

QWidget *WeatherWiseTest::buildTableCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(12, 8, 12, 8);
    v->setSpacing(5);

    QHBoxLayout *top = new QHBoxLayout;

    top->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "▤  Liste des équipements"
                )
            )
        );

    top->addStretch();

    QLabel *hint = new QLabel(
        QString::fromUtf8(
            "Sélectionnez une ligne pour modifier les informations"
            )
        );

    hint->setObjectName("hint");

    top->addWidget(hint);

    v->addLayout(top);

    m_table = new QTableWidget(0, 6);

    QStringList headers;

    headers << "ID Équipement"
            << "Nom"
            << "Localisation"
            << "Date d'installation"
            << "Statut"
            << "Type d'équipement";

    m_table->setHorizontalHeaderLabels(headers);

    m_table->verticalHeader()->setDefaultSectionSize(28);

    m_table->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    m_table->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    m_table->setSelectionMode(
        QAbstractItemView::SingleSelection
        );

    m_table->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    m_table->setFocusPolicy(Qt::NoFocus);

    // Tableau plus compact
    m_table->setMinimumHeight(190);

    v->addWidget(m_table, 1);

    connect(
        m_table,
        SIGNAL(itemSelectionChanged()),
        this,
        SLOT(onRowSelected())
        );

    return card;
}

// ==========================================
// CARTE PREDICTION MAINTENANCE
// ==========================================

QWidget *WeatherWiseTest::buildMaintenanceCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(14, 8, 14, 8);
    v->setSpacing(5);

    v->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "🔧  Prédiction de maintenance"
                )
            )
        );

    QLabel *description = new QLabel(
        QString::fromUtf8(
            "Sélectionnez un équipement dans la liste "
            "puis calculez sa prochaine date de maintenance."
            )
        );

    description->setObjectName("hint");
    description->setWordWrap(true);

    v->addWidget(description);

    QPushButton *bCalculate = new QPushButton(
        QString::fromUtf8(
            "Calculer la prédiction"
            )
        );

    bCalculate->setObjectName("primary");
    bCalculate->setMaximumHeight(32);

    v->addWidget(bCalculate);

    m_maintenanceCount = new QLabel(
        QString::fromUtf8(
            "Aucune prédiction calculée."
            )
        );

    m_maintenanceCount->setStyleSheet(
        "font-weight:600; color:#17407e;"
        );

    v->addWidget(m_maintenanceCount);

    m_maintenanceTable = new QTableWidget(0, 6);

    QStringList headers;

    headers << "ID"
            << "Équipement"
            << "Type"
            << "Date installation"
            << "Date prévue"
            << "État de maintenance";

    m_maintenanceTable->setHorizontalHeaderLabels(headers);

    m_maintenanceTable->horizontalHeader()->setSectionResizeMode(
        QHeaderView::Stretch
        );

    m_maintenanceTable->verticalHeader()->setDefaultSectionSize(25);

    m_maintenanceTable->setEditTriggers(
        QAbstractItemView::NoEditTriggers
        );

    m_maintenanceTable->setSelectionBehavior(
        QAbstractItemView::SelectRows
        );

    // Tableau de prédiction compact
    m_maintenanceTable->setMinimumHeight(65);
    m_maintenanceTable->setMaximumHeight(85);

    v->addWidget(m_maintenanceTable);

    connect(
        bCalculate,
        SIGNAL(clicked()),
        this,
        SLOT(onCalculateMaintenance())
        );

    card->setMinimumHeight(190);

    return card;
}

// ==========================================
// CARTE QR CODE
// ==========================================

QWidget *WeatherWiseTest::buildQrCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(12, 8, 12, 8);
    v->setSpacing(5);

    v->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "▦  Code QR de l'équipement"
                )
            )
        );

    QHBoxLayout *body = new QHBoxLayout;
    body->setSpacing(8);

    // Informations à gauche
    QVBoxLayout *left = new QVBoxLayout;

    m_qrInfo = new QLabel(
        QString::fromUtf8(
            "Sélectionnez un équipement dans le tableau "
            "pour afficher automatiquement son code QR."
            )
        );

    m_qrInfo->setObjectName("hint");
    m_qrInfo->setWordWrap(true);

    left->addWidget(m_qrInfo);
    left->addStretch();

    // Aperçu QR plus petit
    m_qrPreview = new QLabel;

    m_qrPreview->setFixedSize(190, 190);
    m_qrPreview->setAlignment(Qt::AlignCenter);

    m_qrPreview->setStyleSheet(
        "background:white;"
        "border:1px solid #e3e8f0;"
        "border-radius:8px;"
        );

    m_qrPreview->setText("QR");

    body->addLayout(left, 1);
    body->addWidget(m_qrPreview, 0, Qt::AlignCenter);

    v->addLayout(body, 1);

    card->setMinimumHeight(215);
    card->setMaximumHeight(225);

    return card;
}

// ==========================================
// CALCULER LA PREDICTION DE MAINTENANCE
// ==========================================

void WeatherWiseTest::onCalculateMaintenance()
{
    if (m_editId.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Aucun équipement sélectionné",
            QString::fromUtf8(
                "Veuillez sélectionner un équipement dans la liste "
                "avant de calculer sa prédiction."
                )
            );

        return;
    }

    int index = -1;

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        if (m_equipements.at(i).getId() == m_editId)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
    {
        QMessageBox::warning(
            this,
            "Erreur",
            "Équipement introuvable."
            );

        return;
    }

    const Equipement &e = m_equipements.at(index);

    QDate dateInstallation = e.getDateInstallation();

    if (!dateInstallation.isValid())
    {
        QMessageBox::warning(
            this,
            "Date invalide",
            QString::fromUtf8(
                "La date d'installation de cet équipement est invalide."
                )
            );

        return;
    }

    int intervalle = intervalMaintenanceMois(e.getType());

    QDate datePrevue = dateInstallation.addMonths(intervalle);

    QDate aujourdHui = QDate::currentDate();

    int joursRestants = aujourdHui.daysTo(datePrevue);

    QString statutPrediction;
    QColor couleur;

    if (datePrevue < aujourdHui)
    {
        if (e.getStatut().trimmed().compare(
                "En maintenance",
                Qt::CaseInsensitive) == 0)
        {
            statutPrediction = "En maintenance";
            couleur = QColor("#f0a000");
        }
        else
        {
            statutPrediction = QString::fromUtf8(
                "Devrait être en maintenance"
                );

            couleur = QColor("#e5474f");
        }
    }
    else if (e.getStatut().trimmed().compare(
                 "En maintenance",
                 Qt::CaseInsensitive) == 0)
    {
        statutPrediction = "En maintenance";
        couleur = QColor("#f0a000");
    }
    else if (joursRestants <= 30)
    {
        statutPrediction = QString::fromUtf8(
            "Maintenance bientôt nécessaire"
            );

        couleur = QColor("#f0a000");
    }
    else
    {
        statutPrediction = QString::fromUtf8(
            "Aucune maintenance"
            );

        couleur = QColor("#16a068");
    }

    m_maintenanceTable->setRowCount(1);

    m_maintenanceTable->setItem(
        0, 0,
        new QTableWidgetItem(e.getId())
        );

    m_maintenanceTable->setItem(
        0, 1,
        new QTableWidgetItem(e.getNom())
        );

    m_maintenanceTable->setItem(
        0, 2,
        new QTableWidgetItem(e.getType())
        );

    m_maintenanceTable->setItem(
        0, 3,
        new QTableWidgetItem(
            dateInstallation.toString("dd/MM/yyyy")
            )
        );

    m_maintenanceTable->setItem(
        0, 4,
        new QTableWidgetItem(
            datePrevue.toString("dd/MM/yyyy")
            )
        );

    QTableWidgetItem *etatItem =
        new QTableWidgetItem(statutPrediction);

    etatItem->setForeground(QBrush(couleur));
    etatItem->setFont(
        QFont("Arial", 9, QFont::Bold)
        );

    m_maintenanceTable->setItem(
        0, 5,
        etatItem
        );

    m_maintenanceCount->setText(
        QString::fromUtf8(
            "Prédiction pour : %1"
            ).arg(e.getNom())
        );

    m_maintenanceTable->resizeRowsToContents();
}

// ==========================================
// GENERER AUTOMATIQUEMENT LE CODE QR
// ==========================================

void WeatherWiseTest::onGenerateQrCode()
{
    if (m_editId.isEmpty())
    {
        if (m_qrPreview)
        {
            m_qrPreview->clear();
            m_qrPreview->setText("QR");
        }

        if (m_qrInfo)
        {
            m_qrInfo->setText(
                QString::fromUtf8(
                    "Sélectionnez un équipement pour afficher son code QR."
                    )
                );
        }

        m_qrImage = QImage();
        return;
    }

    int index = -1;

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        if (m_equipements.at(i).getId() == m_editId)
        {
            index = i;
            break;
        }
    }

    if (index == -1)
        return;

    const Equipement &e = m_equipements.at(index);

    try
    {
        // Le QR contient toutes les informations de l'équipement
        QString informations =
            "INFORMATIONS DE L'EQUIPEMENT\n\n"
            "ID : " + e.getId() + "\n" +
            "Nom : " + e.getNom() + "\n" +
            "Localisation : " + e.getLocalisation() + "\n" +
            "Statut : " + e.getStatut() + "\n" +
            "Date d'installation : " +
            e.getDateInstallation().toString("dd/MM/yyyy") + "\n" +
            "Type : " + e.getType() + "\n" +
            "ID Station : " + e.getIdStation();

        QByteArray qrText = informations.toUtf8();

        qrcodegen::QrCode qr =
            qrcodegen::QrCode::encodeText(
                qrText.constData(),
                qrcodegen::QrCode::Ecc::MEDIUM
                );

        const int quietZone = 4;
        const int scale = 10;
        const int qrSize = qr.getSize();

        const int imageSize =
            (qrSize + quietZone * 2) * scale;

        m_qrImage = QImage(
            imageSize,
            imageSize,
            QImage::Format_RGB32
            );

        m_qrImage.fill(Qt::white);

        // Dessiner les modules noirs du QR
        for (int y = 0; y < qrSize; ++y)
        {
            for (int x = 0; x < qrSize; ++x)
            {
                if (qr.getModule(x, y))
                {
                    for (int py = 0; py < scale; ++py)
                    {
                        for (int px = 0; px < scale; ++px)
                        {
                            m_qrImage.setPixel(
                                (x + quietZone) * scale + px,
                                (y + quietZone) * scale + py,
                                qRgb(0, 0, 0)
                                );
                        }
                    }
                }
            }
        }

        // Afficher le QR dans un aperçu plus petit
        m_qrPreview->setPixmap(
            QPixmap::fromImage(m_qrImage).scaled(
                180,
                180,
                Qt::KeepAspectRatio,
                Qt::FastTransformation
                )
            );

        // Informations affichées à côté du QR
        m_qrInfo->setText(
            QString::fromUtf8(
                "Code QR généré automatiquement.\n\n"
                "Équipement : %1\n\n"
                "Identifiant : %2\n\n"
                "Le QR Code contient les 7 attributs "
                "de l'équipement."
                ).arg(e.getNom()).arg(e.getId())
            );
    }
    catch (const std::exception &ex)
    {
        QMessageBox::critical(
            this,
            "Erreur QR Code",
            QString::fromUtf8(
                "Impossible de générer le code QR : "
                ) + ex.what()
            );
    }
}

// ==========================================
// ENREGISTRER LE QR CODE EN PNG
// Fonction conservée mais aucun bouton ne l'appelle.
// ==========================================

void WeatherWiseTest::onSaveQrCode()
{
    if (m_qrImage.isNull())
    {
        return;
    }

    QString defaultName = "QR_" + m_editId + ".png";

    QString path = QFileDialog::getSaveFileName(
        this,
        "Enregistrer le code QR",
        defaultName,
        "Images PNG (*.png)"
        );

    if (path.isEmpty())
    {
        return;
    }

    if (!path.endsWith(".png", Qt::CaseInsensitive))
    {
        path += ".png";
    }

    if (m_qrImage.save(path, "PNG"))
    {
        QMessageBox::information(
            this,
            "Succès",
            QString::fromUtf8(
                "Code QR enregistré avec succès :\n"
                ) + path
            );
    }
    else
    {
        QMessageBox::critical(
            this,
            "Erreur",
            "Impossible d'enregistrer le fichier PNG."
            );
    }
}

// ==========================================
// ACTUALISER LE TABLEAU PRINCIPAL
// ==========================================

void WeatherWiseTest::refreshTable()
{
    m_table->setRowCount(0);
    m_table->setRowCount(m_equipements.size());

    for (int r = 0; r < m_equipements.size(); ++r)
    {
        const Equipement &e = m_equipements.at(r);

        m_table->setItem(
            r, 0, new QTableWidgetItem(e.getId())
            );

        m_table->setItem(
            r, 1, new QTableWidgetItem(e.getNom())
            );

        m_table->setItem(
            r, 2, new QTableWidgetItem(e.getLocalisation())
            );

        m_table->setItem(
            r, 3,
            new QTableWidgetItem(
                e.getDateInstallation().toString("dd/MM/yyyy")
                )
            );

        QTableWidgetItem *st =
            new QTableWidgetItem(e.getStatut());

        if (e.getStatut() == "Actif")
        {
            st->setForeground(QBrush(QColor("#16a068")));
        }
        else if (e.getStatut() == "Inactif")
        {
            st->setForeground(QBrush(QColor("#e5474f")));
        }
        else
        {
            st->setForeground(QBrush(QColor("#f0a000")));
        }

        m_table->setItem(r, 4, st);

        m_table->setItem(
            r, 5, new QTableWidgetItem(e.getType())
            );
    }

    if (m_maintenanceTable)
    {
        m_maintenanceTable->setRowCount(0);

        m_maintenanceCount->setText(
            QString::fromUtf8(
                "Cliquez sur le bouton pour recalculer les prédictions."
                )
            );
    }

    updateStats();
}

// ==========================================
// STATISTIQUES
// ==========================================

QWidget *WeatherWiseTest::buildStatsCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    // Carte de statistiques plus compacte
    v->setContentsMargins(12, 7, 12, 7);
    v->setSpacing(3);

    v->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "📊  Statistiques des équipements"
                )
            )
        );

    // Graphique plus petit
    m_donut = new EquipementDonutChart;
    m_donut->setFixedSize(100, 100);

    QLabel *dotA = new QLabel(QString::fromUtf8("●"));
    dotA->setStyleSheet(
        "color:#16a068; background:transparent;"
        );

    QLabel *dotI = new QLabel(QString::fromUtf8("●"));
    dotI->setStyleSheet(
        "color:#e5474f; background:transparent;"
        );

    QLabel *dotM = new QLabel(QString::fromUtf8("●"));
    dotM->setStyleSheet(
        "color:#f0a000; background:transparent;"
        );

    m_actifsLabel = new QLabel;
    m_inactifsLabel = new QLabel;
    m_maintenanceLabel = new QLabel;

    QGridLayout *lg = new QGridLayout;

    lg->setHorizontalSpacing(6);
    lg->setVerticalSpacing(4);

    lg->addWidget(dotA, 0, 0);
    lg->addWidget(new QLabel("Actifs"), 0, 1);
    lg->addWidget(m_actifsLabel, 0, 2);

    lg->addWidget(dotI, 1, 0);
    lg->addWidget(new QLabel("Inactifs"), 1, 1);
    lg->addWidget(m_inactifsLabel, 1, 2);

    lg->addWidget(dotM, 2, 0);
    lg->addWidget(new QLabel("Maintenance"), 2, 1);
    lg->addWidget(m_maintenanceLabel, 2, 2);

    QHBoxLayout *body = new QHBoxLayout;

    body->setSpacing(8);
    body->addWidget(m_donut);
    body->addLayout(lg);

    v->addLayout(body);

    card->setMinimumHeight(125);
    card->setMaximumHeight(140);

    return card;
}

// ==========================================
// ACTUALISER LES STATISTIQUES DEPUIS ORACLE
// ==========================================

void WeatherWiseTest::updateStats()
{
    int actifs = 0;
    int inactifs = 0;
    int maintenance = 0;

    EquipementDAO dao;

    QSqlQueryModel *model = dao.statistiquesParStatut();

    if (!model || model->lastError().isValid())
    {
        qDebug() << "Erreur statistiques Oracle:"
                 << (model ? model->lastError().text() : "Model null");

        delete model;
        return;
    }

    for (int i = 0; i < model->rowCount(); ++i)
    {
        QString statut =
            model->data(model->index(i, 0)).toString().trimmed();

        int nombre =
            model->data(model->index(i, 1)).toInt();

        if (statut == "Actif")
        {
            actifs = nombre;
        }
        else if (statut == "Inactif")
        {
            inactifs = nombre;
        }
        else if (statut == "En maintenance")
        {
            maintenance = nombre;
        }
    }

    delete model;

    const int total = actifs + inactifs + maintenance;

    m_donut->setValues(
        actifs,
        inactifs + maintenance
        );

    m_actifsLabel->setText(
        QString("%1 (%2%)")
            .arg(actifs)
            .arg(total ? qRound(100.0 * actifs / total) : 0)
        );

    m_inactifsLabel->setText(
        QString("%1 (%2%)")
            .arg(inactifs)
            .arg(total ? qRound(100.0 * inactifs / total) : 0)
        );

    m_maintenanceLabel->setText(
        QString("%1 (%2%)")
            .arg(maintenance)
            .arg(total ? qRound(100.0 * maintenance / total) : 0)
        );
}

// ==========================================
// LIRE LE FORMULAIRE
// ==========================================

bool WeatherWiseTest::readForm(Equipement &e)
{
    const QString nom = m_nom->text().trimmed();

    if (nom.isEmpty())
    {
        QMessageBox::warning(
            this,
            "Champs obligatoires",
            QString::fromUtf8(
                "Le nom de l'équipement est obligatoire."
                )
            );

        return false;
    }

    e.setNom(nom);
    e.setLocalisation(m_loc->currentText());
    e.setIdStation(m_station->currentData().toString());
    e.setDateInstallation(m_date->date());
    e.setStatut(m_statut->currentText());
    e.setType(m_type->currentText());

    return true;
}

// ==========================================
// AJOUTER
// ==========================================

void WeatherWiseTest::onAdd()
{
    Equipement e;

    if (!readForm(e))
    {
        return;
    }

    QString id = m_id->text().trimmed().toUpper();

    if (id.isEmpty())
    {
        id = nextId();
    }

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        if (m_equipements.at(i).getId() == id)
        {
            QMessageBox::warning(
                this,
                "ID existant",
                QString::fromUtf8(
                    "Un équipement avec l'ID %1 existe déjà."
                    ).arg(id)
                );

            return;
        }
    }

    EquipementDAO dao;

    if (!dao.ajouter(
            id,
            e.getNom(),
            e.getLocalisation(),
            e.getStatut(),
            e.getDateInstallation(),
            e.getType(),
            e.getIdStation()))
    {
        QMessageBox::critical(
            this,
            "Erreur d'insertion Oracle",
            QString::fromUtf8(
                "L'équipement n'a pas été enregistré dans Oracle :\n"
                ) + dao.derniereErreur()
            );

        return;
    }

    e.setId(id);

    m_equipements.append(e);

    onClear();
    refreshTable();

    QMessageBox::information(
        this,
        "Succès",
        QString::fromUtf8(
            "Équipement enregistré dans Oracle."
            )
        );
}

// ==========================================
// MODIFIER
// ==========================================

void WeatherWiseTest::onEdit()
{
    if (m_editId.isEmpty())
    {
        QMessageBox::information(
            this,
            "Modifier",
            QString::fromUtf8(
                "Sélectionnez d'abord une ligne dans la liste."
                )
            );

        return;
    }

    Equipement ne;

    if (!readForm(ne))
    {
        return;
    }

    ne.setId(m_editId);

    EquipementDAO dao;

    if (!dao.modifier(
            ne.getId(),
            ne.getNom(),
            ne.getLocalisation(),
            ne.getStatut(),
            ne.getDateInstallation(),
            ne.getType(),
            ne.getIdStation()))
    {
        QMessageBox::critical(
            this,
            "Erreur de modification Oracle",
            QString::fromUtf8(
                "La modification n'a pas été confirmée dans Oracle :\n"
                ) + dao.derniereErreur()
            );

        return;
    }

    bool found = false;

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        if (m_equipements.at(i).getId() == m_editId)
        {
            m_equipements[i] = ne;
            found = true;
            break;
        }
    }

    if (!found)
    {
        m_equipements.append(ne);
    }

    onClear();
    refreshTable();

    QMessageBox::information(
        this,
        "Succès",
        QString::fromUtf8(
            "Équipement modifié dans Oracle."
            )
        );
}

// ==========================================
// SUPPRIMER
// ==========================================

void WeatherWiseTest::onDelete()
{
    if (m_editId.isEmpty())
    {
        QMessageBox::information(
            this,
            "Supprimer",
            QString::fromUtf8(
                "Sélectionnez d'abord une ligne dans la liste."
                )
            );

        return;
    }

    const QMessageBox::StandardButton rep =
        QMessageBox::question(
            this,
            "Confirmation",
            QString::fromUtf8(
                "Supprimer l'équipement %1 ?"
                ).arg(m_editId)
            );

    if (rep != QMessageBox::Yes)
    {
        return;
    }

    EquipementDAO dao;

    if (!dao.supprimer(m_editId))
    {
        QMessageBox::critical(
            this,
            "Erreur de suppression Oracle",
            QString::fromUtf8(
                "La suppression n'a pas été confirmée dans Oracle :\n"
                ) + dao.derniereErreur()
            );

        return;
    }

    for (int i = m_equipements.size() - 1; i >= 0; --i)
    {
        if (m_equipements.at(i).getId() == m_editId)
        {
            m_equipements.remove(i);
        }
    }

    onClear();
    refreshTable();

    QMessageBox::information(
        this,
        "Succès",
        QString::fromUtf8(
            "Équipement supprimé d'Oracle."
            )
        );
}

// ==========================================
// VIDER
// ==========================================

void WeatherWiseTest::onClear()
{
    m_editId.clear();

    m_id->clear();
    m_id->setReadOnly(false);

    m_nom->clear();

    m_loc->setCurrentIndex(0);

    m_station->setCurrentIndex(0);

    m_date->setDate(QDate::currentDate());

    m_statut->setCurrentIndex(0);

    m_type->setCurrentIndex(0);

    m_table->clearSelection();

    // Effacer le QR Code précédent
    if (m_qrPreview)
    {
        m_qrPreview->clear();
        m_qrPreview->setText("QR");
    }

    if (m_qrInfo)
    {
        m_qrInfo->setText(
            QString::fromUtf8(
                "Sélectionnez un équipement pour afficher son code QR."
                )
            );
    }

    m_qrImage = QImage();
}

// ==========================================
// TRI
// ==========================================

void WeatherWiseTest::onSort()
{
    EquipementDAO dao;

    QSqlQueryModel *model = dao.trier(m_sort->currentText());

    if (!model || model->lastError().isValid())
    {
        qDebug() << "Erreur tri :"
                 << (model ? model->lastError().text() : "Model null");

        delete model;
        return;
    }

    m_table->setRowCount(0);
    m_table->setRowCount(model->rowCount());

    for (int i = 0; i < model->rowCount(); ++i)
    {
        m_table->setItem(
            i, 0,
            new QTableWidgetItem(
                model->data(model->index(i, 0)).toString()
                )
            );

        m_table->setItem(
            i, 1,
            new QTableWidgetItem(
                model->data(model->index(i, 1)).toString()
                )
            );

        m_table->setItem(
            i, 2,
            new QTableWidgetItem(
                model->data(model->index(i, 2)).toString()
                )
            );

        QVariant dateValue = model->data(model->index(i, 4));

        QDate date = convertirDate(dateValue);

        m_table->setItem(
            i, 3,
            new QTableWidgetItem(
                date.isValid() ? date.toString("dd/MM/yyyy")
                               : dateValue.toString()
                )
            );

        QTableWidgetItem *st = new QTableWidgetItem(
            model->data(model->index(i, 3)).toString()
            );

        if (st->text() == "Actif")
        {
            st->setForeground(QBrush(QColor("#16a068")));
        }
        else if (st->text() == "Inactif")
        {
            st->setForeground(QBrush(QColor("#e5474f")));
        }
        else
        {
            st->setForeground(QBrush(QColor("#f0a000")));
        }

        m_table->setItem(i, 4, st);

        m_table->setItem(
            i, 5,
            new QTableWidgetItem(
                model->data(model->index(i, 5)).toString()
                )
            );
    }

    delete model;
}

// ==========================================
// RECHERCHE
// ==========================================

void WeatherWiseTest::onSearch()
{
    EquipementDAO dao;

    QSqlQueryModel *model = dao.rechercher(m_search->text());

    if (!model || model->lastError().isValid())
    {
        qDebug() << "Erreur recherche :"
                 << (model ? model->lastError().text() : "Model null");

        delete model;
        return;
    }

    m_table->setRowCount(0);
    m_table->setRowCount(model->rowCount());

    for (int i = 0; i < model->rowCount(); ++i)
    {
        m_table->setItem(
            i, 0,
            new QTableWidgetItem(
                model->data(model->index(i, 0)).toString()
                )
            );

        m_table->setItem(
            i, 1,
            new QTableWidgetItem(
                model->data(model->index(i, 1)).toString()
                )
            );

        m_table->setItem(
            i, 2,
            new QTableWidgetItem(
                model->data(model->index(i, 2)).toString()
                )
            );

        QVariant dateValue = model->data(model->index(i, 4));

        QDate date = convertirDate(dateValue);

        m_table->setItem(
            i, 3,
            new QTableWidgetItem(
                date.isValid() ? date.toString("dd/MM/yyyy")
                               : dateValue.toString()
                )
            );

        QTableWidgetItem *st = new QTableWidgetItem(
            model->data(model->index(i, 3)).toString()
            );

        if (st->text() == "Actif")
        {
            st->setForeground(QBrush(QColor("#16a068")));
        }
        else if (st->text() == "Inactif")
        {
            st->setForeground(QBrush(QColor("#e5474f")));
        }
        else
        {
            st->setForeground(QBrush(QColor("#f0a000")));
        }

        m_table->setItem(i, 4, st);

        m_table->setItem(
            i, 5,
            new QTableWidgetItem(
                model->data(model->index(i, 5)).toString()
                )
            );
    }

    delete model;
}

// ==========================================
// SELECTION D'UNE LIGNE
// ==========================================

void WeatherWiseTest::onRowSelected()
{
    const QList<QTableWidgetItem *> sel =
        m_table->selectedItems();

    if (sel.isEmpty())
    {
        return;
    }

    QTableWidgetItem *first =
        m_table->item(sel.first()->row(), 0);

    if (!first)
    {
        return;
    }

    const QString id = first->text();

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        const Equipement &e = m_equipements.at(i);

        if (e.getId() != id)
        {
            continue;
        }

        m_editId = id;

        m_id->setText(e.getId());
        m_id->setReadOnly(true);

        m_nom->setText(e.getNom());

        m_loc->setCurrentText(e.getLocalisation());

        int stationIndex =
            m_station->findData(e.getIdStation());

        if (stationIndex >= 0)
        {
            m_station->setCurrentIndex(stationIndex);
        }
        else
        {
            m_station->setCurrentIndex(0);
        }

        m_date->setDate(e.getDateInstallation());

        m_statut->setCurrentText(e.getStatut());

        m_type->setCurrentText(e.getType());

        // Générer automatiquement le QR du nouvel équipement sélectionné
        onGenerateQrCode();

        return;
    }
}

// ==========================================
// EXPORT PDF DEPUIS ORACLE
// ==========================================

void WeatherWiseTest::exportPdf()
{
    const QString path =
        QFileDialog::getSaveFileName(
            this,
            "Exporter en PDF",
            "equipements.pdf",
            "PDF (*.pdf)"
            );

    if (path.isEmpty())
    {
        return;
    }

    EquipementDAO dao;

    QSqlQueryModel *model = dao.donneesPDF();

    if (!model || model->lastError().isValid())
    {
        QString message = model
                              ? model->lastError().text()
                              : QString::fromUtf8("Impossible de récupérer les données.");

        delete model;

        QMessageBox::critical(
            this,
            "Erreur Oracle",
            QString::fromUtf8(
                "Impossible de récupérer les équipements pour le PDF :\n"
                ) + message
            );

        return;
    }

    QString html = QString::fromUtf8(
        "<h2 style='color:#0f3d91'>"
        "Liste des équipements"
        "</h2>"
        "<table border='1' "
        "cellspacing='0' "
        "cellpadding='6' "
        "width='100%'>"
        "<tr style='background:#e8f0fc'>"
        "<th>ID</th>"
        "<th>Nom</th>"
        "<th>Localisation</th>"
        "<th>Date d'installation</th>"
        "<th>Statut</th>"
        "<th>Type</th>"
        "</tr>"
        );

    for (int i = 0; i < model->rowCount(); ++i)
    {
        QString id =
            model->data(model->index(i, 0)).toString();

        QString nom =
            model->data(model->index(i, 1)).toString();

        QString localisation =
            model->data(model->index(i, 2)).toString();

        QVariant dateValue =
            model->data(model->index(i, 3));

        QDate date = convertirDate(dateValue);

        QString dateText;

        if (date.isValid())
        {
            dateText = date.toString("dd/MM/yyyy");
        }
        else
        {
            dateText = dateValue.toString();
        }

        QString statut =
            model->data(model->index(i, 4)).toString();

        QString type =
            model->data(model->index(i, 5)).toString();

        html += QString(
                    "<tr>"
                    "<td>%1</td>"
                    "<td>%2</td>"
                    "<td>%3</td>"
                    "<td>%4</td>"
                    "<td>%5</td>"
                    "<td>%6</td>"
                    "</tr>"
                    )
                    .arg(id.toHtmlEscaped())
                    .arg(nom.toHtmlEscaped())
                    .arg(localisation.toHtmlEscaped())
                    .arg(dateText.toHtmlEscaped())
                    .arg(statut.toHtmlEscaped())
                    .arg(type.toHtmlEscaped());
    }

    html += "</table>";

    delete model;

    QPdfWriter writer(path);

    writer.setPageSize(QPageSize(QPageSize::A4));
    writer.setPageOrientation(QPageLayout::Landscape);

    QTextDocument doc;

    doc.setHtml(html);
    doc.print(&writer);

    QMessageBox::information(
        this,
        "Export",
        QString::fromUtf8(
            "PDF exporté avec succès depuis Oracle :\n"
            ) + path
        );
}

// ==========================================
// STYLE
// ==========================================

QString WeatherWiseTest::styleSheet()
{
    return QString::fromUtf8(
        R"QSS(
QMainWindow, #content { background: #f4f7fb; }

#sidebar { background: #17407e; }
#sunIcon { color: #ffc61a; font-size: 46px; background: transparent; }
#brand { color: white; font-size: 17px; font-weight: 700; background: transparent; }

QPushButton#nav {
    color: white;
    text-align: left;
    padding: 13px 22px;
    border: none;
    border-radius: 8px;
    font-size: 14px;
    background: transparent;
    margin: 0px 6px;
}
QPushButton#nav:hover { background: rgba(255,255,255,0.08); }
QPushButton#nav[active="true"] { background: #2f78d1; font-weight: 700; }

#header { background: #2e7ccc; }
#headerTitle { color: white; font-size: 26px; font-weight: 700; background: transparent; }
#headerSub { color: #e3eefc; font-size: 13px; background: transparent; }
#headerInfo { color: white; font-size: 14px; font-weight: 700; background: transparent; }

#card { background: white; border: 1px solid #e3e8f0; border-radius: 10px; }
#card QLabel { background: transparent; color: #24344f; }
#card #cardTitle { font-size: 16px; font-weight: 700; color: #0f2a52; }
#card #hint { font-size: 12px; color: #7b8aa3; }

QPushButton {
    background: white;
    border: 1px solid #cfd8e6;
    border-radius: 6px;
    padding: 8px 16px;
    color: #24344f;
    font-size: 14px;
}
QPushButton:hover { background: #f1f5fb; }
QPushButton#primary { background: #1d63d8; color: white; border: none; font-weight: 600; }
QPushButton#primary:hover { background: #1852b8; }
QPushButton#danger { color: #d93a44; font-weight: 600; }

QLineEdit, QComboBox, QDateEdit {
    background: white;
    border: 1px solid #cfd8e6;
    border-radius: 6px;
    padding: 4px 10px;
    min-height: 26px;
    color: #24344f;
    font-size: 14px;
}
QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #1d63d8; }
QLineEdit:read-only { background: #f4f7fb; }

QTableWidget {
    background: white;
    border: 1px solid #e3e8f0;
    gridline-color: #e8edf5;
    alternate-background-color: #f8fafd;
    selection-background-color: #dbe9fb;
    selection-color: #24344f;
    color: #24344f;
    font-size: 14px;
}
QTableWidget::item { padding-left: 6px; }

QHeaderView::section {
    background: #f1f5fb;
    color: #24344f;
    border: none;
    border-right: 1px solid #e3e8f0;
    border-bottom: 1px solid #e3e8f0;
    padding: 8px 6px;
    font-weight: 700;
}
QTableCornerButton::section { background: #f1f5fb; border: none; }
)QSS"
        );
}