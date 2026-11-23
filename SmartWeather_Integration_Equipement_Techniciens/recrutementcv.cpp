#include "recrutementcv.h"

#include "cvcore.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QFile>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QRegularExpression>
#include <QScrollArea>
#include <QSettings>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QVBoxLayout>

using namespace cvanalyzer;

namespace
{
inline QString U(const std::string &s) { return QString::fromUtf8(s.c_str(), static_cast<int>(s.size())); }

const char *STYLE_DIALOG =
    "QDialog { background: #F4F8FD; }"
    "QLabel { color: #173B6C; }"
    "QLineEdit, QComboBox { background: white; border: 1px solid #C7D7E9; border-radius: 5px; padding: 4px 6px; }"
    "QTableWidget { background: white; border: 1px solid #C7D7E9; border-radius: 6px;"
    "  gridline-color: #E3ECF6; selection-background-color: #DCEBFA; selection-color: #173B6C; }"
    "QHeaderView::section { background: #173B6C; color: white; padding: 7px; border: none; font-weight: bold; }"
    "QPushButton { background: white; color: #173B6C; border: 1px solid #C7D7E9;"
    "  border-radius: 6px; padding: 9px 18px; min-height: 20px; }"
    "QPushButton:hover { background: #EEF6FF; border-color: #287ACB; }"
    "QPushButton#yesButton { background: #1F9D55; color: white; border: none; font-weight: bold; }"
    "QPushButton#yesButton:hover { background: #17824A; }"
    "QPushButton#noButton { background: #D64545; color: white; border: none; font-weight: bold; }"
    "QPushButton#noButton:hover { background: #B83636; }"
    "QPushButton#primaryButton { background: #287ACB; color: white; border: none; font-weight: bold; }"
    "QPushButton#primaryButton:hover { background: #1F66AD; }";

const char *KEY_ORG = "SmartWeather";
const char *KEY_APP = "Recrutement";

const int TYPE_COUNT = 5;
const TypeCritere TYPES[TYPE_COUNT] = {TypeCritere::Formation, TypeCritere::ExperienceMin, TypeCritere::Specialite,
                                       TypeCritere::Competence, TypeCritere::Certification};
}


// ============================================================================
//  Mémorisation des critères
// ============================================================================

std::vector<Critere> chargerCriteres()
{
    std::vector<Critere> r;
    QSettings s(KEY_ORG, KEY_APP);
    const int n = s.beginReadArray("criteres");
    for (int i = 0; i < n; ++i)
    {
        s.setArrayIndex(i);
        const int t = s.value("type", -1).toInt();
        if (t < 0 || t >= TYPE_COUNT)
            continue;
        Critere c;
        c.type = TYPES[t];
        c.valeur = s.value("valeur").toString().toUtf8().toStdString();
        c.obligatoire = s.value("obligatoire", false).toBool();
        c.actif = s.value("actif", true).toBool();
        if (!c.valeur.empty())
            r.push_back(c);
    }
    s.endArray();
    return r;
}

void sauverCriteres(const std::vector<Critere> &criteres)
{
    QSettings s(KEY_ORG, KEY_APP);
    s.remove("criteres");
    s.beginWriteArray("criteres", static_cast<int>(criteres.size()));
    for (int i = 0; i < static_cast<int>(criteres.size()); ++i)
    {
        const Critere &c = criteres[static_cast<std::size_t>(i)];
        s.setArrayIndex(i);
        int t = 0;
        for (int k = 0; k < TYPE_COUNT; ++k)
            if (TYPES[k] == c.type) t = k;
        s.setValue("type", t);
        s.setValue("valeur", U(c.valeur));
        s.setValue("obligatoire", c.obligatoire);
        s.setValue("actif", c.actif);
    }
    s.endArray();
    s.sync();
}


// ============================================================================
//  Lecture d'un CV
// ============================================================================

bool lireTexteCv(const QString &chemin, std::string &texteUtf8, QString &erreur)
{
    texteUtf8.clear();
    erreur.clear();

    QFile f(chemin);
    if (!f.exists())
    {
        erreur = QString::fromUtf8("Le fichier sélectionné est introuvable.");
        return false;
    }
    if (f.size() > 30LL * 1024 * 1024)
    {
        erreur = QString::fromUtf8("Le fichier est trop volumineux (maximum 30 Mo).");
        return false;
    }
    if (!f.open(QIODevice::ReadOnly))
    {
        erreur = QString::fromUtf8("Impossible d'ouvrir le fichier : ") + f.errorString();
        return false;
    }

    const QByteArray data = f.readAll();
    f.close();

    cvcore::Bytes bytes(data.begin(), data.end());
    std::string err;
    if (!cvcore::extraireTexteCv(bytes, texteUtf8, err))
    {
        erreur = U(err);
        texteUtf8.clear();
        return false;
    }
    return true;
}


