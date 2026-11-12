
#include "weatherwisetest.h"
#include "equipement.h"

#include <QSqlQueryModel>
#include <QSqlError>
#include <QDebug>
#include <QAbstractItemModel>
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
#include <QTimeZone>
#include <QTimer>
#include <QVBoxLayout>
#include <QVariant>
#include <QBrush>
#include <algorithm>

// ==========================================
// PAYS UTILISE
// ==========================================

static const QLocale::Territory kForcedCountry = QLocale::Tunisia;


// ==========================================
// DONNEES GEOGRAPHIQUES
// ==========================================

#define COUNT(a) int(sizeof(a) / sizeof(a[0]))

// TUNISIE

static const double TN_OUT[][2] = {
    {37.00, 8.60}, {37.35, 9.80}, {37.10, 11.00},
    {36.40, 10.60}, {35.70, 10.90}, {35.20, 11.10},
    {34.30, 10.30}, {33.90, 10.10}, {33.30, 11.20},
    {33.10, 11.50}, {32.40, 11.20}, {31.70, 10.30},
    {30.30, 9.50}, {32.00, 9.00}, {32.90, 8.30},
    {33.90, 7.50}, {34.50, 8.30}, {35.90, 8.30},
    {36.90, 8.40}
};

static const char *TN_NAMES[] = {
    "Tunis", "Sfax", "Sousse", "Kairouan", "Gabès",
    "Tozeur", "Kasserine", "Bizerte", "Gafsa",
    "Médenine", "Tataouine", "Nabeul", "Monastir",
    "Mahdia", "Béja", "Jendouba", "Le Kef",
    "Siliana", "Zaghouan", "Sidi Bouzid", "Kébili"
};

static const double TN_LAT[] = {
    36.80, 34.74, 35.83, 35.68, 33.88, 33.92,
    35.17, 37.27, 34.43, 33.35, 32.93, 36.45,
    35.78, 35.50, 36.73, 36.50, 36.17, 36.08,
    36.40, 35.04, 33.70
};

static const double TN_LON[] = {
    10.18, 10.76, 10.64, 10.10, 10.10, 8.13,
    8.83, 9.87, 8.78, 10.50, 10.45, 10.74,
    10.83, 11.06, 9.18, 8.78, 8.71, 9.37,
    10.14, 9.49, 8.97
};


// ALGERIE

static const double DZ_OUT[][2] = {
    {35.1, -2.2}, {36.5, 1.0}, {36.8, 3.0},
    {36.9, 5.5}, {37.1, 8.6}, {34.6, 8.3},
    {33.2, 8.2}, {30.2, 9.5}, {27.2, 9.9},
    {25.0, 11.9}, {23.5, 11.9}, {19.1, 12.2},
    {19.1, 4.3}, {21.6, 1.0}, {21.0, -3.0},
    {24.9, -4.8}, {27.3, -8.7}, {29.5, -8.7},
    {32.0, -2.9}
};

static const char *DZ_NAMES[] = {
    "Alger", "Oran", "Constantine", "Annaba", "Sétif",
    "Batna", "Biskra", "Ghardaïa", "Tamanrasset",
    "Béchar", "Tlemcen"
};

static const double DZ_LAT[] = {
    36.75, 35.70, 36.37, 36.90, 36.19,
    35.56, 34.85, 32.49, 22.79, 31.62, 34.88
};

static const double DZ_LON[] = {
    3.06, -0.63, 6.61, 7.77, 5.41,
    6.17, 5.73, 3.67, 5.52, -2.22, -1.32
};


// MAROC

static const double MA_OUT[][2] = {
    {35.8, -5.9}, {35.2, -2.9}, {34.7, -1.9},
    {32.1, -1.2}, {31.0, -3.5}, {30.0, -5.5},
    {28.5, -8.7}, {27.7, -8.7}, {27.7, -12.9},
    {29.0, -10.0}, {30.4, -9.8}, {32.0, -9.3},
    {33.6, -7.6}, {35.0, -6.5}
};

static const char *MA_NAMES[] = {
    "Rabat", "Casablanca", "Marrakech", "Fès",
    "Tanger", "Agadir", "Oujda", "Meknès",
    "Ouarzazate"
};

