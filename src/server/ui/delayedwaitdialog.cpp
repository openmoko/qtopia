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

#include "delayedwaitdialog.h"

#include <QLabel>
#include <QTimer>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPainter>
#include <QApplication>
#include <QDesktopWidget>
#include <QSoftMenuBar>
#include <QStyle>
#include <QKeyEvent>

class Icon : public QLabel
{
public:
    Icon( QWidget *parent = 0, Qt::WFlags fl = 0 )
        : QLabel( parent, fl ), mOpacity( 0.0 )
    {
        setPixmap( QPixmap( ":icon/play" ) );
    }
    ~Icon()
    {
    }

    void setOpacity( qreal opacity )
    {
        mOpacity = opacity;
    }

    qreal opacity()
    {
        return mOpacity;
    }

protected:
    virtual void paintEvent( QPaintEvent *pe )
    {
        Q_UNUSED( pe );
        QPainter painter( this );
        painter.setOpacity( mOpacity );
        painter.drawPixmap( 0, 0, *pixmap() );
    }
private:
    qreal mOpacity;
};

DelayedWaitDialog::DelayedWaitDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl | Qt::FramelessWindowHint ), mDelay(0), mTid(0)
{
    QVBoxLayout *vl = new QVBoxLayout( this );
    vl->setMargin( 0 );

    QHBoxLayout *hl = new QHBoxLayout;
    text = new QLabel( this );
    hl->addStretch();
    hl->addWidget( text );
    hl->addStretch();
    vl->addLayout( hl );

    hl = new QHBoxLayout;
    hl->setMargin( 0 );
    hl->addStretch();
    Icon *icon = 0;
    for ( int i = 0; i < NUMBEROFICON; ++i ) {
        icon = new Icon( this );
        hl->addWidget( icon );
        mIconList.append( icon );
    }
    hl->addStretch();
    vl->addLayout( hl, 1 );

    QRect d = QApplication::desktop()->screenGeometry();
    int dw = d.width();
    int dh = d.height();
    setGeometry(0, 70*dh/100, dw, QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));

    mTimer = new QTimer( this );
    // two seconds to complete each round
    mTimer->setInterval( 2000 / NUMBEROFICON );
    connect( mTimer, SIGNAL(timeout()), this, SLOT(update()) );
    QSoftMenuBar::setLabel( this, QSoftMenuBar::Back, QSoftMenuBar::NoLabel );
}

DelayedWaitDialog::~DelayedWaitDialog()
{
}

void DelayedWaitDialog::setText( const QString &str )
{
    text->setText( str );
}

/*!
    The dialog will appear after \a ms milliseconds delay.
*/
void DelayedWaitDialog::setDelay( int ms )
{
    mDelay = ms;
}

void DelayedWaitDialog::show()
{
    if ( mDelay )
        mTid = startTimer( mDelay );
    else
        QDialog::show();
}

void DelayedWaitDialog::hide()
{
    if ( mTid )
        killTimer( mTid );
    mTid = 0;
    QDialog::hide();
}

void DelayedWaitDialog::showEvent( QShowEvent *se )
{
    QDialog::showEvent( se );
    mTimer->start();
}

void DelayedWaitDialog::hideEvent( QHideEvent *he )
{
    QDialog::hideEvent( he );
    mTimer->stop();
}

void DelayedWaitDialog::keyReleaseEvent( QKeyEvent *ke )
{
    if ( ke->key() == Qt::Key_Hangup
            || ke->key() == Qt::Key_Call )
        QDialog::keyReleaseEvent( ke );
    else
        ke->ignore();
}

void DelayedWaitDialog::timerEvent( QTimerEvent *te )
{
    // display wait widget, new command might take time to arrive
    if ( te->timerId() == mTid ) {
        QDialog::show();
        killTimer( mTid );
        mTid = 0;
    }
}

void DelayedWaitDialog::update()
{
    int static highlightIndex = 0;
    int stepsToHighlightIndex = 0;
    for( int i = 0; i < NUMBEROFICON; ++i ) {
        stepsToHighlightIndex = i < highlightIndex ?
            highlightIndex - i : NUMBEROFICON - i + highlightIndex;
        mIconList[i]->setOpacity( 1.0 - ( 1.0 / NUMBEROFICON * stepsToHighlightIndex ) );
        mIconList[i]->repaint();
    }
    highlightIndex++;
    if ( highlightIndex == NUMBEROFICON )
        highlightIndex = 0;
}