// ============================================================================
//  CriteresDialog
// ============================================================================

CriteresDialog::CriteresDialog(QWidget *parent)
    : QDialog(parent), m_table(nullptr)
{
    setWindowTitle(QString::fromUtf8("Critères de recrutement"));
    setModal(true);
    resize(900, 560);
    setStyleSheet(STYLE_DIALOG);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(20, 18, 20, 16);
    layout->setSpacing(10);

    QLabel *titre = new QLabel(QString::fromUtf8("📋  Critères de recrutement"));
    titre->setStyleSheet("font-size: 20px; font-weight: bold; color: #173B6C;");
    layout->addWidget(titre);

    QLabel *aide = new QLabel(QString::fromUtf8(
        "Définissez les critères recherchés. Chaque critère peut être <b>Obligatoire</b> ou <b>Souhaitable</b> "
        "et peut être décoché (« Retenu ») s'il n'est pas important pour ce recrutement.<br>"
        "• <b>Formation / Spécialité / Certification</b> : plusieurs valeurs possibles séparées par « , » ou « / » "
        "(une seule suffit).<br>"
        "• <b>Compétence</b> : une compétence par ligne (ou séparées par « , » : chacune devient un critère).<br>"
        "• <b>Expérience minimale</b> : nombre d'années (ex. 3)."));
    aide->setWordWrap(true);
    aide->setStyleSheet("color: #4A5F7A; font-size: 12px;");
    layout->addWidget(aide);

    m_table = new QTableWidget;
    m_table->setColumnCount(4);
    m_table->setHorizontalHeaderLabels(QStringList() << "Retenu" << "Type" << "Valeur" << "Importance");
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->verticalHeader()->setVisible(false);
    m_table->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(1, QHeaderView::Fixed);
    m_table->horizontalHeader()->setSectionResizeMode(2, QHeaderView::Stretch);
    m_table->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Fixed);
    m_table->setColumnWidth(0, 70);
    m_table->setColumnWidth(1, 200);
    m_table->setColumnWidth(3, 150);
    m_table->verticalHeader()->setDefaultSectionSize(38);
    layout->addWidget(m_table, 1);

    QHBoxLayout *boutons = new QHBoxLayout;
    QPushButton *btnAjouter = new QPushButton(QString::fromUtf8("+  Ajouter un critère"));
    QPushButton *btnSupprimer = new QPushButton("Supprimer le critère");
    QPushButton *btnEnregistrer = new QPushButton("Enregistrer");
    btnEnregistrer->setObjectName("primaryButton");
    QPushButton *btnAnnuler = new QPushButton("Annuler");
    boutons->addWidget(btnAjouter);
    boutons->addWidget(btnSupprimer);
    boutons->addStretch();
    boutons->addWidget(btnEnregistrer);
    boutons->addWidget(btnAnnuler);
    layout->addLayout(boutons);

    connect(btnAjouter, &QPushButton::clicked, this, qOverload<>(&CriteresDialog::ajouterLigne));
    connect(btnSupprimer, &QPushButton::clicked, this, &CriteresDialog::supprimerLigne);
    connect(btnEnregistrer, &QPushButton::clicked, this, &CriteresDialog::enregistrer);
    connect(btnAnnuler, &QPushButton::clicked, this, &QDialog::reject);

    const std::vector<Critere> existants = chargerCriteres();
    for (const Critere &c : existants)
        ajouterLigne(c);
    if (existants.empty())
        ajouterLigne();
}

void CriteresDialog::ajouterLigne()
{
    Critere c;
    c.type = TypeCritere::Competence;
    c.obligatoire = false;
    c.actif = true;
    ajouterLigne(c);
    m_table->setCurrentCell(m_table->rowCount() - 1, 2);
    m_table->editItem(m_table->item(m_table->rowCount() - 1, 2));
}

