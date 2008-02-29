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

#include <QFrame>
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPointer>

#include "qactionconfirm_p.h"

class QActionConfirmDialog : public QFrame
{
public:
    QActionConfirmDialog( QWidget *parent = 0 );

    void show( const QPixmap &pix, const QString &text );
    void close();

protected:
    void timerEvent( QTimerEvent *e );
    void paintEvent( QPaintEvent *e );

private:
    QLabel *m_pixLabel, *m_textLabel;
    int m_timerId;
};

int mgn = 2;
QActionConfirmDialog::QActionConfirmDialog( QWidget *parent )
    : QFrame( parent, (Qt::Tool|Qt::FramelessWindowHint)), m_timerId( 0 )
{
    if ( !this->parent() && qApp )
        setParent(QApplication::desktop()->screen(QApplication::desktop()->primaryScreen()));
    setFrameStyle(QFrame::WinPanel|QFrame::Raised);
    QHBoxLayout *l = new QHBoxLayout( this );
    l->setMargin( mgn );
    m_pixLabel = new QLabel( this );
    m_pixLabel->setAlignment( Qt::AlignCenter  );
    l->addWidget( m_pixLabel );
    m_textLabel = new QLabel( this );
    m_textLabel->setAlignment( Qt::AlignCenter );
    m_textLabel->setWordWrap( true );
    l->addWidget( m_textLabel );
}

void QActionConfirmDialog::show( const QPixmap &pix, const QString &text )
{
    if( m_timerId != 0 )
    {
        killTimer( m_timerId );
        m_timerId = 0;
    }
    m_pixLabel->setPixmap( pix );
    m_textLabel->setText( text );


    int w = m_pixLabel->sizeHint().width() + m_textLabel->sizeHint().width()
            + (mgn*2);
    int h = qMax(m_pixLabel->sizeHint().height(),
                m_textLabel->heightForWidth(w)) + (mgn*2);

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect(desktop->screenGeometry(desktop->screenNumber(this)));
    int x = desktopRect.width() / 2 - w / 2;
    int y = desktopRect.height() / 2 - h /2;

    setGeometry( x, y, w, h );
    m_timerId = startTimer( 1500 );
    QFrame::show();
    activateWindow();
}

void QActionConfirmDialog::close()
{
    killTimer( m_timerId );
    m_timerId = 0;
    QWidget::close();
}

void QActionConfirmDialog::timerEvent( QTimerEvent *e )
{
    if( m_timerId && e->timerId() == m_timerId )
        close();
}

void QActionConfirmDialog::paintEvent( QPaintEvent *e )
{
    QFrame::paintEvent( e );
    QPainter p( this );
    p.setPen( palette().text().color() );
    p.drawRect( 0, 0, width(), height() );
}

class QActionConfirmPrivate
{
public:
    static QPointer<QActionConfirmDialog> m_dialog;
};

QPointer<QActionConfirmDialog> QActionConfirmPrivate::m_dialog = 0;

void QActionConfirm::display( const QPixmap &pix, const QString &text )
{
    if (!QActionConfirmPrivate::m_dialog)
        QActionConfirmPrivate::m_dialog = new QActionConfirmDialog;
    QActionConfirmPrivate::m_dialog->show( pix, text );
}
