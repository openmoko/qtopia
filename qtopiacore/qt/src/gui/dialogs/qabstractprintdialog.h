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

#ifndef QABSTRACTPRINTDIALOG_H
#define QABSTRACTPRINTDIALOG_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PRINTER

class QAbstractPrintDialogPrivate;
class QPrinter;

// ### Qt 5: remove this class
class Q_GUI_EXPORT QAbstractPrintDialog : public QDialog
{
    Q_DECLARE_PRIVATE(QAbstractPrintDialog)
    Q_OBJECT

public:
    enum PrintRange {
        AllPages,
        Selection,
        PageRange
    };

    enum PrintDialogOption {
        None                    = 0x0000,
        PrintToFile             = 0x0001,
        PrintSelection          = 0x0002,
        PrintPageRange          = 0x0004,
        PrintCollateCopies      = 0x0010
    };

    Q_DECLARE_FLAGS(PrintDialogOptions, PrintDialogOption)

#ifndef QT_NO_PRINTDIALOG

    explicit QAbstractPrintDialog(QPrinter *printer, QWidget *parent = 0);

    virtual int exec() = 0;

    void addEnabledOption(PrintDialogOption option);
    void setEnabledOptions(PrintDialogOptions options);
    PrintDialogOptions enabledOptions() const;
    bool isOptionEnabled(PrintDialogOption option) const;

    void setPrintRange(PrintRange range);
    PrintRange printRange() const;

    void setMinMax(int min, int max);
    int minPage() const;
    int maxPage() const;

    void setFromTo(int fromPage, int toPage);
    int fromPage() const;
    int toPage() const;

    QPrinter *printer() const;

protected:
    QAbstractPrintDialog(QAbstractPrintDialogPrivate &ptr, QPrinter *printer, QWidget *parent = 0);

private:
    Q_DISABLE_COPY(QAbstractPrintDialog)

#endif // QT_NO_PRINTDIALOG
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractPrintDialog::PrintDialogOptions)

#endif // QT_NO_PRINTER

QT_END_HEADER

#endif // QABSTRACTPRINTDIALOG_H