static const double MA_LAT[] = {
    34.02, 33.57, 31.63, 34.03, 35.76,
    30.43, 34.68, 33.89, 30.92
};

static const double MA_LON[] = {
    -6.84, -7.59, -8.01, -5.00, -5.80,
    -9.60, -1.91, -5.55, -6.89
};


// LIBYE

static const double LY_OUT[][2] = {
    {33.1, 11.5}, {32.9, 13.0}, {32.5, 15.2},
    {30.3, 18.5}, {32.6, 20.0}, {32.9, 22.0},
    {31.6, 25.1}, {22.0, 25.0}, {20.0, 24.0},
    {21.9, 15.7}, {23.5, 14.0}, {23.5, 11.9},
    {25.0, 11.9}, {27.2, 9.9}, {30.2, 9.5},
    {32.2, 10.4}
};

static const char *LY_NAMES[] = {
    "Tripoli", "Benghazi", "Misrata", "Sebha",
    "Tobrouk", "Syrte", "Ghadamès", "Al Bayda"
};

static const double LY_LAT[] = {
    32.89, 32.12, 32.38, 27.04,
    32.08, 31.21, 30.13, 32.77
};

static const double LY_LON[] = {
    13.19, 20.07, 15.09, 14.43,
    23.98, 16.59, 9.50, 21.75
};


// EGYPTE

static const double EG_OUT[][2] = {
    {31.6, 25.1}, {31.3, 29.0}, {31.5, 31.0},
    {31.2, 32.3}, {31.2, 34.2}, {29.5, 34.9},
    {27.8, 34.3}, {25.5, 34.8}, {24.1, 35.6},
    {22.0, 36.9}, {22.0, 25.0}
};

static const char *EG_NAMES[] = {
    "Le Caire", "Alexandrie", "Louxor", "Assouan",
    "Port-Saïd", "Suez", "Hurghada",
    "Charm el-Cheikh", "Marsa Matruh", "Assiout"
};

static const double EG_LAT[] = {
    30.04, 31.20, 25.69, 24.09, 31.27,
    29.97, 27.26, 27.91, 31.35, 27.18
};

static const double EG_LON[] = {
    31.24, 29.92, 32.64, 32.90, 32.30,
    32.55, 33.81, 34.33, 27.24, 31.18
};


// FRANCE

static const double FR_OUT[][2] = {
    {51.0, 2.5}, {50.1, 1.6}, {49.4, 0.2},
    {49.7, -1.6}, {48.7, -1.6}, {48.7, -4.5},
    {47.9, -4.7}, {47.3, -2.5}, {46.2, -1.2},
    {44.6, -1.2}, {43.4, -1.8}, {42.7, 0.0},
    {42.4, 3.2}, {43.5, 4.0}, {43.3, 5.4},
    {43.1, 6.5}, {43.8, 7.5}, {45.1, 6.9},
    {46.0, 7.0}, {46.4, 6.1}, {47.6, 7.5},
    {48.9, 8.2}, {49.5, 6.4}, {49.5, 5.5},
    {50.1, 4.8}, {50.7, 3.2}
};

static const char *FR_NAMES[] = {
    "Paris", "Marseille", "Lyon", "Toulouse", "Nice",
    "Nantes", "Strasbourg", "Bordeaux", "Lille", "Rennes"
};

static const double FR_LAT[] = {
    48.86, 43.30, 45.76, 43.60, 43.70,
    47.22, 48.57, 44.84, 50.63, 48.11
};

static const double FR_LON[] = {
    2.35, 5.37, 4.84, 1.44, 7.27,
    -1.55, 7.75, -0.58, 3.06, -1.68
};


// ==========================================
// REMPLISSAGE DES DONNEES GEOGRAPHIQUES
// ==========================================

static void fillGeo(
    EquipementCountryGeo &g,
    const char *nom,
    const double (*out)[2],
    int nOut,
    const char **names,
    const double *lat,
    const double *lon,
    int nC)
{
    g = EquipementCountryGeo();
    g.nom = QString::fromUtf8(nom);

    for (int i = 0; i < nOut; ++i)
    {
        g.contour.append(
            QPointF(out[i][0], out[i][1])
            );
    }

    for (int i = 0; i < nC; ++i)
    {
        g.addVille(
            QString::fromUtf8(names[i]),
            lat[i],
            lon[i]
            );
    }
}


