#include "cvcore.h"

#include <algorithm>
#include <cctype>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>

namespace cvcore
{

// ============================================================================
//  UTF-8 / Unicode
// ============================================================================

void appendUtf8(std::string &out, char32_t cp)
{
    if (cp < 0x80) {
        out.push_back(static_cast<char>(cp));
    } else if (cp < 0x800) {
        out.push_back(static_cast<char>(0xC0 | (cp >> 6)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x10000) {
        out.push_back(static_cast<char>(0xE0 | (cp >> 12)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    } else if (cp < 0x110000) {
        out.push_back(static_cast<char>(0xF0 | (cp >> 18)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
        out.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
    }
}

std::u32string utf8ToU32(const std::string &s)
{
    std::u32string r;
    r.reserve(s.size());
    std::size_t i = 0;
    const std::size_t n = s.size();
    while (i < n) {
        const unsigned char c = static_cast<unsigned char>(s[i]);
        char32_t cp = 0xFFFD;
        std::size_t len = 1;
        if (c < 0x80) {
            cp = c;
        } else if ((c >> 5) == 0x6 && i + 1 < n) {
            cp = ((c & 0x1Fu) << 6) | (static_cast<unsigned char>(s[i + 1]) & 0x3Fu);
            len = 2;
        } else if ((c >> 4) == 0xE && i + 2 < n) {
            cp = ((c & 0x0Fu) << 12) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 6)
                 | (static_cast<unsigned char>(s[i + 2]) & 0x3Fu);
            len = 3;
        } else if ((c >> 3) == 0x1E && i + 3 < n) {
            cp = ((c & 0x07u) << 18) | ((static_cast<unsigned char>(s[i + 1]) & 0x3Fu) << 12)
                 | ((static_cast<unsigned char>(s[i + 2]) & 0x3Fu) << 6)
                 | (static_cast<unsigned char>(s[i + 3]) & 0x3Fu);
            len = 4;
        }
        r.push_back(cp);
        i += len;
    }
    return r;
}

std::string u32ToUtf8(const std::u32string &s)
{
    std::string r;
    r.reserve(s.size());
    for (char32_t c : s)
        appendUtf8(r, c);
    return r;
}

// ============================================================================
//  DEFLATE (RFC 1951) - decompression
// ============================================================================

namespace
{
const std::size_t kMaxInflated = 256u * 1024u * 1024u;   // garde-fou (zip bomb)

struct BitReader
{
    const std::uint8_t *p;
    std::size_t size;
    std::size_t pos;      // octet courant
    std::uint32_t bitBuf; // tampon de bits
    int bitCnt;

    BitReader(const std::uint8_t *d, std::size_t n) : p(d), size(n), pos(0), bitBuf(0), bitCnt(0) {}

    // lit n bits (n <= 16) ; retourne -1 si fin des données
    int bits(int n)
    {
        while (bitCnt < n) {
            if (pos >= size)
                return -1;
            bitBuf |= static_cast<std::uint32_t>(p[pos++]) << bitCnt;
            bitCnt += 8;
        }
        const int v = static_cast<int>(bitBuf & ((1u << n) - 1u));
        bitBuf >>= n;
        bitCnt -= n;
        return v;
    }
};

struct Huffman
{
    short count[16];
    short symbol[320];
};

// Construit les tables à partir des longueurs de codes (algorithme "puff").
int buildHuffman(Huffman &h, const short *length, int n)
{
    for (int i = 0; i < 16; ++i)
        h.count[i] = 0;
    for (int s = 0; s < n; ++s)
        h.count[length[s]]++;
    if (h.count[0] == n)
        return 0; // aucun code (valide)

    int left = 1;
    for (int len = 1; len < 16; ++len) {
        left <<= 1;
        left -= h.count[len];
        if (left < 0)
            return left; // sur-souscrit
    }

    short offs[16];
    offs[1] = 0;
    for (int len = 1; len < 15; ++len)
        offs[len + 1] = static_cast<short>(offs[len] + h.count[len]);

    for (int s = 0; s < n; ++s)
        if (length[s] != 0)
            h.symbol[offs[length[s]]++] = static_cast<short>(s);
    return left;
}

int decodeSymbol(BitReader &br, const Huffman &h)
{
    int code = 0, first = 0, index = 0;
    for (int len = 1; len < 16; ++len) {
        const int b = br.bits(1);
        if (b < 0)
            return -1;
        code |= b;
        const int count = h.count[len];
        if (code - count < first)
            return h.symbol[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -2;
}

const short kLenBase[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27, 31,
                            35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
const short kLenExtra[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
                             3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
const short kDistBase[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129, 193,
                             257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145,
                             8193, 12289, 16385, 24577};
const short kDistExtra[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
                              7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

bool inflateBlock(BitReader &br, Bytes &out, const Huffman &lencode, const Huffman &distcode)
{
    for (;;) {
        int sym = decodeSymbol(br, lencode);
        if (sym < 0)
            return false;
        if (sym < 256) {
            out.push_back(static_cast<std::uint8_t>(sym));
        } else if (sym == 256) {
            return true;
        } else {
            sym -= 257;
            if (sym >= 29)
                return false;
            int e = br.bits(kLenExtra[sym]);
            if (e < 0)
                return false;
            const int len = kLenBase[sym] + e;
            const int ds = decodeSymbol(br, distcode);
            if (ds < 0 || ds >= 30)
                return false;
            e = br.bits(kDistExtra[ds]);
            if (e < 0)
                return false;
            const std::size_t dist = static_cast<std::size_t>(kDistBase[ds] + e);
            if (dist > out.size())
                return false;
            std::size_t from = out.size() - dist;
            for (int i = 0; i < len; ++i)
                out.push_back(out[from + static_cast<std::size_t>(i)]);
        }
        if (out.size() > kMaxInflated)
            return false;
    }
}
} // namespace

bool inflateRaw(const std::uint8_t *data, std::size_t size, Bytes &out)
{
    out.clear();
    BitReader br(data, size);

    static Huffman fixedLen, fixedDist;
    static bool fixedReady = false;
    if (!fixedReady) {
        short lengths[320];
        int s = 0;
        for (; s < 144; ++s) lengths[s] = 8;
        for (; s < 256; ++s) lengths[s] = 9;
        for (; s < 280; ++s) lengths[s] = 7;
        for (; s < 288; ++s) lengths[s] = 8;
        buildHuffman(fixedLen, lengths, 288);
        for (s = 0; s < 30; ++s) lengths[s] = 5;
        buildHuffman(fixedDist, lengths, 30);
        fixedReady = true;
    }

    int last = 0;
    do {
        last = br.bits(1);
        const int type = br.bits(2);
        if (last < 0 || type < 0)
            return false;

        if (type == 0) {
            // bloc non compressé
            br.bitBuf = 0;
            br.bitCnt = 0;
            if (br.pos + 4 > br.size)
                return false;
            const unsigned len = br.p[br.pos] | (static_cast<unsigned>(br.p[br.pos + 1]) << 8);
            const unsigned nlen = br.p[br.pos + 2] | (static_cast<unsigned>(br.p[br.pos + 3]) << 8);
            br.pos += 4;
            if ((len ^ 0xFFFFu) != nlen)
                return false;
            if (br.pos + len > br.size)
                return false;
            out.insert(out.end(), br.p + br.pos, br.p + br.pos + len);
            br.pos += len;
        } else if (type == 1) {
            if (!inflateBlock(br, out, fixedLen, fixedDist))
                return false;
        } else if (type == 2) {
            const int nlen = br.bits(5);
            const int ndist = br.bits(5);
            const int ncode = br.bits(4);
            if (nlen < 0 || ndist < 0 || ncode < 0)
                return false;
            const int nl = nlen + 257;
            const int nd = ndist + 1;
            if (nl > 286 || nd > 30)
                return false;

            static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
            short lengths[320];
            int index = 0;
            for (; index < ncode + 4; ++index) {
                const int v = br.bits(3);
                if (v < 0)
                    return false;
                lengths[order[index]] = static_cast<short>(v);
            }
            for (; index < 19; ++index)
                lengths[order[index]] = 0;

            Huffman lencode, distcode;
            if (buildHuffman(lencode, lengths, 19) != 0)
                return false;

            index = 0;
            while (index < nl + nd) {
                int sym = decodeSymbol(br, lencode);
                if (sym < 0)
                    return false;
                if (sym < 16) {
                    lengths[index++] = static_cast<short>(sym);
                } else {
                    int len = 0, rep = 0;
                    if (sym == 16) {
                        if (index == 0)
                            return false;
                        len = lengths[index - 1];
                        rep = 3 + br.bits(2);
                    } else if (sym == 17) {
                        rep = 3 + br.bits(3);
                    } else {
                        rep = 11 + br.bits(7);
                    }
                    if (rep < 3 || index + rep > nl + nd)
                        return false;
                    while (rep--)
                        lengths[index++] = static_cast<short>(len);
                }
            }
            if (lengths[256] == 0)
                return false;

            int err = buildHuffman(lencode, lengths, nl);
            if (err < 0 || (err > 0 && nl - lencode.count[0] != 1))
                return false;
            err = buildHuffman(distcode, lengths + nl, nd);
            if (err < 0 || (err > 0 && nd - distcode.count[0] != 1))
                return false;

            if (!inflateBlock(br, out, lencode, distcode))
                return false;
        } else {
            return false;
        }
    } while (!last);
    return true;
}

bool inflateZlib(const std::uint8_t *data, std::size_t size, Bytes &out)
{
    out.clear();
    if (size < 2)
        return false;
    const int cmf = data[0];
    const int flg = data[1];
    if ((cmf & 0x0F) != 8 || ((cmf << 8) | flg) % 31 != 0) {
        // en-tête invalide : on tente quand même en DEFLATE brut
        return inflateRaw(data, size, out);
    }
    std::size_t skip = 2;
    if (flg & 0x20)
        skip += 4; // dictionnaire prédéfini (non supporté, mais on saute l'identifiant)
    if (skip > size)
        return false;
    return inflateRaw(data + skip, size - skip, out);
}

// ============================================================================
//  ZIP
// ============================================================================

namespace
{
inline std::uint32_t rd16(const Bytes &b, std::size_t o)
{
    return static_cast<std::uint32_t>(b[o]) | (static_cast<std::uint32_t>(b[o + 1]) << 8);
}
inline std::uint32_t rd32(const Bytes &b, std::size_t o)
{
    return rd16(b, o) | (rd16(b, o + 2) << 16);
}

struct ZipEntry
{
    std::string name;
    std::uint32_t method = 0;
    std::uint32_t csize = 0;
    std::uint32_t usize = 0;
    std::uint32_t localOffset = 0;
};

bool zipDirectory(const Bytes &z, std::vector<ZipEntry> &entries)
{
    entries.clear();
    if (z.size() < 22)
        return false;

    // End Of Central Directory : signature 0x06054b50
    std::size_t eocd = std::string::npos;
    const std::size_t minPos = z.size() > 65557 ? z.size() - 65557 : 0;
    for (std::size_t i = z.size() - 22 + 1; i-- > minPos;) {
        if (rd32(z, i) == 0x06054b50u) {
            eocd = i;
            break;
        }
    }
    if (eocd == std::string::npos)
        return false;

    const std::uint32_t total = rd16(z, eocd + 10);
    std::size_t pos = rd32(z, eocd + 16);

    for (std::uint32_t k = 0; k < total; ++k) {
        if (pos + 46 > z.size() || rd32(z, pos) != 0x02014b50u)
            break;
        ZipEntry e;
        e.method = rd16(z, pos + 10);
        e.csize = rd32(z, pos + 20);
        e.usize = rd32(z, pos + 24);
        const std::size_t nlen = rd16(z, pos + 28);
        const std::size_t xlen = rd16(z, pos + 30);
        const std::size_t clen = rd16(z, pos + 32);
        e.localOffset = rd32(z, pos + 42);
        if (pos + 46 + nlen > z.size())
            break;
        e.name.assign(reinterpret_cast<const char *>(&z[pos + 46]), nlen);
        entries.push_back(e);
        pos += 46 + nlen + xlen + clen;
    }
    return !entries.empty();
}
} // namespace

std::vector<std::string> zipEntryNames(const Bytes &zip)
{
    std::vector<ZipEntry> entries;
    std::vector<std::string> names;
    if (zipDirectory(zip, entries))
        for (const ZipEntry &e : entries)
            names.push_back(e.name);
    return names;
}

bool zipReadEntry(const Bytes &zip, const std::string &name, Bytes &out)
{
    out.clear();
    std::vector<ZipEntry> entries;
    if (!zipDirectory(zip, entries))
        return false;

    for (const ZipEntry &e : entries) {
        if (e.name != name)
            continue;
        std::size_t lo = e.localOffset;
        if (lo + 30 > zip.size() || rd32(zip, lo) != 0x04034b50u)
            return false;
        const std::size_t nlen = rd16(zip, lo + 26);
        const std::size_t xlen = rd16(zip, lo + 28);
        const std::size_t dataPos = lo + 30 + nlen + xlen;
        if (dataPos + e.csize > zip.size())
            return false;
        if (e.method == 0) {
            out.assign(zip.begin() + static_cast<std::ptrdiff_t>(dataPos),
                       zip.begin() + static_cast<std::ptrdiff_t>(dataPos + e.csize));
            return true;
        }
        if (e.method == 8)
            return inflateRaw(&zip[dataPos], e.csize, out);
        return false; // méthode de compression non supportée
    }
    return false;
}

// ============================================================================
//  DOCX
// ============================================================================

namespace
{
void decodeXmlEntities(const std::string &in, std::string &out)
{
    for (std::size_t i = 0; i < in.size(); ++i) {
        if (in[i] != '&') {
            out.push_back(in[i]);
            continue;
        }
        const std::size_t semi = in.find(';', i);
        if (semi == std::string::npos || semi - i > 10) {
            out.push_back('&');
            continue;
        }
        const std::string ent = in.substr(i + 1, semi - i - 1);
        if (ent == "amp") out.push_back('&');
        else if (ent == "lt") out.push_back('<');
        else if (ent == "gt") out.push_back('>');
        else if (ent == "quot") out.push_back('"');
        else if (ent == "apos") out.push_back('\'');
        else if (!ent.empty() && ent[0] == '#') {
            const bool hex = ent.size() > 1 && (ent[1] == 'x' || ent[1] == 'X');
            const long cp = std::strtol(ent.c_str() + (hex ? 2 : 1), nullptr, hex ? 16 : 10);
            if (cp > 0)
                appendUtf8(out, static_cast<char32_t>(cp));
        } else {
            out.append(in, i, semi - i + 1);
        }
        i = semi;
    }
}

// Extrait le texte d'un fichier WordprocessingML (document.xml, header*.xml)
void wordXmlToText(const std::string &xml, std::string &text)
{
    std::size_t i = 0;
    const std::size_t n = xml.size();
    int fallbackDepth = 0;   // contenu mc:Fallback ignoré (doublon des zones de texte)

    while (i < n) {
        if (xml[i] != '<') {
            ++i;
            continue;
        }
        const std::size_t end = xml.find('>', i);
        if (end == std::string::npos)
            break;

        std::string tag = xml.substr(i + 1, end - i - 1);
        const bool closing = !tag.empty() && tag[0] == '/';
        const bool selfClosing = !tag.empty() && tag.back() == '/';
        if (closing)
            tag.erase(0, 1);
        // nom du tag = jusqu'au premier espace ou '/'
        std::size_t ne = tag.find_first_of(" \t\r\n/");
        const std::string name = tag.substr(0, ne);

        if (name == "mc:Fallback") {
            if (closing) { if (fallbackDepth > 0) --fallbackDepth; }
            else if (!selfClosing) ++fallbackDepth;
        } else if (fallbackDepth == 0) {
            if (name == "w:t" && !closing && !selfClosing) {
                const std::size_t close = xml.find("</w:t>", end + 1);
                if (close == std::string::npos)
                    break;
                decodeXmlEntities(xml.substr(end + 1, close - end - 1), text);
                i = close + 6;
                continue;
            } else if (name == "w:tab" && !closing) {
                text.push_back('\t');
            } else if ((name == "w:br" || name == "w:cr") && !closing) {
                text.push_back('\n');
            } else if (name == "w:p" && closing) {
                text.push_back('\n');
            } else if (name == "w:tc" && closing) {
                text.push_back('\t');
            } else if (name == "w:tr" && closing) {
                text.push_back('\n');
            }
        }
        i = end + 1;
    }
}
} // namespace

bool docxToText(const Bytes &file, std::string &text, std::string &err)
{
    text.clear();
    err.clear();

    Bytes doc;
    if (!zipReadEntry(file, "word/document.xml", doc)) {
        err = "Le fichier n'est pas un document Word (.docx) valide ou est protégé par un mot de passe.";
        return false;
    }

    // En-têtes (le nom du candidat s'y trouve souvent) puis corps du document
    std::vector<std::string> names = zipEntryNames(file);
    std::sort(names.begin(), names.end());
    for (const std::string &nm : names) {
        if (nm.rfind("word/header", 0) == 0 && nm.size() > 4 && nm.substr(nm.size() - 4) == ".xml") {
            Bytes h;
            if (zipReadEntry(file, nm, h))
                wordXmlToText(std::string(h.begin(), h.end()), text);
        }
    }
    wordXmlToText(std::string(doc.begin(), doc.end()), text);

    int letters = 0;
    for (unsigned char c : text)
        if (std::isalpha(c) || c >= 0x80)
            ++letters;
    if (letters < 30) {
        err = "Aucun texte exploitable n'a été trouvé dans ce document Word.";
        return false;
    }
    return true;
}

// ============================================================================
//  PDF
// ============================================================================

namespace
{
struct PVal
{
    enum Type { Null, Bool, Int, Real, Name, Str, Arr, Dict, Ref, Op };
    Type type = Null;
    double num = 0;
    int ref = 0;
    std::string s;
    std::vector<PVal> arr;
    std::vector<std::pair<std::string, PVal>> dict;

    const PVal *get(const std::string &key) const
    {
        for (const auto &kv : dict)
            if (kv.first == key)
                return &kv.second;
        return nullptr;
    }
    bool isName(const char *n) const { return type == Name && s == n; }
    double number() const { return num; }
    bool isNum() const { return type == Int || type == Real; }
};

inline bool isWs(int c)
{
    return c == 0 || c == 9 || c == 10 || c == 12 || c == 13 || c == 32;
}
inline bool isDelim(int c)
{
    return c == '(' || c == ')' || c == '<' || c == '>' || c == '[' || c == ']'
           || c == '{' || c == '}' || c == '/' || c == '%';
}

class Lexer
{
public:
    Lexer(const std::uint8_t *b, std::size_t n, std::size_t start = 0) : p_(b), n_(n), i_(start) {}

    std::size_t pos() const { return i_; }
    void setPos(std::size_t i) { i_ = i; }
    bool atEnd()
    {
        skipWs();
        return i_ >= n_;
    }

    void skipWs()
    {
        while (i_ < n_) {
            if (isWs(p_[i_])) {
                ++i_;
            } else if (p_[i_] == '%') {
                while (i_ < n_ && p_[i_] != '\n' && p_[i_] != '\r')
                    ++i_;
            } else {
                break;
            }
        }
    }

    // Lit une valeur ; en mode contenu (allowRef = false) les mots-clés sont des opérateurs.
    bool value(PVal &v, bool allowRef, int depth = 0)
    {
        skipWs();
        if (i_ >= n_ || depth > 40)
            return false;
        const int c = p_[i_];

        if (c == '<' && i_ + 1 < n_ && p_[i_ + 1] == '<') {
            i_ += 2;
            v = PVal();
            v.type = PVal::Dict;
            for (;;) {
                skipWs();
                if (i_ >= n_)
                    return false;
                if (p_[i_] == '>' && i_ + 1 < n_ && p_[i_ + 1] == '>') {
                    i_ += 2;
                    return true;
                }
                PVal key;
                if (!value(key, allowRef, depth + 1))
                    return false;
                if (key.type != PVal::Name) {
                    // clé invalide : on évite de boucler indéfiniment
                    if (key.type == PVal::Op)
                        return false;
                    continue;
                }
                PVal val;
                skipWs();
                if (i_ + 1 < n_ && p_[i_] == '>' && p_[i_ + 1] == '>') {
                    v.dict.emplace_back(key.s, PVal());
                    continue;
                }
                if (!value(val, allowRef, depth + 1))
                    return false;
                v.dict.emplace_back(key.s, val);
            }
        }
        if (c == '[') {
            ++i_;
            v = PVal();
            v.type = PVal::Arr;
            for (;;) {
                skipWs();
                if (i_ >= n_)
                    return false;
                if (p_[i_] == ']') {
                    ++i_;
                    return true;
                }
                PVal e;
                if (!value(e, allowRef, depth + 1))
                    return false;
                v.arr.push_back(e);
            }
        }
        if (c == '(') {
            ++i_;
            v = PVal();
            v.type = PVal::Str;
            int level = 1;
            while (i_ < n_) {
                int ch = p_[i_++];
                if (ch == '\\') {
                    if (i_ >= n_)
                        break;
                    const int e = p_[i_++];
                    switch (e) {
                    case 'n': v.s.push_back('\n'); break;
                    case 'r': v.s.push_back('\r'); break;
                    case 't': v.s.push_back('\t'); break;
                    case 'b': v.s.push_back('\b'); break;
                    case 'f': v.s.push_back('\f'); break;
                    case '\r': if (i_ < n_ && p_[i_] == '\n') ++i_; break;
                    case '\n': break;
                    default:
                        if (e >= '0' && e <= '7') {
                            int code = e - '0';
                            for (int k = 0; k < 2 && i_ < n_ && p_[i_] >= '0' && p_[i_] <= '7'; ++k)
                                code = code * 8 + (p_[i_++] - '0');
                            v.s.push_back(static_cast<char>(code & 0xFF));
                        } else {
                            v.s.push_back(static_cast<char>(e));
                        }
                    }
                } else if (ch == '(') {
                    ++level;
                    v.s.push_back('(');
                } else if (ch == ')') {
                    if (--level == 0)
                        return true;
                    v.s.push_back(')');
                } else {
                    v.s.push_back(static_cast<char>(ch));
                }
            }
            return true;
        }
        if (c == '<') {
            ++i_;
            v = PVal();
            v.type = PVal::Str;
            int hi = -1;
            while (i_ < n_ && p_[i_] != '>') {
                const int h = hexVal(p_[i_++]);
                if (h < 0)
                    continue;
                if (hi < 0) hi = h;
                else {
                    v.s.push_back(static_cast<char>((hi << 4) | h));
                    hi = -1;
                }
            }
            if (hi >= 0)
                v.s.push_back(static_cast<char>(hi << 4));
            if (i_ < n_)
                ++i_;
            return true;
        }
        if (c == '/') {
            ++i_;
            v = PVal();
            v.type = PVal::Name;
            while (i_ < n_ && !isWs(p_[i_]) && !isDelim(p_[i_])) {
                if (p_[i_] == '#' && i_ + 2 < n_ && hexVal(p_[i_ + 1]) >= 0 && hexVal(p_[i_ + 2]) >= 0) {
                    v.s.push_back(static_cast<char>((hexVal(p_[i_ + 1]) << 4) | hexVal(p_[i_ + 2])));
                    i_ += 3;
                } else {
                    v.s.push_back(static_cast<char>(p_[i_++]));
                }
            }
            return true;
        }
        if (c == ']' || c == '>' || c == ')' || c == '{' || c == '}') {
            ++i_;
            v = PVal();
            v.type = PVal::Op;
            v.s.assign(1, static_cast<char>(c));
            return true;
        }

        // nombre ou mot-clé
        const std::size_t start = i_;
        while (i_ < n_ && !isWs(p_[i_]) && !isDelim(p_[i_]))
            ++i_;
        if (i_ == start) {
            ++i_;
            return false;
        }
        const std::string tok(reinterpret_cast<const char *>(p_ + start), i_ - start);

        if (isNumber(tok)) {
            v = PVal();
            v.num = std::strtod(tok.c_str(), nullptr);
            v.type = (tok.find('.') == std::string::npos) ? PVal::Int : PVal::Real;
            if (allowRef && v.type == PVal::Int && v.num >= 0) {
                // référence indirecte : "n g R"
                const std::size_t save = i_;
                std::size_t j = i_;
                while (j < n_ && isWs(p_[j])) ++j;
                const std::size_t gs = j;
                while (j < n_ && p_[j] >= '0' && p_[j] <= '9') ++j;
                if (j > gs) {
                    std::size_t k = j;
                    while (k < n_ && isWs(p_[k])) ++k;
                    if (k > j && k < n_ && p_[k] == 'R'
                        && (k + 1 >= n_ || isWs(p_[k + 1]) || isDelim(p_[k + 1]))) {
                        v.type = PVal::Ref;
                        v.ref = static_cast<int>(v.num);
                        i_ = k + 1;
                        return true;
                    }
                }
                i_ = save;
            }
            return true;
        }

        v = PVal();
        if (tok == "true" || tok == "false") {
            v.type = PVal::Bool;
            v.num = (tok == "true") ? 1 : 0;
        } else if (tok == "null") {
            v.type = PVal::Null;
        } else {
            v.type = PVal::Op;
            v.s = tok;
        }
        return true;
    }

    static int hexVal(int c)
    {
        if (c >= '0' && c <= '9') return c - '0';
        if (c >= 'a' && c <= 'f') return c - 'a' + 10;
        if (c >= 'A' && c <= 'F') return c - 'A' + 10;
        return -1;
    }

private:
    static bool isNumber(const std::string &t)
    {
        if (t.empty())
            return false;
        std::size_t i = 0;
        if (t[0] == '+' || t[0] == '-')
            i = 1;
        bool digit = false, dot = false;
        for (; i < t.size(); ++i) {
            if (t[i] >= '0' && t[i] <= '9') digit = true;
            else if (t[i] == '.' && !dot) dot = true;
            else if (t[i] == '-' && digit) continue; // "12-3" mal formé : toléré
            else return false;
        }
        return digit;
    }

    const std::uint8_t *p_;
    std::size_t n_;
    std::size_t i_;
};

// ----------------------------------------------------------------------------
//  Filtres de flux
// ----------------------------------------------------------------------------

bool ascii85Decode(const Bytes &in, Bytes &out)
{
    out.clear();
    std::uint32_t tuple = 0;
    int count = 0;
    for (std::size_t i = 0; i < in.size(); ++i) {
        const int c = in[i];
        if (isWs(c))
            continue;
        if (c == '~')
            break;
        if (c == 'z' && count == 0) {
            out.insert(out.end(), 4, 0);
            continue;
        }
        if (c < '!' || c > 'u')
            return false;
        tuple = tuple * 85 + static_cast<std::uint32_t>(c - '!');
        if (++count == 5) {
            for (int k = 3; k >= 0; --k)
                out.push_back(static_cast<std::uint8_t>((tuple >> (8 * k)) & 0xFF));
            tuple = 0;
            count = 0;
        }
    }
    if (count > 1) {
        for (int k = count; k < 5; ++k)
            tuple = tuple * 85 + 84;
        for (int k = 3; k > 4 - count; --k)
            out.push_back(static_cast<std::uint8_t>((tuple >> (8 * k)) & 0xFF));
    }
    return true;
}

bool asciiHexDecode(const Bytes &in, Bytes &out)
{
    out.clear();
    int hi = -1;
    for (std::uint8_t c : in) {
        if (c == '>')
            break;
        const int h = Lexer::hexVal(c);
        if (h < 0)
            continue;
        if (hi < 0) hi = h;
        else {
            out.push_back(static_cast<std::uint8_t>((hi << 4) | h));
            hi = -1;
        }
    }
    if (hi >= 0)
        out.push_back(static_cast<std::uint8_t>(hi << 4));
    return true;
}

void undoPngPredictor(Bytes &data, int columns, int colors, int bpc)
{
    const int bpp = std::max(1, (colors * bpc + 7) / 8);
    const std::size_t rowBytes = static_cast<std::size_t>((columns * colors * bpc + 7) / 8);
    if (rowBytes == 0)
        return;
    const std::size_t rows = data.size() / (rowBytes + 1);
    Bytes out(rows * rowBytes, 0);
    Bytes prev(rowBytes, 0);
    for (std::size_t r = 0; r < rows; ++r) {
        const std::uint8_t ft = data[r * (rowBytes + 1)];
        const std::uint8_t *src = &data[r * (rowBytes + 1) + 1];
        std::uint8_t *dst = &out[r * rowBytes];
        for (std::size_t i = 0; i < rowBytes; ++i) {
            const int a = (i >= static_cast<std::size_t>(bpp)) ? dst[i - static_cast<std::size_t>(bpp)] : 0;
            const int b = prev[i];
            const int c = (i >= static_cast<std::size_t>(bpp)) ? prev[i - static_cast<std::size_t>(bpp)] : 0;
            int v = src[i];
            switch (ft) {
            case 1: v += a; break;
            case 2: v += b; break;
            case 3: v += (a + b) / 2; break;
            case 4: {
                const int p = a + b - c;
                const int pa = std::abs(p - a), pb = std::abs(p - b), pc = std::abs(p - c);
                v += (pa <= pb && pa <= pc) ? a : (pb <= pc ? b : c);
                break;
            }
            default: break;
            }
            dst[i] = static_cast<std::uint8_t>(v & 0xFF);
        }
        std::memcpy(prev.data(), dst, rowBytes);
    }
    data.swap(out);
}

// ----------------------------------------------------------------------------
//  Document PDF
// ----------------------------------------------------------------------------

struct FontInfo
{
    bool decodable = true;
    bool hasToUnicode = false;
    int codeBytes = 1;
    std::map<std::uint32_t, std::string> toUni;   // code -> UTF-8
    std::string simple[256];                      // police simple sans ToUnicode
    bool simpleReady = false;
};

const char32_t kCp1252[32] = {
    0x20AC, 0x0081, 0x201A, 0x0192, 0x201E, 0x2026, 0x2020, 0x2021, 0x02C6, 0x2030, 0x0160, 0x2039, 0x0152, 0x008D, 0x017D, 0x008F,
    0x0090, 0x2018, 0x2019, 0x201C, 0x201D, 0x2022, 0x2013, 0x2014, 0x02DC, 0x2122, 0x0161, 0x203A, 0x0153, 0x009D, 0x017E, 0x0178};

struct GlyphName { const char *name; char32_t cp; };
const GlyphName kGlyphs[] = {
    {"space", 32}, {"exclam", 33}, {"quotedbl", 34}, {"numbersign", 35}, {"dollar", 36}, {"percent", 37},
    {"ampersand", 38}, {"quotesingle", 39}, {"parenleft", 40}, {"parenright", 41}, {"asterisk", 42},
    {"plus", 43}, {"comma", 44}, {"hyphen", 45}, {"minus", 0x2212}, {"period", 46}, {"slash", 47},
    {"zero", 48}, {"one", 49}, {"two", 50}, {"three", 51}, {"four", 52}, {"five", 53}, {"six", 54},
    {"seven", 55}, {"eight", 56}, {"nine", 57}, {"colon", 58}, {"semicolon", 59}, {"less", 60},
    {"equal", 61}, {"greater", 62}, {"question", 63}, {"at", 64}, {"bracketleft", 91},
    {"backslash", 92}, {"bracketright", 93}, {"asciicircum", 94}, {"underscore", 95}, {"grave", 96},
    {"braceleft", 123}, {"bar", 124}, {"braceright", 125}, {"asciitilde", 126}, {"endash", 0x2013},
    {"emdash", 0x2014}, {"quoteleft", 0x2018}, {"quoteright", 0x2019}, {"quotedblleft", 0x201C},
    {"quotedblright", 0x201D}, {"bullet", 0x2022}, {"ellipsis", 0x2026}, {"fi", 0xFB01}, {"fl", 0xFB02},
    {"degree", 0xB0}, {"copyright", 0xA9}, {"registered", 0xAE}, {"trademark", 0x2122}, {"euro", 0x20AC},
    {"guillemotleft", 0xAB}, {"guillemotright", 0xBB}, {"nbspace", 0xA0}, {"periodcentered", 0xB7},
    {"agrave", 0xE0}, {"aacute", 0xE1}, {"acircumflex", 0xE2}, {"atilde", 0xE3}, {"adieresis", 0xE4},
    {"aring", 0xE5}, {"ae", 0xE6}, {"ccedilla", 0xE7}, {"egrave", 0xE8}, {"eacute", 0xE9},
    {"ecircumflex", 0xEA}, {"edieresis", 0xEB}, {"igrave", 0xEC}, {"iacute", 0xED}, {"icircumflex", 0xEE},
    {"idieresis", 0xEF}, {"ntilde", 0xF1}, {"ograve", 0xF2}, {"oacute", 0xF3}, {"ocircumflex", 0xF4},
    {"otilde", 0xF5}, {"odieresis", 0xF6}, {"oe", 0x153}, {"ugrave", 0xF9}, {"uacute", 0xFA},
    {"ucircumflex", 0xFB}, {"udieresis", 0xFC}, {"yacute", 0xFD}, {"ydieresis", 0xFF}, {"germandbls", 0xDF},
    {"Agrave", 0xC0}, {"Aacute", 0xC1}, {"Acircumflex", 0xC2}, {"Atilde", 0xC3}, {"Adieresis", 0xC4},
    {"Aring", 0xC5}, {"AE", 0xC6}, {"Ccedilla", 0xC7}, {"Egrave", 0xC8}, {"Eacute", 0xC9},
    {"Ecircumflex", 0xCA}, {"Edieresis", 0xCB}, {"Igrave", 0xCC}, {"Iacute", 0xCD}, {"Icircumflex", 0xCE},
    {"Idieresis", 0xCF}, {"Ntilde", 0xD1}, {"Ograve", 0xD2}, {"Oacute", 0xD3}, {"Ocircumflex", 0xD4},
    {"Otilde", 0xD5}, {"Odieresis", 0xD6}, {"OE", 0x152}, {"Ugrave", 0xD9}, {"Uacute", 0xDA},
    {"Ucircumflex", 0xDB}, {"Udieresis", 0xDC}, {"Yacute", 0xDD}, {"Ydieresis", 0x178}};

char32_t glyphNameToUnicode(const std::string &n)
{
    if (n.size() == 1)
        return static_cast<unsigned char>(n[0]);
    for (const GlyphName &g : kGlyphs)
        if (n == g.name)
            return g.cp;
    if (n.size() >= 7 && n.compare(0, 3, "uni") == 0)
        return static_cast<char32_t>(std::strtoul(n.substr(3, 4).c_str(), nullptr, 16));
    if (n.size() >= 5 && n[0] == 'u')
        return static_cast<char32_t>(std::strtoul(n.c_str() + 1, nullptr, 16));
    return 0;
}

class PdfDoc
{
public:
    explicit PdfDoc(const Bytes &data) : d_(data) {}
    bool load(std::string &err);
    std::string extractText(std::string &err);

private:
    const Bytes &d_;
    std::map<int, PVal> objs_;
    std::map<int, std::pair<std::size_t, std::size_t>> streamPos_;  // objet -> (début, fin) des données brutes
    std::map<int, FontInfo> fontCache_;
    int undecodable_ = 0;
    int emitted_ = 0;
    bool encrypted_ = false;

    const PVal *resolve(const PVal *v) const
    {
        int guard = 0;
        while (v && v->type == PVal::Ref && guard++ < 20) {
            auto it = objs_.find(v->ref);
            v = (it == objs_.end()) ? nullptr : &it->second;
        }
        return v;
    }
    const PVal *dictGet(const PVal &dict, const char *key) const
    {
        const PVal *d = resolve(&dict);
        if (!d || d->type != PVal::Dict)
            return nullptr;
        return resolve(d->get(key));
    }

    bool streamData(int objNum, Bytes &out) const;
    bool decodeStream(const PVal &dict, const Bytes &raw, Bytes &out) const;
    void parseObjStreams();
    void scanObjects();

    void collectPages(const PVal *node, const PVal *inheritedRes, std::vector<std::pair<const PVal *, const PVal *>> &pages, int depth,
                      std::set<const PVal *> &seen) const;

    const FontInfo &fontFor(const PVal *fontRef);
    void buildSimpleEncoding(const PVal &fontDict, FontInfo &fi);
    void parseCMap(const Bytes &cmap, FontInfo &fi);

    void interpret(const Bytes &content, const PVal *resources, std::string &out, int depth);
};

void PdfDoc::scanObjects()
{
    const std::size_t n = d_.size();
    const std::uint8_t *b = d_.data();
    std::size_t i = 0;
    while (i + 3 < n) {
        // recherche du mot-clé "obj" précédé de "<num> <gen> "
        if (b[i] == 'o' && b[i + 1] == 'b' && b[i + 2] == 'j' && (i + 3 >= n || !std::isalpha(b[i + 3]))
            && i > 0 && isWs(b[i - 1])) {
            // remonter : gen puis num
            std::size_t j = i - 1;
            while (j > 0 && isWs(b[j])) --j;
            std::size_t gEnd = j + 1;
            while (j > 0 && b[j] >= '0' && b[j] <= '9') --j;
            std::size_t gStart = (b[j] >= '0' && b[j] <= '9') ? j : j + 1;
            if (gStart < gEnd && j > 0 && isWs(b[j])) {
                while (j > 0 && isWs(b[j])) --j;
                std::size_t nEnd = j + 1;
                while (j > 0 && b[j] >= '0' && b[j] <= '9') --j;
                std::size_t nStart = (b[j] >= '0' && b[j] <= '9') ? j : j + 1;
                if (nStart < nEnd && (nStart == 0 || isWs(b[nStart - 1]) || isDelim(b[nStart - 1]) || b[nStart - 1] == 'j')) {
                    const int num = std::atoi(std::string(reinterpret_cast<const char *>(b + nStart), nEnd - nStart).c_str());
                    Lexer lx(b, n, i + 3);
                    PVal v;
                    if (num > 0 && lx.value(v, true)) {
                        // flux associé ?
                        lx.skipWs();
                        std::size_t after = lx.pos();
                        if (v.type == PVal::Dict && after + 6 <= n && std::memcmp(b + after, "stream", 6) == 0) {
                            std::size_t ds = after + 6;
                            if (ds < n && b[ds] == '\r') ++ds;
                            if (ds < n && b[ds] == '\n') ++ds;
                            std::size_t de = std::string::npos;
                            // /Length direct si cohérent
                            const PVal *len = v.get("Length");
                            if (len && len->type == PVal::Int && len->num >= 0) {
                                const std::size_t cand = ds + static_cast<std::size_t>(len->num);
                                std::size_t k = cand;
                                while (k < n && isWs(b[k])) ++k;
                                if (k + 9 <= n && std::memcmp(b + k, "endstream", 9) == 0)
                                    de = cand;
                            }
                            if (de == std::string::npos) {
                                const char *pat = "endstream";
                                auto it = std::search(d_.begin() + static_cast<std::ptrdiff_t>(ds), d_.end(), pat, pat + 9);
                                if (it != d_.end()) {
                                    de = static_cast<std::size_t>(it - d_.begin());
                                    while (de > ds && (b[de - 1] == '\n' || b[de - 1] == '\r')) --de;
                                }
                            }
                            if (de != std::string::npos && de >= ds) {
                                streamPos_[num] = std::make_pair(ds, de);
                                i = de;
                            }
                        }
                        objs_[num] = v;
                    }
                }
            }
        }
        ++i;
    }
}

bool PdfDoc::decodeStream(const PVal &dict, const Bytes &raw, Bytes &out) const
{
    std::vector<std::string> filters;
    std::vector<const PVal *> parms;

    const PVal *f = resolve(dict.get("Filter"));
    if (!f)
        f = resolve(dict.get("F"));
    const PVal *dp = resolve(dict.get("DecodeParms"));
    if (!dp)
        dp = resolve(dict.get("DP"));

    if (f && f->type == PVal::Name) {
        filters.push_back(f->s);
        parms.push_back(dp && dp->type == PVal::Dict ? dp : (dp && dp->type == PVal::Arr && !dp->arr.empty() ? resolve(&dp->arr[0]) : nullptr));
    } else if (f && f->type == PVal::Arr) {
        for (std::size_t k = 0; k < f->arr.size(); ++k) {
            const PVal *fk = resolve(&f->arr[k]);
            if (fk && fk->type == PVal::Name) {
                filters.push_back(fk->s);
                const PVal *pk = (dp && dp->type == PVal::Arr && k < dp->arr.size()) ? resolve(&dp->arr[k]) : nullptr;
                parms.push_back(pk);
            }
        }
    }

    Bytes cur = raw;
    for (std::size_t k = 0; k < filters.size(); ++k) {
        const std::string &fn = filters[k];
        Bytes next;
        if (fn == "FlateDecode" || fn == "Fl") {
            inflateZlib(cur.data(), cur.size(), next);   // sortie partielle tolérée
            if (next.empty() && !cur.empty())
                return false;
            const PVal *pm = parms[k];
            if (pm && pm->type == PVal::Dict) {
                const PVal *pr = pm->get("Predictor");
                const int predictor = (pr && pr->isNum()) ? static_cast<int>(pr->num) : 1;
                if (predictor >= 10) {
                    auto geti = [&](const char *key, int def) {
                        const PVal *x = pm->get(key);
                        return (x && x->isNum()) ? static_cast<int>(x->num) : def;
                    };
                    undoPngPredictor(next, geti("Columns", 1), geti("Colors", 1), geti("BitsPerComponent", 8));
                }
            }
        } else if (fn == "ASCII85Decode" || fn == "A85") {
            ascii85Decode(cur, next);
        } else if (fn == "ASCIIHexDecode" || fn == "AHx") {
            asciiHexDecode(cur, next);
        } else {
            return false;   // LZW, DCT, JPX, CCITT... : non supporté (images, etc.)
        }
        cur.swap(next);
    }
    out.swap(cur);
    return true;
}

bool PdfDoc::streamData(int objNum, Bytes &out) const
{
    auto sp = streamPos_.find(objNum);
    auto ob = objs_.find(objNum);
    if (sp == streamPos_.end() || ob == objs_.end())
        return false;
    Bytes raw(d_.begin() + static_cast<std::ptrdiff_t>(sp->second.first),
              d_.begin() + static_cast<std::ptrdiff_t>(sp->second.second));
    return decodeStream(ob->second, raw, out);
}

void PdfDoc::parseObjStreams()
{
    std::vector<int> osNums;
    for (const auto &kv : objs_) {
        const PVal *t = kv.second.type == PVal::Dict ? kv.second.get("Type") : nullptr;
        if (t && t->isName("ObjStm") && streamPos_.count(kv.first))
            osNums.push_back(kv.first);
    }
    for (int num : osNums) {
        Bytes data;
        if (!streamData(num, data))
            continue;
        const PVal &dict = objs_[num];
        const PVal *nv = dict.get("N");
        const PVal *fv = dict.get("First");
        if (!nv || !fv)
            continue;
        const int count = static_cast<int>(nv->num);
        const std::size_t first = static_cast<std::size_t>(fv->num);

        Lexer hdr(data.data(), data.size(), 0);
        std::vector<std::pair<int, std::size_t>> table;
        for (int k = 0; k < count; ++k) {
            PVal a, b;
            if (!hdr.value(a, false) || !hdr.value(b, false) || !a.isNum() || !b.isNum())
                break;
            table.emplace_back(static_cast<int>(a.num), static_cast<std::size_t>(b.num));
        }
        for (const auto &e : table) {
            if (first + e.second >= data.size())
                continue;
            Lexer lx(data.data(), data.size(), first + e.second);
            PVal v;
            if (lx.value(v, true) && !objs_.count(e.first))
                objs_[e.first] = v;
        }
    }
}

bool PdfDoc::load(std::string &err)
{
    if (d_.size() < 8 || std::memcmp(d_.data(), "%PDF", 4) != 0) {
        // certains fichiers ont quelques octets avant l'en-tête
        const char *sig = "%PDF-";
        if (std::search(d_.begin(), d_.begin() + static_cast<std::ptrdiff_t>(std::min<std::size_t>(d_.size(), 1024)), sig, sig + 5)
            == d_.begin() + static_cast<std::ptrdiff_t>(std::min<std::size_t>(d_.size(), 1024))) {
            err = "Le fichier n'est pas un PDF valide.";
            return false;
        }
    }
    scanObjects();
    if (objs_.empty()) {
        err = "Structure PDF illisible (fichier corrompu ou format non pris en charge).";
        return false;
    }
    parseObjStreams();

    // PDF chiffré ?
    for (const auto &kv : objs_) {
        if (kv.second.type == PVal::Dict && kv.second.get("Encrypt")) {
            encrypted_ = true;
            break;
        }
    }
    if (!encrypted_) {
        const char *pat = "/Encrypt";
        // trailer classique : recherche dans les 4 derniers Ko
        const std::size_t tail = std::min<std::size_t>(d_.size(), 4096);
        if (std::search(d_.end() - static_cast<std::ptrdiff_t>(tail), d_.end(), pat, pat + 8) != d_.end())
            encrypted_ = true;
    }
    if (encrypted_) {
        err = "Ce PDF est chiffré ou protégé : son contenu ne peut pas être lu.";
        return false;
    }
    return true;
}

void PdfDoc::collectPages(const PVal *node, const PVal *inheritedRes,
                          std::vector<std::pair<const PVal *, const PVal *>> &pages, int depth,
                          std::set<const PVal *> &seen) const
{
    node = resolve(node);
    if (!node || node->type != PVal::Dict || depth > 30 || seen.count(node))
        return;
    seen.insert(node);

    const PVal *res = node->get("Resources") ? node->get("Resources") : inheritedRes;
    const PVal *type = resolve(node->get("Type"));
    const PVal *kids = resolve(node->get("Kids"));

    if (kids && kids->type == PVal::Arr) {
        for (const PVal &k : kids->arr)
            collectPages(&k, res, pages, depth + 1, seen);
    } else if (type && type->isName("Page")) {
        pages.emplace_back(node, res);
    } else if (!type && node->get("Contents")) {
        pages.emplace_back(node, res);
    }
}

void PdfDoc::parseCMap(const Bytes &cmap, FontInfo &fi)
{
    Lexer lx(cmap.data(), cmap.size(), 0);
    std::vector<PVal> stack;
    PVal tok;

    auto decodeDst = [](const std::string &s) {
        // UTF-16BE -> UTF-8
        std::string out;
        for (std::size_t i = 0; i + 1 < s.size(); i += 2) {
            std::uint32_t cu = (static_cast<std::uint8_t>(s[i]) << 8) | static_cast<std::uint8_t>(s[i + 1]);
            if (cu >= 0xD800 && cu < 0xDC00 && i + 3 < s.size()) {
                const std::uint32_t lo = (static_cast<std::uint8_t>(s[i + 2]) << 8) | static_cast<std::uint8_t>(s[i + 3]);
                cu = 0x10000 + ((cu - 0xD800) << 10) + (lo - 0xDC00);
                i += 2;
            }
            appendUtf8(out, cu);
        }
        return out;
    };
    auto toCode = [](const std::string &s) {
        std::uint32_t c = 0;
        for (unsigned char ch : s)
            c = (c << 8) | ch;
        return c;
    };

    int mode = 0;   // 0 rien, 1 bfchar, 2 bfrange, 3 codespace
    bool codeLenSet = false;
    while (!lx.atEnd()) {
        if (!lx.value(tok, false))
            continue;
        if (tok.type == PVal::Op) {
            if (tok.s == "beginbfchar") mode = 1;
            else if (tok.s == "beginbfrange") mode = 2;
            else if (tok.s == "begincodespacerange") mode = 3;
            else if (tok.s == "endbfchar" || tok.s == "endbfrange" || tok.s == "endcodespacerange") mode = 0;
            stack.clear();
            continue;
        }
        if (mode == 1) {
            stack.push_back(tok);
            if (stack.size() == 2) {
                if (stack[0].type == PVal::Str && stack[1].type == PVal::Str)
                    fi.toUni[toCode(stack[0].s)] = decodeDst(stack[1].s);
                stack.clear();
            }
        } else if (mode == 2) {
            stack.push_back(tok);
            if (stack.size() == 3) {
                if (stack[0].type == PVal::Str && stack[1].type == PVal::Str) {
                    const std::uint32_t lo = toCode(stack[0].s), hi = toCode(stack[1].s);
                    if (hi >= lo && hi - lo < 70000) {
                        if (stack[2].type == PVal::Str) {
                            std::string base = decodeDst(stack[2].s);
                            std::u32string u = utf8ToU32(base);
                            for (std::uint32_t c = lo; c <= hi; ++c) {
                                std::u32string cur = u;
                                if (!cur.empty())
                                    cur.back() += (c - lo);
                                fi.toUni[c] = u32ToUtf8(cur);
                            }
                        } else if (stack[2].type == PVal::Arr) {
                            for (std::uint32_t c = lo; c <= hi && (c - lo) < stack[2].arr.size(); ++c)
                                if (stack[2].arr[c - lo].type == PVal::Str)
                                    fi.toUni[c] = decodeDst(stack[2].arr[c - lo].s);
                        }
                    }
                }
                stack.clear();
            }
        } else if (mode == 3) {
            stack.push_back(tok);
            if (stack.size() == 2) {
                if (stack[0].type == PVal::Str && !codeLenSet && !stack[0].s.empty()) {
                    fi.codeBytes = static_cast<int>(stack[0].s.size());
                    codeLenSet = true;
                }
                stack.clear();
            }
        }
    }
}

void PdfDoc::buildSimpleEncoding(const PVal &fontDict, FontInfo &fi)
{
    // base : WinAnsi (Latin-1 + cp1252)
    for (int c = 0; c < 256; ++c) {
        char32_t cp = static_cast<char32_t>(c);
        if (c >= 0x80 && c < 0xA0)
            cp = kCp1252[c - 0x80];
        fi.simple[c].clear();
        if (c >= 32 || c == 9 || c == 10)
            appendUtf8(fi.simple[c], cp);
    }
    const PVal *enc = dictGet(fontDict, "Encoding");
    if (enc && enc->type == PVal::Dict) {
        const PVal *diff = resolve(enc->get("Differences"));
        if (diff && diff->type == PVal::Arr) {
            int code = 0;
            for (const PVal &e : diff->arr) {
                if (e.isNum()) {
                    code = static_cast<int>(e.num);
                } else if (e.type == PVal::Name) {
                    if (code >= 0 && code < 256) {
                        const char32_t cp = glyphNameToUnicode(e.s);
                        fi.simple[code].clear();
                        if (cp)
                            appendUtf8(fi.simple[code], cp);
                    }
                    ++code;
                }
            }
        }
    }
    fi.simpleReady = true;
}

const FontInfo &PdfDoc::fontFor(const PVal *fontRef)
{
    static const FontInfo empty = [] { FontInfo f; f.decodable = false; return f; }();
    if (!fontRef)
        return empty;
    const int key = (fontRef->type == PVal::Ref) ? fontRef->ref : -1;
    if (key >= 0) {
        auto it = fontCache_.find(key);
        if (it != fontCache_.end())
            return it->second;
    }
    const PVal *fd = resolve(fontRef);
    FontInfo fi;
    if (!fd || fd->type != PVal::Dict) {
        fi.decodable = false;
    } else {
        const PVal *sub = resolve(fd->get("Subtype"));
        const bool type0 = sub && sub->isName("Type0");
        const PVal *tu = fd->get("ToUnicode");
        if (tu && tu->type == PVal::Ref) {
            Bytes cm;
            if (streamData(tu->ref, cm)) {
                parseCMap(cm, fi);
                fi.hasToUnicode = !fi.toUni.empty();
            }
        }
        if (type0) {
            if (fi.codeBytes < 1 || fi.codeBytes > 2)
                fi.codeBytes = 2;
            if (!fi.hasToUnicode)
                fi.decodable = false;   // identifiants de glyphes : non exploitables sans ToUnicode
            else if (fi.codeBytes == 1)
                fi.codeBytes = 2;
        } else {
            fi.codeBytes = 1;
            if (!fi.hasToUnicode)
                buildSimpleEncoding(*fd, fi);
        }
    }
    if (key >= 0)
        return fontCache_.emplace(key, std::move(fi)).first->second;
    // police en ligne : mise en cache temporaire
    static FontInfo tmp;
    tmp = std::move(fi);
    return tmp;
}

void PdfDoc::interpret(const Bytes &content, const PVal *resources, std::string &out, int depth)
{
    const PVal *res = resolve(resources);
    const PVal *fonts = (res && res->type == PVal::Dict) ? resolve(res->get("Font")) : nullptr;
    const PVal *xobjs = (res && res->type == PVal::Dict) ? resolve(res->get("XObject")) : nullptr;

    Lexer lx(content.data(), content.size(), 0);
    std::vector<PVal> ops;
    const FontInfo *font = nullptr;

    double curX = 0, curY = 0;       // position de la ligne de texte courante
    double lineX = 0, lineY = 0;
    double lastY = 1e18;
    bool lineStart = true;
    bool moved = false;              // un déplacement a eu lieu depuis le dernier texte
    bool wasNewLine = false;

    auto lastChar = [&]() -> char { return out.empty() ? '\n' : out.back(); };
    auto ensureSeparator = [&](bool newline) {
        if (newline) {
            while (!out.empty() && (out.back() == ' ' || out.back() == '\t'))
                out.pop_back();
            if (!out.empty() && out.back() != '\n')
                out.push_back('\n');
        } else if (lastChar() != ' ' && lastChar() != '\n' && lastChar() != '\t') {
            out.push_back(' ');
        }
    };

    auto beforeShow = [&]() {
        if (lineStart) {
            lineStart = false;
            lastY = curY;
            moved = false;
            wasNewLine = false;
            return;
        }
        if (std::fabs(curY - lastY) > 1.0) {
            ensureSeparator(true);
            wasNewLine = true;
        } else if (moved) {
            ensureSeparator(false);
        }
        lastY = curY;
        moved = false;
    };

    auto showString = [&](const std::string &s) {
        if (!font || s.empty())
            return;
        if (!font->decodable) {
            undecodable_ += static_cast<int>(s.size());
            return;
        }
        beforeShow();
        if (font->hasToUnicode) {
            const int cb = font->codeBytes;
            for (std::size_t i = 0; i + static_cast<std::size_t>(cb) <= s.size(); i += static_cast<std::size_t>(cb)) {
                std::uint32_t code = 0;
                for (int k = 0; k < cb; ++k)
                    code = (code << 8) | static_cast<std::uint8_t>(s[i + static_cast<std::size_t>(k)]);
                auto it = font->toUni.find(code);
                if (it != font->toUni.end()) {
                    out += it->second;
                    ++emitted_;
                } else if (cb == 1 && code == 32) {
                    out.push_back(' ');
                } else if (code == 32 || code == 0x0020) {
                    out.push_back(' ');
                }
            }
        } else {
            for (unsigned char c : s) {
                out += font->simple[c];
                ++emitted_;
            }
        }
    };

    auto num = [&](std::size_t idx, double def = 0) {
        return idx < ops.size() && ops[idx].isNum() ? ops[idx].num : def;
    };

    PVal tok;
    while (!lx.atEnd()) {
        if (!lx.value(tok, false))
            continue;
        if (tok.type != PVal::Op) {
            ops.push_back(tok);
            if (ops.size() > 512)
                ops.erase(ops.begin(), ops.begin() + 256);
            continue;
        }
        const std::string &op = tok.s;

        if (op == "BT") {
            curX = curY = lineX = lineY = 0;
        } else if (op == "ET") {
            // fin de bloc : le texte suivant sera comparé à la position précédente
        } else if (op == "Tf") {
            if (ops.size() >= 2 && ops[ops.size() - 2].type == PVal::Name && fonts && fonts->type == PVal::Dict) {
                const PVal *fr = fonts->get(ops[ops.size() - 2].s);
                font = fr ? &fontFor(fr) : nullptr;
            }
        } else if (op == "Td" || op == "TD") {
            const double tx = num(ops.size() >= 2 ? ops.size() - 2 : 0);
            const double ty = num(ops.size() >= 1 ? ops.size() - 1 : 0);
            lineX += tx;
            lineY += ty;
            curX = lineX;
            curY = lineY;
            if (tx != 0 || ty != 0)
                moved = true;
        } else if (op == "T*") {
            lineY -= 12;
            curY = lineY;
            curX = lineX;
            moved = true;
        } else if (op == "Tm") {
            if (ops.size() >= 6) {
                const double e = ops[ops.size() - 2].num;
                const double f = ops[ops.size() - 1].num;
                lineX = curX = e;
                lineY = curY = f;
                moved = true;
            }
        } else if (op == "Tj") {
            if (!ops.empty() && ops.back().type == PVal::Str)
                showString(ops.back().s);
        } else if (op == "'" || op == "\"") {
            lineY -= 12;
            curY = lineY;
            moved = true;
            if (!ops.empty() && ops.back().type == PVal::Str)
                showString(ops.back().s);
        } else if (op == "TJ") {
            if (!ops.empty() && ops.back().type == PVal::Arr) {
                for (const PVal &e : ops.back().arr) {
                    if (e.type == PVal::Str) {
                        showString(e.s);
                    } else if (e.isNum() && e.num < -200 && font && font->decodable && !out.empty()
                               && lastChar() != ' ' && lastChar() != '\n') {
                        out.push_back(' ');
                    }
                }
            }
        } else if (op == "Do") {
            if (depth < 3 && xobjs && xobjs->type == PVal::Dict && !ops.empty() && ops.back().type == PVal::Name) {
                const PVal *xr = xobjs->get(ops.back().s);
                if (xr && xr->type == PVal::Ref) {
                    const PVal *xd = resolve(xr);
                    const PVal *st = xd ? resolve(xd->get("Subtype")) : nullptr;
                    if (xd && st && st->isName("Form")) {
                        Bytes fdata;
                        if (streamData(xr->ref, fdata)) {
                            const PVal *fres = xd->get("Resources") ? xd->get("Resources") : resources;
                            ensureSeparator(true);
                            interpret(fdata, fres, out, depth + 1);
                            ensureSeparator(true);
                            lineStart = true;
                        }
                    }
                }
            }
        }
        (void)wasNewLine;
        ops.clear();
    }
}

std::string PdfDoc::extractText(std::string &err)
{
    std::vector<std::pair<const PVal *, const PVal *>> pages;
    std::set<const PVal *> seen;

    // racine du document : objet /Type /Catalog -> /Pages
    const PVal *pagesRoot = nullptr;
    for (const auto &kv : objs_) {
        if (kv.second.type == PVal::Dict) {
            const PVal *t = resolve(kv.second.get("Type"));
            if (t && t->isName("Catalog")) {
                pagesRoot = kv.second.get("Pages");
                break;
            }
        }
    }
    if (pagesRoot)
        collectPages(pagesRoot, nullptr, pages, 0, seen);

    if (pages.empty()) {
        // repli : toutes les pages dans l'ordre des numéros d'objets
        for (const auto &kv : objs_) {
            if (kv.second.type == PVal::Dict) {
                const PVal *t = resolve(kv.second.get("Type"));
                if (t && t->isName("Page"))
                    pages.emplace_back(&kv.second, kv.second.get("Resources"));
            }
        }
    }
    if (pages.empty()) {
        err = "Aucune page n'a pu être lue dans ce PDF.";
        return std::string();
    }

    std::string all;
    for (const auto &pg : pages) {
        const PVal *contents = pg.first->get("Contents");
        std::vector<const PVal *> parts;
        const PVal *rc = resolve(contents);
        if (rc && rc->type == PVal::Arr) {
            for (const PVal &c : rc->arr)
                parts.push_back(&c);
        } else if (contents) {
            parts.push_back(contents);
        }

        Bytes page;
        for (const PVal *p : parts) {
            if (p->type != PVal::Ref)
                continue;
            Bytes data;
            if (streamData(p->ref, data)) {
                page.insert(page.end(), data.begin(), data.end());
                page.push_back('\n');
            }
        }
        std::string pageText;
        interpret(page, pg.second, pageText, 0);
        all += pageText;
        if (!all.empty() && all.back() != '\n')
            all.push_back('\n');
        all.push_back('\n');
    }
    return all;
}
} // namespace

bool pdfToText(const Bytes &file, std::string &text, std::string &err)
{
    text.clear();
    err.clear();
    PdfDoc doc(file);
    if (!doc.load(err))
        return false;

    std::string t = doc.extractText(err);
    if (!err.empty())
        return false;

    // vérification de qualité : on ne fabrique jamais de texte
    int letters = 0;
    for (unsigned char c : t)
        if (std::isalpha(c) || c >= 0x80)
            ++letters;
    if (letters < 40) {
        err = "Aucun texte exploitable n'a pu être extrait de ce PDF. Il s'agit probablement d'un document "
              "scanné (image) ou d'un PDF dont les polices ne contiennent pas de table de correspondance "
              "des caractères. Utilisez un CV au format texte (PDF généré depuis Word, par exemple) ou un fichier DOCX.";
        return false;
    }
    text = t;
    return true;
}

bool extraireTexteCv(const Bytes &file, std::string &text, std::string &err)
{
    text.clear();
    err.clear();
    if (file.size() < 8) {
        err = "Le fichier est vide ou trop petit pour être un CV.";
        return false;
    }
    if (file[0] == 'P' && file[1] == 'K')
        return docxToText(file, text, err);

    const std::size_t lim = std::min<std::size_t>(file.size(), 1024);
    const char *sig = "%PDF-";
    if (std::search(file.begin(), file.begin() + static_cast<std::ptrdiff_t>(lim), sig, sig + 5)
        != file.begin() + static_cast<std::ptrdiff_t>(lim))
        return pdfToText(file, text, err);

    err = "Format non reconnu. Seuls les fichiers PDF et DOCX sont acceptés.";
    return false;
}

} // namespace cvcore