void CriteresDialog::ajouterLigne(const Critere &c)
{
    const int row = m_table->rowCount();
    m_table->insertRow(row);

    QTableWidgetItem *chk = new QTableWidgetItem;
    chk->setFlags(Qt::ItemIsUserCheckable | Qt::ItemIsEnabled | Qt::ItemIsSelectable);
    chk->setCheckState(c.actif ? Qt::Checked : Qt::Unchecked);
    chk->setTextAlignment(Qt::AlignCenter);
    m_table->setItem(row, 0, chk);

    QComboBox *type = new QComboBox;
    for (int k = 0; k < TYPE_COUNT; ++k)
        type->addItem(U(libelleType(TYPES[k])));
    for (int k = 0; k < TYPE_COUNT; ++k)
        if (TYPES[k] == c.type) type->setCurrentIndex(k);
    m_table->setCellWidget(row, 1, type);

    m_table->setItem(row, 2, new QTableWidgetItem(U(c.valeur)));

    QComboBox *imp = new QComboBox;
    imp->addItem("Obligatoire");
    imp->addItem("Souhaitable");
    imp->setCurrentIndex(c.obligatoire ? 0 : 1);
    m_table->setCellWidget(row, 3, imp);
}

void CriteresDialog::supprimerLigne()
{
    const int row = m_table->currentRow();
    if (row < 0)
    {
        QMessageBox::information(this, "Critères", QString::fromUtf8("Sélectionnez d'abord un critère à supprimer."));
        return;
    }
    m_table->removeRow(row);
}

bool CriteresDialog::lireTableau(std::vector<Critere> &out, QString &erreur) const
{
    out.clear();
    for (int row = 0; row < m_table->rowCount(); ++row)
    {
        QComboBox *type = qobject_cast<QComboBox *>(m_table->cellWidget(row, 1));
        QComboBox *imp = qobject_cast<QComboBox *>(m_table->cellWidget(row, 3));
        QTableWidgetItem *chk = m_table->item(row, 0);
        QTableWidgetItem *val = m_table->item(row, 2);
        if (!type || !imp || !chk)
            continue;

        const QString valeur = val ? val->text().simplified() : QString();
        if (valeur.isEmpty())
            continue;   // ligne vide ignorée

        Critere c;
        c.type = TYPES[type->currentIndex()];
        c.valeur = valeur.toUtf8().toStdString();
        c.obligatoire = (imp->currentIndex() == 0);
        c.actif = (chk->checkState() == Qt::Checked);

        if (c.type == TypeCritere::ExperienceMin)
        {
            const QRegularExpressionMatch m = QRegularExpression("^\\s*(\\d{1,3})").match(valeur);
            const int n = m.hasMatch() ? m.captured(1).toInt() : -1;
            if (n < 0 || n > 60)
            {
                erreur = QString::fromUtf8("Ligne %1 : l'expérience minimale doit être un nombre d'années (0 à 60).").arg(row + 1);
                return false;
            }
            c.valeur = QString::number(n).toStdString();
        }
        out.push_back(c);
    }
    return true;
}

void CriteresDialog::enregistrer()
{
    std::vector<Critere> criteres;
    QString erreur;
    if (!lireTableau(criteres, erreur))
    {
        QMessageBox::warning(this, QString::fromUtf8("Critères invalides"), erreur);
        return;
    }

    bool unActif = false;
    for (const Critere &c : criteres) if (c.actif) unActif = true;
    if (!unActif)
    {
        QMessageBox::warning(this, QString::fromUtf8("Critères"),
                             QString::fromUtf8("Définissez et retenez au moins un critère de recrutement."));
        return;
    }

    sauverCriteres(criteres);
    QMessageBox::information(this, QString::fromUtf8("Critères"),
                             QString::fromUtf8("Les critères de recrutement ont été enregistrés.\n"
                                               "Vous pouvez les modifier à tout moment."));
    accept();
}


// ============================================================================
//  CvResultDialog
// ============================================================================

