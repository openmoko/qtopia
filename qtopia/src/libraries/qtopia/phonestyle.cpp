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

#include <qtopia/qpeapplication.h>
#include <qtopia/custom.h>
#include <qpushbutton.h>
#include <qtoolbutton.h>
#include <qpainter.h>
#include <qfontmetrics.h>
#include <qcombobox.h>
#include <qpalette.h>
#include <qdrawutil.h>
#include <qscrollbar.h>
#include <qbutton.h>
#include <qframe.h>
#include <qfile.h>
#include <qtabbar.h>
#include <qspinbox.h>
#include <qlistbox.h>
#include <qlistview.h>
#include <qlineedit.h>
#include <qmap.h>
#include <qfocusdata.h>

#define INCLUDE_MENUITEM_DEF
#include <qmenudata.h>
#include <qpopupmenu.h>

#include "phonestyle.h"
#include "themedview_p.h"
#include "bgexport_p.h"

#define QCOORDARRLEN(x) sizeof(x)/(sizeof(QCOORD)*2)

extern bool mousePreferred; // can't call Global::mousePreferred in libqtopia2 from libqtopia

class SpinBoxHack : public QSpinBox
{
public:
    void setFlatButtons( bool f ) {
	upButton()->setFlat( f );
	downButton()->setFlat( f );
    }

    void rearrange() {
	upButton()->hide();
	downButton()->hide();
	editor()->setGeometry( frameWidth(), frameWidth(),
		width() - 2*frameWidth(), height() - 2*frameWidth() );
    }
};

class MarginHack : public QFrame
{
public:
    void apply() {
	QWidget *w = focusData()->next();
	for (int i = 0; i < focusData()->count() && w; i++) {
	    if ( w != this && (w->focusPolicy() & QWidget::TabFocus) == QWidget::TabFocus &&
		    !w->focusProxy() && w->isEnabled() ) {
		QWidget *hw = w;
		while (hw && !hw->isTopLevel()) {
		    if (hw->isHidden())
			break;
		    hw = hw->parentWidget();
		}
		if (hw->isTopLevel()) {
		    setMargin(2);
		    return;
		}
	    }
	    w = focusData()->next();
	}
	setMargin(0);
    }

    void restore() {
	setMargin(0);
    }
};

class ManagerAccessor : public QWidget
{
public:
    QWSManager *manager() {
	return topData()->qwsManager;
    }
};

class PhoneStylePrivate : public QObject
{
    Q_OBJECT
public:
    PhoneStylePrivate() : QObject()
    {
	theme_hscrollbar = 0;
	theme_vscrollbar = 0;
	bgExport = new BgExport(this);
    }

    ~PhoneStylePrivate()
    {
	delete theme_hscrollbar;
	delete theme_vscrollbar;
    }

    void setTheme(const QString& file)
    {
	delete theme_hscrollbar;
	delete theme_vscrollbar;
	if ( file.isEmpty() ) {
	    theme_hscrollbar = 0;
	    theme_vscrollbar = 0;
	    return;
	}
        QString fn;
	fn = file + "-hscrollbar.xml";
	if ( QFile::exists(fn) ) {
	    theme_hscrollbar = new ThemedView();
	    theme_hscrollbar->setSource(fn);
	    theme_hscrollbar_add = theme_hscrollbar->findItem("add",ThemedView::Item);
	    theme_hscrollbar_sub = theme_hscrollbar->findItem("sub",ThemedView::Item);
	    theme_hscrollbar_slider = theme_hscrollbar->findItem("slider",ThemedView::Item);
	    theme_hscrollbar_groove = theme_hscrollbar->findItem("groove",ThemedView::Item);
	    if ( !theme_hscrollbar_slider || !theme_hscrollbar_groove ) {
		setTheme(QString::null);
		return;
	    }
	    // Ensure extents are available
	    theme_hscrollbar->setGeometryAndLayout(0,0,32,theme_hscrollbar_groove->geometryHint().height());
	} else {
	    theme_hscrollbar = 0;
	}
	fn = file + "-vscrollbar.xml";
	if ( QFile::exists(fn) ) {
	    theme_vscrollbar = new ThemedView();
	    theme_vscrollbar->setSource(fn);
	    theme_vscrollbar_add = theme_vscrollbar->findItem("add",ThemedView::Item);
	    theme_vscrollbar_sub = theme_vscrollbar->findItem("sub",ThemedView::Item);
	    theme_vscrollbar_slider = theme_vscrollbar->findItem("slider",ThemedView::Item);
	    theme_vscrollbar_groove = theme_vscrollbar->findItem("groove",ThemedView::Item);
	    if ( !theme_vscrollbar_slider || !theme_vscrollbar_groove ) {
		setTheme(QString::null);
		return;
	    }
	    // Ensure extents are available
	    theme_vscrollbar->setGeometryAndLayout(0,0,theme_vscrollbar_groove->geometryHint().width(),32);
	} else {
	    theme_vscrollbar = 0;
	}
    }
    ThemedView *theme_hscrollbar;
    ThemedView *theme_vscrollbar;
    ThemeItem *theme_hscrollbar_slider;
    ThemeItem *theme_vscrollbar_slider;
    ThemeItem *theme_hscrollbar_groove;
    ThemeItem *theme_vscrollbar_groove;
    ThemeItem *theme_hscrollbar_add;
    ThemeItem *theme_vscrollbar_add;
    ThemeItem *theme_hscrollbar_sub;
    ThemeItem *theme_vscrollbar_sub;
    void positionThemeSlider(const QScrollBar *sb, int sliderStart, int sliderLength)
    {
	if (sb->orientation() == QScrollBar::Horizontal) {
	    theme_hscrollbar->setGeometryAndLayout(0,0,sb->width(),sb->height());
	    QRect r = theme_hscrollbar_slider->geometry();
	    theme_hscrollbar_slider->setGeometry(QRect(sliderStart,r.y(),sliderLength,r.height()));
	} else {
	    theme_vscrollbar->setGeometryAndLayout(0,0,sb->width(),sb->height());
	    QRect r = theme_vscrollbar_slider->geometry();
	    theme_vscrollbar_slider->setGeometry(QRect(r.x(),sliderStart,r.width(),sliderLength));
	}
    }

    bool eventFilter( QObject *o, QEvent *e ) {
	if ( e->type() == QEvent::ParentPaletteChange ) {
	    if ( o->inherits( "QMenuBar" ) ) {
		QWidget *w = (QWidget *)o;
		if ( w->parentWidget() ) {
		    QPalette p = w->parentWidget()->palette();
		    QColorGroup a = p.active();
		    a.setColor( QColorGroup::Light, a.dark() );
//		    a.setColor( QColorGroup::Dark, a.foreground() );
		    p.setActive( a );
		    p.setInactive( a );
		    w->setPalette( p );
		}
	    } else if ( o->inherits( "QHeader" ) ) {
		QWidget *w = (QWidget *)o;
		if ( w->parentWidget() ) {
		    QPalette p = w->parentWidget()->palette();
		    QColorGroup a = p.active();
		    a.setColor( QColorGroup::Light, a.button() );
		    p.setActive( a );
		    p.setInactive( a );
		    w->setPalette( p );
		}
	    } else if (mousePreferred && (o->inherits("QLineEdit") || o->inherits("QMultiLineEdit"))) {
		QWidget *w = (QWidget *)o;
		if ( w->parentWidget() ) {
		    QPalette p = w->parentWidget()->palette();
		    QColorGroup a = p.active();
		    a.setBrush(QColorGroup::Base, a.brush(QColorGroup::Background));
		    p.setActive( a );
		    w->setPalette( p );
		}
	    }
	} else if ( !mousePreferred && e->type() == QEvent::Resize && o->inherits("QSpinBox") ) {
	    ((SpinBoxHack*)o)->rearrange();
	    return TRUE;
	} else if ( !mousePreferred && e->type() == QEvent::EnterModalEdit ) {
	    QWidget *w = (QWidget *)o;
	    if ( w->inherits("QListView") || w->inherits("QTable")
		    || w->inherits("QListBox") || w->inherits("QTextView") ) {
		// It's possible that the original attempt to determine
		// whether we're single focus was bogus because the
		// entire interface wasn't yet constructed.
		// This is our last chance to get it right.
		((MarginHack *)w)->apply();
		w->removeEventFilter( this );
	    }
	} else if (mousePreferred && e->type() == QEvent::FocusIn) {
	    QWidget *w = (QWidget *)o;
	    if (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit")) {
		QPalette p = w->palette();
		QColorGroup a = p.active();
		a.setBrush(QColorGroup::Base, qApp->palette().active().brush(QColorGroup::Base));
		p.setActive( a );
		w->setPalette( p );
	    }
	} else if (mousePreferred && e->type() == QEvent::FocusOut) {
	    QWidget *w = (QWidget *)o;
	    if (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit")) {
		QPalette p = w->palette();
		QColorGroup a = p.active();
		a.setBrush(QColorGroup::Base, a.brush(QColorGroup::Background));
		p.setActive( a );
		w->setPalette( p );
	    }
	}
	return FALSE;
    }

