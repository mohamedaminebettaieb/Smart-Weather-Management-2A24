#include "cvanalyzer.h"
#include "cvcore.h"

#include <algorithm>
#include <cctype>
#include <cstdlib>
#include <map>
#include <set>

namespace cvanalyzer
{

// ============================================================================
//  Normalisation (minuscules, sans accents)
// ============================================================================

namespace
{
const char *baseLetters(char32_t cp)
{
    static std::string buf;
    auto one = [&](char c) { buf.assign(1, c); return buf.c_str(); };
    if (cp >= 0xC0 && cp <= 0xC5) return one('a');
    if (cp >= 0xE0 && cp <= 0xE5) return one('a');
    if (cp == 0xC6 || cp == 0xE6) return "ae";
    if (cp == 0xC7 || cp == 0xE7) return one('c');
    if ((cp >= 0xC8 && cp <= 0xCB) || (cp >= 0xE8 && cp <= 0xEB)) return one('e');
    if ((cp >= 0xCC && cp <= 0xCF) || (cp >= 0xEC && cp <= 0xEF)) return one('i');
    if (cp == 0xD1 || cp == 0xF1) return one('n');
    if ((cp >= 0xD2 && cp <= 0xD6) || cp == 0xD8 || (cp >= 0xF2 && cp <= 0xF6) || cp == 0xF8) return one('o');
    if ((cp >= 0xD9 && cp <= 0xDC) || (cp >= 0xF9 && cp <= 0xFC)) return one('u');
    if (cp == 0xDD || cp == 0xFD || cp == 0xFF || cp == 0x178) return one('y');
    if (cp == 0xDF) return "ss";
    if (cp == 0x152 || cp == 0x153) return "oe";
    if (cp >= 0x100 && cp <= 0x105) return one('a');
    if (cp >= 0x106 && cp <= 0x10D) return one('c');
    if (cp >= 0x10E && cp <= 0x111) return one('d');
    if (cp >= 0x112 && cp <= 0x11B) return one('e');
    if (cp >= 0x11C && cp <= 0x123) return one('g');
    if (cp >= 0x124 && cp <= 0x127) return one('h');
    if (cp >= 0x128 && cp <= 0x131) return one('i');
    if (cp == 0x134 || cp == 0x135) return one('j');
    if (cp >= 0x136 && cp <= 0x138) return one('k');
    if (cp >= 0x139 && cp <= 0x142) return one('l');
    if (cp >= 0x143 && cp <= 0x14B) return one('n');
    if (cp >= 0x14C && cp <= 0x151) return one('o');
    if (cp >= 0x154 && cp <= 0x159) return one('r');
    if (cp >= 0x15A && cp <= 0x161) return one('s');
    if (cp >= 0x162 && cp <= 0x167) return one('t');
    if (cp >= 0x168 && cp <= 0x173) return one('u');
    if (cp == 0x174 || cp == 0x175) return one('w');
    if (cp == 0x176 || cp == 0x177) return one('y');
    if (cp >= 0x179 && cp <= 0x17E) return one('z');
    if (cp == 0xFB01) return "fi";
    if (cp == 0xFB02) return "fl";
    return nullptr;
}
} // namespace

std::string normaliser(const std::string &utf8)
{
    const std::u32string u = cvcore::utf8ToU32(utf8);
    std::string out;
    out.reserve(u.size());
    for (char32_t cp : u) {
        if (cp < 0x80) {
            char c = static_cast<char>(cp);
            if (c >= 'A' && c <= 'Z')
                c = static_cast<char>(c - 'A' + 'a');
            if (c == '\r' || c == '\t' || c == '\f' || c == '\v')
                c = ' ';
            out.push_back(c);
            continue;
        }
        if (cp == 0x2013 || cp == 0x2014 || cp == 0x2212 || cp == 0x2010 || cp == 0x2011) { out.push_back('-'); continue; }
        if (cp == 0x2018 || cp == 0x2019 || cp == 0x2032 || cp == 0x00B4) { out.push_back('\''); continue; }
        if (cp == 0x201C || cp == 0x201D) { out.push_back('"'); continue; }
        if (cp == 0x2022 || cp == 0xB7 || cp == 0x25CF || cp == 0x25AA || cp == 0x25A0) { out.push_back('*'); continue; }
        if (cp == 0x2026) { out.append("..."); continue; }
        if (cp == 0xA0 || cp == 0x202F || cp == 0x2009) { out.push_back(' '); continue; }
        const char *b = (cp == 0xD7 || cp == 0xF7) ? nullptr : baseLetters(cp);
        if (b) {
            out.append(b);
            // majuscules accentuées : déjà ramenées à la lettre de base minuscule
            continue;
        }
        out.push_back(' ');
    }
    return out;
}

std::string libelleType(TypeCritere t)
{
    switch (t) {
    case TypeCritere::Formation:     return "Formation";
    case TypeCritere::ExperienceMin: return "Exp\xC3\xA9rience minimale";
    case TypeCritere::Specialite:    return "Sp\xC3\xA9" "cialit\xC3\xA9";
    case TypeCritere::Competence:    return "Comp\xC3\xA9tence";
    case TypeCritere::Certification: return "Certification";
    }
    return "";
}

std::string libelleVerdict(Verdict v)
{
    switch (v) {
    case Verdict::Respecte:    return "Respect\xC3\xA9";
    case Verdict::NonRespecte: return "Non respect\xC3\xA9";
    case Verdict::Manquant:    return "Information manquante";
    }
    return "";
}

// ============================================================================
//  Outils texte
// ============================================================================

namespace
{
bool isAl(unsigned char c) { return std::isalnum(c) != 0; }

std::string trim(const std::string &s)
{
    std::size_t a = 0, b = s.size();
    while (a < b && std::isspace(static_cast<unsigned char>(s[a]))) ++a;
    while (b > a && std::isspace(static_cast<unsigned char>(s[b - 1]))) --b;
    return s.substr(a, b - a);
}

std::string collapseSpaces(const std::string &s)
{
    std::string r;
    bool sp = false;
    for (unsigned char c : s) {
        if (c == ' ' || c == '\t' || c == '\r' || c == 0xA0) {
            sp = true;
        } else {
            if (sp && !r.empty())
                r.push_back(' ');
            sp = false;
            r.push_back(static_cast<char>(c));
        }
    }
    return r;
}

std::vector<std::string> splitWords(const std::string &s)
{
    std::vector<std::string> w;
    std::string cur;
    for (unsigned char c : s) {
        if (std::isspace(c)) {
            if (!cur.empty()) { w.push_back(cur); cur.clear(); }
        } else {
            cur.push_back(static_cast<char>(c));
        }
    }
    if (!cur.empty()) w.push_back(cur);
    return w;
}

bool startsWith(const std::string &s, const std::string &p) { return s.compare(0, p.size(), p) == 0; }

bool contains(const std::string &s, const std::string &p) { return s.find(p) != std::string::npos; }

// Retire une puce (•, -, *, >) au début de la ligne d'origine.
std::string stripBullet(const std::string &line)
{
    std::string s = trim(line);
    const char *bullets[] = {"\xE2\x80\xA2", "\xC2\xB7", "\xE2\x96\xAA", "\xE2\x96\xA0", "\xE2\x97\x8F", "\xE2\x96\xBA",
                             "\xE2\x9E\xA2", "\xE2\x97\xA6", "\xE2\x80\x93", "\xE2\x80\x94", "\xC2\xBB", "-", "*", ">"};
    for (const char *b : bullets) {
        const std::string bs(b);
        if (startsWith(s, bs) && s.size() > bs.size() && (s[bs.size()] == ' ' || bs.size() == 1)) {
            if (bs == "-" && s.size() > 1 && s[1] != ' ')
                break;
            s = trim(s.substr(bs.size()));
            break;
        }
    }
    return s;
}

// Recherche d'un terme avec limites de mots (utile pour "c++", "c#", ".net", "sql"...).
bool findTerm(const std::string &hay, const std::string &term, std::size_t from = 0, std::size_t *where = nullptr)
{
    if (term.empty())
        return false;
    std::size_t pos = from;
    while ((pos = hay.find(term, pos)) != std::string::npos) {
        const bool startAl = isAl(static_cast<unsigned char>(term.front()));
        const bool endAl = isAl(static_cast<unsigned char>(term.back()));
        bool ok = true;
        if (startAl && pos > 0 && isAl(static_cast<unsigned char>(hay[pos - 1])))
            ok = false;
        const std::size_t e = pos + term.size();
        if (ok && endAl && e < hay.size()) {
            const unsigned char n = static_cast<unsigned char>(hay[e]);
            if (isAl(n) || n == '+' || n == '#')
                ok = false;
        }
        if (ok) {
            if (where) *where = pos;
            return true;
        }
        ++pos;
    }
    return false;
}

// Mot présent (début de mot, suffixe libre : "informatique" trouve "informatiques")
bool findWordPrefix(const std::string &hay, const std::string &word)
{
    std::size_t pos = 0;
    while ((pos = hay.find(word, pos)) != std::string::npos) {
        if (pos == 0 || !isAl(static_cast<unsigned char>(hay[pos - 1]))) {
            if (word.size() > 3)
                return true;
            const std::size_t e = pos + word.size();
            if (e >= hay.size() || !isAl(static_cast<unsigned char>(hay[e])))
                return true;
        }
        ++pos;
    }
    return false;
}

bool isStopWord(const std::string &w)
{
    static const std::set<std::string> s = {"de", "des", "du", "en", "et", "d", "l", "la", "le", "les", "a", "au", "aux",
                                            "of", "in", "and", "the", "for", "&", "-", "/", "ou", "or", "un", "une", "sur"};
    return s.count(w) > 0;
}

// Tous les mots significatifs du critère doivent apparaître dans la zone.
bool matchWords(const std::string &zoneNorm, const std::string &criterionNorm)
{
    std::string cleaned;
    for (unsigned char c : criterionNorm)
        cleaned.push_back(isAl(c) || c == '+' || c == '#' ? static_cast<char>(c) : ' ');
    const std::vector<std::string> words = splitWords(cleaned);
    int considered = 0;
    for (const std::string &w : words) {
        if (isStopWord(w))
            continue;
        ++considered;
        if (!findWordPrefix(zoneNorm, w))
            return false;
    }
    return considered > 0;
}

std::vector<std::string> splitAlternatives(const std::string &v, bool slash)
{
    std::vector<std::string> r;
    std::string cur;
    auto push = [&]() {
        const std::string t = trim(cur);
        if (!t.empty()) r.push_back(t);
        cur.clear();
    };
    for (std::size_t i = 0; i < v.size(); ++i) {
        const char c = v[i];
        if (c == ',' || c == ';' || c == '|' || c == '\n' || (slash && c == '/')) {
            push();
        } else {
            cur.push_back(c);
        }
    }
    push();
    return r;
}

// ============================================================================
//  Découpage du CV en lignes et sections
// ============================================================================

enum Section { SecHeader, SecProfile, SecEducation, SecExperience, SecSkills, SecCerts, SecOther };

struct Line
{
    std::string orig;    // texte d'origine (UTF-8), puces retirées
    std::string norm;    // minuscules sans accents
    Section section = SecHeader;
    bool heading = false;
};

int headingSection(const std::string &normIn)
{
    // retire ponctuation de début/fin et numérotation
    std::string t = normIn;
    while (!t.empty() && !std::isalpha(static_cast<unsigned char>(t.front()))) t.erase(t.begin());
    while (!t.empty() && !std::isalnum(static_cast<unsigned char>(t.back()))) t.pop_back();
    t = collapseSpaces(t);
    if (t.empty() || t.size() > 45)
        return -1;

    static const std::map<std::string, Section> h = {
        {"formation", SecEducation}, {"formations", SecEducation}, {"education", SecEducation}, {"etudes", SecEducation},
        {"parcours academique", SecEducation}, {"parcours scolaire", SecEducation}, {"parcours universitaire", SecEducation},
        {"diplomes", SecEducation}, {"diplome", SecEducation}, {"cursus", SecEducation}, {"cursus universitaire", SecEducation},
        {"etudes et formations", SecEducation}, {"formation academique", SecEducation}, {"formation et diplomes", SecEducation},
        {"formations et diplomes", SecEducation}, {"academic background", SecEducation}, {"academic qualifications", SecEducation},
        {"qualifications", SecEducation}, {"scolarite", SecEducation}, {"education and training", SecEducation},
        {"formation initiale", SecEducation}, {"diplomes et formations", SecEducation},

        {"experience", SecExperience}, {"experiences", SecExperience}, {"experience professionnelle", SecExperience},
        {"experiences professionnelles", SecExperience}, {"parcours professionnel", SecExperience},
        {"work experience", SecExperience}, {"professional experience", SecExperience}, {"employment history", SecExperience},
        {"employment", SecExperience}, {"work history", SecExperience}, {"stages", SecExperience}, {"stage", SecExperience},
        {"stages et experiences", SecExperience}, {"experiences et stages", SecExperience},
        {"activites professionnelles", SecExperience}, {"carriere", SecExperience}, {"emplois", SecExperience},
        {"experience et stages", SecExperience}, {"stages et projets", SecExperience}, {"experience de travail", SecExperience},

        {"competences", SecSkills}, {"competence", SecSkills}, {"competences techniques", SecSkills},
        {"competences informatiques", SecSkills}, {"skills", SecSkills}, {"technical skills", SecSkills},
        {"technologies", SecSkills}, {"outils", SecSkills}, {"langages", SecSkills}, {"connaissances techniques", SecSkills},
        {"connaissances informatiques", SecSkills}, {"environnement technique", SecSkills}, {"savoir-faire", SecSkills},
        {"technical expertise", SecSkills}, {"expertise", SecSkills}, {"competences cles", SecSkills},
        {"competences professionnelles", SecSkills}, {"key skills", SecSkills}, {"competences et outils", SecSkills},
        {"langages et outils", SecSkills}, {"outils et technologies", SecSkills}, {"connaissances", SecSkills},

        {"certifications", SecCerts}, {"certification", SecCerts}, {"certificats", SecCerts}, {"certificates", SecCerts},
        {"attestations", SecCerts}, {"licenses", SecCerts}, {"certifications et attestations", SecCerts},
        {"certifications et formations complementaires", SecCerts}, {"formations complementaires", SecCerts},
        {"certifications professionnelles", SecCerts}, {"licenses and certifications", SecCerts},

        {"profil", SecProfile}, {"profil professionnel", SecProfile}, {"resume", SecProfile}, {"summary", SecProfile},
        {"professional summary", SecProfile}, {"objectif", SecProfile}, {"objectif professionnel", SecProfile},
        {"a propos", SecProfile}, {"about me", SecProfile}, {"about", SecProfile}, {"presentation", SecProfile},
        {"career objective", SecProfile}, {"profile", SecProfile}, {"objectifs", SecProfile}, {"resume professionnel", SecProfile},

        {"langues", SecOther}, {"languages", SecOther}, {"loisirs", SecOther}, {"centres d'interet", SecOther},
        {"centres d interet", SecOther}, {"interets", SecOther}, {"hobbies", SecOther}, {"interests", SecOther},
        {"references", SecOther}, {"projets", SecOther}, {"projects", SecOther}, {"projet", SecOther},
        {"benevolat", SecOther}, {"vie associative", SecOther}, {"informations personnelles", SecOther},
        {"contact", SecOther}, {"coordonnees", SecOther}, {"personal information", SecOther}, {"divers", SecOther},
        {"publications", SecOther}, {"distinctions", SecOther}, {"awards", SecOther}, {"activites extra-professionnelles", SecOther},
        {"activites associatives", SecOther}, {"qualites", SecOther}, {"atouts", SecOther}, {"projets personnels", SecOther},
        {"projets academiques", SecOther}, {"realisations", SecOther}, {"soft skills", SecOther}};
    auto it = h.find(t);
    return it == h.end() ? -1 : static_cast<int>(it->second);
}

std::vector<Line> decouper(const std::string &texteUtf8)
{
    std::vector<Line> lines;
    std::string cur;
    Section section = SecHeader;

    auto pushLine = [&](const std::string &raw) {
        const std::string clean = stripBullet(collapseSpaces(raw));
        if (clean.empty())
            return;
        Line L;
        L.orig = clean;
        L.norm = collapseSpaces(normaliser(clean));
        if (L.norm.empty())
            return;

        // 1) titre de section seul
        const int hs = headingSection(L.norm);
        if (hs >= 0) {
            section = static_cast<Section>(hs);
            L.heading = true;
            L.section = section;
            lines.push_back(L);
            return;
        }
        // 2) "Titre : contenu" sur la même ligne
        const std::size_t cn = L.norm.find(':');
        const std::size_t co = clean.find(':');
        if (cn != std::string::npos && co != std::string::npos && cn < 40) {
            const int hl = headingSection(L.norm.substr(0, cn));
            if (hl >= 0) {
                section = static_cast<Section>(hl);
                Line H;
                H.orig = clean.substr(0, co);
                H.norm = L.norm.substr(0, cn);
                H.section = section;
                H.heading = true;
                lines.push_back(H);
                const std::string rest = trim(clean.substr(co + 1));
                if (!rest.empty()) {
                    Line C;
                    C.orig = rest;
                    C.norm = collapseSpaces(normaliser(rest));
                    C.section = section;
                    lines.push_back(C);
                }
                return;
            }
        }
        L.section = section;
        lines.push_back(L);
    };

    for (char c : texteUtf8) {
        if (c == '\n') {
            pushLine(cur);
            cur.clear();
        } else {
            cur.push_back(c);
        }
    }
    pushLine(cur);
    return lines;
}

// ============================================================================
//  Dictionnaires
// ============================================================================

struct Term { const char *norm; const char *display; };

const Term kSkills[] = {
    {"c++", "C++"}, {"c#", "C#"}, {"java", "Java"}, {"javascript", "JavaScript"}, {"typescript", "TypeScript"},
    {"python", "Python"}, {"php", "PHP"}, {"sql", "SQL"}, {"pl/sql", "PL/SQL"}, {"oracle", "Oracle"}, {"mysql", "MySQL"},
    {"postgresql", "PostgreSQL"}, {"sql server", "SQL Server"}, {"mongodb", "MongoDB"}, {"html", "HTML"}, {"css", "CSS"},
    {"react", "React"}, {"angular", "Angular"}, {"vue.js", "Vue.js"}, {"node.js", "Node.js"}, {"qt", "Qt"},
    {"qt creator", "Qt Creator"}, {".net", ".NET"}, {"spring", "Spring"}, {"django", "Django"}, {"flask", "Flask"},
    {"linux", "Linux"}, {"windows server", "Windows Server"}, {"windows", "Windows"}, {"unix", "Unix"}, {"docker", "Docker"},
    {"kubernetes", "Kubernetes"}, {"git", "Git"}, {"github", "GitHub"}, {"jenkins", "Jenkins"}, {"ansible", "Ansible"},
    {"azure", "Azure"}, {"aws", "AWS"}, {"google cloud", "Google Cloud"}, {"vmware", "VMware"}, {"tcp/ip", "TCP/IP"},
    {"vlan", "VLAN"}, {"vpn", "VPN"}, {"dns", "DNS"}, {"dhcp", "DHCP"}, {"lan", "LAN"}, {"wan", "WAN"}, {"cisco", "Cisco"},
    {"cisco ios", "Cisco IOS"}, {"wireshark", "Wireshark"}, {"packet tracer", "Packet Tracer"}, {"gns3", "GNS3"},
    {"fortinet", "Fortinet"}, {"active directory", "Active Directory"}, {"matlab", "Matlab"}, {"simulink", "Simulink"},
    {"labview", "LabVIEW"}, {"autocad", "AutoCAD"}, {"solidworks", "SolidWorks"}, {"excel", "Excel"}, {"power bi", "Power BI"},
    {"tableau", "Tableau"}, {"scada", "SCADA"}, {"automate programmable", "Automate programmable"}, {"plc", "PLC"},
    {"arduino", "Arduino"}, {"raspberry pi", "Raspberry Pi"}, {"stm32", "STM32"}, {"iot", "IoT"}, {"uml", "UML"},
    {"scrum", "Scrum"}, {"agile", "Agile"}, {"itil", "ITIL"}, {"maintenance industrielle", "Maintenance industrielle"},
    {"maintenance preventive", "Maintenance pr\xC3\xA9ventive"}, {"maintenance corrective", "Maintenance corrective"},
    {"automatisme", "Automatisme"}, {"electronique", "\xC3\x89lectronique"}, {"instrumentation", "Instrumentation"},
    {"diagnostic de pannes", "Diagnostic de pannes"}, {"supervision", "Supervision"}, {"telecommunications", "T\xC3\xA9l\xC3\xA9" "communications"},
    {"support technique", "Support technique"}, {"reseaux", "R\xC3\xA9seaux"}, {"networking", "Networking"},
    {"electrotechnique", "\xC3\x89lectrotechnique"}, {"mecatronique", "M\xC3\xA9" "catronique"}, {"systemes embarques", "Syst\xC3\xA8mes embarqu\xC3\xA9s"},
    {"meteorologie", "M\xC3\xA9t\xC3\xA9orologie"}, {"opencv", "OpenCV"}, {"machine learning", "Machine Learning"},
    {"tensorflow", "TensorFlow"}, {"pandas", "Pandas"}, {"numpy", "NumPy"}, {"bash", "Bash"}, {"powershell", "PowerShell"},
    {"mqtt", "MQTT"}, {"modbus", "Modbus"}, {"snmp", "SNMP"}, {"nagios", "Nagios"}, {"zabbix", "Zabbix"}, {"gps", "GPS"},
    {"capteurs", "Capteurs"}, {"gestion de projet", "Gestion de projet"}, {"cybersecurite", "Cybers\xC3\xA9" "curit\xC3\xA9"}};

const Term kCerts[] = {
    {"ccna", "Cisco CCNA"}, {"ccnp", "Cisco CCNP"}, {"ccie", "Cisco CCIE"}, {"ccent", "Cisco CCENT"}, {"comptia", "CompTIA"},
    {"a+", "CompTIA A+"}, {"network+", "CompTIA Network+"}, {"security+", "CompTIA Security+"}, {"mcsa", "Microsoft MCSA"},
    {"mcse", "Microsoft MCSE"}, {"az-900", "Microsoft AZ-900"}, {"az-104", "Microsoft AZ-104"}, {"az-500", "Microsoft AZ-500"},
    {"ms-900", "Microsoft MS-900"}, {"aws certified", "AWS Certified"}, {"pmp", "PMP"}, {"itil", "ITIL"}, {"prince2", "PRINCE2"},
    {"scrum master", "Scrum Master"}, {"psm", "Professional Scrum Master"}, {"cissp", "CISSP"}, {"ceh", "CEH"}, {"oscp", "OSCP"},
    {"lpic", "LPIC"}, {"rhcsa", "Red Hat RHCSA"}, {"rhce", "Red Hat RHCE"}, {"ocp", "Oracle OCP"}, {"vcp", "VMware VCP"},
    {"togaf", "TOGAF"}, {"iso 27001", "ISO 27001"}, {"iso 9001", "ISO 9001"}, {"six sigma", "Six Sigma"}, {"toeic", "TOEIC"},
    {"toefl", "TOEFL"}, {"ielts", "IELTS"}};

const char *kSpecialiteMots[] = {
    "maintenance", "reseau", "network", "telecom", "electronique", "electronic", "electrique", "electrotechnique",
    "automatisme", "automatique", "mecanique", "mecatronique", "informatique", "logiciel", "software", "developpement",
    "development", "web", "mobile", "data", "donnees", "cybersecurite", "securite", "systeme", "embarque", "iot",
    "meteo", "instrumentation", "industriel", "genie", "civil", "energie", "climatisation", "froid", "hydraulique",
    "topograph", "support", "cloud", "devops", "qualite", "logistique", "production", "telecommunication"};

// Alias de certifications (le critère "Cisco" est satisfait par une certification CCNA...)
std::vector<std::string> certAliases(const std::string &c)
{
    static const std::map<std::string, std::vector<std::string>> a = {
        {"cisco", {"cisco", "ccna", "ccnp", "ccie", "ccent", "ccda"}},
        {"microsoft", {"microsoft", "mcsa", "mcse", "mcsd", "az-900", "az-104", "az-500", "ms-900", "md-100", "sc-900", "pl-300"}},
        {"azure", {"azure", "az-900", "az-104", "az-500"}},
        {"aws", {"aws", "amazon web services"}},
        {"comptia", {"comptia", "a+", "network+", "security+", "linux+"}},
        {"google", {"google", "gcp"}},
        {"oracle", {"oracle", "ocp", "oca", "ocm"}},
        {"pmp", {"pmp", "project management professional"}},
        {"scrum", {"scrum", "psm", "csm", "psp"}},
        {"red hat", {"red hat", "redhat", "rhcsa", "rhce"}},
        {"linux", {"linux", "lpic", "lfcs", "rhcsa", "lfcs"}},
        {"vmware", {"vmware", "vcp"}}};
    auto it = a.find(c);
    if (it != a.end())
        return it->second;
    return {c};
}

// ============================================================================
//  Extraction des informations
// ============================================================================

bool hasJobWord(const std::string &norm)
{
    static const char *jobs[] = {"technicien", "technicienne", "ingenieur", "ingenieure", "developpeur", "developpeuse", "etudiant",
                                 "etudiante", "engineer", "manager", "responsable", "chef", "directeur", "directrice", "consultant",
                                 "consultante", "analyste", "administrateur", "administratrice", "assistant", "assistante", "agent",
                                 "stagiaire", "operateur", "specialiste", "expert", "architect", "designer", "developer", "technician",
                                 "student", "intern", "graduate", "junior", "senior", "curriculum", "vitae", "cv", "resume", "profil",
                                 "profile", "email", "mail", "tel", "telephone", "adresse", "address", "tunisie", "tunis", "france",
                                 "maintenance", "informatique", "reseaux", "electronique", "genie", "systemes", "cherche", "recherche",
                                 "poste", "emploi", "candidat", "disponible", "phone", "mobile", "gsm", "linkedin", "github", "nationalite",
                                 "date", "naissance", "ne", "nee", "celibataire", "marie", "permis"};
    for (const std::string &w : splitWords(norm)) {
        std::string t;
        for (unsigned char c : w)
            if (std::isalpha(c)) t.push_back(static_cast<char>(c));
        for (const char *j : jobs)
            if (t == j) return true;
    }
    return false;
}

bool alphaWord(const std::string &w)
{
    // lettres (y compris UTF-8), apostrophe, trait d'union
    int letters = 0;
    for (unsigned char c : w) {
        if (std::isalpha(c) || c >= 0x80) ++letters;
        else if (c != '\'' && c != '-') return false;
    }
    return letters >= 2;
}

bool isUpperWord(const std::string &w)
{
    int up = 0, low = 0;
    const std::u32string u = cvcore::utf8ToU32(w);
    for (char32_t c : u) {
        if ((c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDE && c != 0xD7)) ++up;
        else if ((c >= 'a' && c <= 'z') || (c >= 0xDF && c <= 0xFF)) ++low;
    }
    return up >= 2 && low == 0;
}

bool startsUpper(const std::string &w)
{
    const std::u32string u = cvcore::utf8ToU32(w);
    if (u.empty()) return false;
    const char32_t c = u[0];
    return (c >= 'A' && c <= 'Z') || (c >= 0xC0 && c <= 0xDE && c != 0xD7);
}

void splitNom(const std::string &full, CvInfo &info, bool labelled)
{
    const std::vector<std::string> words = splitWords(full);
    std::vector<std::string> nomW, prenomW;
    for (const std::string &w : words)
        (isUpperWord(w) ? nomW : prenomW).push_back(w);
    auto join = [](const std::vector<std::string> &v) {
        std::string r;
        for (const std::string &x : v) { if (!r.empty()) r += ' '; r += x; }
        return r;
    };
    if (!nomW.empty() && !prenomW.empty()) {
        info.nom = join(nomW);
        info.prenom = join(prenomW);
        info.nomIncertain = false;
    } else if (words.size() >= 2) {
        info.prenom = words.front();
        info.nom = join(std::vector<std::string>(words.begin() + 1, words.end()));
        info.nomIncertain = !labelled;
    } else if (words.size() == 1) {
        info.nom = words.front();
        info.nomIncertain = true;
    }
}

void extraireNom(const std::vector<Line> &lines, CvInfo &info)
{
    // 1) libellés explicites
    std::string nomLabel, prenomLabel, full;
    const int lim = std::min<int>(static_cast<int>(lines.size()), 40);
    for (int i = 0; i < lim; ++i) {
        const Line &L = lines[static_cast<std::size_t>(i)];
        const std::size_t c = L.norm.find(':');
        const std::size_t co = L.orig.find(':');
        if (c == std::string::npos || co == std::string::npos)
            continue;
        const std::string key = trim(L.norm.substr(0, c));
        const std::string val = trim(L.orig.substr(co + 1));
        if (val.empty() || val.size() > 60)
            continue;
        if (key == "nom et prenom" || key == "nom & prenom" || key == "nom prenom" || key == "nom complet" || key == "name"
            || key == "full name" || key == "prenom et nom" || key == "prenom nom") {
            if (full.empty()) full = val;
        } else if (key == "nom" || key == "nom de famille" || key == "last name" || key == "surname") {
            if (nomLabel.empty()) nomLabel = val;
        } else if (key == "prenom" || key == "first name" || key == "prenoms") {
            if (prenomLabel.empty()) prenomLabel = val;
        }
    }
    if (!nomLabel.empty() || !prenomLabel.empty()) {
        info.nom = nomLabel;
        info.prenom = prenomLabel;
        if (info.nom.empty() || info.prenom.empty())
            info.nomIncertain = false;
        return;
    }
    if (!full.empty()) {
        splitNom(full, info, true);
        return;
    }

    // 2) première ligne qui ressemble à un nom, avant toute section
    int seen = 0;
    for (const Line &L : lines) {
        if (L.section != SecHeader || L.heading)
            break;
        if (++seen > 12)
            break;
        if (L.orig.find('@') != std::string::npos || L.orig.find(',') != std::string::npos
            || L.orig.find('|') != std::string::npos || L.orig.find(':') != std::string::npos)
            continue;
        bool digit = false;
        for (unsigned char c : L.orig) if (std::isdigit(c)) digit = true;
        if (digit)
            continue;
        const std::vector<std::string> w = splitWords(L.orig);
        if (w.size() < 2 || w.size() > 4)
            continue;
        if (hasJobWord(L.norm))
            continue;
        bool ok = true;
        for (const std::string &x : w)
            if (!alphaWord(x) || !(startsUpper(x) || isUpperWord(x))) { ok = false; break; }
        if (!ok)
            continue;
        splitNom(L.orig, info, false);
        return;
    }
}

void extraireEmail(const std::string &text, CvInfo &info)
{
    std::size_t pos = 0;
    while ((pos = text.find('@', pos)) != std::string::npos) {
        std::size_t a = pos, b = pos + 1;
        auto okLocal = [](unsigned char c) { return std::isalnum(c) || c == '.' || c == '_' || c == '%' || c == '+' || c == '-'; };
        auto okDom = [](unsigned char c) { return std::isalnum(c) || c == '.' || c == '-'; };
        while (a > 0 && okLocal(static_cast<unsigned char>(text[a - 1]))) --a;
        while (b < text.size() && okDom(static_cast<unsigned char>(text[b]))) ++b;
        std::string e = text.substr(a, b - a);
        while (!e.empty() && (e.back() == '.' || e.back() == '-')) e.pop_back();
        const std::size_t at = e.find('@');
        const std::size_t dot = e.rfind('.');
        if (at != std::string::npos && at > 0 && dot != std::string::npos && dot > at + 1 && e.size() - dot - 1 >= 2) {
            info.email = e;
            return;
        }
        ++pos;
    }
}

void extraireTelephone(const std::vector<Line> &lines, CvInfo &info)
{
    int idx = 0;
    for (const Line &L : lines) {
        ++idx;
        const bool kw = contains(L.norm, "tel") || contains(L.norm, "phone") || contains(L.norm, "mobile") || contains(L.norm, "gsm");
        if (!kw && !(L.section == SecHeader && idx <= 12))
            continue;
        if (L.section == SecExperience || L.section == SecEducation)
            continue;

        // séquences de chiffres séparées par espaces, points, tirets, parenthèses
        std::size_t i = 0;
        const std::string &s = L.orig;
        while (i < s.size()) {
            if (!(std::isdigit(static_cast<unsigned char>(s[i])) || s[i] == '+')) { ++i; continue; }
            std::string digits;
            std::vector<int> groups;
            int g = 0;
            std::size_t j = i;
            if (s[j] == '+') { digits = "+"; ++j; }
            while (j < s.size()) {
                const unsigned char c = static_cast<unsigned char>(s[j]);
                if (std::isdigit(c)) { digits.push_back(static_cast<char>(c)); ++g; }
                else if (c == ' ' || c == '.' || c == '-' || c == '(' || c == ')') { if (g) { groups.push_back(g); g = 0; } }
                else break;
                ++j;
            }
            if (g) groups.push_back(g);
            i = std::max(j, i + 1);

            std::string d;
            for (char c : digits) if (std::isdigit(static_cast<unsigned char>(c))) d.push_back(c);
            // plage d'années "2017 - 2020" : ce n'est pas un téléphone
            if (groups.size() == 2 && groups[0] == 4 && groups[1] == 4 && (startsWith(d, "19") || startsWith(d, "20")))
                continue;
            if (startsWith(d, "00216")) d = d.substr(5);
            else if (startsWith(d, "216") && d.size() == 11) d = d.substr(3);
            if (d.size() == 8) { info.telephone = d; return; }
        }
    }
}

int rangFormation(const std::string &n)
{
    if (contains(n, "doctorat") || findTerm(n, "phd")) return 5;
    if (findWordPrefix(n, "master") || contains(n, "mastere") || findTerm(n, "msc") || findTerm(n, "mba")
        || contains(n, "cycle ingenieur") || contains(n, "diplome d'ingenieur") || contains(n, "diplome d ingenieur")
        || contains(n, "ingenieur") || contains(n, "engineering degree") || contains(n, "engineer's degree")) return 4;
    if (findWordPrefix(n, "licence") || findWordPrefix(n, "bachelor") || findTerm(n, "bsc") || findTerm(n, "licence")) return 3;
    if (findTerm(n, "bts") || findTerm(n, "dut") || findTerm(n, "deug") || contains(n, "technicien superieur")
        || contains(n, "cycle preparatoire") || contains(n, "classes preparatoires")) return 2;
    if (contains(n, "baccalaureat") || findTerm(n, "bac") || contains(n, "high school")) return 1;
    return 0;
}

bool strongDegree(const std::string &n)
{
    return findWordPrefix(n, "licence") || findWordPrefix(n, "master") || contains(n, "mastere") || contains(n, "doctorat")
           || findTerm(n, "phd") || findWordPrefix(n, "bachelor") || findTerm(n, "bts") || findTerm(n, "dut")
           || contains(n, "diplome") || findTerm(n, "msc") || findTerm(n, "bsc") || contains(n, "cycle ingenieur")
           || contains(n, "diplome d'ingenieur");
}

void extraireFormation(const std::vector<Line> &lines, CvInfo &info)
{
    std::vector<const Line *> cand;
    std::string details;
    for (const Line &L : lines) {
        if (L.heading)
            continue;
        const bool inEdu = (L.section == SecEducation);
        const bool outside = (L.section == SecHeader || L.section == SecProfile || L.section == SecOther);
        if (inEdu || (outside && strongDegree(L.norm))) {
            cand.push_back(&L);
            if (!details.empty()) details += "\n";
            details += L.orig;
        }
    }
    info.formationDetails = details;
    if (cand.empty())
        return;

    const Line *best = nullptr;
    int bestRank = 0;
    for (const Line *L : cand) {
        const int r = rangFormation(L->norm);
        if (r > bestRank) { bestRank = r; best = L; }
    }
    if (!best)
        best = cand.front();
    info.formation = best->orig;
}

// ---- Expérience ------------------------------------------------------------

struct DateTok { int year = 0; int month = 0; bool present = false; std::size_t start = 0, end = 0; };

int monthFromName(const std::string &w)
{
    static const char *names[12][3] = {{"janvier", "jan", "january"}, {"fevrier", "fev", "february"}, {"mars", "mar", "march"},
                                       {"avril", "avr", "april"}, {"mai", "may", ""}, {"juin", "jun", "june"},
                                       {"juillet", "juil", "july"}, {"aout", "aou", "august"}, {"septembre", "sept", "september"},
                                       {"octobre", "oct", "october"}, {"novembre", "nov", "november"}, {"decembre", "dec", "december"}};
    for (int m = 0; m < 12; ++m)
        for (int k = 0; k < 3; ++k)
            if (names[m][k][0] && w == names[m][k]) return m + 1;
    return 0;
}

std::vector<DateTok> dateTokens(const std::string &n, int curYear)
{
    std::vector<DateTok> toks;
    std::size_t i = 0;
    const std::size_t len = n.size();
    while (i < len) {
        // mots-clés "présent"
        static const char *pres[] = {"aujourd'hui", "aujourdhui", "presente", "present", "actuellement", "actuel", "en cours", "ce jour", "current", "now", "today", "to date"};
        bool matched = false;
        for (const char *p : pres) {
            const std::string ps(p);
            if (n.compare(i, ps.size(), ps) == 0 && (i == 0 || !isAl(static_cast<unsigned char>(n[i - 1])))
                && (i + ps.size() >= len || !isAl(static_cast<unsigned char>(n[i + ps.size()])))) {
                DateTok t;
                t.present = true;
                t.start = i;
                t.end = i + ps.size();
                toks.push_back(t);
                i += ps.size();
                matched = true;
                break;
            }
        }
        if (matched)
            continue;

        // MM/YYYY, MM-YYYY, MM.YYYY  ou YYYY
        if (std::isdigit(static_cast<unsigned char>(n[i])) && (i == 0 || !isAl(static_cast<unsigned char>(n[i - 1])))) {
            std::size_t j = i;
            while (j < len && std::isdigit(static_cast<unsigned char>(n[j]))) ++j;
            const std::string a = n.substr(i, j - i);
            if (a.size() == 4 && (a[0] == '1' || a[0] == '2') && (j >= len || !isAl(static_cast<unsigned char>(n[j])))) {
                const int y = std::atoi(a.c_str());
                if (y >= 1970 && y <= curYear + 1) {
                    DateTok t;
                    t.year = y;
                    t.start = i;
                    t.end = j;
                    toks.push_back(t);
                }
                i = j;
                continue;
            }
            if (a.size() <= 2 && j + 5 <= len && (n[j] == '/' || n[j] == '-' || n[j] == '.')) {
                std::size_t k = j + 1;
                std::size_t k2 = k;
                while (k2 < len && std::isdigit(static_cast<unsigned char>(n[k2]))) ++k2;
                const std::string b = n.substr(k, k2 - k);
                const int m = std::atoi(a.c_str());
                int y = 0;
                if (b.size() == 4) y = std::atoi(b.c_str());
                else if (b.size() == 2) y = 2000 + std::atoi(b.c_str());
                if (m >= 1 && m <= 12 && y >= 1970 && y <= curYear + 1 && (b.size() == 4 || b.size() == 2)) {
                    DateTok t;
                    t.year = y;
                    t.month = m;
                    t.start = i;
                    t.end = k2;
                    toks.push_back(t);
                    i = k2;
                    continue;
                }
            }
            i = j;
            continue;
        }

        // "mars 2021", "sept. 2020"
        if (std::isalpha(static_cast<unsigned char>(n[i])) && (i == 0 || !isAl(static_cast<unsigned char>(n[i - 1])))) {
            std::size_t j = i;
            while (j < len && std::isalpha(static_cast<unsigned char>(n[j]))) ++j;
            const int m = monthFromName(n.substr(i, j - i));
            if (m) {
                std::size_t k = j;
                if (k < len && n[k] == '.') ++k;
                while (k < len && n[k] == ' ') ++k;
                std::size_t k2 = k;
                while (k2 < len && std::isdigit(static_cast<unsigned char>(n[k2]))) ++k2;
                if (k2 - k == 4) {
                    const int y = std::atoi(n.substr(k, 4).c_str());
                    if (y >= 1970 && y <= curYear + 1) {
                        DateTok t;
                        t.year = y;
                        t.month = m;
                        t.start = i;
                        t.end = k2;
                        toks.push_back(t);
                        i = k2;
                        continue;
                    }
                }
            }
            i = j;
            continue;
        }
        ++i;
    }
    return toks;
}

bool separatorBetween(const std::string &s)
{
    const std::string t = trim(s);
    static const char *ok[] = {"-", "a", "au", "to", "->", "jusqu'a", "jusqu'au", "jusqu a", "until", "/", "through", "et", "--", "~"};
    for (const char *o : ok)
        if (t == o) return true;
    return false;
}

int experienceCalculee(const std::vector<Line> &lines, int curYear, int curMonth)
{
    std::vector<std::pair<int, int>> iv;
    bool hasSection = false;
    for (const Line &L : lines) {
        if (L.section != SecExperience || L.heading)
            continue;
        hasSection = true;
        const std::vector<DateTok> toks = dateTokens(L.norm, curYear);
        for (std::size_t i = 0; i + 1 < toks.size(); ++i) {
            const DateTok &a = toks[i];
            const DateTok &b = toks[i + 1];
            if (a.present)
                continue;
            if (!separatorBetween(L.norm.substr(a.end, b.start - a.end)))
                continue;
            int s = a.year * 12 + ((a.month ? a.month : 1) - 1);
            int e;
            if (b.present)
                e = curYear * 12 + (curMonth - 1);
            else if (!a.month && !b.month)
                e = b.year * 12;                 // "2019 - 2022" -> 3 ans
            else
                e = b.year * 12 + ((b.month ? b.month : 12) - 1);
            if (e > s && e - s <= 50 * 12)
                iv.emplace_back(s, e);
            ++i;   // la fin de cette période ne démarre pas une nouvelle période
        }
    }
    if (!hasSection || iv.empty())
        return -1;
    std::sort(iv.begin(), iv.end());
    int total = 0, cs = iv[0].first, ce = iv[0].second;
    for (std::size_t i = 1; i < iv.size(); ++i) {
        if (iv[i].first <= ce) ce = std::max(ce, iv[i].second);
        else { total += ce - cs; cs = iv[i].first; ce = iv[i].second; }
    }
    total += ce - cs;
    return total / 12;
}

int experienceDeclaree(const std::vector<Line> &lines)
{
    int best = -1;
    for (const Line &L : lines) {
        if (L.heading)
            continue;
        const std::string &n = L.norm;
        if (!contains(n, "experience"))
            continue;
        std::size_t i = 0;
        while (i < n.size()) {
            if (!std::isdigit(static_cast<unsigned char>(n[i])) || (i > 0 && (isAl(static_cast<unsigned char>(n[i - 1])) || n[i - 1] == '/'))) { ++i; continue; }
            std::size_t j = i;
            while (j < n.size() && std::isdigit(static_cast<unsigned char>(n[j]))) ++j;
            const std::string num = n.substr(i, j - i);
            std::size_t k = j;
            while (k < n.size() && (n[k] == ' ' || n[k] == '+')) ++k;
            static const char *units[] = {"ans", "an", "annees", "annee", "years", "year", "yrs", "yr"};
            for (const char *u : units) {
                const std::string us(u);
                if (n.compare(k, us.size(), us) == 0 && (k + us.size() >= n.size() || !isAl(static_cast<unsigned char>(n[k + us.size()])))) {
                    if (num.size() <= 2) {
                        const int v = std::atoi(num.c_str());
                        if (v >= 0 && v <= 60) best = std::max(best, v);
                    }
                    break;
                }
            }
            i = j;
        }
    }
    return best;
}

// ---- Spécialité ------------------------------------------------------------

bool hasSpecialityWord(const std::string &n)
{
    for (const char *w : kSpecialiteMots)
        if (findWordPrefix(n, w)) return true;
    return false;
}

std::string stripTitlePrefix(const std::string &orig, const std::string &norm)
{
    static const char *titles[] = {"technicienne", "technicien", "ingenieure", "ingenieur", "etudiante", "etudiant", "responsable",
                                   "chef de projet", "consultante", "consultant", "analyste", "administrateur", "administratrice",
                                   "assistante", "assistant", "developpeur", "developpeuse", "specialiste", "expert"};
    static const char *links[] = {" specialise en ", " specialisee en ", " specialiste en ", " en ", " de ", " d'", " des ", " du ", " d ", " "};
    for (const char *t : titles) {
        const std::string ts(t);
        if (!startsWith(norm, ts))
            continue;
        for (const char *l : links) {
            const std::string ls(l);
            if (norm.compare(ts.size(), ls.size(), ls) == 0 && ts.size() + ls.size() < norm.size()) {
                // même position dans le texte d'origine (la normalisation conserve la longueur des mots ASCII ;
                // on recoupe donc par nombre de mots pour rester fiable avec les accents)
                const std::size_t wordsToDrop = splitWords(ts + ls).size();
                const std::vector<std::string> w = splitWords(orig);
                if (wordsToDrop >= w.size())
                    return orig;
                std::string r;
                for (std::size_t i = wordsToDrop; i < w.size(); ++i) {
                    if (!r.empty()) r += ' ';
                    r += w[i];
                }
                // "d'électronique" : on enlève l'élision
                if (startsWith(norm.substr(ts.size()), " d'")) {
                    const std::size_t ap = orig.find('\'');
                    if (ap != std::string::npos) return trim(orig.substr(ap + 1));
                }
                return r;
            }
        }
    }
    return orig;
}

void extraireSpecialite(const std::vector<Line> &lines, CvInfo &info)
{
    // 1) libellé explicite
    for (const Line &L : lines) {
        const std::size_t c = L.norm.find(':');
        const std::size_t co = L.orig.find(':');
        if (c == std::string::npos || co == std::string::npos)
            continue;
        const std::string key = trim(L.norm.substr(0, c));
        if (key == "specialite" || key == "specialty" || key == "domaine" || key == "domaine d'expertise" || key == "specialisation"
            || key == "domaine de competence") {
            const std::string v = trim(L.orig.substr(co + 1));
            if (!v.empty() && v.size() <= 80) {
                info.specialite = v;
                return;
            }
        }
    }
    // 2) intitulé du profil sous le nom (avant toute section)
    int seen = 0;
    for (const Line &L : lines) {
        if (L.section != SecHeader || L.heading)
            break;
        if (++seen > 8)
            break;
        if (L.orig.find('@') != std::string::npos || L.orig.find(':') != std::string::npos)
            continue;
        bool digit = false;
        for (unsigned char c : L.orig) if (std::isdigit(c)) digit = true;
        if (digit)
            continue;
        const std::vector<std::string> w = splitWords(L.orig);
        if (w.size() < 2 || w.size() > 9)
            continue;
        if (!hasSpecialityWord(L.norm))
            continue;
        if (L.norm == normaliser(info.prenom + " " + info.nom) || L.norm == normaliser(info.nom + " " + info.prenom))
            continue;
        info.specialite = stripTitlePrefix(L.orig, L.norm);
        return;
    }
    // 3) mot-clé de spécialité le plus fréquent (en-tête, profil, titres de postes)
    std::map<std::string, int> freq;
    static const Term domains[] = {{"maintenance", "Maintenance"}, {"reseau", "R\xC3\xA9seaux"}, {"telecom", "T\xC3\xA9l\xC3\xA9" "communications"},
                                   {"electronique", "\xC3\x89lectronique"}, {"automatisme", "Automatisme"},
                                   {"informatique", "Informatique"}, {"mecanique", "M\xC3\xA9" "canique"},
                                   {"genie civil", "G\xC3\xA9nie civil"}, {"developpement", "D\xC3\xA9veloppement"},
                                   {"cybersecurite", "Cybers\xC3\xA9" "curit\xC3\xA9"}, {"energie", "\xC3\x89nergie"},
                                   {"meteo", "M\xC3\xA9t\xC3\xA9orologie"}};
    for (const Line &L : lines) {
        if (L.heading)
            continue;
        if (L.section == SecHeader || L.section == SecProfile || L.section == SecExperience)
            for (const Term &t : domains)
                if (findWordPrefix(L.norm, t.norm))
                    freq[t.display]++;
    }
    int best = 0;
    for (const auto &kv : freq)
        if (kv.second > best) { best = kv.second; info.specialite = kv.first; }
}

// ---- Compétences -----------------------------------------------------------

void ajouterUnique(std::vector<std::string> &list, std::set<std::string> &seen, const std::string &display, std::size_t max)
{
    const std::string key = collapseSpaces(normaliser(display));
    if (key.empty() || seen.count(key) || list.size() >= max)
        return;
    seen.insert(key);
    list.push_back(display);
}

void extraireCompetences(const std::vector<Line> &lines, const std::string &fullNorm, CvInfo &info)
{
    std::set<std::string> seen;
    // 1) dictionnaire : uniquement ce qui est réellement écrit dans le CV
    for (const Term &t : kSkills)
        if (findTerm(fullNorm, t.norm))
            ajouterUnique(info.competences, seen, t.display, 60);

    // 2) éléments listés dans la section "Compétences"
    for (const Line &L : lines) {
        if (L.section != SecSkills || L.heading)
            continue;
        std::string content = L.orig;
        const std::size_t colon = content.find(':');
        if (colon != std::string::npos && colon < 40)
            content = content.substr(colon + 1);
        std::string item;
        auto flush = [&]() {
            std::string t = trim(item);
            while (!t.empty() && (t.back() == '.' || t.back() == ';')) t.pop_back();
            item.clear();
            if (t.size() < 2 || t.size() > 35 || splitWords(t).size() > 4)
                return;
            ajouterUnique(info.competences, seen, t, 60);
        };
        for (std::size_t i = 0; i < content.size(); ++i) {
            const char c = content[i];
            const bool dot = (static_cast<unsigned char>(c) == 0xC2 && i + 1 < content.size() && static_cast<unsigned char>(content[i + 1]) == 0xB7);
            const bool bullet = (static_cast<unsigned char>(c) == 0xE2 && i + 2 < content.size() && static_cast<unsigned char>(content[i + 1]) == 0x80
                                 && static_cast<unsigned char>(content[i + 2]) == 0xA2);
            if (c == ',' || c == ';' || c == '|' || dot || bullet) {
                flush();
                if (dot) ++i;
                if (bullet) i += 2;
            } else {
                item.push_back(c);
            }
        }
        flush();
    }
}

// ---- Certifications --------------------------------------------------------

void extraireCertifications(const std::vector<Line> &lines, const std::string &fullNorm, CvInfo &info, std::string &zoneNorm)
{
    std::set<std::string> seen;
    std::string zone;
    for (const Line &L : lines) {
        if (L.heading)
            continue;
        const bool inSec = (L.section == SecCerts);
        const bool kw = contains(L.norm, "certifi") || contains(L.norm, "attestation");
        if (inSec || (kw && L.section != SecExperience)) {
            std::string t = L.orig;
            if (t.size() > 90)
                t = t.substr(0, 90);
            ajouterUnique(info.certifications, seen, t, 15);
            zone += L.norm + "\n";
        }
    }
    for (const Term &t : kCerts) {
        if (findTerm(fullNorm, t.norm)) {
            zone += std::string(t.norm) + "\n";
            // évite le doublon si la ligne du CV contient déjà ce code
            bool already = false;
            for (const std::string &c : info.certifications)
                if (findTerm(normaliser(c), t.norm)) { already = true; break; }
            if (!already)
                ajouterUnique(info.certifications, seen, t.display, 15);
        }
    }
    zoneNorm = zone;
}
} // namespace

// ============================================================================
//  Analyse d'un CV
// ============================================================================

CvInfo analyserTexte(const std::string &texteUtf8, int anneeCourante, int moisCourant)
{
    CvInfo info;
    const std::vector<Line> lines = decouper(texteUtf8);
    info.nbLignes = static_cast<int>(lines.size());

    std::string fullNorm;
    for (const Line &L : lines)
        fullNorm += L.norm + "\n";

    extraireNom(lines, info);
    extraireEmail(texteUtf8, info);
    extraireTelephone(lines, info);
    extraireFormation(lines, info);

    const int decl = experienceDeclaree(lines);
    const int calc = experienceCalculee(lines, anneeCourante, moisCourant);
    if (decl >= 0 || calc >= 0) {
        if (calc > decl) {
            info.experienceAns = calc;
            info.experienceSource = "calcul\xC3\xA9" "e \xC3\xA0 partir des p\xC3\xA9riodes indiqu\xC3\xA9" "es";
        } else {
            info.experienceAns = decl;
            info.experienceSource = "d\xC3\xA9" "clar\xC3\xA9" "e dans le CV";
        }
    }

    extraireSpecialite(lines, info);
    extraireCompetences(lines, fullNorm, info);
    std::string certZone;
    extraireCertifications(lines, fullNorm, info, certZone);

    // zones utilisées plus tard par evaluer() : stockées dans l'objet via champs dédiés
    info.zoneFormation = normaliser(info.formationDetails);
    info.zoneCertifications = certZone;
    {
        std::string zs;
        int seen = 0;
        for (const Line &L : lines) {
            if (L.heading)
                continue;
            if (L.section == SecHeader) {
                if (++seen <= 8) zs += L.norm + "\n";
            } else if (L.section == SecProfile || L.section == SecExperience) {
                zs += L.norm + "\n";
            } else if (contains(L.norm, "specialite") || contains(L.norm, "domaine")) {
                zs += L.norm + "\n";
            }
        }
        zs += normaliser(info.specialite) + "\n";
        info.zoneSpecialite = zs;
    }
    return info;
}

// ============================================================================
//  Évaluation
// ============================================================================

std::vector<Critere> normaliserCriteres(const std::vector<Critere> &criteres)
{
    std::vector<Critere> r;
    for (const Critere &c : criteres) {
        if (c.type == TypeCritere::Competence) {
            for (const std::string &v : splitAlternatives(c.valeur, false)) {
                Critere x = c;
                x.valeur = v;
                r.push_back(x);
            }
        } else if (!trim(c.valeur).empty()) {
            Critere x = c;
            x.valeur = trim(c.valeur);
            r.push_back(x);
        }
    }
    return r;
}

namespace
{
std::string joinShort(const std::vector<std::string> &v, std::size_t max)
{
    std::string r;
    for (std::size_t i = 0; i < v.size() && i < max; ++i) {
        if (!r.empty()) r += ", ";
        r += v[i];
    }
    if (v.size() > max) r += ", ...";
    return r;
}
}

Evaluation evaluer(const CvInfo &info, const std::string &texteUtf8, const std::vector<Critere> &criteresIn)
{
    Evaluation ev;
    const std::vector<Critere> criteres = normaliserCriteres(criteresIn);

    std::string fullNorm;
    for (const Line &L : decouper(texteUtf8))
        fullNorm += L.norm + "\n";

    for (const Critere &c : criteres) {
        if (!c.actif)
            continue;
        ResultatCritere r;
        r.critere = c;
        const std::string vNorm = collapseSpaces(normaliser(c.valeur));

        switch (c.type) {
        case TypeCritere::ExperienceMin: {
            int need = 0;
            for (char ch : vNorm) if (std::isdigit(static_cast<unsigned char>(ch))) { need = need * 10 + (ch - '0'); if (need > 100) break; }
            if (info.experienceAns < 0) {
                r.verdict = Verdict::Manquant;
                r.detail = "Aucune dur\xC3\xA9" "e d'exp\xC3\xA9rience n'a pu \xC3\xAA" "tre d\xC3\xA9" "termin\xC3\xA9" "e dans le CV (ni "
                           "\xC2\xAB X ans d'exp\xC3\xA9rience \xC2\xBB, ni p\xC3\xA9riodes dat\xC3\xA9" "es).";
            } else if (info.experienceAns >= need) {
                r.verdict = Verdict::Respecte;
                r.detail = std::to_string(info.experienceAns) + " an(s) d'exp\xC3\xA9rience (" + info.experienceSource + ") pour "
                           + std::to_string(need) + " an(s) demand\xC3\xA9" "s.";
            } else {
                r.verdict = Verdict::NonRespecte;
                r.detail = "Seulement " + std::to_string(info.experienceAns) + " an(s) d'exp\xC3\xA9rience (" + info.experienceSource
                           + ") pour " + std::to_string(need) + " an(s) demand\xC3\xA9" "s.";
            }
            break;
        }
        case TypeCritere::Formation: {
            if (info.zoneFormation.empty()) {
                r.verdict = Verdict::Manquant;
                r.detail = "Aucune formation ni aucun dipl\xC3\xB4" "me n'a \xC3\xA9t\xC3\xA9 d\xC3\xA9tect\xC3\xA9 dans le CV.";
                break;
            }
            bool ok = false;
            for (const std::string &alt : splitAlternatives(c.valeur, true))
                if (matchWords(info.zoneFormation, collapseSpaces(normaliser(alt)))) { ok = true; break; }
            r.verdict = ok ? Verdict::Respecte : Verdict::NonRespecte;
            r.detail = ok ? "Trouv\xC3\xA9 dans la formation : \xC2\xAB " + info.formation + " \xC2\xBB."
                          : "Formation indiqu\xC3\xA9" "e dans le CV : \xC2\xAB " + info.formation + " \xC2\xBB (ne correspond pas).";
            break;
        }
        case TypeCritere::Specialite: {
            bool ok = false;
            for (const std::string &alt : splitAlternatives(c.valeur, true))
                if (matchWords(info.zoneSpecialite, collapseSpaces(normaliser(alt)))) { ok = true; break; }
            if (ok) {
                r.verdict = Verdict::Respecte;
                r.detail = info.specialite.empty() ? "Mentionn\xC3\xA9" "e dans le profil ou l'exp\xC3\xA9rience du CV."
                                                   : "Sp\xC3\xA9" "cialit\xC3\xA9 d\xC3\xA9tect\xC3\xA9" "e : \xC2\xAB " + info.specialite + " \xC2\xBB.";
            } else if (info.specialite.empty()) {
                r.verdict = Verdict::Manquant;
                r.detail = "Aucune sp\xC3\xA9" "cialit\xC3\xA9 n'a pu \xC3\xAA" "tre identifi\xC3\xA9" "e dans le CV.";
            } else {
                r.verdict = Verdict::NonRespecte;
                r.detail = "Sp\xC3\xA9" "cialit\xC3\xA9 d\xC3\xA9tect\xC3\xA9" "e : \xC2\xAB " + info.specialite + " \xC2\xBB (ne correspond pas).";
            }
            break;
        }
        case TypeCritere::Competence: {
            if (findTerm(fullNorm, vNorm)) {
                r.verdict = Verdict::Respecte;
                r.detail = "Mentionn\xC3\xA9" "e dans le CV.";
            } else if (info.competences.empty()) {
                r.verdict = Verdict::Manquant;
                r.detail = "Aucune comp\xC3\xA9tence n'a pu \xC3\xAA" "tre d\xC3\xA9tect\xC3\xA9" "e dans le CV.";
            } else {
                r.verdict = Verdict::NonRespecte;
                r.detail = "Non mentionn\xC3\xA9" "e dans le CV.";
            }
            break;
        }
        case TypeCritere::Certification: {
            bool ok = false;
            for (const std::string &alt : splitAlternatives(c.valeur, true)) {
                const std::string an = collapseSpaces(normaliser(alt));
                for (const std::string &al : certAliases(an)) {
                    const bool prefix = !al.empty() && al.back() == '-';
                    if (prefix ? contains(info.zoneCertifications, al) : findTerm(info.zoneCertifications, al)) { ok = true; break; }
                }
                if (ok) break;
            }
            if (ok) {
                r.verdict = Verdict::Respecte;
                r.detail = "Certification trouv\xC3\xA9" "e : " + joinShort(info.certifications, 4) + ".";
            } else if (info.certifications.empty()) {
                r.verdict = Verdict::Manquant;
                r.detail = "Aucune certification n'est mentionn\xC3\xA9" "e dans le CV.";
            } else {
                r.verdict = Verdict::NonRespecte;
                r.detail = "Certifications du CV : " + joinShort(info.certifications, 4) + " (ne correspondent pas).";
            }
            break;
        }
        }

        const int w = c.obligatoire ? POIDS_OBLIGATOIRE : POIDS_SOUHAITABLE;
        ev.poidsTotal += w;
        if (r.verdict == Verdict::Respecte)
            ev.poidsObtenu += w;
        else if (c.obligatoire)
            ev.nbObligatoiresNonRespectes++;
        ev.resultats.push_back(r);
    }

    if (ev.poidsTotal > 0)
        ev.pourcentage = (100 * ev.poidsObtenu + ev.poidsTotal / 2) / ev.poidsTotal;
    ev.conforme = !ev.resultats.empty() && ev.nbObligatoiresNonRespectes == 0;

    if (ev.resultats.empty()) {
        ev.raison = "Aucun crit\xC3\xA8re actif n'est d\xC3\xA9" "fini.";
    } else if (ev.conforme) {
        ev.raison = "Tous les crit\xC3\xA8res obligatoires sont respect\xC3\xA9s.";
    } else {
        std::string s = "Crit\xC3\xA8res obligatoires non satisfaits : ";
        bool first = true;
        for (const ResultatCritere &r : ev.resultats) {
            if (!r.critere.obligatoire || r.verdict == Verdict::Respecte)
                continue;
            if (!first) s += " ; ";
            first = false;
            s += libelleType(r.critere.type) + " \xC2\xAB " + r.critere.valeur + " \xC2\xBB ("
                 + (r.verdict == Verdict::Manquant ? "information manquante" : "non respect\xC3\xA9") + ")";
        }
        ev.raison = s + ".";
    }
    return ev;
}

} // namespace cvanalyzer