// ==========================================
// CONSTRUCTION DE LA CARTE
// ==========================================

static bool buildGeo(
    QLocale::Territory t,
    EquipementCountryGeo &g)
{
    switch (t)
    {
    case QLocale::Tunisia:
        fillGeo(g, "Tunisie", TN_OUT, COUNT(TN_OUT),
                TN_NAMES, TN_LAT, TN_LON, COUNT(TN_NAMES));
        return true;

    case QLocale::Algeria:
        fillGeo(g, "Algérie", DZ_OUT, COUNT(DZ_OUT),
                DZ_NAMES, DZ_LAT, DZ_LON, COUNT(DZ_NAMES));
        return true;

    case QLocale::Morocco:
        fillGeo(g, "Maroc", MA_OUT, COUNT(MA_OUT),
                MA_NAMES, MA_LAT, MA_LON, COUNT(MA_NAMES));
        return true;

    case QLocale::Libya:
        fillGeo(g, "Libye", LY_OUT, COUNT(LY_OUT),
                LY_NAMES, LY_LAT, LY_LON, COUNT(LY_NAMES));
        return true;

    case QLocale::Egypt:
        fillGeo(g, "Égypte", EG_OUT, COUNT(EG_OUT),
                EG_NAMES, EG_LAT, EG_LON, COUNT(EG_NAMES));
        return true;

    case QLocale::France:
        fillGeo(g, "France", FR_OUT, COUNT(FR_OUT),
                FR_NAMES, FR_LAT, FR_LON, COUNT(FR_NAMES));
        return true;

    default:
        return false;
    }
}


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


// ==========================================
// FONCTIONS DE TRI
// ==========================================

static bool lessNom(
    const Equipement &a,
    const Equipement &b)
{
    return a.getNom().localeAwareCompare(b.getNom()) < 0;
}

static bool lessLocalisation(
    const Equipement &a,
    const Equipement &b)
{
    return a.getLocalisation().localeAwareCompare(
               b.getLocalisation()) < 0;
}

static bool lessStatut(
    const Equipement &a,
    const Equipement &b)
{
    return a.getStatut().localeAwareCompare(b.getStatut()) < 0;
}

