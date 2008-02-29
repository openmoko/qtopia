/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QCUPS_P_H
#define QCUPS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//
#include "QtCore/qstring.h"
#include "QtCore/qstringlist.h"
#include "QtGui/qprinter.h"

#ifndef QT_NO_CUPS
#include <QtCore/qlibrary.h>
#include <cups/cups.h>

class QCUPSSupport
{
public:
    QCUPSSupport();
    ~QCUPSSupport();

    static bool isAvailable();
    static int cupsVersion() { return isAvailable() ? CUPS_VERSION_MAJOR*10000+CUPS_VERSION_MINOR*100+CUPS_VERSION_PATCH : 0; }
    int availablePrintersCount() const;
    const cups_dest_t* availablePrinters() const;
    int currentPrinterIndex() const;
    const ppd_file_t* setCurrentPrinter(int index);

    const ppd_file_t* currentPPD() const;
    const ppd_option_t* ppdOption(const char *key) const;

//     const cups_option_t* printerOption(const QString &key) const;
    const ppd_option_t* pageSizes() const;

    int markOption(const char* name, const char* value);
    void saveOptions(QList<const ppd_option_t*> options, QList<const char*> markedOptions);

    QRect paperRect(const char *choice) const;
    QRect pageRect(const char *choice) const;

    QStringList options() const;

    static bool printerHasPPD(const char *printerName);

    QString unicodeString(const char *s);

private:
    void collectMarkedOptions(QStringList& list, const ppd_group_t* group = 0) const;
    void collectMarkedOptionsHelper(QStringList& list, const ppd_group_t* group) const;

    int prnCount;
    cups_dest_t *printers;
    const ppd_option_t* page_sizes;
    int currPrinterIndex;
    ppd_file_t *currPPD;
#ifndef QT_NO_TEXTCODEC
    QTextCodec *codec;
#endif
};

#endif // QT_NO_CUPS
#endif