    BgExport *bgExport;
    QPalette bgPal;
    QBrush altBrush;

public slots:
    void updateDecoration() {
	QTimer::singleShot(0, this, SLOT(doUpdateDecoration()));
    }

    void doUpdateDecoration() {
	QWidget *active = qApp->activeWindow();
	if (active) {
	    QApplication::postEvent((QObject*)((ManagerAccessor *)active)->manager(),
			new QPaintEvent(qApp->desktop()->rect(), TRUE ) );
	}
    }

private:
    QMap<QFrame *,int> frameStyles;
};

PhoneStyle::PhoneStyle() : fillBtnBorder(FALSE), revItem(FALSE)
{
    // No need for margins - no boxes
    setPixelMetric(ComboBoxFocusMargin,0);
    setPixelMetric(ComboBoxTextHMargin,0);
    setPixelMetric(ComboBoxTextVMargin,0);
    setPixelMetric(ComboBoxTextVMarginExtra,0);

    setPixelMetric(SpinBoxButtonScale,0);
    setPixelMetric(SpinBoxHMargin, 0);
    setPixelMetric(SpinBoxVMargin,0);
    setPixelMetric(SpinBoxHExtraScale,0);

    setPixelMetric(IdealHeightLimit,0);
    setPixelMetric(IdealHeight,0);
    setPixelMetric(CheckBoxGap,4);
    setPixelMetric(TabHMargin,0);
    
    setButtonMargin(1);

    d = new PhoneStylePrivate;

    QSize sbe = scrollBarExtent();
    setScrollBarExtent( sbe.width(), sbe.height() );

    setButtonDefaultIndicatorWidth(0);
}

PhoneStyle::~PhoneStyle()
{
    delete d;
}

int PhoneStyle::buttonMargin() const
{
    return 1;
}

QSize PhoneStyle::scrollBarExtent() const
{
    int extentW, extentH;

    if ( d->theme_vscrollbar )
	extentW = d->theme_vscrollbar->width();
    else if ( mousePreferred )
	extentW = 15;
    else
	extentW = 8;

    if ( d->theme_hscrollbar )
	extentH = d->theme_hscrollbar->height();
    else if( mousePreferred )
	extentH = 15;
    else
	extentH = 8;

    return QSize(extentW, extentH);
}

void PhoneStyle::polish ( QPalette &pal )
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
	d->bgPal = pal;
	d->bgPal.setBrush(QColorGroup::Background, QBrush(pal.color(QPalette::Normal, QColorGroup::Background), d->bgExport->background()));
	d->bgPal.setBrush(QColorGroup::Base, QBrush(pal.color(QPalette::Normal, QColorGroup::Base), d->bgExport->base()));
	d->altBrush = QBrush(extendedBrush(QStyle::AlternateBase).color(), d->bgExport->alternateBase());
	setExtendedBrush(QStyle::AlternateBase, d->altBrush);
	pal = d->bgPal;
    }
#endif
}

class QStyleHackWidget : public QWidget {
public:
    static void setGlobalBrushOrigin(QWidget *w) {
	((QStyleHackWidget*)w)->setWState(WState_GlobalBrushOrigin);
    }
    static void clearGlobalBrushOrigin(QWidget *w) {
	((QStyleHackWidget*)w)->clearWState(WState_GlobalBrushOrigin);
    }
};

void PhoneStyle::polish( QWidget *w )
{
#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable()) {
	QStyleHackWidget::setGlobalBrushOrigin(w);
	if (extendedBrush(QStyle::AlternateBase) != d->altBrush) {
	    setExtendedBrush(QStyle::AlternateBase, d->altBrush);
	}
	if (w->inherits("QScrollView"))
	    ((QScrollView*)w)->setStaticBackground(TRUE);
    }
#endif
    if ( w->inherits( "QFrame" ) ) {
	QFrame *f = (QFrame *)w;
	if ( f->frameShape() == QFrame::HLine || f->frameShape() == QFrame::VLine )
	    f->setFrameShadow( QFrame::Plain );
	else if ( f->frameShape() != QFrame::NoFrame )
	    f->setFrameShape( QFrame::StyledPanel );
	f->setLineWidth( 1 );
    }
    if ( w->inherits( "QSpinBox" ) ) {
	SpinBoxHack *sb = (SpinBoxHack*)w;
	sb->setFlatButtons( TRUE );
	if (!mousePreferred) {
	    sb->rearrange();
	    w->installEventFilter( d );
	}
    } else if ( w->inherits( "QMenuBar" ) ) {
	// make selected item look flat
	QPalette p = w->palette();
	QColorGroup a = p.active();
	a.setColor( QColorGroup::Light, a.dark() );
//	a.setColor( QColorGroup::Dark, a.foreground() );
	p.setActive( a );
	p.setInactive( a );
	w->setPalette( p );
	w->installEventFilter( d );
    } else if ( w->inherits( "QHeader" ) ) {
	// make headers look flat
	QPalette p = w->palette();
	QColorGroup a = p.active();
	a.setColor( QColorGroup::Light, a.button() );
	p.setActive( a );
	p.setInactive( a );
	w->setPalette( p );
	w->installEventFilter( d );
    } else if ( !mousePreferred && ((w->inherits("QListView") || w->inherits("QTable")
		|| w->inherits("QListBox") || w->inherits("QTextView")
		|| w->inherits("QTableView")))) {
	((MarginHack *)w)->apply();
	w->installEventFilter( d );
    } else if (w->inherits("QWidgetStack")) {
	connect(w, SIGNAL(aboutToShow(int)), d, SLOT(updateDecoration()));
    }
    
    if (mousePreferred && (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit"))) {
	if (!w->hasFocus()) {
	    QPalette p = w->palette();
	    QColorGroup a = p.active();
	    a.setBrush(QColorGroup::Base, a.brush(QColorGroup::Background));
	    p.setActive( a );
	    w->setPalette( p );
	}
	if (w->inherits("QLineEdit") || mousePreferred) // not for QMLE if !mousePreferred - already added for QTableView
	    w->installEventFilter( d );
    }
    
    if (w->backgroundMode() == QWidget::PaletteBackground) {
	QWidget *p = w->parentWidget();
	if (!p || strcmp(p->name(), "qt_viewport"))
	    w->setBackgroundOrigin(QWidget::AncestorOrigin);
    }
}

