/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qunicodetables_p.h"
#include "qunicodedata.cpp"

#define CURRENT_VERSION QChar::Unicode_4_0

static inline const QUnicodeTables::Properties *qGetProp(uint ucs4)
{
    int index = GET_PROP_INDEX(ucs4);
    return uc_properties + index;
}

static inline const QUnicodeTables::Properties *qGetProp(ushort ucs2)
{
    int index = GET_PROP_INDEX_UCS2(ucs2);
    return uc_properties + index;
}

Q_CORE_EXPORT const QUnicodeTables::Properties *QUnicodeTables::properties(uint ucs4)
{
    int index = GET_PROP_INDEX(ucs4);
    return uc_properties + index;
}

Q_CORE_EXPORT QChar::Category QUnicodeTables::category(uint ucs4)
{
    return (QChar::Category) qGetProp(ucs4)->category;
}

Q_CORE_EXPORT unsigned char QUnicodeTables::combiningClass(uint ucs4)
{
    return (unsigned char) qGetProp(ucs4)->combiningClass;
}

Q_CORE_EXPORT QChar::Direction QUnicodeTables::direction(uint ucs4)
{
    return (QChar::Direction) qGetProp(ucs4)->direction;
}

Q_CORE_EXPORT QUnicodeTables::LineBreakClass QUnicodeTables::lineBreakClass(uint ucs4)
{
    return (QUnicodeTables::LineBreakClass) qGetProp(ucs4)->line_break_class;
}

Q_CORE_EXPORT QChar::Joining QUnicodeTables::joining(uint ucs4)
{
    return (QChar::Joining) qGetProp(ucs4)->joining;
}

Q_CORE_EXPORT QChar::UnicodeVersion QUnicodeTables::unicodeVersion(uint ucs4)
{
    return (QChar::UnicodeVersion) qGetProp(ucs4)->unicode_version;
}

Q_CORE_EXPORT int QUnicodeTables::digitValue(uint ucs4)
{
    int val = qGetProp(ucs4)->digit_value;
    return val == 0xf ? -1 : val;
}

Q_CORE_EXPORT bool QUnicodeTables::mirrored(uint ucs4)
{
    return qGetProp(ucs4)->mirrorDiff != 0;
}

Q_CORE_EXPORT int QUnicodeTables::mirroredChar(uint ucs4)
{
    return ucs4 + qGetProp(ucs4)->mirrorDiff;
}

