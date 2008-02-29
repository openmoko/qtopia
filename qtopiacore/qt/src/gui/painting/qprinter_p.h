/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QPRINTER_P_H
#define QPRINTER_P_H

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


#include "QtCore/qglobal.h"

#ifndef QT_NO_PRINTER

#include "QtGui/qprinter.h"
#include "QtGui/qprintdialog.h"
#include "QtCore/qpointer.h"

class QPrintEngine;

class QPrinterPrivate
{
    Q_DECLARE_PUBLIC(QPrinter)
public:
    QPrinterPrivate(QPrinter *printer)
        : printEngine(0)
        , paintEngine(0)
        , q_ptr(printer)
#if !(defined(QT_NO_PRINTDIALOG))
        , ownPrintDialog(0)
        , printDialog(0)
#endif
        , use_default_engine(true)
    {
    }

    ~QPrinterPrivate() {
#if !(defined(QT_NO_PRINTDIALOG))
        delete ownPrintDialog;
        ownPrintDialog = 0;
#endif
    }

    void createDefaultEngines();

    QPrinter::PrinterMode printerMode;
    QPrinter::OutputFormat outputFormat;
    QPrintEngine *printEngine;
    QPaintEngine *paintEngine;
    QPrinter *q_ptr;

#if !(defined(QT_NO_PRINTDIALOG))
    mutable QPrintDialog *ownPrintDialog;
    mutable QPointer<QAbstractPrintDialog> printDialog;
#endif

    bool use_default_engine;
#ifndef QT_NO_PRINTDIALOG
    void ensurePrintDialog() const {
        if (printDialog)
            return;
        if (!ownPrintDialog) {
            ownPrintDialog = new QPrintDialog(q_ptr); // printDialog is set here.
        } else {
            printDialog = ownPrintDialog;
        }
    }
#endif
};

#endif // QT_NO_PRINTER

#endif // QPRINTER_P_H
