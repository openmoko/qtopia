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

#ifndef QABSTRACTPRINTDIALOG_P_H
#define QABSTRACTPRINTDIALOG_P_H

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

#ifndef QT_NO_PRINTDIALOG

#include "private/qdialog_p.h"
#include "QtGui/qabstractprintdialog.h"

class QPrinter;

class QAbstractPrintDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QAbstractPrintDialog)
public:
    QAbstractPrintDialogPrivate()
        :
        printer(0),
        options(QAbstractPrintDialog::PrintToFile | QAbstractPrintDialog::PrintPageRange),
        printRange(QAbstractPrintDialog::AllPages),
        minPage(1),
        maxPage(1),
        fromPage(0),
        toPage(0)
    {
    }

    void init(QAbstractPrintDialogPrivate *d) {
        options = d->options;
        printRange = d->printRange;
        minPage = d->minPage;
        maxPage = d->maxPage;
        fromPage = d->fromPage;
        toPage = d->toPage;
    }

    QPrinter *printer;
    QAbstractPrintDialog::PrintDialogOptions options;
    QAbstractPrintDialog::PrintRange printRange;
    int minPage, maxPage, fromPage, toPage;
};

#endif // QT_NO_PRINTDIALOG

#endif // QABSTRACTPRINTDIALOG_P_H