void PhoneStyle::unPolish( QWidget *w )
{
    if ( w->inherits("QFrame") ) {
	QFrame *f = (QFrame *)w;
	if ( f->frameShape() == QFrame::HLine || f->frameShape() == QFrame::VLine ) {
	    f->setFrameShadow( QFrame::Sunken );
	} else if ( f->frameShape() != QFrame::NoFrame ) {
	    f->setFrameShape( QFrame::StyledPanel );
	    f->setLineWidth( 2 );
	}
    }
    if ( w->inherits("QSpinBox") ) {
	((SpinBoxHack*)w)->setFlatButtons( FALSE );
	w->removeEventFilter( d );
    } else if ( w->inherits("QMenuBar") || w->inherits("QHeader") ) {
	w->unsetPalette();
	w->removeEventFilter( d );
    } else if ( !mousePreferred && (w->inherits("QListView") || w->inherits("QTable")
		|| w->inherits("QListBox") || w->inherits("QTextView")
		|| w->inherits("QTableView")) ) {
	((MarginHack *)w)->restore();
	w->removeEventFilter( d );
    }
    if (mousePreferred && (w->inherits("QLineEdit") || w->inherits("QMultiLineEdit"))) {
	w->removeEventFilter( d );
    }
    w->setBackgroundOrigin(QWidget::WidgetOrigin);
    w->unsetPalette();

#ifdef QTOPIA_ENABLE_GLOBAL_BACKGROUNDS
    if (d->bgExport->isAvailable())
	QStyleHackWidget::clearGlobalBrushOrigin(w);
#endif
}

int PhoneStyle::defaultFrameWidth() const
{
    return 1;
}

void PhoneStyle::drawArrow( QPainter *p, Qt::ArrowType type, bool down,
	int x, int y, int w, int h,
	const QColorGroup &g, bool enabled, const QBrush *fill ) {
    revItem = FALSE;
    QWindowsStyle::drawArrow(p,type,down,x,y,w,h,g,enabled,fill);

}

static QPaintDevice* revDevice = 0;

void PhoneStyle::drawItem( QPainter *p, int x, int y, int w, int h,
                    int flags, const QColorGroup &g, bool enabled,
		    const QPixmap *pixmap, const QString& text, int len,
		    const QColor* penColor )
{
    QColor pc( penColor ? *penColor : g.foreground() );
    QColorGroup cg( g );
    if ( !enabled )
	cg.setColor( QColorGroup::Light, cg.background() );
    if ( revItem ) {
        if( p->device() == revDevice ) pc = cg.button();
        revItem = FALSE;
    }
    QWindowsStyle::drawItem( p, x, y, w, h, flags, cg, enabled, pixmap, text, len, &pc );
}

void PhoneStyle::drawPanel ( QPainter * p, int x, int y, int w, int h,
			    const QColorGroup &g, bool /*sunken*/, int lineWidth, const QBrush * fill )
{
    if ( fill )
	p->setBrush( *fill );

    QPen pen(g.shadow(), lineWidth);

    if ( p->device()->devType() == QInternal::Widget ) {
	QWidget *widget = (QWidget *)p->device();
	bool modalEdit = (mousePreferred ? FALSE : 
	     ((!widget->focusProxy() && widget->isModalEditing()) ||
	    (widget->focusProxy() && widget->focusProxy()->isModalEditing())));

	if ( !modalEdit ) {
	    if ( widget->inherits("QMultiLineEdit") ) {
		    if (!widget->hasFocus()){
			p->setPen( QPen::NoPen );
			p->drawRect( x, y, w, h );
			pen.setStyle(DotLine);
		    }
	    } else if ( widget->inherits("QLineEdit")
		    || widget->inherits("QComboBox")
		    || widget->inherits("QSpinBox") ) {
		p->fillRect( x, y, w, h, g.brush(QColorGroup::Background) );
		if (!widget->hasFocus()){
		    p->setPen( QPen(g.shadow(), lineWidth, DotLine) );
		    p->drawLine( x, y+h-1, x+w-1, y+h-1 );
		}
		return;
	    }
	}
    }

    p->setPen( pen );
    p->drawRect( x, y, w, h );
}

void PhoneStyle::drawButton( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &cg, bool , const QBrush* fill )
{
    QPen oldPen = p->pen();
    p->fillRect( x+1, y+1, w-2, h-2, fill?(*fill):cg.brush(QColorGroup::Button) );

    p->drawRect( x, y, w, h );
    //if ( sunken )
    p->setPen( oldPen );
}

void PhoneStyle::drawButtonMask ( QPainter * p, int x, int y, int w, int h )
{
    x++; y++;
    x-=2; y-=2;
    p->fillRect( x, y, w, h, color1 );
}

void PhoneStyle::drawBevelButton( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool /*sunken*/, const QBrush* fill )
{
    p->fillRect( x+1, y+1, w-2, h-2, fill?(*fill):g.brush(QColorGroup::Button) );
    p->setPen( g.shadow() );
    p->setBrush( NoBrush );
    p->drawRect( x, y, w, h );
}

void PhoneStyle::drawToolButton( QPainter *p, int x, int y, int w, int h,
                                const QColorGroup &g, bool sunken, const QBrush* fill )
{
    bool toggleOn = FALSE;
    if ( p->device()->devType() == QInternal::Widget ) {
	QWidget *w = (QWidget *)p->device();
	if ( w->inherits("QToolButton") ) {
	    QToolButton *btn = (QToolButton *)w;
	    if ( btn->parentWidget() ) {
		btnBg = btn->parentWidget()->backgroundColor();
		fillBtnBorder = TRUE;
	    }
	    toggleOn = btn->isToggleButton() && btn->isOn();
	}
    } else {
	toggleOn = fill && fill->style() == Dense4Pattern;
    }
    QBrush fb( fill ? *fill : g.button() );
    if ( (sunken && fb == g.brush( QColorGroup::Button )) || toggleOn ) {
	fb = g.buttonText();
	revItem = TRUE;	// ugh
        revDevice = p->device();
    }
    drawButton( p, x, y, w, h, g, sunken, &fb );
    fillBtnBorder = FALSE;
}

void PhoneStyle::drawFocusRect( QPainter *p, const QRect &r, const QColorGroup &cg, const QColor *, bool ab)
{
    if ( p->device()->devType() == QInternal::Widget ) {
	QWidget *widget = (QWidget *)p->device();
	if ( (mousePreferred || !widget->isModalEditing()) && 
		widget->inherits("QComboBox") ) {
	    return;
	}
	QWidget *pw = widget->parentWidget();
	if (pw && pw->inherits("QScrollView")) {
	    if ((pw->inherits("QListBox") && ((QListBox*)pw)->selectionMode() == QListBox::Single)
	    || (pw->inherits("QListView") && ((QListView*)pw)->selectionMode() == QListView::Single))
		return;
	}
    }

    QPen op = p->pen();
    p->setPen(cg.highlight());
    
    p->drawRect(r.x()+1, r.y()+1, r.width()-2, r.height()-2);
    if (ab)
	p->drawRect(r.x()+2, r.y()+2, r.width()-4, r.height()-4);
    else
	p->drawRect(r);
    
    p->setPen( op );
}

