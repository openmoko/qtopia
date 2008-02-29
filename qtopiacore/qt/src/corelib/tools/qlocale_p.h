/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtCore module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QLOCALE_P_H
#define QLOCALE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists for the convenience
// of internal files.  This header file may change from version to version
// without notice, or even be removed.
//
// We mean it.
//

#include "QtCore/qstring.h"
#include "QtCore/qvarlengtharray.h"

struct QLocalePrivate
{
public:
    QChar decimal() const { return QChar(m_decimal); }
    QChar group() const { return QChar(m_group); }
    QChar list() const { return QChar(m_list); }
    QChar percent() const { return QChar(m_percent); }
    QChar zero() const { return QChar(m_zero); }
    QChar plus() const { return QLatin1Char('+'); }
    QChar minus() const { return QChar(m_minus); }
    QChar exponential() const { return QChar(m_exponential); }

    quint32 languageId() const { return m_language_id; }
    quint32 countryId() const { return m_country_id; }

    enum DoubleForm {
        DFExponent = 0,
        DFDecimal,
        DFSignificantDigits,
        _DFMax = DFSignificantDigits
    };

    enum Flags {
        NoFlags             = 0,
        Alternate           = 0x01,
        ZeroPadded          = 0x02,
        LeftAdjusted        = 0x04,
        BlankBeforePositive = 0x08,
        AlwaysShowSign      = 0x10,
        ThousandsGroup      = 0x20,
        CapitalEorX         = 0x40
    };

    enum GroupSeparatorMode {
        FailOnGroupSeparators,
        ParseGroupSeparators
    };

    QString doubleToString(double d,
                           int precision = -1,
                           DoubleForm form = DFSignificantDigits,
                           int width = -1,
                           unsigned flags = NoFlags) const;
    QString longLongToString(qint64 l, int precision = -1,
                             int base = 10,
                             int width = -1,
                             unsigned flags = NoFlags) const;
    QString unsLongLongToString(quint64 l, int precision = -1,
                                int base = 10,
                                int width = -1,
                                unsigned flags = NoFlags) const;
    double stringToDouble(const QString &num, bool *ok, GroupSeparatorMode group_sep_mode) const;
    qint64 stringToLongLong(const QString &num, int base, bool *ok, GroupSeparatorMode group_sep_mode) const;
    quint64 stringToUnsLongLong(const QString &num, int base, bool *ok, GroupSeparatorMode group_sep_mode) const;


    static double bytearrayToDouble(const char *num, bool *ok);
    static qint64 bytearrayToLongLong(const char *num, int base, bool *ok);
    static quint64 bytearrayToUnsLongLong(const char *num, int base, bool *ok);

    typedef QVarLengthArray<char, 256> CharBuff;
    bool numberToCLocale(const QString &num,
    	    	    	  GroupSeparatorMode group_sep_mode,
                          CharBuff *result) const;

    static void updateSystemPrivate();

    quint32 m_language_id, m_country_id;

    quint16 m_decimal, m_group, m_list, m_percent,
        m_zero, m_minus, m_exponential;

    quint32 m_short_date_format_idx, m_long_date_format_idx;
    quint32 m_short_time_format_idx, m_long_time_format_idx;
    quint32 m_short_month_names_idx, m_long_month_names_idx;
    quint32 m_short_day_names_idx, m_long_day_names_idx;
};

#endif // QLOCALE_P_H