static bool lessType(
    const Equipement &a,
    const Equipement &b)
{
    return a.getType().localeAwareCompare(b.getType()) < 0;
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
    m_map(0),
    m_dateLabel(0),
    m_timeLabel(0),
    m_id(0),
    m_nom(0),
    m_loc(0),
    m_date(0),
    m_statut(0),
    m_type(0)
{
    setWindowTitle(
        "Smart Weather Management - Gestion des équipements"
        );

    setStyleSheet(styleSheet());
    detectCountry();

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

    QWidget *content = new QWidget;
    content->setObjectName("content");

    QGridLayout *g = new QGridLayout(content);

    g->setContentsMargins(20, 12, 20, 12);
    g->setSpacing(12);

    g->addWidget(buildFormCard(), 0, 0, 1, 2);
    g->addWidget(buildToolbar(), 1, 0, 1, 2);
    g->addWidget(buildTableCard(), 2, 0);

    QWidget *col = new QWidget;
    QVBoxLayout *cv = new QVBoxLayout(col);

    cv->setContentsMargins(0, 0, 0, 0);
    cv->setSpacing(12);

    cv->addWidget(buildStatsCard(), 1);
    cv->addWidget(buildMapCard(), 1);

    g->addWidget(col, 2, 1);

    g->setColumnStretch(0, 3);
    g->setColumnStretch(1, 2);
    g->setRowStretch(2, 1);

    rv->addWidget(content, 1);
    root->addWidget(right, 1);

    setCentralWidget(central);


    // Charger les équipements depuis Oracle

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

            QDate date = dateValue.toDate();

            if (!date.isValid())
            {
                const QString dateText = dateValue.toString();

                date = QDate::fromString(dateText, "yyyy-MM-dd");

                if (!date.isValid())
                {
                    date = QDate::fromString(
                        dateText, "dd/MM/yyyy"
                        );
                }

                if (!date.isValid())
                {
                    date = QDate::fromString(
                        dateText.left(10), "yyyy-MM-dd"
                        );
                }
            }

            e.setDateInstallation(date);

            e.setType(
                model->data(model->index(row, 5)).toString()
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
// PAYS
// ==========================================

void WeatherWiseTest::detectCountry()
{
    QLocale::Territory cand[3];

    cand[0] = kForcedCountry;
    cand[1] = QLocale::system().territory();
    cand[2] = QTimeZone::systemTimeZone().territory();

    for (int i = 0; i < 3; ++i)
    {
        if (cand[i] != QLocale::AnyTerritory &&
            buildGeo(cand[i], m_geo))
        {
            return;
        }
    }

    buildGeo(QLocale::Tunisia, m_geo);

    QLocale::Territory detected =
        cand[1] != QLocale::AnyTerritory ? cand[1] : cand[2];

    m_geoNote =
        QString::fromUtf8(" (carte de %1 indisponible)")
            .arg(QLocale::territoryToString(detected));
}


// ==========================================
// DONNEES EXEMPLES
// ==========================================

void WeatherWiseTest::loadSampleData()
{
    m_equipements.clear();

    // Informations des équipements
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

    const int villes[] = {
        0, 1, 2, 3, 4, 5, 6, 7, 8, 9
    };

    const char *statuts[] = {
        "Actif",
        "Actif",
        "Actif",
        "Inactif",
        "Actif",
        "Actif",
        "Inactif",
        "Actif",
        "En maintenance",
        "Actif"
    };

    const char *types[] = {
        "Station météo",
        "Thermomètre",
        "Baromètre",
        "Anémomètre",
        "Pluviomètre",
        "Hygromètre",
        "Capteur météo",
        "Station météo",
        "Capteur météo",
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

        e.setNom(
            QString::fromUtf8(noms[i])
            );

        if (!m_geo.villes.isEmpty() &&
            villes[i] < m_geo.villes.size())
        {
            e.setLocalisation(
                m_geo.villes.at(villes[i])
                );
        }
        else
        {
            e.setLocalisation("Tunis");
        }

        e.setStatut(
            QString::fromUtf8(statuts[i])
            );

        e.setType(
            QString::fromUtf8(types[i])
            );

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

    QLabel *t = new QLabel(
        "Gestion des équipements"
        );

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

    m_dateLabel->setText(
        now.toString("dd/MM/yyyy")
        );

    m_timeLabel->setText(
        now.toString("HH:mm")
        );
}


// ==========================================
// FORMULAIRE
// ==========================================

QWidget *WeatherWiseTest::buildFormCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(20, 10, 20, 12);
    v->setSpacing(8);

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
    m_loc->addItems(m_geo.villes);

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
    g->setVerticalSpacing(8);

    g->setColumnStretch(1, 1);
    g->setColumnStretch(3, 1);

    addField(g, 0, 0, "ID Équipement :", m_id);
    addField(g, 0, 1, "Nom :", m_nom);
    addField(g, 1, 0, "Localisation :", m_loc);
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

    l->setContentsMargins(14, 8, 14, 8);
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

    connect(m_sort, SIGNAL(currentIndexChanged(int)),
            this, SLOT(onSort()));

    connect(bFind, SIGNAL(clicked()),
            this, SLOT(onSearch()));

    connect(m_search, SIGNAL(returnPressed()),
            this, SLOT(onSearch()));

    connect(m_search, SIGNAL(textChanged(QString)),
            this, SLOT(onSearch()));

    connect(bPdf, SIGNAL(clicked()),
            this, SLOT(exportPdf()));

    return card;
}


// ==========================================
// TABLEAU
// ==========================================

QWidget *WeatherWiseTest::buildTableCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(16, 14, 16, 14);
    v->setSpacing(8);

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

    // Six colonnes pour six informations
    m_table = new QTableWidget(0, 6);

    QStringList headers;

    headers << "ID Équipement"
            << "Nom"
            << "Localisation"
            << "Date d'installation"
            << "Statut"
            << "Type d'équipement";

    m_table->setHorizontalHeaderLabels(headers);

    m_table->verticalHeader()->setDefaultSectionSize(34);

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
    m_table->setMinimumHeight(120);

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
// ACTUALISER LE TABLEAU
// ==========================================

void WeatherWiseTest::refreshTable()
{
    const QString q = m_search->text().trimmed();

    QVector<Equipement> list;

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        const Equipement &e = m_equipements.at(i);

        const QString date =
            e.getDateInstallation().toString("dd/MM/yyyy");

        bool match =
            q.isEmpty()
            || e.getId().contains(q, Qt::CaseInsensitive)
            || e.getNom().contains(q, Qt::CaseInsensitive)
            || e.getLocalisation().contains(q, Qt::CaseInsensitive)
            || e.getStatut().contains(q, Qt::CaseInsensitive)
            || date.contains(q, Qt::CaseInsensitive)
            || e.getType().contains(q, Qt::CaseInsensitive);

        if (match)
        {
            list.append(e);
        }
    }

    if (m_sortMode == 0)
    {
        std::sort(list.begin(), list.end(), lessNom);
    }
    else if (m_sortMode == 1)
    {
        std::sort(list.begin(), list.end(), lessLocalisation);
    }
    else if (m_sortMode == 2)
    {
        std::sort(list.begin(), list.end(), lessStatut);
    }
    else if (m_sortMode == 3)
    {
        std::sort(list.begin(), list.end(), lessType);
    }

    m_table->setRowCount(0);
    m_table->setRowCount(list.size());

    for (int r = 0; r < list.size(); ++r)
    {
        const Equipement &e = list.at(r);

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
            st->setForeground(
                QBrush(QColor("#16a068"))
                );
        }
        else if (e.getStatut() == "Inactif")
        {
            st->setForeground(
                QBrush(QColor("#e5474f"))
                );
        }
        else
        {
            st->setForeground(
                QBrush(QColor("#f0a000"))
                );
        }

        m_table->setItem(r, 4, st);

        m_table->setItem(
            r, 5, new QTableWidgetItem(e.getType())
            );
    }

    updateStats();

    m_map->setEquipements(m_equipements);
}


// ==========================================
// STATISTIQUES
// ==========================================

QWidget *WeatherWiseTest::buildStatsCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(16, 10, 16, 10);
    v->setSpacing(6);

    v->addWidget(
        makeCardTitle(
            QString::fromUtf8(
                "📊  Statistiques des équipements"
                )
            )
        );

    m_donut = new EquipementDonutChart;

    QLabel *dotA = new QLabel(
        QString::fromUtf8("●")
        );

    dotA->setStyleSheet(
        "color:#16a068; background:transparent;"
        );

    QLabel *dotI = new QLabel(
        QString::fromUtf8("●")
        );

    dotI->setStyleSheet(
        "color:#e5474f; background:transparent;"
        );

    QLabel *dotM = new QLabel(
        QString::fromUtf8("●")
        );

    dotM->setStyleSheet(
        "color:#f0a000; background:transparent;"
        );

    m_actifsLabel = new QLabel;
    m_inactifsLabel = new QLabel;
    m_maintenanceLabel = new QLabel;

    QGridLayout *lg = new QGridLayout;

    lg->setHorizontalSpacing(10);
    lg->setVerticalSpacing(8);

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

    body->addWidget(m_donut, 1);
    body->addLayout(lg);

    v->addLayout(body, 1);

    return card;
}


