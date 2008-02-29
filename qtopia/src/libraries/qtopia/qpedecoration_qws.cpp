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
#ifdef QWS
#define QTOPIA_INTERNAL_LANGLIST
#include <qapplication.h>
#include <qstyle.h>
#include <qwidget.h>
#include <qpainter.h>
#include <qtimer.h>
#include <qwhatsthis.h>
#include <qpopupmenu.h>
#include "qcopenvelope_qws.h"
#include "qpedecoration_p.h"
#include <qdialog.h>
#include <qdrawutil.h>
#include <qgfx_qws.h>
#include "qpeapplication.h"
#include "resource.h"
#include "global.h"
#include "qlibrary.h"
#include "windowdecorationinterface.h"
#include "pluginloader_p.h"
#include <qfile.h>
#include <qfileinfo.h>
#include <qsignal.h>

#include <stdlib.h>

#ifdef QTOPIA_PHONE
#include "phonedecoration_p.h"
#endif

extern Q_EXPORT QRect qt_maxWindowRect;

#define WHATSTHIS_MODE

#ifndef QT_NO_QWS_QPE_WM_STYLE

// Embed required XPM's see $QPEDIR/pics/qpe/ for these XPM's
#include <qpe_close.xpm>
#include <qpe_accept.xpm>

class DecorHackWidget : public QWidget
{
public:
    bool needsOk() {
	return (getWState() & WState_Reserved1 ) ||
		 (inherits("QDialog") && !inherits("QMessageBox") 
		  && !inherits("QWizard") );
    }
};

static QImage scaleButton( const QImage &img, int height )
{
    if ( img.height() != 0 && img.height() != height ) {
	return img.smoothScale( img.width()*height/img.height(), height );
    } else {
	return img;
    }
}

class TLWidget : public QWidget
{
public:
    QWSManager *manager()
    {
	return topData()->qwsManager;
    }

    QTLWExtra *topExtra()
    {
	return topData();
    }

    void setWState( uint s ) { QWidget::setWState( s ); }
    void clearWState( uint s ) { QWidget::clearWState( s ); }
};


QPEManager::QPEManager( QPEDecoration *d, QObject *parent )
    : QObject( parent ), decoration( d ), helpState(0), inWhatsThis(FALSE)
{
    wtTimer = new QTimer( this );
    connect( wtTimer, SIGNAL(timeout()), this, SLOT(whatsThisTimeout()) );
}


void QPEManager::updateActive()
{
    QWidget *newActive = qApp->activeWindow();
    if ( newActive && (QWidget*)active == newActive )
	return;

    if ( active && (!newActive || ((TLWidget *)newActive)->manager()) ) {
	((TLWidget *)(QWidget*)active)->manager()->removeEventFilter( this );
    }

    if ( newActive && ((TLWidget *)newActive)->manager() ) {
	active = newActive;
	((TLWidget *)(QWidget*)active)->manager()->installEventFilter( this );
    } else if ( !newActive ) {
	active = 0;
    }
}

int QPEManager::pointInQpeRegion( QWidget *w, const QPoint &p )
{
    QRect rect(w->geometry());

    if ( decoration->region( w, rect,
		(QWSDecoration::Region)QPEDecoration::Help ).contains(p) )
	return QPEDecoration::Help;

    for (int i = QWSDecoration::LastRegion; i >= QWSDecoration::Title; i--) {
	if (decoration->region(w, rect, (QWSDecoration::Region)i).contains(p))
	    return (QWSDecoration::Region)i;
    }

    return QWSDecoration::None;
}