void PhoneStyle::drawPushButton( QPushButton *btn, QPainter *p )
{
    QColorGroup g = btn->colorGroup();
    int x1, y1, x2, y2;

    btn->rect().coords( &x1, &y1, &x2, &y2 );	// get coordinates

    p->setPen( g.shadow() );
    p->setBrush( QBrush(g.button(),NoBrush) );

//    int diw = buttonDefaultIndicatorWidth();
    /*
    if ( btn->isDefault() || btn->autoDefault() ) {
	if ( btn->isDefault() ) {
	    p->setPen( g.shadow() );
	    p->drawRect( x1, y1, x2-x1+1, y2-y1+1 );
	}
	x1 += diw;
	y1 += diw;
	x2 -= diw;
	y2 -= diw;
    }
    */

    if ( btn->parentWidget() ) {
	btnBg = btn->parentWidget()->backgroundColor();
	fillBtnBorder = TRUE;
    }

    bool clearButton = TRUE;
    if ( btn->isDown() ) {
	drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, TRUE,
		&g.brush( QColorGroup::Text ) );
    } else {
	if ( !btn->isFlat() )
	    drawButton( p, x1, y1, x2-x1+1, y2-y1+1, g, !btn->isToggleButton() 
		    || btn->isOn(), &g.brush( QColorGroup::Button ) );
	if ( btn->isToggleButton() && btn->isOn() && btn->isEnabled() ) {
	    p->drawRect( x1+buttonMargin(), y1+buttonMargin(),
		    x2-x1+1-buttonMargin()*2, y2-y1+1-buttonMargin()*2 ); 
	    clearButton = FALSE;
	}
    }
    /*
    if ( clearButton ) {
	if (btn->isDown())
	    p->setBrushOrigin(p->brushOrigin() + QPoint(1,1));
	p->fillRect( x1+2, y1+2, x2-x1-3, y2-y1-3,
		     g.brush( QColorGroup::Button ) );
	if (btn->isDown())
	    p->setBrushOrigin(p->brushOrigin() - QPoint(1,1));
    }
    */

    fillBtnBorder = FALSE;
    if ( p->brush().style() != NoBrush )
	p->setBrush( NoBrush );
}

void PhoneStyle::drawPushButtonLabel( QPushButton *btn, QPainter *p )
{
    QRect r = pushButtonContentsRect( btn );
    int x, y, w, h;
    r.rect( &x, &y, &w, &h );
    QColorGroup cg = btn->colorGroup();
    if ( btn->isToggleButton() && btn->isOn() && btn->isEnabled() && !btn->isDown() )
	cg.setColor( QColorGroup::ButtonText, btn->colorGroup().text() );
    else if ( btn->isDown() || btn->isOn() )
	cg.setColor( QColorGroup::ButtonText, btn->colorGroup().button() );
    if ( btn->isMenuButton() ) {
	int dx = menuButtonIndicatorWidth( btn->height() );
	drawArrow( p, DownArrow, FALSE,
		x+w-dx, y+2, dx-4, h-4,
		cg,
		btn->isEnabled() );
	w -= dx;
    }

    if ( btn->iconSet() && !btn->iconSet()->isNull() ) {
	QIconSet::Mode mode = btn->isEnabled()
	    ? QIconSet::Normal : QIconSet::Disabled;
	if ( mode == QIconSet::Normal && btn->hasFocus() )
	    mode = QIconSet::Active;
	QPixmap pixmap = btn->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	p->drawPixmap( x+2, y+h/2-pixh/2, pixmap );
	x += pixw + 4;
	w -= pixw + 4;
    }
    drawItem( p, x, y, w, h,
	    AlignCenter | ShowPrefix,
	    cg, btn->isEnabled(),
	    btn->pixmap(), btn->text(), -1, &cg.buttonText() );

}

QRect PhoneStyle::comboButtonRect( int x, int y, int w, int h)
{
    return QRect(x+1, y+1, w-14, h-2);
}



QRect PhoneStyle::comboButtonFocusRect( int x, int y, int w, int h)
{
    return QRect(x+1, y+1, w-15, h-2);
}

void PhoneStyle::drawComboButton( QPainter *p, int x, int y, int w, int h,
				     const QColorGroup &g, bool sunken,
				     bool /*editable*/,
				     bool enabled,
				     const QBrush * /*fill*/ )
{
    QColorGroup cg( g );
    p->setPen( g.shadow() );
    p->setBrush( QBrush(NoBrush) );
    int arroww = 12;
    drawPanel(p, x, y, w-arroww, h, g, 1);
    if ( sunken ) {
	cg.setColor( QColorGroup::ButtonText, g.button() );
	cg.setColor( QColorGroup::Button, g.buttonText() );
    }
    drawArrow( p, QStyle::DownArrow, FALSE,
	       x+w-arroww, y, arroww, h-1, cg, enabled,
	       &cg.brush( QColorGroup::Background ) );

}


void PhoneStyle::drawExclusiveIndicator ( QPainter * p, int x, int y, int w,
	int h, const QColorGroup &cg, bool on, bool down, bool enabled )
{
    static const QCOORD pts1[] = {              // dark lines
	1,9, 1,8, 0,7, 0,4, 1,3, 1,2, 2,1, 3,1, 4,0, 7,0, 8,1, 9,1 };
    static const QCOORD pts4[] = {              // white lines
	2,10, 3,10, 4,11, 7,11, 8,10, 9,10, 10,9, 10,8, 11,7,
	11,4, 10,3, 10,2 };
    static const QCOORD pts5[] = {              // inner fill
	4,2, 7,2, 9,4, 9,7, 7,9, 4,9, 2,7, 2,4 };

    p->eraseRect( x, y, w, h );
    QPointArray a( QCOORDARRLEN(pts1), pts4 );
    a.translate( x, y );
    p->setPen( cg.shadow() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts4), pts1 );
    a.translate( x, y );
    p->setPen( cg.shadow() );
    p->drawPolyline( a );
    a.setPoints( QCOORDARRLEN(pts5), pts5 );
    a.translate( x, y );
    QColor fillColor = ( down || !enabled ) ? cg.button() : cg.base();
    p->setPen( fillColor );
    p->setBrush( fillColor  ) ;
    p->drawPolygon( a );
    if ( on ) {
	p->setPen( NoPen );
	p->setBrush( cg.text() );
	p->drawRect( x+5, y+4, 2, 4 );
	p->drawRect( x+4, y+5, 4, 2 );
    }
}

void PhoneStyle::drawIndicator ( QPainter * p, int x, int y, int w, int h,
	const QColorGroup &cg, int state, bool down, bool enabled )
{
    QColorGroup mycg( cg );
    mycg.setBrush( QColorGroup::Button, QBrush() );
    QBrush fill;
    drawButton( p, x, y, w, h, mycg, TRUE, 0 );
    if ( down )
	fill = cg.brush( QColorGroup::Button );
    else
	fill = cg.brush( enabled ? QColorGroup::Base : QColorGroup::Background );
    mycg.setBrush( QColorGroup::Button, fill );
    p->fillRect( x+1, y+1, w-2, h-2, fill );
    if ( state != QButton::Off ) {
	QPointArray a( 7*2 );
	int i, xx, yy;
	xx = x+3;
	yy = y+5;
	for ( i=0; i<3; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy++;
	}
	yy -= 2;
	for ( i=3; i<7; i++ ) {
	    a.setPoint( 2*i,   xx, yy );
	    a.setPoint( 2*i+1, xx, yy+2 );
	    xx++; yy--;
	}
	if ( state == QButton::NoChange ) {
	    p->setPen( mycg.dark() );
	} else {
	    p->setPen( mycg.text() );
	}
	p->drawLineSegments( a );
    }
}

#define HORIZONTAL	(sb->orientation() == QScrollBar::Horizontal)
#define VERTICAL	!HORIZONTAL
#define MOTIF_BORDER	2

static const int sb_gap=0;
static const int sb_side_margin=2;

/*! \reimp */

