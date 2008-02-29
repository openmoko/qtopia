/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "qprintdialogcreator_p.h"
#include "ui_printdialogbase.h"

#include <QDebug>

/*!
    \internal
    Creates a print dialog.
*/
static void createPrintDialog(QPrintDialog *parent)
{
    Ui::PrintDialogBase *dialogBase = new Ui::PrintDialogBase();
    dialogBase->setupUi(parent);
}

typedef void (*QPrintDialogCreator)(QPrintDialog *parent);
extern QPrintDialogCreator _qt_print_dialog_creator;
/*!
    \internal
    _qt_print_dialog_creator is defined in QPrintDialog
    and called when a QPrintDialog is contructed to ensure
    that graphic representation of QPrintDialog fits to handheld devices.
*/
static int installCreatePrintDialog()
{
    _qt_print_dialog_creator = &createPrintDialog;
    return 0;
}

Q_CONSTRUCTOR_FUNCTION(installCreatePrintDialog)
