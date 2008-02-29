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

#include "qabstractpagesetupdialog.h"
#include "qabstractpagesetupdialog_p.h"

#ifndef QT_NO_PRINTDIALOG

#include <QtGui/qprinter.h>

/*!
    \internal
    \class QAbstractPageSetupDialog

    \brief The QAbstractPageSetupDialog class provides a base for
    implementations of page setup dialogs.
*/

/*!
    Constructs the page setup dialog for the printer \a printer with
    \a parent as parent widget.
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QPrinter *printer, QWidget *parent)
    : QDialog(*(new QAbstractPageSetupDialogPrivate), parent)
{
    Q_D(QAbstractPageSetupDialog);
    d->printer = printer;

    if (printer->outputFormat() != QPrinter::NativeFormat) {
        qWarning("QAbstractPageSetupDialog::QAbstractPageSetupDialog: Page setup dialog cannot be "
                 "used on non-native printers");    
    }
}

/*!
    \internal
*/
QAbstractPageSetupDialog::QAbstractPageSetupDialog(QAbstractPageSetupDialogPrivate &ptr,
                                                   QPrinter *printer, QWidget *parent)
    : QDialog(ptr, parent)
{
    Q_D(QAbstractPageSetupDialog);
    d->printer = printer;

    if (printer->outputFormat() != QPrinter::NativeFormat) {
        qWarning("QAbstractPageSetupDialog::QAbstractPageSetupDialog: Page setup dialog cannot be "
                 "used on non-native printers");    
    }
}

/*!
    Returns the printer that this page setup dialog is operating on.
*/
QPrinter *QAbstractPageSetupDialog::printer()
{
    Q_D(QAbstractPageSetupDialog);
    return d->printer;
}

/*!
    \fn int QAbstractPageSetupDialog::exec()

    This virtual function is called to pop up the dialog. It must be
    reimplemented in subclasses.
*/

#endif // QT_NO_PRINTDIALOG
