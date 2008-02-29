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

#include "samplewindow.h"
#include <QFontDatabase>
#include <QSettings>
#include <qtopiaapplication.h>
#include <qpluginmanager.h>
#include <qwindowdecorationinterface.h>
#include <qtopiaipcenvelope.h>
#ifdef QTOPIA_PHONE
# include <qtopia/private/phonedecoration_p.h>
# include <qtopia/qsoftmenubar.h>
# include <qtopia/phonestyle.h>
#else
# include <qtopia/qtopiastyle.h>
#endif

#include <QLabel>
#include <QCheckBox>
#include <QRadioButton>
#include <QTabWidget>
#include <QSlider>
#include <QFile>
#include <QTextStream>
#include <QDataStream>
#include <QMessageBox>
#include <QComboBox>
#include <QListWidget>
#include <QDir>
#include <QGroupBox>
#include <QScrollBar>
#include <QWindowsStyle>
#include <QLayout>
#include <QMenuBar>
#include <QPushButton>
#include <QWhatsThis>
#include <QPixmapCache>
#include <QStyleFactory>
#include <QPainter>
#include <QDesktopWidget>

#include <stdlib.h>

#define SAMPLE_HEIGHT   115

class SampleText : public QWidget
{
public:
    SampleText( const QString &t, bool h, QWidget *parent )
        : QWidget( parent ), hl(h), text(t)
    {
    }

    QSize sizeHint() const
    {
        QFontMetrics fm(font());
        return QSize( fm.width(text)+10, fm.height()+4 );
    }

    void paintEvent( QPaintEvent * )
    {
        QPainter p(this);
        if ( hl ) {
            p.setPen( palette().color(QPalette::HighlightedText) );
            p.fillRect( rect(), QBrush( QColor( palette().color( QPalette::Highlight) ) ) );
        } else
            p.setPen( palette().color(QPalette::Text) );
        p.drawText( rect(), Qt::AlignLeft | Qt::AlignVCenter, text );
    }

private:
    bool hl;
    QString text;
};

//-------------------------------------------------------------------------

SampleWindow::SampleWindow( QWidget *parent )
    : QWidget(parent), iface(0), popup(0), mb(0)
{
    setSizePolicy( QSizePolicy( QSizePolicy::Preferred, QSizePolicy::Minimum ) );
    init();
}

QSize SampleWindow::sizeHint() const
{
    int w = qMin( container->sizeHint().width() + 10, desktopSize.width()-10 );
    int h = container->sizeHint().height() + 30;
    return QSize( w, qMax(SAMPLE_HEIGHT,h) );
}

void SampleWindow::setFont( const QFont &f )
{
    QWidget::setFont( f );
    if (popup)
        popup->setFont( f );
}

void SampleWindow::setDecoration( QWindowDecorationInterface *i )
{
    iface = i;
    wd.rect = QRect( 0, 0, 150, 75 );
    wd.caption = tr("Sample");
    wd.palette = palette();
    wd.flags = QWindowDecorationInterface::WindowData::Dialog
                | QWindowDecorationInterface::WindowData::Active;
    wd.reserved = 1;

    th = iface->metric(QWindowDecorationInterface::TitleHeight, &wd);
    tb = iface->metric(QWindowDecorationInterface::TopBorder, &wd);
    lb = iface->metric(QWindowDecorationInterface::LeftBorder, &wd);
    rb = iface->metric(QWindowDecorationInterface::RightBorder, &wd);
    bb = iface->metric(QWindowDecorationInterface::BottomBorder, &wd);

    int yoff = th + tb;
    int xoff = lb;

    wd.rect.setX( 0 );
    wd.rect.setWidth( width() - lb - rb );
    wd.rect.setY( 0 );
    wd.rect.setHeight( height() - yoff - bb );

    container->setGeometry( QRect( xoff, yoff, wd.rect.width(), wd.rect.height() ) );
    setMinimumSize( qMin(container->sizeHint().width()+lb+rb, desktopSize.width()-10),
                    qMax(SAMPLE_HEIGHT,container->sizeHint().height()+tb+th+bb) );
}

