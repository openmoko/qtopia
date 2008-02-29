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

#include "findwidget_p.h"

#include <qcategorymanager.h>
#include <qcategoryselector.h>
#include <qtimestring.h>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QToolButton>

FindWidget::FindWidget( const QString &appName, QWidget *parent )
    : QWidget( parent ),
      mStrApp( appName ),
      mDate( QDate::currentDate() )
{
    setupUi( this );
    setMaximumSize( sizeHint() );
    cmbCat->selectFilter( QCategoryFilter(QCategoryFilter::All) );
    // hide junk for the moment...
    lblStartDate->hide();
    cmdDate->hide();
    QObject::connect( cmdDate, SIGNAL(valueChanged(const QDate&)),
                      this, SLOT(setDate(const QDate&)) );

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
                                false,
                                cmbCat->selectedFilter() );
    else
        emit signalFindClicked( findText(), chkCase->isChecked(),
                                false,
                                cmbCat->selectedFilter() );
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

void FindWidget::slotNotFound()
{
    lblStatus->setText( tr("String Not Found.") );
}

void FindWidget::slotWrapAround()
{
    lblStatus->setText( tr("End reached, starting at beginning") );
}