bool QPEManager::eventFilter( QObject *o, QEvent *e )
{
    QWSManager *mgr = (QWSManager *)o;
    QWidget *w = mgr->widget();
    switch ( e->type() ) {
	case QEvent::MouseButtonPress:
	    {
		pressTime = QTime::currentTime();
		QPoint p = ((QMouseEvent*)e)->globalPos();
		int inRegion = pointInQpeRegion( w, p );
#ifdef WHATSTHIS_MODE
		if ( !w->geometry().contains(p) && QWhatsThis::inWhatsThisMode() ) {
		    QString text;
		    switch ( inRegion ) {
			case QWSDecoration::Close:
			    if ( ((DecorHackWidget*)w)->needsOk() )
				text = tr("Click to close this window, discarding changes.");
			    else
				text = tr("Click to close this window.");
			    break;
			case QWSDecoration::Minimize:
			    text = tr("Click to close this window and apply changes.");
			    break;
			case QWSDecoration::Maximize:
			    if ( w->isMaximized() )
				text = tr("Click to make this window movable.");
			    else
				text = tr("Click to make this window use all available screen area.");
			    break;
			default:
			    break;
		    }
		    QWhatsThis::leaveWhatsThisMode( text );
		    whatsThisTimeout();
		    helpState = 0;
		    return true;
		}
#endif
		if ( inRegion == QPEDecoration::Help ) {
#ifdef WHATSTHIS_MODE
		    wtTimer->start( 400, TRUE );
#endif
		    helpState = QWSButton::Clicked|QWSButton::MouseOver;
		    drawButton( w, QPEDecoration::Help, helpState );
		    return true;
		}
	    }
	    break;
	case QEvent::MouseButtonRelease:
	    if ( helpState & QWSButton::Clicked ) {
		wtTimer->stop();
		helpState = 0;
		drawButton( w, QPEDecoration::Help, helpState );
		QPoint p = ((QMouseEvent*)e)->globalPos();
		if ( pointInQpeRegion( w, p ) == QPEDecoration::Help ) {
		    decoration->help( w );
		}
		return true;
	    }
	    break;
	case QEvent::MouseMove:
	    if ( helpState & QWSButton::Clicked ) {
		int oldState = helpState;
		QPoint p = ((QMouseEvent*)e)->globalPos();
		if ( pointInQpeRegion( w, p ) == QPEDecoration::Help ) {
		    helpState = QWSButton::Clicked|QWSButton::MouseOver;
		} else {
		    helpState = 0;
		}
		if ( helpState != oldState )
		    drawButton( w, QPEDecoration::Help, helpState );
	    }
	    break;
	default:
	    break;
    }
    return QObject::eventFilter( o, e );
}

void QPEManager::drawButton( QWidget *w, QPEDecoration::QPERegion r, int state )
{
    QPainter painter(w);
    QRegion rgn = ((TLWidget *)w)->topExtra()->decor_allocated_region;
    painter.internalGfx()->setWidgetDeviceRegion( rgn );
    painter.setClipRegion(decoration->region(w, w->rect(),QWSDecoration::All));
    decoration->paintButton( &painter, w, (QWSDecoration::Region)r, state );
}

void QPEManager::drawTitle( QWidget *w )
{
    QPainter painter(w);
    QRegion rgn = ((TLWidget *)w)->topExtra()->decor_allocated_region;
    painter.internalGfx()->setWidgetDeviceRegion( rgn );
    painter.setClipRegion(decoration->region(w, w->rect(),QWSDecoration::All));
    decoration->paint( &painter, w );
    decoration->paintButton(&painter, w, QWSDecoration::Menu, 0);
    decoration->paintButton(&painter, w, QWSDecoration::Close, 0);
    decoration->paintButton(&painter, w, QWSDecoration::Minimize, 0);
    decoration->paintButton(&painter, w, QWSDecoration::Maximize, 0);
}

void QPEManager::whatsThisTimeout()
{
    if ( !QWhatsThis::inWhatsThisMode() ) {
	if ( inWhatsThis ) {
	    if ( whatsThis ) {
		QWidget *w = whatsThis;
		whatsThis = 0;
		drawTitle( w );
	    }
	    wtTimer->stop();
	} else {
	    QWhatsThis::enterWhatsThisMode();
	    helpState = 0;
	    updateActive();
	    if ( active ) {
		whatsThis = active;
		drawTitle( active );
		// check periodically to see if we've left whats this mode
		wtTimer->start( 250 );
	    }
	}
	inWhatsThis = !inWhatsThis;
    }
}

//===========================================================================

static QImage *okImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
	delete i;
	i = new QImage(scaleButton(Resource::loadImage("qpe/pda/OKButton"),th));
    }
    return i;
}

static QImage *closeImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
	delete i;
	i = new QImage(scaleButton(Resource::loadImage("qpe/pda/CloseButton"),th));
    }
    return i;
}

static QImage *helpImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
	delete i;
	i = new QImage(scaleButton(Resource::loadImage("qpe/pda/HelpButton"),th));
    }
    return i;
}

