/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "findwidget_p.h"

#include <qtopia/categories.h>
#include <qtopia/categoryselect.h>
#include <qtopia/datetimeedit.h>
#include <qtopia/timestring.h>

#include <qcheckbox.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qtoolbutton.h>

FindWidget::FindWidget( const QString &appName, QWidget *parent,
			const char *name )
    : FindWidgetBase( parent, name ),
      mStrApp( appName ),
      mDate( QDate::currentDate() )
{
    setMaximumSize( sizeHint() );
    QArray<int> vl(0);
    cmbCat->setCategories( vl, mStrApp );
    cmbCat->setRemoveCategoryEdit( TRUE );
    cmbCat->setAllCategories( TRUE );
    // hide junk for the moment...
    lblStartDate->hide();
    cmdDate->hide();
    QObject::connect( cmdDate, SIGNAL(valueChanged(const QDate&)), this, SLOT(setDate(const QDate&)) );
    QObject::connect( cmdFind, SIGNAL(clicked()), this, SLOT(slotFindClicked()) );
}

FindWidget::~FindWidget()
{
}

QString FindWidget::findText() const
{
    return txtFind->text();
}

void FindWidget::slotFindClicked()
{
    lblStatus->setText( "" );
    if ( cmdDate->isVisible() )
	emit signalFindClicked( findText(),
				mDate,
				chkCase->isChecked(),
				FALSE,
				cmbCat->currentCategory() );
    else
	emit signalFindClicked( findText(), chkCase->isChecked(),
				FALSE,
				cmbCat->currentCategory() );
}

void FindWidget::setUseDate( bool show )
{
    if ( show ) {
	lblStartDate->show();
	cmdDate->show();
    } else {
	lblStartDate->hide();
	cmdDate->hide();
    }
    //chkBackwards->setDisabled( show );
}

void FindWidget::setDate( const QDate &dt )
{
    mDate = dt;
}

void FindWidget::findDone( int status )
{
    switch ( status ) {
        case 1:
            lblStatus->setText( tr("String Not Found.") );
            break;
        case 2:
            lblStatus->setText( tr("End reached, starting at beginning") );
            break;
        default:
            break;
    }
}

