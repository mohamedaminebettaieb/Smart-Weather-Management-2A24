#ifndef CVCORE_H
#define CVCORE_H

/*
 * cvcore : extraction du texte d'un CV (PDF ou DOCX).
 *
 * Ce module est volontairement écrit en C++17 standard (aucune dépendance
 * à Qt ni à une bibliothèque externe) :
 *   - décompression DEFLATE (RFC 1951) écrite à la main ;
 *   - lecture d'une archive ZIP (DOCX = ZIP contenant du XML) ;
 *   - extraction du texte d'un PDF (flux FlateDecode, polices avec
 *     table ToUnicode, flux d'objets /ObjStm).
 *
 * Aucune donnée n'est envoyée à l'extérieur : tout est traité en mémoire.
 * Si le texte ne peut pas être extrait (PDF scanné, protégé, corrompu...),
 * les fonctions retournent false avec un message d'erreur explicatif :
 * elles ne fabriquent jamais de texte.
 */

#include <cstdint>
#include <string>
#include <vector>

namespace cvcore
{
typedef std::vector<std::uint8_t> Bytes;

// Décompresse un flux DEFLATE brut. Retourne false si le flux est invalide.
bool inflateRaw(const std::uint8_t *data, std::size_t size, Bytes &out);

// Décompresse un flux zlib (en-tête de 2 octets + DEFLATE + Adler32).
bool inflateZlib(const std::uint8_t *data, std::size_t size, Bytes &out);

// Lit un fichier de l'archive ZIP (retourne false s'il est absent/illisible).
bool zipReadEntry(const Bytes &zip, const std::string &name, Bytes &out);

// Liste les noms des fichiers de l'archive ZIP.
std::vector<std::string> zipEntryNames(const Bytes &zip);

// Extraction du texte (UTF-8). En cas d'échec : retourne false et remplit err.
bool docxToText(const Bytes &file, std::string &text, std::string &err);
bool pdfToText(const Bytes &file, std::string &text, std::string &err);

// Détecte le format (signature du fichier) et appelle la bonne extraction.
bool extraireTexteCv(const Bytes &file, std::string &text, std::string &err);

// Utilitaires UTF-8 / Unicode
void appendUtf8(std::string &out, char32_t cp);
std::u32string utf8ToU32(const std::string &s);
std::string u32ToUtf8(const std::u32string &s);
}

#endif // CVCORE_H
