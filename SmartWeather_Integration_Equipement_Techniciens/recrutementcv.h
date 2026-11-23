#ifndef RECRUTEMENTCV_H
#define RECRUTEMENTCV_H

#include <QDialog>
#include <QString>
#include <string>
#include <vector>

#include "cvanalyzer.h"

class QComboBox;
class QLabel;
class QPushButton;
class QTableWidget;

/*
 * Métier avancé 2 : recrutement intelligent avec analyse automatique des CV.
 *
 *  - CriteresDialog : interface « Critères de recrutement » (modifiable à tout moment)
 *  - CvResultDialog : fenêtre « Analyse du CV terminée » (OUI / NON) ou profil non conforme
 *  - chargerCriteres / sauverCriteres : mémorisation des critères (QSettings, aucune donnée de candidat)
 *  - lireTexteCv : lecture locale d'un PDF / DOCX (aucun envoi vers un service externe)
 */

std::vector<cvanalyzer::Critere> chargerCriteres();
void sauverCriteres(const std::vector<cvanalyzer::Critere> &criteres);

// Lit un CV (PDF ou DOCX) et en extrait le texte UTF-8. Retourne false avec un message
// explicatif si le texte n'est pas extractible (ex. CV scanné).
bool lireTexteCv(const QString &chemin, std::string &texteUtf8, QString &erreur);


// ============================================================================
class CriteresDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CriteresDialog(QWidget *parent = nullptr);

private slots:
    void ajouterLigne();
    void supprimerLigne();
    void enregistrer();

private:
    void ajouterLigne(const cvanalyzer::Critere &c);
    bool lireTableau(std::vector<cvanalyzer::Critere> &out, QString &erreur) const;

    QTableWidget *m_table;
};


// ============================================================================
class CvResultDialog : public QDialog
{
    Q_OBJECT

public:
    enum Action { Aucune, Confirmer, Refuser, AutreCv };

    CvResultDialog(const cvanalyzer::CvInfo &info,
                   const cvanalyzer::Evaluation &evaluation,
                   const QString &nomFichier,
                   QWidget *parent = nullptr);

    Action action() const { return m_action; }

private:
    void terminer(Action a);

    Action m_action;
};

#endif // RECRUTEMENTCV_H
