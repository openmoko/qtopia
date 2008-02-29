/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
    QObject::connect( cmdDate, SIGNAL(valueChanged(const QDate &)),
		      this, SLOT(setDate(const QDate &)) );

    QObject::connect( cmdFind, SIGNAL(clicked()),
		      this, SLOT(slotFindClicked()) );
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
				chkBackwards->isChecked(),
				cmbCat->currentCategory() );
    else
	emit signalFindClicked( findText(), chkCase->isChecked(),
				chkBackwards->isChecked(),
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
    chkBackwards->setDisabled( show );
}

void FindWidget::setDate( const QDate &dt )
{
    mDate = dt;
}

void FindWidget::slotNotFound()
{
    lblStatus->setText( tr("String Not Found.") );
}

void FindWidget::slotWrapAround()
{
    lblStatus->setText( tr("End reached, starting at beginning") );
}
