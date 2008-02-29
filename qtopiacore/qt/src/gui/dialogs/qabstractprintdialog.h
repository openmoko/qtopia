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

#ifndef QABSTRACTPRINTDIALOG_H
#define QABSTRACTPRINTDIALOG_H

#include <QtGui/qdialog.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_PRINTDIALOG

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
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QAbstractPrintDialog::PrintDialogOptions)

#endif // QT_NO_PRINTDIALOG

QT_END_HEADER

#endif // QABSTRACTPRINTDIALOG_H