static QImage *maximizeImage( int th )
{
    static QImage *i = 0;
    if ( !i || i->height() != th ) {
	delete i;
	i = new QImage(scaleButton(Resource::loadImage("qpe/pda/MaximizeButton"),th));
    }
    return i;
}

int WindowDecorationInterface::metric( Metric m, const WindowData *wd ) const
{
    switch ( m ) {
	case TitleHeight:
	    if ( QApplication::desktop()->height() > 320 )
		return 19;
	    else
		return 15;
	case LeftBorder:
	case RightBorder:
	case TopBorder:
	case BottomBorder:
	    return 4;
	case OKWidth:
	    return okImage(metric(TitleHeight,wd))->width();
	case CloseWidth:
	    return closeImage(metric(TitleHeight,wd))->width();
	case HelpWidth:
	    return helpImage(metric(TitleHeight,wd))->width();
	case MaximizeWidth:
	    return maximizeImage(metric(TitleHeight,wd))->width();
	case CornerGrabSize:
	    return 16;
    }

    return 0;
}

void WindowDecorationInterface::drawArea( Area a, QPainter *p, const WindowData *wd ) const
{
    int th = metric( TitleHeight, wd );
    QRect r = wd->rect;

    switch ( a ) {
	case Border:
	    {
		const QColorGroup &cg = wd->palette.active();
		qDrawWinPanel(p, r.x()-metric(LeftBorder,wd),
			r.y()-th-metric(TopBorder,wd),
			r.width()+metric(LeftBorder,wd)+metric(RightBorder,wd),
			r.height()+th+metric(TopBorder,wd)+metric(BottomBorder,wd),
			cg, FALSE, &cg.brush(QColorGroup::Background));
	    }
	    break;
	case Title:
	    {
		const QColorGroup &cg = wd->palette.active();
		QBrush titleBrush;
		QPen   titleLines;

		if ( wd->flags & WindowData::Active ) {
		    titleBrush = cg.brush(QColorGroup::Highlight);
		    titleLines = titleBrush.color().dark();
		} else {
		    titleBrush = cg.brush(QColorGroup::Background);
		    titleLines = titleBrush.color();
		}

		p->fillRect( r.x(), r.y()-th, r.width(), th, titleBrush);

		p->setPen( titleLines );
		for ( int i = r.y()-th; i < r.y(); i += 2 )
		    p->drawLine( r.left(), i, r.right(), i );
	    }
	    break;
	case TitleText:
	    p->drawText( r.x()+3+metric(HelpWidth,wd), r.top()-th,
		r.width()-metric(OKWidth,wd)-metric(CloseWidth,wd),
		th, QPainter::AlignVCenter, wd->caption);
	    break;
    }
}

void WindowDecorationInterface::drawButton( Button b, QPainter *p, const WindowData *wd, int x, int y, int, int, QWSButton::State state ) const
{
    QImage *img = 0;
    switch ( b ) {
	case OK:
	    img = okImage(metric(TitleHeight,wd));
	    break;
	case Close:
	    img = closeImage(metric(TitleHeight,wd));
	    break;
	case Help:
	    img = helpImage(metric(TitleHeight,wd));
	    break;
	case Maximize:
	    img = maximizeImage(metric(TitleHeight,wd));
	    break;
    }

    if ( img ) {
	if ((state & QWSButton::MouseOver) && (state & QWSButton::Clicked))
	    p->drawImage(x+2, y+2, *img);
	else
	    p->drawImage(x+1, y+1, *img);
    }
}

QRegion WindowDecorationInterface::mask( const WindowData *wd ) const
{
    int th = metric(TitleHeight,wd);
    QRect rect( wd->rect );
    QRect r(rect.left() - metric(LeftBorder,wd),
	    rect.top() - th - metric(TopBorder,wd),
	    rect.width() + metric(LeftBorder,wd) + metric(RightBorder,wd),
	    rect.height() + th + metric(TopBorder,wd) + metric(BottomBorder,wd));
    return QRegion(r) - rect;
}

class DefaultWindowDecoration : public WindowDecorationInterface
{
public:
    DefaultWindowDecoration() : ref(0) {}
    QString name() const {
	return qApp->translate("WindowDecoration", "Default", 
		"List box text for default window decoration");
    }
    QPixmap icon() const {
	return QPixmap();
    }
    QRESULT queryInterface( const QUuid &uuid, QUnknownInterface **iface ) {
	*iface = 0;
	if ( uuid == IID_QUnknown )
	    *iface = this;
	else if ( uuid == IID_WindowDecoration )
	    *iface = this;

	if ( *iface )
	    (*iface)->addRef();
	return QS_OK;
    }
    Q_REFCOUNT

private:
    ulong ref;
};