void PhoneStyle::scrollBarMetrics( const QScrollBar* sb, int &sliderMin, int &sliderMax, int &sliderLength, int& buttonDim )
{
    QSize sbe = scrollBarExtent();

    int maxLength;
    int length = HORIZONTAL ? sb->width()  : sb->height();

    static const int endspace=0;
    if ( mousePreferred ) {
	if ( HORIZONTAL ) {
	    buttonDim = sbe.height();
	} else {
	    buttonDim = sbe.width();
	}
    } else {
	buttonDim = 0;
    }

    if ( d->theme_vscrollbar ) {
	// Themed...
	if ( HORIZONTAL ) {
	    d->theme_hscrollbar->setGeometryAndLayout(0,0,sb->width(),sb->height());
	    sliderMin = d->theme_hscrollbar_groove->geometry().x();
	    maxLength = d->theme_hscrollbar_groove->geometry().width()-1;
	} else {
	    d->theme_vscrollbar->setGeometryAndLayout(0,0,sb->width(),sb->height());
	    sliderMin = d->theme_vscrollbar_groove->geometry().y();
	    maxLength = d->theme_vscrollbar_groove->geometry().height()-1;
	}
    } else {
	// Non-themed...
	sliderMin = sb_gap+endspace;
	maxLength = length - (sb_gap+endspace+buttonDim)*2 + (mousePreferred ? 1 : 0);
    }

    int SLIDER_MIN = 9;// ### motif says 6 but that's too small
    if( mousePreferred )
	SLIDER_MIN = 15;

    if ( sb->maxValue() == sb->minValue() ) {
        sliderLength = maxLength;
    } else {
        sliderLength = (sb->pageStep()*maxLength)/
       		(sb->maxValue()-sb->minValue()+sb->pageStep());
        uint range = sb->maxValue()-sb->minValue();
        if ( sliderLength < SLIDER_MIN || range > INT_MAX/2 )
            sliderLength = SLIDER_MIN;
        if ( sliderLength > maxLength )
            sliderLength = maxLength;
    }

    sliderMax = sliderMin + maxLength - sliderLength;
}

/*!\reimp
 */
QStyle::ScrollControl PhoneStyle::scrollBarPointOver( const QScrollBar* sb, int sliderStart, const QPoint& p )
{
    if ( !sb->rect().contains( p ) )
	return NoScroll;
    int sliderMin, sliderMax, sliderLength, buttonDim, pos;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );
    if ( d->theme_vscrollbar ) {
	ThemeItem *hit;
	int pd;
	d->positionThemeSlider(sb,sliderStart,sliderLength);
	if (sb->orientation() == QScrollBar::Horizontal) {
	    hit = d->theme_hscrollbar->itemAt(p);
	    pd = p.x();
	} else {
	    hit = d->theme_vscrollbar->itemAt(p);
	    pd = p.y();
	}
	if ( hit ) {
	    if (hit->itemName() == "add") {
		return AddLine;
	    } else if (hit->itemName() == "sub") {
		return SubLine;
	    } else if (hit->itemName() == "slider") {
		return Slider;
	    }
	}
	// Assume groove
	if (pd <= sliderStart)
	    return SubPage;
	else
	    return AddPage;
    } else {
	if (sb->orientation() == QScrollBar::Horizontal)
	    pos = p.x();
	else
	    pos = p.y();

	if (pos < sliderStart)
	    return SubPage;
	if (pos < sliderStart + sliderLength)
	    return Slider;
	if (pos < sliderMax + sliderLength)
	    return AddPage;
	if (pos < sliderMax + sliderLength + buttonDim)
	    return SubLine;
	return AddLine;
    }
}

/*! \reimp */