CvResultDialog::CvResultDialog(const CvInfo &info, const Evaluation &ev, const QString &nomFichier, QWidget *parent)
    : QDialog(parent), m_action(Aucune)
{
    const bool conforme = ev.conforme;

    setWindowTitle(conforme ? QString::fromUtf8("Analyse du CV terminée")
                            : QString::fromUtf8("Profil non conforme aux critères"));
    setModal(true);
    resize(960, 720);
    setStyleSheet(STYLE_DIALOG);

    QVBoxLayout *root = new QVBoxLayout(this);
    root->setContentsMargins(20, 16, 20, 16);
    root->setSpacing(10);

    QLabel *titre = new QLabel(conforme ? QString::fromUtf8("✅  Analyse du CV terminée")
                                        : QString::fromUtf8("⚠  Le profil ne correspond pas aux critères"));
    titre->setStyleSheet(QString("font-size: 21px; font-weight: bold; color: %1;")
                             .arg(conforme ? "#1B6B3A" : "#A12622"));
    root->addWidget(titre);

    QLabel *fichier = new QLabel(QString::fromUtf8("Fichier analysé : %1").arg(nomFichier.toHtmlEscaped()));
    fichier->setStyleSheet("color: #7B8CA5; font-size: 11px;");
    root->addWidget(fichier);

    // ---- Informations du candidat ----------------------------------------
    const QString absent = QString::fromUtf8("<i style='color:#A0A9B8'>Non trouvé dans le CV</i>");
    auto val = [&](const std::string &s) { return s.empty() ? absent : U(s).toHtmlEscaped(); };

    QString nomComplet;
    if (!info.nom.empty() || !info.prenom.empty())
    {
        nomComplet = (U(info.prenom) + " " + U(info.nom)).trimmed().toHtmlEscaped();
        if (info.nomIncertain)
            nomComplet += QString::fromUtf8(" <i style='color:#B7791F'>(ordre nom / prénom à vérifier)</i>");
    }
    else
        nomComplet = absent;

    QString exp = absent;
    if (info.experienceAns >= 0)
        exp = QString::fromUtf8("%1 an(s) <i style='color:#7B8CA5'>(%2)</i>")
                  .arg(info.experienceAns).arg(U(info.experienceSource).toHtmlEscaped());

    QString comp = absent;
    if (!info.competences.empty())
    {
        QStringList l;
        for (const std::string &c : info.competences) l << U(c).toHtmlEscaped();
        comp = l.join(", ");
    }
    QString cert = absent;
    if (!info.certifications.empty())
    {
        QStringList l;
        for (const std::string &c : info.certifications) l << U(c).toHtmlEscaped();
        cert = l.join(" ; ");
    }

    QWidget *card = new QWidget;
    card->setStyleSheet("QWidget { background: white; border: 1px solid #C7D7E9; border-radius: 8px; }"
                        "QLabel { border: none; }");
    QGridLayout *g = new QGridLayout(card);
    g->setContentsMargins(16, 12, 16, 12);
    g->setHorizontalSpacing(14);
    g->setVerticalSpacing(6);
    g->setColumnStretch(1, 1);

    int r = 0;
    auto addRow = [&](const QString &label, const QString &html) {
        QLabel *l = new QLabel("<b>" + label + "</b>");
        l->setAlignment(Qt::AlignTop | Qt::AlignLeft);
        QLabel *v = new QLabel(html);
        v->setWordWrap(true);
        v->setTextFormat(Qt::RichText);
        g->addWidget(l, r, 0);
        g->addWidget(v, r, 1);
        ++r;
    };
    addRow(QString::fromUtf8("Nom et prénom :"), nomComplet);
    addRow("Formation :", val(info.formation));
    addRow(QString::fromUtf8("Expérience :"), exp);
    addRow(QString::fromUtf8("Spécialité :"), val(info.specialite));
    addRow(QString::fromUtf8("Compétences :"), comp);
    addRow("Certifications :", cert);
    root->addWidget(card);

    // ---- Pourcentage -------------------------------------------------------
    int nbOk = 0, nbKo = 0, nbManq = 0;
    for (const ResultatCritere &rc : ev.resultats)
    {
        if (rc.verdict == Verdict::Respecte) ++nbOk;
        else if (rc.verdict == Verdict::NonRespecte) ++nbKo;
        else ++nbManq;
    }

    QHBoxLayout *ligne = new QHBoxLayout;
    QLabel *pct = new QLabel(QString::fromUtf8("Niveau de correspondance : <b>%1 %</b>").arg(ev.pourcentage));
    pct->setStyleSheet("font-size: 15px;");
    QProgressBar *bar = new QProgressBar;
    bar->setRange(0, 100);
    bar->setValue(ev.pourcentage);
    bar->setTextVisible(false);
    bar->setFixedHeight(16);
    const QString couleur = ev.pourcentage >= 75 ? "#1F9D55" : (ev.pourcentage >= 50 ? "#E0A100" : "#D64545");
    bar->setStyleSheet(QString("QProgressBar { border: 1px solid #C7D7E9; border-radius: 8px; background: white; }"
                               "QProgressBar::chunk { background: %1; border-radius: 7px; }").arg(couleur));
    ligne->addWidget(pct);
    ligne->addWidget(bar, 1);
    root->addLayout(ligne);

    QLabel *resume = new QLabel(QString::fromUtf8(
        "✔ Critères respectés : <b>%1</b> &nbsp;&nbsp; ✘ Non respectés : <b>%2</b> &nbsp;&nbsp; "
        "? Informations manquantes : <b>%3</b>").arg(nbOk).arg(nbKo).arg(nbManq));
    root->addWidget(resume);

    QLabel *methode = new QLabel(QString::fromUtf8(
        "Méthode de calcul : critère obligatoire = poids %1, critère souhaitable = poids %2. "
        "Pourcentage = poids des critères respectés ÷ poids total des critères retenus. "
        "Un critère non respecté ou dont l'information est absente du CV rapporte 0.")
            .arg(POIDS_OBLIGATOIRE).arg(POIDS_SOUHAITABLE));
    methode->setWordWrap(true);
    methode->setStyleSheet("color: #7B8CA5; font-size: 11px;");
    root->addWidget(methode);

    // ---- Tableau des critères ---------------------------------------------
    QTableWidget *t = new QTableWidget;
    t->setColumnCount(4);
    t->setHorizontalHeaderLabels(QStringList() << QString::fromUtf8("Critère") << "Importance"
                                               << QString::fromUtf8("Résultat") << QString::fromUtf8("Explication"));
    t->setEditTriggers(QAbstractItemView::NoEditTriggers);
    t->setSelectionMode(QAbstractItemView::NoSelection);
    t->verticalHeader()->setVisible(false);
    t->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    t->horizontalHeader()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    t->horizontalHeader()->setSectionResizeMode(2, QHeaderView::ResizeToContents);
    t->horizontalHeader()->setSectionResizeMode(3, QHeaderView::Stretch);
    t->setWordWrap(true);

    for (const ResultatCritere &rc : ev.resultats)
    {
        const int row = t->rowCount();
        t->insertRow(row);
        QColor bg("#E8F5EC");
        QString mark = QString::fromUtf8("✔ ") + U(libelleVerdict(rc.verdict));
        if (rc.verdict == Verdict::NonRespecte) { bg = QColor("#FDECEA"); mark = QString::fromUtf8("✘ ") + U(libelleVerdict(rc.verdict)); }
        if (rc.verdict == Verdict::Manquant)    { bg = QColor("#FFF6E0"); mark = QString::fromUtf8("? ") + U(libelleVerdict(rc.verdict)); }

        QTableWidgetItem *i0 = new QTableWidgetItem(U(libelleType(rc.critere.type)) + " : " + U(rc.critere.valeur));
        QTableWidgetItem *i1 = new QTableWidgetItem(rc.critere.obligatoire ? "Obligatoire" : "Souhaitable");
        QTableWidgetItem *i2 = new QTableWidgetItem(mark);
        QTableWidgetItem *i3 = new QTableWidgetItem(U(rc.detail));
        for (QTableWidgetItem *it : {i0, i1, i2, i3})
            it->setBackground(bg);
        t->setItem(row, 0, i0);
        t->setItem(row, 1, i1);
        t->setItem(row, 2, i2);
        t->setItem(row, 3, i3);
    }
    t->resizeRowsToContents();
    root->addWidget(t, 1);

    QLabel *raison = new QLabel(U(ev.raison));
    raison->setWordWrap(true);
    raison->setStyleSheet(conforme
        ? "background: #E8F5EC; color: #1B6B3A; border: 1px solid #B5DEC4; border-radius: 6px; padding: 8px;"
        : "background: #FDECEA; color: #A12622; border: 1px solid #F3B8B4; border-radius: 6px; padding: 8px;");
    root->addWidget(raison);

    // ---- Boutons -----------------------------------------------------------
    QHBoxLayout *boutons = new QHBoxLayout;
    boutons->addStretch();
    if (conforme)
    {
        QPushButton *oui = new QPushButton(QString::fromUtf8("Confirmer OUI"));
        oui->setObjectName("yesButton");
        QPushButton *non = new QPushButton(QString::fromUtf8("Refuser NON"));
        non->setObjectName("noButton");
        boutons->addWidget(oui);
        boutons->addWidget(non);
        connect(oui, &QPushButton::clicked, this, [this]() { terminer(Confirmer); });
        connect(non, &QPushButton::clicked, this, [this]() { terminer(Refuser); });
    }
    else
    {
        QPushButton *autre = new QPushButton(QString::fromUtf8("Analyser un autre CV"));
        autre->setObjectName("primaryButton");
        QPushButton *fermer = new QPushButton("Fermer");
        boutons->addWidget(autre);
        boutons->addWidget(fermer);
        connect(autre, &QPushButton::clicked, this, [this]() { terminer(AutreCv); });
        connect(fermer, &QPushButton::clicked, this, [this]() { terminer(Aucune); });
    }
    root->addLayout(boutons);
}

void CvResultDialog::terminer(Action a)
{
    m_action = a;
    accept();
}
