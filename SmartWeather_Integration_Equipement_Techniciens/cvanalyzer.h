#ifndef CVANALYZER_H
#define CVANALYZER_H

/*
 * cvanalyzer : analyse du texte d'un CV et comparaison avec les critères
 * de recrutement.
 *
 * Module en C++17 standard (std::string en UTF-8), sans dépendance à Qt.
 *
 * Règle fondamentale : une information absente du CV reste VIDE
 * (chaîne vide, -1 pour l'expérience). Rien n'est supposé ni inventé.
 */

#include <string>
#include <vector>

namespace cvanalyzer
{

// ----------------------------------------------------------------------------
//  Informations extraites d'un CV
// ----------------------------------------------------------------------------
struct CvInfo
{
    std::string nom;                  // vide si non trouvé
    std::string prenom;               // vide si non trouvé
    bool nomIncertain = false;        // vrai si l'ordre nom/prénom a été deviné
    std::string email;
    std::string telephone;            // 8 chiffres (numéro tunisien) si reconnu
    std::string formation;            // meilleur diplôme trouvé (ligne du CV)
    std::string formationDetails;     // tout le texte de la section formation
    int experienceAns = -1;           // -1 = non déterminable
    std::string experienceSource;     // "déclarée dans le CV" / "calculée à partir des périodes"
    std::string specialite;
    std::vector<std::string> competences;      // compétences et technologies réellement mentionnées
    std::vector<std::string> certifications;   // certifications trouvées
    int nbLignes = 0;                 // taille du texte analysé (information de diagnostic)

    // Zones de texte normalisées (minuscules, sans accents) servant à l'évaluation
    std::string zoneFormation;
    std::string zoneSpecialite;
    std::string zoneCertifications;
};

// ----------------------------------------------------------------------------
//  Critères de recrutement
// ----------------------------------------------------------------------------
enum class TypeCritere { Formation, ExperienceMin, Specialite, Competence, Certification };

struct Critere
{
    TypeCritere type = TypeCritere::Competence;
    std::string valeur;          // texte du critère (ou nombre d'années pour ExperienceMin)
    bool obligatoire = false;    // false = souhaitable
    bool actif = true;           // false = critère ignoré (non retenu pour ce recrutement)
};

enum class Verdict { Respecte, NonRespecte, Manquant };

struct ResultatCritere
{
    Critere critere;
    Verdict verdict = Verdict::NonRespecte;
    std::string detail;          // explication lisible (ce qui a été trouvé / pas trouvé)
};

struct Evaluation
{
    std::vector<ResultatCritere> resultats;
    int pourcentage = 0;         // 0..100
    bool conforme = false;       // tous les critères obligatoires sont respectés
    int poidsTotal = 0;
    int poidsObtenu = 0;
    int nbObligatoiresNonRespectes = 0;
    std::string raison;          // phrase de synthèse
};

// Poids utilisés pour le pourcentage
const int POIDS_OBLIGATOIRE = 3;
const int POIDS_SOUHAITABLE = 1;

// ----------------------------------------------------------------------------
//  API
// ----------------------------------------------------------------------------

// Analyse le texte (UTF-8) d'un CV. anneeCourante / moisCourant servent à
// interpréter les périodes "jusqu'à aujourd'hui" (ex. "2021 - Présent").
CvInfo analyserTexte(const std::string &texteUtf8, int anneeCourante, int moisCourant);

// Compare un CV avec les critères (les critères inactifs sont ignorés).
// texteUtf8 : texte complet du CV (utilisé pour retrouver les compétences
// citées par l'utilisateur et non présentes dans le dictionnaire interne).
Evaluation evaluer(const CvInfo &info, const std::string &texteUtf8, const std::vector<Critere> &criteres);

// Éclate les valeurs multiples ("C++, SQL ; Oracle") en un critère par valeur
// pour les compétences. Les autres types gardent la liste comme alternatives.
std::vector<Critere> normaliserCriteres(const std::vector<Critere> &criteres);

// Utilitaires exposés pour les tests / l'interface
std::string normaliser(const std::string &utf8);                 // minuscules sans accents
std::string libelleType(TypeCritere t);                          // "Formation", "Expérience minimale", ...
std::string libelleVerdict(Verdict v);

} // namespace cvanalyzer

#endif // CVANALYZER_H