Q_CORE_EXPORT int QUnicodeTables::upper(uint ucs4)
{
    const QUnicodeTables::Properties *p = qGetProp(ucs4);
    if (p->category == QChar::Letter_Lowercase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

Q_CORE_EXPORT int QUnicodeTables::lower(uint ucs4)
{
    const QUnicodeTables::Properties *p = qGetProp(ucs4);
    if (p->category == QChar::Letter_Uppercase || p->category == QChar::Letter_Titlecase)
        return ucs4 + p->caseDiff;
    return ucs4;
}

enum {
    Hangul_SBase = 0xac00,
    Hangul_LBase = 0x1100,
    Hangul_VBase = 0x1161,
    Hangul_TBase = 0x11a7,
    Hangul_SCount = 11172,
    Hangul_LCount = 19,
    Hangul_VCount = 21,
    Hangul_TCount = 28,
    Hangul_NCount = 21*28
};

// buffer has to have a length of 3. It's needed for Hangul decomposition
static const unsigned short * QT_FASTCALL decomposition(uint ucs4, int *length, int *tag, unsigned short *buffer)
{
    if (ucs4 >= Hangul_SBase && ucs4 < Hangul_SBase + Hangul_SCount) {
        int SIndex = ucs4 - Hangul_SBase;
        buffer[0] = Hangul_LBase + SIndex / Hangul_NCount; // L
        buffer[1] = Hangul_VBase + (SIndex % Hangul_NCount) / Hangul_TCount; // V
        buffer[2] = Hangul_TBase + SIndex % Hangul_TCount; // T
        *length = buffer[2] == Hangul_TBase ? 2 : 3;
        *tag = QChar::Canonical;
        return buffer;
    }

    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff)
        return 0;
    const unsigned short *decomposition = uc_decomposition_map+index;
    *tag = (*decomposition) & 0xff;
    *length = (*decomposition) >> 8;
    return decomposition+1;
}

QString QUnicodeTables::decomposition(uint ucs4)
{
    unsigned short buffer[3];
    int length;
    int tag;
    const unsigned short *d = ::decomposition(ucs4, &length, &tag, buffer);
    return QString::fromUtf16(d, length);
}

QChar::Decomposition QUnicodeTables::decompositionTag(uint ucs4)
{
    const unsigned short index = GET_DECOMPOSITION_INDEX(ucs4);
    if (index == 0xffff)
        return QChar::NoDecomposition;
    return (QChar::Decomposition)(uc_decomposition_map[index] & 0xff);
}

ushort QUnicodeTables::ligature(ushort u1, ushort u2)
{
    // hangul L-V pair
    int LIndex = u1 - Hangul_LBase;
    if (0 <= LIndex && LIndex < Hangul_LCount) {
        int VIndex = u2 - Hangul_VBase;
        if (0 <= VIndex && VIndex < Hangul_VCount)
            return Hangul_SBase + (LIndex * Hangul_VCount + VIndex) * Hangul_TCount;
    }

    // hangul LV-T pair
    int SIndex = u1 - Hangul_SBase;
    if (0 <= SIndex && SIndex < Hangul_SCount && (SIndex % Hangul_TCount) == 0) {
        int TIndex = u2 - Hangul_TBase;
        if (0 <= TIndex && TIndex <= Hangul_TCount)
            return u1 + TIndex;
    }

    const unsigned short index = GET_LIGATURE_INDEX(u2);
    if (index == 0xffff)
        return 0;
    const unsigned short *ligatures = uc_ligature_map+index;
    ushort length = *ligatures;
    ++ligatures;
    // ### use bsearch
    for (uint i = 0; i < length; ++i)
        if (ligatures[2*i] == u1)
            return ligatures[2*i+1];
    return 0;
}


static QString decompose(const QString &str, bool canonical, QChar::UnicodeVersion version)
{
    unsigned short buffer[3];

    QString s = str;

    const unsigned short *utf16 = s.utf16();
    const unsigned short *uc = utf16 + s.length();
    while (uc != utf16) {
        uint ucs4 = *(--uc);
        if (QUnicodeTables::isLowSurrogate(ucs4) && uc != utf16) {
            ushort high = *(uc - 1);
            if (QUnicodeTables::isHighSurrogate(high)) {
                --uc;
                ucs4 = QUnicodeTables::surrogateToUcs4(high, ucs4);
            }
        }
        if (QUnicodeTables::unicodeVersion(ucs4) > version)
            continue;
        int length;
        int tag;
        const unsigned short *d = decomposition(ucs4, &length, &tag, buffer);
        if (!d || (canonical && tag != QChar::Canonical))
            continue;

        s.replace(uc - utf16, ucs4 > 0x10000 ? 2 : 1, (const QChar *)d, length);
        // since the insert invalidates the pointers and we do decomposition recursive
        int pos = uc - utf16;
        utf16 = s.utf16();
        uc = utf16 + pos + length;
    }

    return s;
}

static QString compose(const QString &str)
{
    QString s = str;

    if (s.length() < 2)
        return s;

    // the loop can partly ignore high Unicode as all ligatures are in the BMP
    int starter = 0;
    int lastCombining = 0;
    int pos = 0;
    while (pos < s.length()) {
        uint uc = s.utf16()[pos];
        if (QUnicodeTables::isHighSurrogate(uc) && pos < s.length()-1) {
            ushort low = s.utf16()[pos+1];
            if (QUnicodeTables::isLowSurrogate(low)) {
                uc = QUnicodeTables::surrogateToUcs4(uc, low);
                ++pos;
            }
        }
        int combining = QUnicodeTables::combiningClass(uc);
        if (starter == pos - 1 || combining != lastCombining) {
            // allowed to form ligature with S
            QChar ligature = QUnicodeTables::ligature(s.utf16()[starter], uc);
            if (ligature.unicode()) {
                s[starter] = ligature;
                s.remove(pos, 1);
                continue;
            }
        }
        if (!combining)
            starter = pos;
        lastCombining = combining;
        ++pos;
    }
    return s;
}


static QString canonicalOrder(const QString &str, QChar::UnicodeVersion version)
{
    QString s = str;
    const int l = s.length()-1;
    int pos = 0;
    while (pos < l) {
        int p2 = pos+1;
        uint u1 = s.at(pos).unicode();
        if (QUnicodeTables::isHighSurrogate(u1)) {
            ushort low = s.at(pos+1).unicode();
            if (QUnicodeTables::isLowSurrogate(low)) {
                p2++;
                u1 = QUnicodeTables::surrogateToUcs4(u1, low);
                if (p2 >= l)
                    break;
            }
        }
        uint u2 = s.at(p2).unicode();
        if (QUnicodeTables::isHighSurrogate(u2) && p2 < l-1) {
            ushort low = s.at(p2+1).unicode();
            if (QUnicodeTables::isLowSurrogate(low)) {
                p2++;
                u2 = QUnicodeTables::surrogateToUcs4(u2, low);
            }
        }

        int c2 = QUnicodeTables::combiningClass(u2);
        if (QUnicodeTables::unicodeVersion(u2) > version)
            c2 = 0;

        if (c2 == 0) {
            pos = p2+1;
            continue;
        }
        int c1 = QUnicodeTables::combiningClass(u1);
        if (QUnicodeTables::unicodeVersion(u1) > version)
            c1 = 0;

        if (c1 > c2) {
            QChar *uc = s.data();
            int p = pos;
            // exchange characters
            if (u2 < 0x10000) {
                uc[p++] = u2;
            } else {
                uc[p++] = QUnicodeTables::highSurrogate(u2);
                uc[p++] = QUnicodeTables::lowSurrogate(u2);
            }
            if (u1 < 0x10000) {
                uc[p++] = u1;
            } else {
                uc[p++] = QUnicodeTables::highSurrogate(u1);
                uc[p++] = QUnicodeTables::lowSurrogate(u1);
            }
            if (pos > 0)
                --pos;
            if (pos > 0 && QUnicodeTables::isLowSurrogate(s.at(pos).unicode()))
                --pos;
        } else {
            ++pos;
            if (u1 > 0x10000)
                ++pos;
        }
    }
    return s;
}

QString QUnicodeTables::normalize(const QString &str, QString::NormalizationForm mode)
{
    return normalize(str, mode, CURRENT_VERSION);
}

QString QUnicodeTables::normalize(const QString &str, QString::NormalizationForm mode, QChar::UnicodeVersion version)
{
    QString s = str;
    if (version != CURRENT_VERSION) {
        for (int i = 0; i < NumNormalizationCorrections; ++i) {
            const NormalizationCorrection n = uc_normalization_corrections[i];
            if (n.version > version) {
                QString orig;
                orig += QChar(highSurrogate(n.ucs4));
                orig += QChar(lowSurrogate(n.ucs4));
                QString replacement;
                replacement += QChar(highSurrogate(n.old_mapping));
                replacement += QChar(lowSurrogate(n.old_mapping));
                s.replace(orig, replacement);
            }
        }
    }
    s = decompose(s, mode < QString::NormalizationForm_KD, version);

    s = canonicalOrder(s, version);

    if (mode == QString::NormalizationForm_D || mode == QString::NormalizationForm_KD)
        return s;

    return compose(s);

}

int QUnicodeTables::script(unsigned int uc)
{
    if (uc > 0xffff)
        return Common;
    int script = uc_scripts[uc >> 7];
    if (script < ScriptSentinel)
        return script;
    script = (((script - ScriptSentinel) * UnicodeBlockSize) + UnicodeBlockCount);
    script = uc_scripts[script + (uc & 0x7f)];
    return script;
}