static WindowDecorationInterface *wdiface = 0;
static PluginLoaderIntern *wdLoader = 0;

//===========================================================================
bool QPEDecoration::helpExists() const
{
    if ( helpFile.isNull() ) {
	QStringList helpPath = Global::helpPath();
	QFileInfo fi( QString(qApp->argv()[0]) );
	QString hf = fi.baseName() + ".html";
	bool he = FALSE;
	for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end() && !he; ++it)
	    he = QFile::exists( *it + "/" + hf );
	((QPEDecoration*)this)->helpFile = hf;
	((QPEDecoration*)this)->helpexists = he;
	return he;
    }
    return helpexists;
}

QPEDecoration::QPEDecoration()
    : QWSDefaultDecoration()
{
    if ( wdLoader ) {
	delete wdLoader;
	wdLoader = 0;
    } else {
	delete wdiface;
    }
#ifdef QTOPIA_PHONE
    wdiface = new PhoneDecoration;
#else
    wdiface = new DefaultWindowDecoration;
#endif

    helpexists = FALSE; // We don't know (flagged by helpFile being null)
    qpeManager = new QPEManager( this );
    imageOk = Resource::loadImage( "qpe/pda/OKButton" );
    imageClose = Resource::loadImage( "qpe/pda/CloseButton" );
    imageHelp = Resource::loadImage( "qpe/pda/HelpButton" );
}

QPEDecoration::QPEDecoration( const QString &
#ifndef QTOPIA_PHONE
        plugin
#endif
 )   : QWSDefaultDecoration()
{
    if ( wdLoader ) {
	delete wdLoader;
	wdLoader = 0;
    } else {
	delete wdiface;
    }
#ifdef QTOPIA_PHONE
    wdiface = new PhoneDecoration;
#else
    if ( plugin == "Qtopia" ) { // No tr
	wdiface = new DefaultWindowDecoration;
    } else {
	wdLoader = new PluginLoaderIntern( "decorations" ); // No tr
	WindowDecorationInterface *iface = 0;
	if ( !wdLoader->inSafeMode() && wdLoader->isEnabled(plugin) &&
		wdLoader->queryInterface( plugin, IID_WindowDecoration, (QUnknownInterface**)&iface ) == QS_OK && iface ) {
	    wdiface = iface;
	} else {
	    delete wdLoader;
	    wdLoader = 0;
	    wdiface = new DefaultWindowDecoration;
	}
    }
#endif
    helpexists = FALSE; // We don't know (flagged by helpFile being null)
    qpeManager = new QPEManager( this );
}

QPEDecoration::~QPEDecoration()
{
    delete qpeManager;
}

const char **QPEDecoration::menuPixmap()
{
    return (const char **)0;
}

const char **QPEDecoration::closePixmap()
{
    return (const char **)qpe_close_xpm;
}

const char **QPEDecoration::minimizePixmap()
{
    return (const char **)qpe_accept_xpm;
}

const char **QPEDecoration::maximizePixmap()
{
    return (const char **)0;
}

const char **QPEDecoration::normalizePixmap()
{
    return (const char **)0;
}

int QPEDecoration::getTitleHeight( const QWidget *w )
{
    WindowDecorationInterface::WindowData wd;
    windowData( w, wd );
    return wdiface->metric(WindowDecorationInterface::TitleHeight,&wd);
}