void PhoneStyle::drawScrollBarControls( QPainter* p, const QScrollBar* sb, int sliderStart, uint controls, uint activeControl )
{
#define ADD_LINE_ACTIVE ( activeControl == AddLine )
#define SUB_LINE_ACTIVE ( activeControl == SubLine )
#define SLIDER_ACTIVE ( activeControl == Slider )
    QColorGroup g  = sb->colorGroup();

    int sliderMin, sliderMax, sliderLength, buttonDim;
    scrollBarMetrics( sb, sliderMin, sliderMax, sliderLength, buttonDim );

    if (sliderStart > sliderMax) { // sanity check
	sliderStart = sliderMax;
    }

    if ( d->theme_hscrollbar )
	d->positionThemeSlider(sb,sliderStart,sliderLength);
    if ( HORIZONTAL ) {
	if ( d->theme_hscrollbar ) {
	    if ( d->theme_hscrollbar_sub )
		d->theme_hscrollbar_sub->setPressed(SUB_LINE_ACTIVE);
	    if ( d->theme_hscrollbar_add )
		d->theme_hscrollbar_add->setPressed(ADD_LINE_ACTIVE);
	    d->theme_hscrollbar_slider->setPressed(SLIDER_ACTIVE);
	    QRect clip;
	    if ((activeControl & AddPage) && (activeControl & SubPage)) {
		// must be a repaint.
		clip = sb->rect();
	    } else {
		if (d->theme_hscrollbar_sub && activeControl & SubLine)
		    clip |= d->theme_hscrollbar_sub->geometry();
		if (d->theme_hscrollbar_add && activeControl & AddLine)
		    clip |= d->theme_hscrollbar_add->geometry();
		if (activeControl & Slider) {
		    clip |= d->theme_hscrollbar_slider->geometry();
		    clip |= d->theme_hscrollbar_groove->geometry();
		}
		if (clip.isEmpty())
		    clip = sb->rect();
	    }
	    QPixmap pm(clip.width(), clip.height());
	    pm.fill(sb, clip.topLeft());
	    QPainter ppm(&pm, sb);
	    d->theme_hscrollbar->paint(&ppm,clip);
	    p->drawPixmap(clip.x(), clip.y(), pm);
//	    d->theme_hscrollbar->paint(p,sb->rect());// XXX CLIP! p->clipRegion().boundingRect());
	    return;
	}
    } else {
	if ( d->theme_vscrollbar ) {
	    if ( d->theme_vscrollbar_sub )
		d->theme_vscrollbar_sub->setPressed(SUB_LINE_ACTIVE);
	    if ( d->theme_vscrollbar_add )
		d->theme_vscrollbar_add->setPressed(ADD_LINE_ACTIVE);
	    d->theme_vscrollbar_slider->setPressed(SLIDER_ACTIVE);
	    QRect clip;
	    if ((activeControl & AddPage) && (activeControl & SubPage)) {
		// must be a repaint.
		clip = sb->rect();
	    } else {
		if (d->theme_vscrollbar_sub && activeControl & SubLine)
		    clip |= d->theme_vscrollbar_sub->geometry();
		if (d->theme_vscrollbar_add && activeControl & AddLine)
		    clip |= d->theme_vscrollbar_add->geometry();
		if (activeControl & Slider) {
		    clip |= d->theme_vscrollbar_slider->geometry();
		    clip |= d->theme_vscrollbar_groove->geometry();
		}
		if (clip.isEmpty())
		    clip = sb->rect();
	    }
	    QPixmap pm(clip.width(), clip.height());
	    pm.fill(sb, clip.topLeft());
	    QPainter ppm(&pm, sb);
	    d->theme_vscrollbar->paint(&ppm,clip);
	    p->drawPixmap(clip.x(), clip.y(), pm);
//	    d->theme_vscrollbar->paint(p,sb->rect());//p->clipRegion().boundingRect());
	    return;
	}
    }

    int dimB = buttonDim;
    QRect addB;
    QRect subB;
    QRect addPageR;
    QRect subPageR;
    QRect sliderR;
    int addX, addY, subX, subY;
    int length = HORIZONTAL ? sb->width()  : sb->height();
    int extent = HORIZONTAL ? sb->height() : sb->width();

    if ( HORIZONTAL ) {
	subY = addY = ( extent - dimB ) / 2;
	subX = length - dimB - dimB + 1;
	addX = length - dimB;
    } else {
	subX = addX = ( extent - dimB ) / 2;
	subY = length - dimB - dimB + 1;
	addY = length - dimB;
    }

    bool maxedOut = (sb->maxValue() == sb->minValue());
    p->setPen( g.shadow() );
    p->setBrush( g.brush( QColorGroup::Mid ) );

    int sliderEnd = sliderStart + sliderLength;
    int sliderW = extent;
    if ( HORIZONTAL ) {
	subB.setRect( subX,subY,dimB,dimB );
	addB.setRect( addX,addY,dimB,dimB );
	subPageR.setRect( sb_gap, 0, sliderStart, sliderW );
	addPageR.setRect( sliderEnd, 0, subX - sliderEnd - sb_gap, sliderW );
	sliderR .setRect( sliderStart, 0, sliderLength, sliderW );

	if ( controls & SubPage && subPageR.width()>0 ) {
	    p->fillRect( subPageR.x(), subPageR.y(), subPageR.width(), sb_side_margin, g.brush(QColorGroup::Background) );
	    p->drawRect( subPageR.x(), subPageR.y()+sb_side_margin, subPageR.width()+1, subPageR.height()-sb_side_margin*2 );
	    p->fillRect( subPageR.x(), subPageR.y()+subPageR.height()-sb_side_margin, subPageR.width(), sb_side_margin, g.brush(QColorGroup::Background) );
	}
	if ( controls & AddPage && addPageR.y() < addPageR.bottom() &&
		addPageR.x() < addPageR.right() && addPageR.width()>0 ) {
	    p->fillRect( addPageR.x(), addPageR.y(), addPageR.width(), sb_side_margin, g.brush(QColorGroup::Background) );
	    p->drawRect( addPageR.x()-1, addPageR.y()+sb_side_margin, addPageR.width()+(mousePreferred?1:0), addPageR.height()-sb_side_margin*2 );
	    p->fillRect( addPageR.x(), addPageR.y()+subPageR.height()-sb_side_margin, addPageR.width(), sb_side_margin, g.brush(QColorGroup::Background) );
	}
    } else {
	subB.setRect( subX,subY,dimB,dimB );
	addB.setRect( addX,addY,dimB,dimB );
	subPageR.setRect( 0, sb_gap, sliderW, sliderStart );
	addPageR.setRect( 0, sliderEnd, sliderW, subY - sliderEnd - sb_gap );
	sliderR .setRect( 0, sliderStart, sliderW, sliderLength );

	if ( controls & SubPage && subPageR.height()>0 ) {
	    p->fillRect( subPageR.x(), subPageR.y(), sb_side_margin, subPageR.height(), g.brush(QColorGroup::Background) );
	    p->drawRect( subPageR.x()+sb_side_margin, subPageR.y(), subPageR.width()-sb_side_margin*2, subPageR.height()+1 );
	    p->fillRect( subPageR.x()+subPageR.width()-sb_side_margin, subPageR.y(), sb_side_margin, subPageR.height(), g.brush(QColorGroup::Background) );
	}
	if ( controls & AddPage && addPageR.y() < addPageR.bottom() &&
		addPageR.x() < addPageR.right() && addPageR.height()>0 ) {
	    p->fillRect( addPageR.x(), addPageR.y(), sb_side_margin, addPageR.height(), g.brush(QColorGroup::Background) );
	    p->drawRect( addPageR.x()+sb_side_margin, addPageR.y()-1, addPageR.width()-sb_side_margin*2, addPageR.height()+(mousePreferred?1:0) );
	    p->fillRect( addPageR.x()+subPageR.width()-sb_side_margin, addPageR.y(), sb_side_margin, addPageR.height(), g.brush(QColorGroup::Background) );
	}
    }
    if ( controls & Slider ) {
	QPoint bo = p->brushOrigin();
	if ( !sb->testWState(WState_GlobalBrushOrigin) )
	    p->setBrushOrigin(sliderR.topLeft());
	p->setPen( g.shadow() );
	if (SLIDER_ACTIVE)
	    p->setBrush( g.buttonText() );
	else
	    p->setBrush( g.button() );
	p->drawRect( sliderR.x(), sliderR.y(), sliderR.width(), sliderR.height() );
	p->setBrushOrigin(bo);
	QColorGroup cg( g );
	if ( maxedOut )
	    cg.setColor( QColorGroup::ButtonText, g.mid() );
	else if ( SLIDER_ACTIVE ) {
	    cg.setColor( QColorGroup::ButtonText, g.button() );
	}
	drawRiffles( p, sliderR.x(), sliderR.y(),
		     sliderR.width(), sliderR.height(), cg, HORIZONTAL );
    }

    if ( mousePreferred ) {
	if ( controls & AddLine ) {
	    p->setBrush( ADD_LINE_ACTIVE ? g.foreground() : g.button() );
	    p->drawRect( addB.x(), addB.y(), addB.width(), addB.height() );
	    p->setPen( ADD_LINE_ACTIVE ? g.button() : g.foreground() );
	    QColorGroup cg( g );
	    if ( maxedOut )
		cg.setColor( QColorGroup::ButtonText, g.mid() );
	    else if ( ADD_LINE_ACTIVE )
		cg.setColor( QColorGroup::ButtonText, g.button() );
	    int xo = VERTICAL ? 1 : 0;
	    drawArrow( p, VERTICAL ? DownArrow : RightArrow, FALSE,
		       addB.x()+2+xo, addB.y()+2, addB.width()-4-xo, addB.height()-4,
		       cg, TRUE, &p->brush() );
	}
	if ( controls & SubLine ) {
	    p->setBrush( SUB_LINE_ACTIVE ? g.foreground() : g.button() );
	    p->drawRect( subB.x(), subB.y(), subB.width(), subB.height() );
	    p->setPen( SUB_LINE_ACTIVE ? g.button() : g.foreground() );
	    QColorGroup cg( g );
	    if ( maxedOut )
		cg.setColor( QColorGroup::ButtonText, g.mid() );
	    else if ( SUB_LINE_ACTIVE )
		cg.setColor( QColorGroup::ButtonText, g.button() );
	    int xo = VERTICAL ? 1 : 0;
	    drawArrow( p, VERTICAL ? UpArrow : LeftArrow, FALSE,
		       subB.x()+2+xo, subB.y()+2, subB.width()-4-xo, subB.height()-4,
		       cg, TRUE, &p->brush() );
	}
    }

    // ### perhaps this should not be able to accept focus if maxedOut?
    if ( sb->hasFocus() && (controls & Slider) )
	p->drawWinFocusRect( sliderR.x()+2, sliderR.y()+2,
			     sliderR.width()-5, sliderR.height()-5,
			     sb->backgroundColor() );

}

void PhoneStyle::drawRiffles( QPainter* p,  int x, int y, int w, int h,
                      const QColorGroup &g, bool horizontal )
{
    const int n = 3;
    const int spacing=3;
    if (!horizontal) {
	if (h > 12) {
	    int my = y+h/2-spacing*(n/2);
	    int i ;
	    p->setPen(g.buttonText());
	    for (i=0; i<n; i++) {
		p->drawLine(x+4, my+spacing*i, x+w-5, my+3*i);
	    }
	}
    } else {
	if (w > 12) {
	    int mx = x+w/2-spacing*(n/2);
	    int i ;
	    p->setPen(g.buttonText());
	    for (i=0; i<n; i++) {
		p->drawLine(mx+spacing*i, y+4, mx + 3*i, y+h-5);
	    }
	}
    }
}

int PhoneStyle::sliderLength() const
{
    return 12; // FIXME : slider works out its own thickness so changing this for touchscreen doesn't really work. need to adjust qslider also
}

