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
#include "statusbar.h"

#include <qdebug.h>
#include <QMouseEvent>
#include <QLabel>
#include <QHBoxLayout>
#include <QTimer>

class ClickableLabel : public QLabel
{
    Q_OBJECT
public:
    ClickableLabel() : QLabel() {}
    virtual ~ClickableLabel() {}

signals:
    void clicked();

private:
    void mousePressEvent( QMouseEvent *e )
    {
        if ( e->button() == Qt::LeftButton )
            emit clicked();
        QLabel::mousePressEvent( e );
    }
};

// ====================================================================

StatusBar::StatusBar( QWidget *parent )
    : QFrame( parent )
{
    setFrameShadow( Raised );
    setFrameShape( StyledPanel );
    layout = new QHBoxLayout( this );
    layout->setMargin( 0 );
    layout->setSpacing( 0 );
    statusLabel = new ClickableLabel;
    connect( statusLabel, SIGNAL(clicked()), this, SIGNAL(clicked()) );
    addWidget( statusLabel, 1 );
    statusClearer = new QTimer( this );
    statusClearer->setSingleShot( true );
    connect( statusClearer, SIGNAL(timeout()), statusLabel, SLOT(clear()) );
}

StatusBar::~StatusBar()
{
}

void StatusBar::addWidget( QWidget *widget, int stretch, bool /*permanent*/ )
{
    QFrame *holder = new QFrame;
    holder->setFrameShadow( Sunken );
    holder->setFrameShape( StyledPanel );
    QHBoxLayout *hbox = new QHBoxLayout( holder );
    hbox->setMargin( 0 );
    hbox->setSpacing( 0 );
    hbox->addWidget( widget );
    widgets[widget] = holder;
    layout->addWidget( holder, stretch );
}

void StatusBar::removeWidget( QWidget *widget )
{
    QFrame *holder = widgets[widget];
    widgets.remove( widget );
    layout->removeWidget( holder );
    widget->setParent( 0 );
    delete holder;
}

void StatusBar::showMessage( const QString &message, int timeout )
{
    if ( statusClearer->isActive() )
        statusClearer->stop();
    statusLabel->setText( message );
    if ( timeout > 0 )
        statusClearer->start( timeout );
}

#include "statusbar.moc"