void SampleWindow::paintEvent( QPaintEvent * )
{
    if ( !iface )
        return;

    QPainter p( this );

    p.translate( lb, th+tb );

    iface->drawArea(QWindowDecorationInterface::Border, &p, &wd);
    iface->drawArea(QWindowDecorationInterface::Title, &p, &wd);

    p.setPen(palette().color(QPalette::HighlightedText));
    QFont f( font() );
    f.setWeight( QFont::Bold );
    p.setFont(f);
    iface->drawArea(QWindowDecorationInterface::TitleText, &p, &wd);

    QRect brect( 0, -th, iface->metric(QWindowDecorationInterface::HelpWidth,&wd), th );
    iface->drawButton( QWindowDecorationInterface::Help, &p, &wd,
        brect.x(), brect.y(), brect.width(), brect.height(), (QDecoration::DecorationState)0 );
    brect.translate( wd.rect.width() -
        iface->metric(QWindowDecorationInterface::OKWidth,&wd) -
        iface->metric(QWindowDecorationInterface::CloseWidth,&wd), 0 );
    iface->drawButton( QWindowDecorationInterface::Close, &p, &wd,
        brect.x(), brect.y(), brect.width(), brect.height(), (QDecoration::DecorationState)0 );
    brect.translate( iface->metric(QWindowDecorationInterface::CloseWidth,&wd), 0 );
    iface->drawButton( QWindowDecorationInterface::OK, &p, &wd,
        brect.x(), brect.y(), brect.width(), brect.height(), (QDecoration::DecorationState)0 );
}

void SampleWindow::init()
{
    QDesktopWidget *desktop = QApplication::desktop();
    desktopSize = desktop->availableGeometry(desktop->primaryScreen()).size();
    int dheight = desktopSize.height();
    int dwidth = desktopSize.width();
    bool wide = ( dheight < 300 && dwidth > dheight );

    container = new QWidget( this );
    QVBoxLayout *vbLayout = new QVBoxLayout;

    popup = new QMenu( tr("Menu") );
    popup->addAction( tr("Normal Item") );
    QAction *action = popup->addAction( tr("Disabled Item") );
    action->setEnabled(false);
    mb = new QMenuBar();
    mb->addMenu( popup );
    vbLayout->setMenuBar( mb );

    QWidget *hb = new QWidget( this );
    QHBoxLayout *hbLayout = new QHBoxLayout;

    QWidget *w = new QWidget();

    int m = 4;
    if ( desktopSize.width() < 200 )
        m = 2;

    QGridLayout *gl = new QGridLayout( w );
    gl->setMargin(m);

    SampleText *t1 = new SampleText( tr("Normal Text"), false, w );

    SampleText *t2 = new SampleText( tr("Highlighted Text"), true, w );

    QPushButton *pb = new QPushButton( tr("Button"), w );
    pb->setFocusPolicy( Qt::NoFocus );

    QCheckBox *cb = new QCheckBox( tr("Check Box"), w );
    cb->setFocusPolicy( Qt::NoFocus );
    cb->setChecked( true );

    if ( wide ) {
        gl->addWidget( t1, 0, 0 );
        gl->addWidget( t2, 1, 0 );
        gl->addWidget( pb, 2, 0 );
        gl->addWidget( cb, 3, 0 );
    } else {
        gl->addWidget( t1, 0, 0 );
        gl->addWidget( t2, 1, 0 );
        gl->addWidget( pb, 0, 1 );
        gl->addWidget( cb, 1, 1 );
    }

    hbLayout->addWidget(w);
    hbLayout->addWidget( new QScrollBar() );

    hb->setLayout( hbLayout );

    vbLayout->addWidget( hb );
    container->setLayout( vbLayout );

    setWhatsThis( tr("Sample window using the selected settings.") );
}

bool SampleWindow::eventFilter( QObject *, QEvent *e )
{
    switch ( e->type() ) {
        case QEvent::MouseButtonPress:
        case QEvent::MouseButtonRelease:
        case QEvent::MouseButtonDblClick:
        case QEvent::MouseMove:
        case QEvent::KeyPress:
        case QEvent::KeyRelease:
            return true;
        default:
            break;
    }

    return false;
}

void SampleWindow::changeEvent( QEvent *e )
{
    QWidget::changeEvent(e);
    if (e->type() == QEvent::PaletteChange) {
        wd.palette = palette();
        if (mb)
            mb->update();
        if (popup)
            popup->setPalette(palette());
    }
}

void SampleWindow::resizeEvent( QResizeEvent *re )
{
    wd.rect.setX( 0 );
    wd.rect.setWidth( width() - lb - rb );
    wd.rect.setY( 0 );
    wd.rect.setHeight( height() - th - tb - bb );

    container->setGeometry( QRect( lb, th+tb, wd.rect.width(), wd.rect.height() ) );
    QWidget::resizeEvent( re );
}

void SampleWindow::setUpdatesEnabled( bool e )
{
    QWidget::setUpdatesEnabled( e );
    QObjectList ol = children();
    QObjectList::ConstIterator it;
    for ( it = ol.begin(); it != ol.end(); ++it ) {
        QObject *o = *it;
        if( o->isWidgetType() ) {
            ((QWidget *)o)->setUpdatesEnabled( e );
        }
    }
}

void SampleWindow::fixGeometry()
{
    int w = qMin(container->sizeHint().width()+lb+rb, desktopSize.width()-10);
    int h = container->sizeHint().height()+tb+th+bb;
    setMinimumSize( w, qMax(SAMPLE_HEIGHT,h) );
}