void PhoneStyle::drawSlider( QPainter *p, int x, int y, int w, int h,
	const QColorGroup &g, Orientation o, bool tickAbove, bool tickBelow )
{
    int a = tickAbove ? 3 : 0;
    int b = tickBelow ? 3 : 0;

    QPen grooveCol(g.shadow());
    p->setPen(g.shadow());
    if ( p->device()->devType() == QInternal::Widget ) {
	QWidget *widget = (QWidget *)p->device();
	if (mousePreferred || !widget->isModalEditing()) {
	    p->setBrush( g.button() );
	} else if( !mousePreferred ) {
	    grooveCol = g.highlightedText();
	    p->setBrush( g.highlight() );
	}
    }
    if ( o == Horizontal ) {
	p->drawRect( x, y+a, w, h-a-b );
	int xp = x + w/2;
	p->setPen(grooveCol);
	p->drawLine( xp-1, y+a+3, xp-1, y+h-b-4 );
	p->drawLine( xp, y+a+3, xp, y+h-b-4 );
    } else {
	p->drawRect( x+a, y, w-a-b, h );
	int yp = y + h/2;
	p->setPen(grooveCol);
	p->drawLine( x+a+3, yp-1, x+w-b-4, yp-1 );
	p->drawLine( x+a+3, yp, x+w-b-4, yp );
    }
}

void PhoneStyle::drawSliderMask ( QPainter * p, int x, int y, int w, int h,
	Orientation o, bool tickAbove, bool tickBelow )
{
    int a = tickAbove ? 3 : 0;
    int b = tickBelow ? 3 : 0;
    if ( o == Horizontal )
	p->fillRect( x, y+a, w, h-a-b, color1 );
    else
	p->fillRect( x+a, y, w-a-b, h, color1 );
}

/*!\reimp
 */
void PhoneStyle::drawSliderGrooveMask( QPainter *p,
					int x, int y, int w, int h,
					const QColorGroup& , QCOORD c,
					Orientation orient )
{
    if ( orient == Horizontal )
	p->fillRect( x, y + c - 2,  w, 4, color1 );
    else
	p->fillRect( x + c - 2, y, 4, h, color1 );
}

void PhoneStyle::drawSliderGroove( QPainter *p, int x, int y, int w, int h, const QColorGroup &g, QCOORD c, Orientation orient )
{
    if ( orient == Horizontal )
	p->fillRect( x, y + c - 2,  w, 4, g.shadow() );
    else
	p->fillRect( x + c - 2, y, 4, h, g.shadow() );
}

void PhoneStyle::tabbarMetrics( const QTabBar* t, int& hframe, int& vframe, int& overlap)
{
    overlap = 3;
    hframe = 10;
    hframe += pixelMetric(QStyle::TabHMargin);
    vframe = 0;
    if ( t->shape() == QTabBar::RoundedAbove || t->shape() == QTabBar::RoundedBelow )
	vframe += 8;
}