// ==========================================
// ACTUALISER LES STATISTIQUES
// ==========================================

void WeatherWiseTest::updateStats()
{
    int actifs = 0;
    int inactifs = 0;
    int maintenance = 0;

    for (int k = 0; k < m_equipements.size(); ++k)
    {
        const Equipement &e = m_equipements.at(k);

        if (e.getStatut() == "Actif")
        {
            ++actifs;
        }
        else if (e.getStatut() == "Inactif")
        {
            ++inactifs;
        }
        else
        {
            ++maintenance;
        }
    }

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
// CARTE GEOGRAPHIQUE
// ==========================================

QWidget *WeatherWiseTest::buildMapCard()
{
    QFrame *card = makeCard();

    QVBoxLayout *v = new QVBoxLayout(card);

    v->setContentsMargins(14, 12, 14, 14);

    QLabel *mt = makeCardTitle(
        QString::fromUtf8(
            "📍  Localisation géographique – "
            ) + m_geo.nom + m_geoNote
        );

    mt->setWordWrap(true);

    v->addWidget(mt);

    m_map = new EquipementMapWidget;

    m_map->setCountry(m_geo);

    v->addWidget(m_map, 1);

    return card;
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
            e.getType()))
    {
        QMessageBox::critical(
            this,
            "Erreur d'insertion Oracle",
            QString::fromUtf8(
                "L'équipement n'a pas été enregistré dans Oracle.\n"
                "Vérifiez Application Output pour le détail de l'erreur."
                )
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
            ne.getType()))
    {
        QMessageBox::critical(
            this,
            "Erreur de modification Oracle",
            QString::fromUtf8(
                "La modification n'a pas été confirmée dans Oracle.\n"
                "Vérifiez que l'ID existe et consultez Application Output."
                )
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
                "La suppression n'a pas été confirmée dans Oracle.\n"
                "Vérifiez que l'ID existe et consultez Application Output."
                )
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

    m_date->setDate(QDate::currentDate());

    m_statut->setCurrentIndex(0);

    m_type->setCurrentIndex(0);

    m_table->clearSelection();
}


