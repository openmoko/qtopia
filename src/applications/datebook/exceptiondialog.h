/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#ifndef EXCEPTIONDIALOG_H
#define EXCEPTIONDIALOG_H

#include "ui_exceptiondialogbase.h"


class ExceptionDialog : public QDialog, public Ui::ExceptionDialogBase
{
    Q_OBJECT
public:
    ExceptionDialog( QWidget *parent, Qt::WFlags f = 0 );

    enum SeriesRange
    {
        Earlier= 0x1,
        Selected= 0x02,
        Later= 0x04,

        All= Earlier| Selected | Later,
        Cancel = 0x0,
        RetainSelected = Earlier | Later,
        NotEarlier = Selected | Later,
        NotLater = Earlier | Selected,
    };

    int exec(bool editmode);
    int result() const;
};


#endif
