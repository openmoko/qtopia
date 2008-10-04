/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qprinterinfo.h"

#include <qstringlist.h>

#include <windows.h>

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

extern QPrinter::PaperSize mapDevmodePaperSize(int s);

class QPrinterInfoPrivate
{
Q_DECLARE_PUBLIC(QPrinterInfo)
public:
    ~QPrinterInfoPrivate();
    QPrinterInfoPrivate();
    QPrinterInfoPrivate(const QString& name);

private:
    QString                     m_name;
    bool                        m_default;
    bool                        m_isNull;

    QPrinterInfo*               q_ptr;
};

static QPrinterInfoPrivate nullQPrinterInfoPrivate;

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QList<QPrinterInfo> QPrinterInfo::availablePrinters()
{
    QList<QPrinterInfo> printers;
    LPBYTE buffer;
    DWORD needed;
    DWORD returned;
    DWORD size;

    size = 512;
    buffer = new BYTE[size];
    BOOL ret = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 1, buffer, size, &needed, &returned);
    if (!ret) {
        size += needed;
        delete [] buffer;
        buffer = new BYTE[size];
        BOOL ret = EnumPrinters(PRINTER_ENUM_LOCAL, NULL, 1, buffer, size, &needed, &returned);
        if (!ret) {
            // Failed. Return empty list.
            delete [] buffer;
            return printers;
        }
    }

    PPRINTER_INFO_1 infoList = reinterpret_cast<PPRINTER_INFO_1>(buffer);
    QPrinterInfo defPrn = defaultPrinter();

    for (int c = 0; c < (int)returned; ++c) {
        printers.append(QPrinterInfo(QT_WA_INLINE(
            QString::fromUtf16(reinterpret_cast<const USHORT*>(infoList[c].pName)),
            QString::fromLocal8Bit(reinterpret_cast<const char*>(infoList[c].pName)))));
        if (printers[c].printerName() == defPrn.printerName()) {
            printers[c].d_ptr->m_default = true;
        }
    }

    delete [] buffer;

    return printers;
}

QPrinterInfo QPrinterInfo::defaultPrinter()
{
    LPTSTR section = TEXT("windows");
    LPTSTR key = TEXT("device");
    TCHAR output[256];

    GetProfileString(section, key, NULL, output, 256);

    QString string(QT_WA_INLINE(
        QString::fromUtf16(reinterpret_cast<const USHORT*>(output)),
        QString::fromLocal8Bit(reinterpret_cast<const char*>(output))));

    // Filter out the name of the printer, which should be everything before
    // a comma.
    QPrinterInfo prn(string.split(QLatin1Char(','))[0]);
    prn.d_ptr->m_default = true;
    if (prn.printerName() == QLatin1String("")) {
        prn.d_ptr->m_isNull = true;
    }
    return prn;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QPrinterInfo::QPrinterInfo()
{
    d_ptr = &nullQPrinterInfoPrivate;
}

QPrinterInfo::QPrinterInfo(const QString& name)
{
    d_ptr = new QPrinterInfoPrivate(name);
    d_ptr->q_ptr = this;
}

QPrinterInfo::QPrinterInfo(const QPrinterInfo& src)
{
    d_ptr = &nullQPrinterInfoPrivate;
    *this = src;
}

QPrinterInfo::QPrinterInfo(const QPrinter& prn)
{
    d_ptr = &nullQPrinterInfoPrivate;
    QList<QPrinterInfo> list = availablePrinters();
    for (int c = 0; c < list.size(); ++c) {
        if (prn.printerName() == list[c].printerName()) {
            *this = list[c];
            return;
        }
    }

    *this = QPrinterInfo();
}

QPrinterInfo::~QPrinterInfo()
{
    if (d_ptr != &nullQPrinterInfoPrivate)
        delete d_ptr;
}

QPrinterInfo& QPrinterInfo::operator=(const QPrinterInfo& src)
{
    Q_ASSERT(d_ptr);
    if (d_ptr != &nullQPrinterInfoPrivate)
        delete d_ptr;
    d_ptr = new QPrinterInfoPrivate(*src.d_ptr);
    d_ptr->q_ptr = this;
    return *this;
}

QString QPrinterInfo::printerName() const
{
    const Q_D(QPrinterInfo);
    return d->m_name;
}

bool QPrinterInfo::isNull() const
{
    const Q_D(QPrinterInfo);
    return d->m_isNull;
}

bool QPrinterInfo::isDefault() const
{
    const Q_D(QPrinterInfo);
    return d->m_default;
}

QList<QPrinter::PaperSize> QPrinterInfo::supportedPaperSizes() const
{
    const Q_D(QPrinterInfo);
    DWORD size;
    WORD* papers;
    QList<QPrinter::PaperSize> paperList;

    QT_WA({
        size = DeviceCapabilitiesW(reinterpret_cast<const WCHAR*>(d->m_name.utf16()),
                NULL, DC_PAPERS, NULL, NULL);
        if ((int)size == -1)
            return paperList;
        papers = new WORD[size];
        size = DeviceCapabilitiesW(reinterpret_cast<const WCHAR*>(d->m_name.utf16()),
                NULL, DC_PAPERS, reinterpret_cast<WCHAR*>(papers), NULL);
    }, {
        size = DeviceCapabilitiesA(d->m_name.toLatin1().data(), NULL, DC_PAPERS, NULL, NULL);
        if ((int)size == -1)
            return paperList;
        papers = new WORD[size];
        size = DeviceCapabilitiesA(d->m_name.toLatin1().data(), NULL, DC_PAPERS,
                reinterpret_cast<char*>(papers), NULL);
    });

    for (int c = 0; c < (int)size; ++c) {
        paperList.append(mapDevmodePaperSize(papers[c]));
    }

    delete [] papers;

    return paperList;
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QPrinterInfoPrivate::QPrinterInfoPrivate() :
    m_default(false),
    m_isNull(true),
    q_ptr(NULL)
{
}

QPrinterInfoPrivate::QPrinterInfoPrivate(const QString& name) :
    m_name(name),
    m_default(false),
    m_isNull(false),
    q_ptr(NULL)
{
}

QPrinterInfoPrivate::~QPrinterInfoPrivate()
{
}

#endif // QT_NO_PRINTER

QT_END_NAMESPACE