/*
    If rect is empty, no frame is added. (a hack, really)
*/
QRegion QPEDecoration::region(const QWidget *widget, const QRect &rect, QWSDecoration::Region type)
{
    qpeManager->updateActive();

    WindowDecorationInterface::WindowData wd;
    windowData( widget, wd );
    wd.rect = rect;

    int titleHeight = wdiface->metric(WindowDecorationInterface::TitleHeight,&wd);
    int okWidth = wdiface->metric(WindowDecorationInterface::OKWidth,&wd);
    int closeWidth = wdiface->metric(WindowDecorationInterface::CloseWidth,&wd);
    int helpWidth = wdiface->metric(WindowDecorationInterface::HelpWidth,&wd);
    int grab = wdiface->metric(WindowDecorationInterface::CornerGrabSize,&wd);

    QRegion region;

    switch ((int)type) {
	case Menu:
#ifdef QTOPIA_PHONE
	    // This is how we stop the mouse from interacting with
	    // windows in phone edition.  QWSManager first checks whether
	    // the mouse is in Menu.  By ensuring the mouse is in
	    // Menu we guarantee that no action is taken on mouse
	    // events.
	    region = wdiface->mask(&wd);
#endif
	    break;
	case Maximize:
	    if ( !widget->inherits( "QDialog" ) && qApp->desktop()->width() > 350 ) {
		int maximizeWidth = wdiface->metric(WindowDecorationInterface::MaximizeWidth,&wd);
		int left = rect.right() - maximizeWidth - closeWidth;
		if ( ((DecorHackWidget *)widget)->needsOk() )
		    left -= okWidth;
		QRect r(left, rect.top() - titleHeight, closeWidth, titleHeight);
		region = r;
	    }
	    break;
	case Minimize:
	    if ( ((DecorHackWidget *)widget)->needsOk() && okWidth > 0) {
		QRect r(rect.right() - okWidth,
		    rect.top() - titleHeight, okWidth, titleHeight);
		if (r.left() > rect.left() + titleHeight)
		    region = r;
	    }
	    break;
	case Close:
	    {
		int left = rect.right() - closeWidth;
		if ( ((DecorHackWidget *)widget)->needsOk() )
		    left -= okWidth;
		QRect r(left, rect.top() - titleHeight, closeWidth, titleHeight);
		if (r.width() > 0)
		    region = r;
	    }
	    break;
    	case Title:
	    if ( !widget->isMaximized() ) {
		int width = rect.width() - helpWidth - closeWidth;
		if ( ((DecorHackWidget *)widget)->needsOk() )
		    width -= okWidth;
		QRect r(rect.left()+helpWidth, rect.top() - titleHeight,
			width, titleHeight);
		if (r.width() > 0)
		    region = r;
	    }
	    break;
	case Help:
	    if ( helpExists() || widget->testWFlags(Qt::WStyle_ContextHelp) ) {
		QRect r(rect.left(), rect.top() - titleHeight,
			  helpWidth, titleHeight);
		if (helpWidth > 0)
		    region = r;
	    }
	    break;
	case Top:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int b = wdiface->metric(WindowDecorationInterface::TopBorder,&wd);
		region = m & QRect( br.left()+grab, br.top(),
				    br.width()-2*grab, b );
	    }
	    break; 
	case Left:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int b = wdiface->metric(WindowDecorationInterface::LeftBorder,&wd);
		region = m & QRect( br.left(), br.top()+grab,
				    b, br.height()-2*grab );
	    }
	    break; 
	case Right:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int b = wdiface->metric(WindowDecorationInterface::RightBorder,&wd);
		region = m & QRect( rect.right(), br.top()+grab,
				    b, br.height()-2*grab );
	    }
	    break; 
	case Bottom:
	    {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int b = wdiface->metric(WindowDecorationInterface::BottomBorder,&wd);
		region = m & QRect( br.left()+grab, rect.bottom(),
				    br.width()-2*grab, b );
	    }
	    break;
	case TopLeft:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int tb = wdiface->metric(WindowDecorationInterface::TopBorder,&wd);
		int lb = wdiface->metric(WindowDecorationInterface::LeftBorder,&wd);
		QRegion crgn( br.left(), br.top(), grab, tb );
		crgn |= QRect( br.left(), br.top(), lb, grab );
		region = m & crgn;
	    }
	    break; 
	case TopRight:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		int tb = wdiface->metric(WindowDecorationInterface::TopBorder,&wd);
		int rb = wdiface->metric(WindowDecorationInterface::RightBorder,&wd);
		QRegion crgn( br.right()-grab, br.top(), grab, tb );
		crgn |= QRect( br.right()-rb, br.top(), rb, grab );
		region = m & crgn;
	    }
	    break;
	case BottomLeft:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		region = m & QRect( br.left(), br.bottom()-grab, grab, grab );
	    }
	    break; 
	case BottomRight:
	    if ( !widget->isMaximized() ) {
		QRegion m = wdiface->mask(&wd);
		QRect br = m.boundingRect();
		region = m & QRect( br.right()-grab, br.bottom()-grab, grab, grab );
	    }
	    break;
	case All:
#ifndef QTOPIA_PHONE
	    if ( widget->isMaximized() ) {
		QRect r(rect.left(), rect.top() - titleHeight,
			rect.width(), rect.height() + titleHeight);
		region = r;
	    } else
#endif
	    {
		region = wdiface->mask(&wd);
	    }
	    region -= rect;
	    break;
	default:
	    region = QWSDefaultDecoration::region(widget, rect, type);
	    break;
    }

    return region;
}

void QPEDecoration::paint(QPainter *painter, const QWidget *widget)
{
    WindowDecorationInterface::WindowData wd;
    windowData( widget, wd );

    int titleWidth = getTitleWidth(widget);
    int titleHeight = wdiface->metric(WindowDecorationInterface::TitleHeight,&wd);

    QRect rect(widget->rect());

    // title bar rect
    QRect tbr( rect.left(), rect.top() - titleHeight, rect.width(), titleHeight );

#ifndef QT_NO_PALETTE
    QRegion oldClip = painter->clipRegion();
    painter->setClipRegion( oldClip - QRegion( tbr ) );	// reduce flicker
    wdiface->drawArea( WindowDecorationInterface::Border, painter, &wd );
    painter->setClipRegion( oldClip );

    if (titleWidth > 0) {
	const QColorGroup &cg = widget->palette().active();
	QBrush titleBrush;
	QPen   titlePen;

	if ( wd.flags & WindowDecorationInterface::WindowData::Active ) {
	    titleBrush = cg.brush(QColorGroup::Highlight);
	    titlePen   = cg.color(QColorGroup::HighlightedText);
	} else {
	    titleBrush = cg.brush(QColorGroup::Background);
	    titlePen   = cg.color(QColorGroup::Text);
	}

	wdiface->drawArea( WindowDecorationInterface::Title, painter, &wd );

	// Draw caption
	painter->setPen(titlePen);
	QFont f( QApplication::font() );
	f.setWeight( QFont::Bold );
	painter->setFont(f);
	wdiface->drawArea( WindowDecorationInterface::TitleText, painter, &wd );
    }
#endif //QT_NO_PALETTE

    paintButton( painter, widget, (QWSDecoration::Region)Help, 0 );
}

void QPEDecoration::paintButton(QPainter *painter, const QWidget *w,
			QWSDecoration::Region type, int state)
{
    WindowDecorationInterface::Button b;
    switch ((int)type) {
	case Close:
	    b = WindowDecorationInterface::Close;
	    break;
	case Minimize:
	    if ( ((DecorHackWidget *)w)->needsOk() )
		b = WindowDecorationInterface::OK;
	    else if ( helpExists() )
		b = WindowDecorationInterface::Help;
	    else
		return;
	    break;
	case Help:
	    b = WindowDecorationInterface::Help;
	    break;
	case Maximize:
	    b = WindowDecorationInterface::Maximize;
	    break;
	default:
	    return;
    }

    WindowDecorationInterface::WindowData wd;
    windowData( w, wd );

    int titleHeight = wdiface->metric(WindowDecorationInterface::TitleHeight,&wd);
    QRect rect(w->rect());
    QRect tbr( rect.left(), rect.top() - titleHeight, rect.width(), titleHeight );
    QRect brect(region(w, w->rect(), type).boundingRect());

    if (brect.width() > 0 && brect.height() > 0) {
	const QColorGroup &cg = w->palette().active();
	if ( wd.flags & WindowDecorationInterface::WindowData::Active )
	    painter->setPen( cg.color(QColorGroup::HighlightedText) );
	else
	    painter->setPen( cg.color(QColorGroup::Text) );

	QRegion oldClip = painter->clipRegion();
	painter->setClipRegion( QRect(brect.x(), tbr.y(), brect.width(), tbr.height()) ); // reduce flicker
	wdiface->drawArea( WindowDecorationInterface::Title, painter, &wd );
	wdiface->drawButton( b, painter, &wd, brect.x(), brect.y(), brect.width(), brect.height(), (QWSButton::State)state );
	painter->setClipRegion( oldClip );
    }
}

//#define QPE_DONT_SHOW_TITLEBAR

