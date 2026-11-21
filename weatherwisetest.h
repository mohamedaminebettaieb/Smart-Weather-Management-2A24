
#ifndef WEATHERWISETEST_H
#define WEATHERWISETEST_H

#include <QMainWindow>
#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QColor>
#include <QRectF>
#include <QFont>
#include <QPaintEvent>
#include <QVector>
#include <QString>
#include <QImage>
#include <QDate>

#include "equipement.h"

class QTableWidget;
class QLineEdit;
class QComboBox;
class QDateEdit;
class QLabel;
class QPushButton;

// ==========================================
// CLASSE GRAPHIQUE EN ANNEAU
// ==========================================

class EquipementDonutChart : public QWidget
{
public:

    explicit EquipementDonutChart(QWidget *parent = 0)
        : QWidget(parent),
        m_actifs(0),
        m_inactifs(0)
    {
        setMinimumSize(140, 140);
    }

    void setValues(int actifs, int inactifs)
    {
        m_actifs = actifs;
        m_inactifs = inactifs;
        update();
    }

protected:

    void paintEvent(QPaintEvent *) Q_DECL_OVERRIDE
    {
        QPainter p(this);

        p.setRenderHint(QPainter::Antialiasing);

        const int total = m_actifs + m_inactifs;

        const int side = qMin(width(), height()) - 16;
        const qreal penW = side * 0.16;

        QRectF r(
            (width() - side) / 2.0,
            (height() - side) / 2.0,
            side,
            side
            );

        r.adjust(
            penW / 2,
            penW / 2,
            -penW / 2,
            -penW / 2
            );

        QPen pen;

        pen.setWidthF(penW);
        pen.setCapStyle(Qt::FlatCap);

        if (total == 0)
        {
            pen.setColor(QColor("#e5e7eb"));

            p.setPen(pen);
            p.drawArc(r, 0, 360 * 16);
        }
        else
        {
            const int start = 90 * 16;

            const int spanA =
                -static_cast<int>(
                    360.0 * 16.0 * m_actifs / total
                    );

            const int spanI = -360 * 16 - spanA;

            pen.setColor(QColor("#16a068"));

            p.setPen(pen);
            p.drawArc(r, start, spanA);

            if (m_inactifs > 0)
            {
                pen.setColor(QColor("#e5474f"));

                p.setPen(pen);
                p.drawArc(r, start + spanA, spanI);
            }
        }

        p.setPen(QColor("#0f2a52"));

        QFont f = font();

        f.setPointSize(22);
        f.setBold(true);

        p.setFont(f);

        p.drawText(
            QRectF(0, height() / 2.0 - 30, width(), 32),
            Qt::AlignCenter,
            QString::number(total)
            );

        f.setPointSize(10);
        f.setBold(false);

        p.setFont(f);

        p.drawText(
            QRectF(0, height() / 2.0 + 2, width(), 20),
            Qt::AlignCenter,
            QString::fromUtf8("équipements")
            );
    }

private:

    int m_actifs;
    int m_inactifs;
};

// ==========================================
// FENETRE PRINCIPALE
// ==========================================

class WeatherWiseTest : public QMainWindow
{
    Q_OBJECT

public:

    explicit WeatherWiseTest(QWidget *parent = 0);

private slots:

    // CRUD
    void onAdd();
    void onEdit();
    void onDelete();
    void onClear();

    // Recherche et tri indépendants
    void onSort();
    void onSearch();

    // Sélection d'un équipement
    void onRowSelected();

    // Horloge
    void updateClock();

    // Export PDF
    void exportPdf();

    // Prédiction de maintenance
    void onCalculateMaintenance();

    // Génération et sauvegarde du QR Code
    void onGenerateQrCode();
    void onSaveQrCode();

private:

    // ======================================
    // CONSTRUCTION DE L'INTERFACE
    // ======================================

    QWidget *buildSidebar();
    QWidget *buildHeader();
    QWidget *buildFormCard();
    QWidget *buildToolbar();
    QWidget *buildTableCard();
    QWidget *buildStatsCard();

    // Section prédiction de maintenance
    QWidget *buildMaintenanceCard();

    // Section QR Code
    QWidget *buildQrCard();

    static QString styleSheet();

    // ======================================
    // LOGIQUE
    // ======================================

    void loadSampleData();
    void refreshTable();
    void updateStats();

    bool readForm(Equipement &e);

    QString nextId() const;

    // ======================================
    // EQUIPEMENTS
    // ======================================

    QVector<Equipement> m_equipements;

    QString m_editId;

    /*
     * -1 = ordre d'ajout
     *  0 = nom
     *  1 = localisation
     *  2 = statut
     *  3 = type
     */
    int m_sortMode;

    // ======================================
    // WIDGETS PRINCIPAUX
    // ======================================

    QTableWidget *m_table;

    QLineEdit *m_search;

    QComboBox *m_sort;

    EquipementDonutChart *m_donut;

    QLabel *m_actifsLabel;
    QLabel *m_inactifsLabel;
    QLabel *m_maintenanceLabel;

    QLabel *m_dateLabel;
    QLabel *m_timeLabel;

    // ======================================
    // SECTION MAINTENANCE
    // ======================================

    QTableWidget *m_maintenanceTable;

    QLabel *m_maintenanceCount;

    // ======================================
    // SECTION QR CODE
    // ======================================

    QLabel *m_qrPreview;
    QLabel *m_qrInfo;

    // Image QR générée
    QImage m_qrImage;

    // ======================================
    // FORMULAIRE
    // ======================================

    QLineEdit *m_id;
    QLineEdit *m_nom;

    QComboBox *m_loc;

    QComboBox *m_station;

    QDateEdit *m_date;

    QComboBox *m_statut;

    QComboBox *m_type;
};

#endif // WEATHERWISETEST_H
