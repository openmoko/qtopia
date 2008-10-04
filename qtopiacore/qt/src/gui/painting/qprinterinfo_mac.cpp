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

#include "private/qt_mac_p.h"

QT_BEGIN_NAMESPACE

#ifndef QT_NO_PRINTER

class QPrinterInfoPrivate
{
Q_DECLARE_PUBLIC(QPrinterInfo)
public:
    ~QPrinterInfoPrivate();
    QPrinterInfoPrivate();
    QPrinterInfoPrivate(const QString& name);

private:
    QPrinterInfo*                 q_ptr;

    QString                     m_name;
    bool                        m_default;
    bool                        m_isNull;
};

static QPrinterInfoPrivate nullQPrinterInfoPrivate;

extern QPrinter::PaperSize qSizeFTopaperSize(const QSizeF& size);

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QList<QPrinterInfo> QPrinterInfo::availablePrinters()
{
    QList<QPrinterInfo> printers;

    OSStatus status = noErr;
    QCFType<CFArrayRef> printerList;
    status = PMServerCreatePrinterList(kPMServerLocal, &printerList);
    if (status == noErr) {
        CFIndex count = CFArrayGetCount(printerList);
        for (CFIndex i=0; i<count; ++i) {
            PMPrinter printer = static_cast<PMPrinter>(const_cast<void *>(CFArrayGetValueAtIndex(printerList, i)));
            QString name = QCFString::toQString(PMPrinterGetName(printer));
            printers.append(QPrinterInfo(name));
            if (PMPrinterIsDefault(printer)) {
                printers[i].d_ptr->m_default = true;
            }
        }
    }

    return printers;
}

QPrinterInfo QPrinterInfo::defaultPrinter(){
    QList<QPrinterInfo> printers = availablePrinters();
    for (int c = 0; c < printers.size(); ++c) {
        if (printers[c].isDefault()) {
            return printers[c];
        }
    }
    return QPrinterInfo();
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

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
#if (MAC_OS_X_VERSION_MAX_ALLOWED < MAC_OS_X_VERSION_10_4)
		return QList<QPrinter::PaperSize>();
#else
	if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_4)
		return QList<QPrinter::PaperSize>();

    const Q_D(QPrinterInfo);

    PMPrinter cfPrn = PMPrinterCreateFromPrinterID(
            QCFString::toCFStringRef(d->m_name));

    if (!cfPrn) return QList<QPrinter::PaperSize>();

    CFArrayRef array;
    OSStatus status = PMPrinterGetPaperList(cfPrn, &array);

    if (status != 0) {
        PMRelease(cfPrn);
        return QList<QPrinter::PaperSize>();
    }

    QList<QPrinter::PaperSize> paperList;
    int count = CFArrayGetCount(array);
    for (int c = 0; c < count; c++) {
        PMPaper paper = static_cast<PMPaper>(
                const_cast<void*>(
                CFArrayGetValueAtIndex(array, c)));
        double width, height;
        status = PMPaperGetWidth(paper, &width);
        status |= PMPaperGetHeight(paper, &height);
        if (status != 0) continue;

        QSizeF size(width * 0.3527, height * 0.3527);
        paperList.append(qSizeFTopaperSize(size));
    }

    PMRelease(cfPrn);

    return paperList;
#endif // MAC_OS_X_VERSION_MAX_ALLOWED
}

/////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////

QPrinterInfoPrivate::QPrinterInfoPrivate() :
    q_ptr(NULL),
    m_default(false),
    m_isNull(true)
{
}

QPrinterInfoPrivate::QPrinterInfoPrivate(const QString& name) :
    q_ptr(NULL),
    m_name(name),
    m_default(false),
    m_isNull(false)
{
}

QPrinterInfoPrivate::~QPrinterInfoPrivate()
{
}

#endif // QT_NO_PRINTER

QT_END_NAMESPACE