void QPEDecoration::maximize( QWidget *widget )
{
#ifdef QPE_DONT_SHOW_TITLEBAR    
    if ( !widget->inherits( "QDialog" ) ) {
	widget->setGeometry( qt_maxWindowRect );
    } else 
#endif	
    {
#ifndef QTOPIA_PHONE
	QWSDecoration::maximize( widget );
#else
	// find out how much space the decoration needs
	WindowDecorationInterface::WindowData wd;
	windowData(widget, wd);

	int th = wdiface->metric(WindowDecorationInterface::TitleHeight, &wd);

	QRect nr = qt_maxWindowRect;
	nr.setTop(nr.top()+th);
	widget->setGeometry(nr);
#endif
    }
}

QPopupMenu *QPEDecoration::menu( const QWidget *, const QPoint & )
{
    QPopupMenu *m = new QPopupMenu();
#ifndef QTOPIA_PHONE
    m->insertItem(QPEManager::tr("Restore"), (int)Normalize);
    m->insertItem(QPEManager::tr("Move"), (int)Title);
    m->insertItem(QPEManager::tr("Size"), (int)BottomRight);
    m->insertItem(QPEManager::tr("Maximize"), (int)Maximize);
    m->insertSeparator();
#endif
    m->insertItem(QPEManager::tr("Close"), (int)Close);

    return m;
}

#ifndef QT_NO_DIALOG
class HackDialog : public QDialog
{
public:
    void acceptIt() {
	if ( isA( "QMessageBox" ) )
	    qApp->postEvent( this, new QKeyEvent( QEvent::KeyPress, Key_Enter, '\n', 0, "\n" ) );
	else
	    accept();
    }
};
#endif


void QPEDecoration::minimize( QWidget *widget )
{
#ifndef QT_NO_DIALOG
    // We use the minimize button as an "accept" button.
    if ( widget->inherits( "QDialog" ) ) {
        HackDialog *d = (HackDialog *)widget;
        d->acceptIt();
    } 
#endif
    else if ( ((DecorHackWidget *)widget)->needsOk() ) {
	QSignal s;
	s.connect( widget, SLOT( accept() ) );
	s.activate();
    } else {
	help( widget );
    }
}

void QPEDecoration::help( QWidget *w )
{
    if ( helpExists() ) {
	QString hf = helpFile;
	QFileInfo fi( QString(qApp->argv()[0]) );
	QString localHelpFile = fi.baseName() + "-" + w->name() + ".html";
	QStringList helpPath = Global::helpPath();
	for (QStringList::ConstIterator it=helpPath.begin(); it!=helpPath.end(); ++it) {
	    if ( QFile::exists( *it + "/" + localHelpFile ) ) {
		hf = localHelpFile;
		break;
	    }
	}
	Global::execute( "helpbrowser", hf );
    } else if ( w && w->testWFlags(Qt::WStyle_ContextHelp) ) {
	QWhatsThis::enterWhatsThisMode();
	QWhatsThis::leaveWhatsThisMode( qApp->translate("QPEDecoration",
	    "<Qt>Comprehensive help is not available for this application, "
	    "however there is context-sensitive help.<p>To use context-sensitive help:<p>"
	    "<ol><li>click and hold the help button."
	    "<li>when the title bar shows <b>What's this...</b>, "
	    "click on any control.</ol></Qt>" ) );
    }
}

void QPEDecoration::windowData( const QWidget *w, WindowDecorationInterface::WindowData &wd ) const
{
    wd.rect = w->rect();
    if ( qpeManager->whatsThisWidget() == w )
	wd.caption = qApp->translate("QPEDecoration","What's this..." );
    else
	wd.caption = w->caption();
    wd.palette = qApp->palette();
    wd.flags = 0;
    wd.flags |= w->isMaximized() ? WindowDecorationInterface::WindowData::Maximized : 0;
    wd.flags |= w->testWFlags(Qt::WStyle_Dialog) ? WindowDecorationInterface::WindowData::Dialog : 0;
    const QWidget *active = qpeManager->activeWidget();
    wd.flags |= w == active ? WindowDecorationInterface::WindowData::Active : 0;
    wd.reserved = 1;
}

/*
#ifndef QT_NO_POPUPMENU
QPopupMenu *QPEDecoration::menu(QWSManager*, const QWidget*, const QPoint&)
{
    return 0;
}
#endif
*/




#endif // QT_NO_QWS_QPE_WM_STYLE
#endif