void PhoneStyle::drawTab( QPainter *p, const QTabBar *tb, QTab *t, bool selected )
{
    QRect r( t->rect() );
    if ( tb->shape()  == QTabBar::RoundedAbove ) {
	p->setPen( tb->colorGroup().shadow() );
	p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );
	if ( r.left() == 0 )
	    p->drawPoint( tb->rect().bottomLeft() );
	else
	    p->drawLine( r.left(), r.bottom(), r.right(), r.bottom() );

	if ( selected ) {
	    p->setPen( tb->colorGroup().background() );
	    p->drawLine( r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
	    p->fillRect( QRect( r.left()+1, r.top()+2, r.width()-2, r.height()-2),
			 tb->colorGroup().brush( QColorGroup::Background ));
	} else {
	    r.setRect( r.left() + 2, r.top() + 2,
		       r.width() - 4, r.height() - 2 );
	    p->setPen( tb->colorGroup().button() );
	    p->drawLine( r.left()+2, r.top()+1, r.right()-2, r.top()+1 );
	    p->fillRect( QRect( r.left()+1, r.top()+2, r.width()-2, r.height()-3),
			 tb->colorGroup().brush( QColorGroup::Button ));
	}

	p->setPen( tb->colorGroup().shadow() );
	p->drawLine( r.left(), r.bottom()-1, r.left(), r.top() + 2 );
	p->drawPoint( r.left()+1, r.top() + 1 );
	p->drawLine( r.left()+2, r.top(),
		     r.right() - 2, r.top() );

	p->drawPoint( r.right() - 1, r.top() + 1 );
	p->drawLine( r.right(), r.top() + 2, r.right(), r.bottom() - 1);
    } else if ( tb->shape() == QTabBar::RoundedBelow ) {
	if ( selected ) {
	    p->setPen( tb->colorGroup().background() );
	    p->drawLine( r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
	    p->fillRect( QRect( r.left()+1, r.top(), r.width()-2, r.height()-2),
			 tb->palette().normal().brush( QColorGroup::Background ));
	} else {
	    p->setPen( tb->colorGroup().shadow() );
	    p->drawLine( r.left(), r.top(),
			 r.right(), r.top() );
	    r.setRect( r.left() + 2, r.top(),
		       r.width() - 4, r.height() - 2 );
	    p->setPen( tb->colorGroup().button() );
	    p->drawLine( r.left()+2, r.bottom()-1, r.right()-2, r.bottom()-1 );
	    p->fillRect( QRect( r.left()+1, r.top()+1, r.width()-2, r.height()-3),
			 tb->palette().normal().brush( QColorGroup::Button ));
	}

	p->setPen( tb->colorGroup().shadow() );
	p->drawLine( r.right(), r.top(),
		     r.right(), r.bottom() - 2 );
	p->drawPoint( r.right() - 1, r.bottom() - 1 );
	p->drawLine( r.right() - 2, r.bottom(),
		     r.left() + 2, r.bottom() );

	p->drawLine( r.left(), r.top()+1,
		     r.left(), r.bottom() - 2 );
	p->drawPoint( r.left() + 1, r.bottom() - 1 );
	if ( r.left() == 0 )
	    p->drawPoint( tb->rect().topLeft() );

    } else {
	QCommonStyle::drawTab( p, tb, t, selected );
    }
}

static const int motifItemFrame		= 0;	// menu item frame width
static const int motifSepHeight		= 2;	// separator item height
static const int motifItemHMargin	= 2;	// menu item hor text margin
static const int motifItemVMargin	= 3;	// menu item ver text margin
static const int motifArrowHMargin	= 0;	// arrow horizontal margin
static const int motifTabSpacing	= 12;	// space between text and tab
static const int motifCheckMarkHMargin	= 1;	// horiz. margins of check mark
static const int windowsRightBorder	= 8;    // right border on windows
static const int windowsCheckMarkWidth  = 2;    // checkmarks width on windows

void PhoneStyle::polishPopupMenu ( QPopupMenu *m )
{
    QWindowsStyle::polishPopupMenu( m );
    m->setLineWidth( 1 );
}

/*! \reimp
*/
int PhoneStyle::extraPopupMenuItemWidth( bool checkable, int maxpmw, QMenuItem* mi, const QFontMetrics& /*fm*/ )
{
#ifndef QT_NO_MENUDATA
    int w = 2*motifItemHMargin + 2*motifItemFrame; // a little bit of border can never harm

    if ( mi->isSeparator() )
	return 10; // arbitrary
    else if ( mi->pixmap() )
	w += mi->pixmap()->width();	// pixmap only

    if ( !mi->text().isNull() ) {
	if ( mi->text().find('\t') >= 0 )	// string contains tab
	    w += motifTabSpacing;
    }

    if ( maxpmw ) { // we have iconsets
	w += maxpmw;
	w += 6; // add a little extra border around the iconset
    }

    if ( checkable && maxpmw < windowsCheckMarkWidth ) {
	w += windowsCheckMarkWidth - maxpmw; // space for the checkmarks
    }

    if ( maxpmw > 0 || checkable ) // we have a check-column ( iconsets or checkmarks)
	w += motifCheckMarkHMargin; // add space to separate the columns

    w += windowsRightBorder; // windows has a strange wide border on the right side

    return w;
#endif
}

/*! \reimp
*/
int PhoneStyle::popupMenuItemHeight( bool /*checkable*/, QMenuItem* mi, const QFontMetrics& fm )
{
#ifndef QT_NO_MENUDATA
    int h = 0;
    if ( mi->isSeparator() )			// separator height
	h = motifSepHeight;
    else if ( mi->pixmap() )		// pixmap height
	h = mi->pixmap()->height() + 2*motifItemFrame;
    else					// text height
	h = fm.height() + 2*motifItemVMargin + 2*motifItemFrame - 1;


    if ( !mi->isSeparator() && mi->iconSet() != 0 ) {
	h = QMAX( h, mi->iconSet()->pixmap( QIconSet::Small, QIconSet::Normal ).height() + 2*motifItemFrame );
    }
    if ( mi->custom() )
	h = QMAX( h, mi->custom()->sizeHint().height() + 2*motifItemVMargin + 2*motifItemFrame ) - 1;

    if ( !mi->isEnabled() )
	h = 0;
    return h;
#endif
}

void PhoneStyle::drawPopupMenuItem( QPainter* p, bool checkable, int maxpmw, int tab, QMenuItem* mi,
				       const QPalette& pal,
				       bool act, bool enabled, int x, int y, int w, int h)
{
#ifndef QT_NO_MENUDATA
    if ( !enabled )
	return;
    const QColorGroup & g = pal.active();
    bool dis	  = !enabled;
    QColorGroup itemg = dis ? pal.disabled() : pal.active();

    if ( checkable )
	maxpmw = QMAX( maxpmw, 8 ); // space for the checkmarks

    int checkcol	  =     maxpmw;

    if ( mi && mi->isSeparator() ) {			// draw separator
	p->setPen( g.dark() );
	p->drawLine( x, y, x+w, y );
	return;
    }

    QBrush fill = act? g.brush( QColorGroup::Highlight ) :
			    g.brush( QColorGroup::Button );
    p->fillRect( x, y, w, h, fill);

    if ( !mi )
	return;

    if ( mi->isChecked() ) {
	if ( act && !dis ) {
	    qDrawShadePanel( p, x, y, checkcol, h,
			     g, TRUE, 1, &g.brush( QColorGroup::Button ) );
	} else {
	    qDrawShadePanel( p, x, y, checkcol, h,
			     g, TRUE, 1, &g.brush( QColorGroup::Midlight ) );
	}
    } else if ( !act ) {
	p->fillRect(x, y, checkcol , h,
		    g.brush( QColorGroup::Button ));
    }

    if ( mi->iconSet() ) {		// draw iconset
	QIconSet::Mode mode = dis ? QIconSet::Disabled : QIconSet::Normal;
	if (act && !dis )
	    mode = QIconSet::Active;
	QPixmap pixmap = mi->iconSet()->pixmap( QIconSet::Small, mode );
	int pixw = pixmap.width();
	int pixh = pixmap.height();
	if ( act && !dis ) {
	    if ( !mi->isChecked() )
		qDrawShadePanel( p, x, y, checkcol, h, g, FALSE,  1, &g.brush( QColorGroup::Button ) );
	}
	QRect cr( x, y, checkcol, h );
	QRect pmr( 0, 0, pixw, pixh );
	pmr.moveCenter( cr.center() );
	p->setPen( itemg.text() );
	p->drawPixmap( pmr.topLeft(), pixmap );

	QBrush fill = act? g.brush( QColorGroup::Highlight ) :
			      g.brush( QColorGroup::Button );
	p->fillRect( x+checkcol + 1, y, w - checkcol - 1, h, fill);
    } else  if ( checkable ) {	// just "checking"...
	int mw = checkcol + motifItemFrame;
	int mh = h - 2*motifItemFrame;
	if ( mi->isChecked() ) {
	    drawCheckMark( p, x + motifItemFrame + 2,
			   y+motifItemFrame, mw, mh, itemg, act, dis );
	}
    }

    p->setPen( act ? g.highlightedText() : g.buttonText() );

    QColor discol;
    if ( dis ) {
	discol = itemg.text();
	p->setPen( discol );
    }

    int xm = motifItemFrame + checkcol + motifItemHMargin;

    if ( mi->custom() ) {
	int m = motifItemVMargin;
	p->save();
	if ( dis && !act ) {
	    p->setPen( g.light() );
	    mi->custom()->paint( p, itemg, act, enabled,
				 x+xm+1, y+m+1, w-xm-tab+1, h-2*m );
	    p->setPen( discol );
	}
	mi->custom()->paint( p, itemg, act, enabled,
			     x+xm, y+m, w-xm-tab+1, h-2*m );
	p->restore();
    }
    QString s = mi->text();
    if ( !s.isNull() ) {			// draw text
	int t = s.find( '\t' );
	int m = motifItemVMargin;
	const int text_flags = AlignVCenter|ShowPrefix | DontClip | SingleLine;
	if ( t >= 0 ) {				// draw tab text
	    if ( dis && !act ) {
		p->setPen( g.light() );
		p->drawText( x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame+1,
			     y+m+1, tab, h-2*m, text_flags, s.mid( t+1 ));
		p->setPen( discol );
	    }
	    p->drawText( x+w-tab-windowsRightBorder-motifItemHMargin-motifItemFrame,
			 y+m, tab, h-2*m, text_flags, s.mid( t+1 ) );
	}
	if ( dis && !act )
	    p->setPen( discol );
	p->drawText( x+xm, y+m, w-xm-tab+1, h-2*m, text_flags, s, t );
    } else if ( mi->pixmap() ) {			// draw pixmap
	QPixmap *pixmap = mi->pixmap();
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( OpaqueMode );
	p->drawPixmap( x+xm, y+motifItemFrame, *pixmap );
	if ( pixmap->depth() == 1 )
	    p->setBackgroundMode( TransparentMode );
    }
    if ( mi->popup() ) {			// draw sub menu arrow
	int dim = (h-2*motifItemFrame) / 2;
	if ( act ) {
	    if ( !dis )
		discol = white;
	    QColorGroup g2( discol, g.highlight(),
			    white, white,
			    dis ? discol : white,
			    discol, white );
	    drawArrow( p, RightArrow, FALSE,
			       x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
			       dim, dim, g2, TRUE );
	} else {
	    drawArrow( p, RightArrow,
			       FALSE,
			       x+w - motifArrowHMargin - motifItemFrame - dim,  y+h/2-dim/2,
			       dim, dim, g, mi->isEnabled() );
	}
    }
#endif
}

void PhoneStyle::getButtonShift( int &x, int &y )
{
    x = 0; y = 0;
}

void PhoneStyle::setTheme(const QString& file)
{
    d->setTheme(file);
    QSize sbe = scrollBarExtent();
    setScrollBarExtent( sbe.width(), sbe.height() );
}

//===========================================================================
#if 0
PhoneStyleImpl::PhoneStyleImpl()
    : phone(0), ref(0)
{
}

PhoneStyleImpl::~PhoneStyleImpl()
{
    // We do not delete the style because Qt does that when a new style
    // is set.
}

QStyle *PhoneStyleImpl::style()
{
    if ( !phone )
	phone = new PhoneStyle();
    return phone;
}

QString PhoneStyleImpl::name() const
{
    return qApp->translate("PhoneStyle", "Phone", "Name of the style Phone");
}

QRESULT PhoneStyleImpl::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_Style )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}

Q_EXPORT_INTERFACE()
{
    Q_CREATE_INSTANCE( PhoneStyleImpl )
}
#endif

#include "phonestyle.moc"
