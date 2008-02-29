/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#include <qtopianamespace.h>

#include <QDebug>
#include <QFile>
#include <QKeyEvent>
#include <QLayout>
#include <QProcess>
#include <QScrollBar>
#include <QTextBrowser>
#include <QTelephonyConfiguration>
#include <QTimer>
#include "modeminfo.h"

ModemInfo::ModemInfo( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    QTimer::singleShot(60, this, SLOT(init()));
}

ModemInfo::~ModemInfo()
{
}

void ModemInfo::init()
{
    infoDisplay = new QTextBrowser();
    infoDisplay->installEventFilter( this );
    infoDisplay->setFrameShape( QFrame::NoFrame );
    //infoDisplay->setFocusPolicy( Qt::NoFocus );
    QVBoxLayout *vb = new QVBoxLayout( this );
    vb->setSpacing( 0 );
    vb->setMargin( 0 );
    vb->addWidget( infoDisplay );

    infoDisplay->setHtml( format() );

    QTelephonyConfiguration *config
            = new QTelephonyConfiguration( "modem" );   // No tr
    connect( config, SIGNAL(notification(QString,QString)),
             this, SLOT(configValue(QString,QString)) );
    config->request( "manufacturer" );      // No tr
    config->request( "model" );             // No tr
    config->request( "revision" );          // No tr
    config->request( "serial" );            // No tr
    config->request( "extraVersion" );      // No tr
}

bool ModemInfo::eventFilter( QObject* /*watched*/, QEvent *event )
{
    if ( event->type() == QEvent::KeyPress )
    {
        QScrollBar* sb = infoDisplay->verticalScrollBar();
        int key = ((QKeyEvent*)event)->key();
        if ( key == Qt::Key_Down )
            sb->triggerAction( QAbstractSlider::SliderSingleStepAdd );
        else if ( key == Qt::Key_Up )
            sb->triggerAction( QAbstractSlider::SliderSingleStepSub );
        else
            return false;
        return true;
    }
    return false;
}

void ModemInfo::configValue( const QString& name, const QString& value )
{
    if ( name == "manufacturer" )       // No tr
        manufacturer = value;
    else if ( name == "model" )         // No tr
        model = value;
    else if ( name == "revision" )      // No tr
        revision = value;
    else if ( name == "serial" )        // No tr
        serial = value;
    else if ( name == "extraVersion" )  // No tr
        extraVersion = value;
    infoDisplay->setHtml( format() );
}

QString ModemInfo::format()
{
    QString infoString;

    infoString += tr("Manufacturer:") +
                  " " + Qt::escape( manufacturer ) +
                  "<br/>";

    infoString += tr("Model:") +
                  " " + Qt::escape( model ) +
                  "<br/>";

    infoString += tr("Revision:") +
                  " " + Qt::escape( revision ) +
                  "<br/>";

    infoString += tr("Serial Number:") +
                  " " + Qt::escape( serial ) +
                  "<br/>";

    if ( !extraVersion.isEmpty() ) {
        infoString += Qt::escape( extraVersion ).replace( "\n", "<br/>" );
    }

    return infoString;
}