// ==========================================
// TRI
// ==========================================

void WeatherWiseTest::onSort()
{
    m_sortMode = m_sort->currentIndex();

    refreshTable();
}


// ==========================================
// RECHERCHE
// ==========================================

void WeatherWiseTest::onSearch()
{
    refreshTable();
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

        m_loc->setCurrentText(
            e.getLocalisation()
            );

        m_date->setDate(
            e.getDateInstallation()
            );

        m_statut->setCurrentText(
            e.getStatut()
            );

        m_type->setCurrentText(
            e.getType()
            );

        return;
    }
}


// ==========================================
// EXPORT PDF
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

    for (int i = 0; i < m_equipements.size(); ++i)
    {
        const Equipement &e = m_equipements.at(i);

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
                    .arg(e.getId().toHtmlEscaped())
                    .arg(e.getNom().toHtmlEscaped())
                    .arg(e.getLocalisation().toHtmlEscaped())
                    .arg(e.getDateInstallation().toString("dd/MM/yyyy"))
                    .arg(e.getStatut().toHtmlEscaped())
                    .arg(e.getType().toHtmlEscaped());
    }

    html += "</table>";

    QPdfWriter writer(path);

    writer.setPageSize(
        QPageSize(QPageSize::A4)
        );

    writer.setPageOrientation(
        QPageLayout::Landscape
        );

    QTextDocument doc;

    doc.setHtml(html);
    doc.print(&writer);

    QMessageBox::information(
        this,
        "Export",
        QString::fromUtf8(
            "PDF exporté avec succès :\n"
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
QPushButton#nav { color: white; text-align: left; padding: 13px 22px; border: none; border-radius: 8px; font-size: 14px; background: transparent; margin: 0px 6px; }
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
QPushButton { background: white; border: 1px solid #cfd8e6; border-radius: 6px; padding: 8px 16px; color: #24344f; font-size: 14px; }
QPushButton:hover { background: #f1f5fb; }
QPushButton#primary { background: #1d63d8; color: white; border: none; font-weight: 600; }
QPushButton#primary:hover { background: #1852b8; }
QPushButton#danger { color: #d93a44; font-weight: 600; }
QLineEdit, QComboBox, QDateEdit { background: white; border: 1px solid #cfd8e6; border-radius: 6px; padding: 4px 10px; min-height: 26px; color: #24344f; font-size: 14px; }
QLineEdit:focus, QComboBox:focus, QDateEdit:focus { border: 1px solid #1d63d8; }
QLineEdit:read-only { background: #f4f7fb; }
QTableWidget { background: white; border: 1px solid #e3e8f0; gridline-color: #e8edf5; alternate-background-color: #f8fafd; selection-background-color: #dbe9fb; selection-color: #24344f; color: #24344f; font-size: 14px; }
QTableWidget::item { padding-left: 6px; }
QHeaderView::section { background: #f1f5fb; color: #24344f; border: none; border-right: 1px solid #e3e8f0; border-bottom: 1px solid #e3e8f0; padding: 10px 6px; font-weight: 700; }
QTableCornerButton::section { background: #f1f5fb; border: none; }
)QSS"
        );
}
