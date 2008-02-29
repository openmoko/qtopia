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

#include "phonedecoration_p.h"
#include "resource.h"
#include "config.h"
#include "qpeapplication.h"
#include <qpainter.h>
#include <qobjectlist.h>
#include <qpixmapcache.h>

static QWidget *topChild(QWidget *w, bool bottom)
{
    if (w && w->testWState(Qt::WState_Visible)) {
	const QObjectList *childObjects=w->children();
	if (childObjects) {
	    QObject *o;
	    QObjectListIt it(*childObjects);
	    it.toLast();
	    while ((o = it.current()) != 0) {
		--it;
		if (o->isWidgetType()) {
		    QWidget *cw = (QWidget*)o;
/*
		    if (cw->isVisible() && !bottom)
			qDebug("pos of %s (%s) is %d, %d, %d x %d, nb: %d",
			    o->className(), o->name(), cw->x(), cw->y(),
			    cw->width(), cw->height(), cw->backgroundMode() != QWidget::NoBackground);
*/
		    if (cw->isVisible() && cw->backgroundMode() != QWidget::NoBackground) {
			if ((!bottom && cw->y() < 3 && cw->x() < 20) ||
			    (bottom && w->height() - cw->geometry().bottom() - 1 < 3 && cw->x() < 20)) {
				return topChild(cw, bottom);
			}
		    }
		}
	    }
	}
    }

    return w;
}

static QPixmap colorizeImage(QImage img, const QColor &col)
{
    QRgb *rgb;
    int count;
    if (img.depth() == 32) {
	rgb = (QRgb*)img.bits();
	count = img.bytesPerLine()/sizeof(QRgb)*img.height();
    } else {
	rgb = img.colorTable();
	count = img.numColors();
    }
    int sr, sg, sb;
    col.rgb(&sr, &sg, &sb);
    for (int r = 0; r < count; r++, rgb++) {
	int tmp = (*rgb >> 16) & 0xff;
	int r = ((sr+tmp)/2);
	tmp = (*rgb >> 8) & 0xff;
	int g = ((sg+tmp)/2);
	tmp = *rgb & 0xff;
	int b = ((sb+tmp)/2);
	*rgb = qRgba(r, g, b, qAlpha(*rgb));
    }
    QPixmap pm;
    pm.convertFromImage(img);
    return pm;
}

static struct {
    const char *name;
    QColorGroup::ColorRole role;
} colorTable[] = {
    { "Foreground", QColorGroup::Foreground },
    { "Button", QColorGroup::Button },
    { "Light", QColorGroup::Light },
    { "Midlight", QColorGroup::Midlight },
    { "Dark", QColorGroup::Dark },
    { "Mid", QColorGroup::Mid },
    { "Text", QColorGroup::Text },
    { "BrightText", QColorGroup::BrightText },
    { "ButtonText", QColorGroup::ButtonText },
    { "Base", QColorGroup::Base },
    { "Background", QColorGroup::Background },
    { "Shadow", QColorGroup::Shadow },
    { "Highlight", QColorGroup::Highlight },
    { "HighlightedText", QColorGroup::HighlightedText },
    { "Widget", QColorGroup::NColorRoles },
    { 0, QColorGroup::Foreground }
};

static int parseColor(const QString &val, QColor &col)
{
    int role = QColorGroup::NColorRoles;
    if (!val.isEmpty()) {
	int i = 0;
	while (colorTable[i].name) {
	    if (QString(colorTable[i].name).lower() == val.lower()) {
		role = colorTable[i].role;
		break;
	    }
	    i++;
	}
	if (!colorTable[i].name) {
	    role = QColorGroup::NColorRoles+1;
	    col.setNamedColor(val);
	}
    }

    return role;
}

static QColor getColor(const QColorGroup &cg, int role, const QColor &col)
{
    if (role < QColorGroup::NColorRoles)
	return cg.color((QColorGroup::ColorRole)role);
    else if (role == QColorGroup::NColorRoles)
	return QColor();
    else
	return col;
}

static const char *metricTable[] = {
    "TitleHeight",
    "LeftBorder",
    "RightBorder",
    "TopBorder",
    "BottomBorder",
    "OKWidth",
    "CloseWidth",
    "HelpWidth",
    "MaximizeWidth",
    "CornerGrabSize",
    0
};

class DecorationBorderData
{
public:
    QPixmap pix;
    QRegion rgn[3];
    int offs[4];

    void read(Config &config, const QString &base, const QString &name, Qt::Orientation, bool colorize=FALSE);
};

void DecorationBorderData::read(Config &cfg, const QString &base, const QString &name, Qt::Orientation orient, bool colorize)
{
    QString val = cfg.readEntry(name+"Pixmap");
    if (val.isEmpty())
	return;
    if (colorize) {
	QColor col = QApplication::palette().color(QPalette::Active, QColorGroup::Highlight);
	QString cv = cfg.readEntry(name+"Color");
	if (!cv.isEmpty()) {
	    int role = parseColor(cv, col);
	    col = getColor(QApplication::palette().active(), role, col);
	}
	QString key = base+val+col.name();
	// Cache only makes sense with shared pixmap cache enabled.
	if (!QPixmapCache::find(key, pix)) {
	    pix = colorizeImage(Resource::loadImage(base+val), col);
	    QPixmapCache::insert(key, pix);
	}
    } else {
	pix = Resource::loadPixmap(base+val);
    }

    QStringList offlist = cfg.readListEntry(name+"Offsets", ',');
    offs[0] = 0;
    if (offlist.count()) {
	for (int i = 0; i < (int)offlist.count() && i < 3; i++)
	    offs[i+1] = offlist[i].toInt();
    } else {
	offs[1] = 0;
	offs[2] = pix.width();
    }
    offs[3] = pix.width();

    const QBitmap *mask = pix.mask();
    QBitmap bm;
    val = cfg.readEntry(name+"Mask");
    if (!val.isEmpty()) {
	bm = Resource::loadBitmap(base+val);
	mask = &bm;
    }

    if (mask && !mask->isNull()) {
	QRegion r = QRegion(*mask);
	if (orient == Qt::Horizontal) {
	    for (int i=0; i < 3; i++ )
		rgn[i] = r & QRegion(offs[i],0,offs[i+1]-offs[i],mask->height());
	} else {
	    for (int i=0; i < 3; i++ )
		rgn[i] = r & QRegion(0, offs[i],mask->width(),offs[i+1]-offs[i]);
	}
    }
}

class DecorationData
{
public:
    DecorationData();

    void read(Config &config, const QString &base);

    enum Type { Title, Overlay, Left, Bottom, Right };
    const DecorationBorderData &area(Type t) const {
	return data[t];
    }

    int metrics[WindowDecorationInterface::CornerGrabSize+1];

    QRect titleTextRect(int width) const;
    int titleTextAlignment() const;

    QColor textColor(const QColorGroup &cg) const;
    QColor borderColor(const QColorGroup &cg) const;

private:
    DecorationBorderData data[5];
    int textColorRole;
    QColor textColorVal;
    int borderColorRole;
    QColor borderColorVal;
    QPoint tlOffset;
    QPoint brOffset;
    int titleAlign;
};

DecorationData::DecorationData()
{
}

void DecorationData::read(Config &cfg, const QString &base)
{
    int i = 0;
    while (metricTable[i]) {
	metrics[i] = cfg.readNumEntry(metricTable[i], 0);
	i++;
    }

    data[Title].read(cfg, base, "Title", Qt::Horizontal);
    data[Overlay].read(cfg, base, "Overlay", Qt::Horizontal, TRUE);
    data[Left].read(cfg, base, "Left", Qt::Vertical);
    data[Right].read(cfg, base, "Right", Qt::Vertical);
    data[Bottom].read(cfg, base, "Bottom", Qt::Horizontal);
    
    QString tc = cfg.readEntry("BorderColor", "Widget");
    borderColorRole = parseColor(tc, borderColorVal);
    
    tc = cfg.readEntry("TitleTextColor", "HighlightedText");
    textColorRole = parseColor(tc, textColorVal);

    titleAlign = Qt::AlignHCenter | Qt::AlignVCenter;
    QString val = cfg.readEntry("TitleTextAlignment");
    if (!val.isEmpty()) {
	QStringList list = QStringList::split(',', val);
	QStringList::Iterator it;
	for (it = list.begin(); it != list.end(); ++it) {
	    val = *it;
	    if (val == "right")
		titleAlign = (titleAlign & 0x38) | Qt::AlignRight;
	    else if (val == "left")
		titleAlign = (titleAlign & 0x38) | Qt::AlignLeft;
	    else if (val == "bottom")
		titleAlign = (titleAlign & 0x07) | Qt::AlignBottom;
	    else if (val == "top")
		titleAlign = (titleAlign & 0x07) | Qt::AlignTop;
	}
    }

    val = cfg.readEntry("TitleTextPosition");
    if (!val.isEmpty()) {
	QStringList list = QStringList::split(',', val);
	tlOffset.setX(list[0].toInt());
	tlOffset.setY(list[1].toInt());
	brOffset.setX(list[2].toInt());
	brOffset.setY(list[3].toInt());
    }
}


QColor DecorationData::textColor(const QColorGroup &cg) const
{
    return getColor(cg, textColorRole, textColorVal);
}

QColor DecorationData::borderColor(const QColorGroup &cg) const
{
    return getColor(cg, borderColorRole, borderColorVal);
}

QRect DecorationData::titleTextRect(int width) const
{
    QRect r;

    r.setLeft(tlOffset.x());
    r.setTop(tlOffset.y());
    r.setRight(width-brOffset.x());
    r.setBottom(metrics[WindowDecorationInterface::TitleHeight]-brOffset.y());

    return r;
}

int DecorationData::titleTextAlignment() const
{
    return titleAlign;
}

class PhoneDecorationPrivate
{
public:
    PhoneDecorationPrivate() {
	Config cfg("qpe");
	cfg.setGroup("Appearance");
	QString theme = cfg.readEntry("DecorationTheme", "qtopia/decorationrc");

	QString themeDir = QPEApplication::qpeDir() + "etc/themes/";
	QString themeFile = themeDir + theme;
	Config dcfg(themeFile, Config::File);
	dcfg.setGroup("Decoration");
	QString base = dcfg.readEntry("Base");
	if (!!base && base[base.length()-1] != '/')
	    base += '/';

	dcfg.setGroup("Maximized");
	maxData.read(dcfg, base);
	dcfg.setGroup("Normal");
	normalData.read(dcfg, base);
    }

    const DecorationData &data(const WindowDecorationInterface::WindowData *wd) {
	return (wd->flags & WindowDecorationInterface::WindowData::Maximized) ? maxData : normalData;
    }

private:
    DecorationData maxData;
    DecorationData normalData;
};

PhoneDecoration::PhoneDecoration() : ref(0)
{
    d = new PhoneDecorationPrivate;
}

PhoneDecoration::~PhoneDecoration()
{
    delete d;
}

int PhoneDecoration::metric( Metric m, const WindowData *wd ) const
{
    return d->data(wd).metrics[m];
}

void PhoneDecoration::drawArea( Area a, QPainter *p, const WindowData *wd ) const
{
    int th = metric( TitleHeight, wd );
    QRect r = wd->rect;

    switch ( a ) {
	case Border:
	    {
		QColorGroup cg = wd->palette.active();
		p->setBrush(d->data(wd).borderColor(cg));
		p->setPen( cg.foreground() );
		int lb = metric(LeftBorder,wd);
		int rb = metric(RightBorder,wd);
		//int tb = metric(TopBorder,wd);
		int bb = metric(BottomBorder,wd);
		/*
		p->drawRect( r.x()-lb, r.y()-tb-th, r.width()+lb+rb,
			     r.height()+th+tb );
		*/

		QBrush b = cg.brush(QColorGroup::Background);
		const DecorationBorderData *data;

		// left border
		if (lb) {
		    data = &d->data(wd).area(DecorationData::Left);
		    if (!data->pix.isNull()) {
			QRect tr(r.x()-lb, r.y(), lb, r.height());
			drawStretch(p, tr, data, b, Qt::Vertical);
		    }
		}

		// right border
		if (rb) {
		    data = &d->data(wd).area(DecorationData::Right);
		    if (!data->pix.isNull()) {
			QRect tr(r.right()+1, r.y(), rb, r.height());
			drawStretch(p, tr, data, b, Qt::Vertical);
		    }
		}

		// bottom border
		data = &d->data(wd).area(DecorationData::Bottom);
		if (bb) {
		    QRect tr(r.x()-lb, r.height(), r.width()+lb+rb, bb);
		    QColor c = d->data(wd).borderColor(cg);
		    if (!c.isValid() && p->device()->devType() == QInternal::Widget) {
			QWidget *widget = (QWidget *)p->device();
			QWidget *tc = topChild(widget, TRUE);
			switch (tc->backgroundMode()) {
			    case QWidget::PaletteBackground:
				b = cg.brush(QColorGroup::Background);
				break;
			    case QWidget::PaletteBase:
				b = cg.brush(QColorGroup::Base);
				break;
			    case QWidget::PaletteButton:
				b = cg.brush(QColorGroup::Button);
				break;
			    case QWidget::FixedColor:
				b = tc->backgroundColor();
				break;
			    case QWidget::FixedPixmap:
				if (tc->backgroundPixmap())
				    b = QBrush(Qt::white, *tc->backgroundPixmap());
				break;
			    default:
				qDebug("Unhandled background mode in decoration");
				break;
			}
//    			qDebug("Using top widget %s (%s)", tc->className(), tc->name());
		    } else {
			b = QBrush(c);
		    }
		    if (!data->pix.isNull())
			drawStretch(p, tr, data, b, Qt::Horizontal);
		    else
			p->fillRect(tr, b);
		}
	    }
	    break;
	case Title:
	    if (r.height() >= 2) {
		int lb = metric(LeftBorder,wd);
		int rb = metric(RightBorder,wd);

		const DecorationBorderData *data = &d->data(wd).area(DecorationData::Title);
		const DecorationBorderData *odata = &d->data(wd).area(DecorationData::Overlay);

		QRect tr(r.x()-lb,r.y()-th,r.width()+lb+rb,th);
		if (!data->pix.isNull() || !odata->pix.isNull()) {
		    QColorGroup cg = wd->palette.active();
		    QBrush b = cg.brush(QColorGroup::Background);
		    if (p->device()->devType() == QInternal::Widget) {
			QWidget *widget = (QWidget *)p->device();
			QWidget *tc = topChild(widget, FALSE);
			cg = tc->palette().active();
			switch (tc->backgroundMode()) {
			    case QWidget::PaletteBackground:
				b = cg.brush(QColorGroup::Background);
				break;
			    case QWidget::PaletteBase:
				b = cg.brush(QColorGroup::Base);
				break;
			    case QWidget::PaletteButton:
				b = cg.brush(QColorGroup::Button);
				break;
			    case QWidget::FixedColor:
				b = tc->backgroundColor();
				break;
			    case QWidget::FixedPixmap:
				if (tc->backgroundPixmap())
				    b = QBrush(Qt::white, *tc->backgroundPixmap());
				break;
			    default:
				break;
			}
//			qDebug("Using top widget %s (%s)", tc->className(), tc->name());
		    }
		    if (!data->pix.isNull()) {
			drawStretch(p, tr, data, b, Qt::Horizontal);
			b = QBrush(QBrush::NoBrush);
		    }
		    if (!odata->pix.isNull())
			drawStretch(p, tr, odata, b, Qt::Horizontal);
		} else {
		    const QColorGroup &cg = wd->palette.active();
		    QColor c = cg.color(QColorGroup::Highlight);
		    p->fillRect( tr, c );
		}
	    } else if ( r.height() < 2 ) {
		WindowDecorationInterface::drawArea( a, p, wd );
	    }
	    break;
	case TitleText:
	    if (!(wd->flags & WindowDecorationInterface::WindowData::Maximized)) {
                //use current font or we run into trouble if non Western language
                QFont f( QApplication::font() ); 
                f.setWeight( QFont::Bold ); 
                f.setPointSize( 12 );
                p->setFont(f);
		if ( !(wd->flags & WindowData::Active) )
		    p->setPen(d->data(wd).textColor(wd->palette.inactive()));
		else
		    p->setPen(d->data(wd).textColor(wd->palette.active()));
		QRect tr = d->data(wd).titleTextRect(r.width());
		p->drawText( r.left()+tr.left(), r.top()-th+tr.top(),
				tr.width(), tr.height(), 
			d->data(wd).titleTextAlignment(), wd->caption );
	    }
	    break;
	default:
	    WindowDecorationInterface::drawArea( a, p, wd );
	    break;
    }
}

void PhoneDecoration::drawButton( Button , QPainter* , const WindowData* ,
    int , int , int /* w */, int /* h */, QWSButton::State ) const
{
}

QRegion PhoneDecoration::mask( const WindowData *wd ) const
{
    QRegion mask;

    int th = metric( TitleHeight, wd );
    int lb = metric( LeftBorder, wd );
    int rb = metric( RightBorder, wd );
    int bb = metric( BottomBorder, wd );
    QRect r(wd->rect);

    // top
    const DecorationBorderData *data = &d->data(wd).area(DecorationData::Title);
    if (data->pix.isNull())
	data = &d->data(wd).area(DecorationData::Overlay);
    QRect tr(r.x()-lb,r.y()-th,r.width()+rb+lb,th);
    mask += maskStretch(tr, data, Qt::Horizontal);

    // left
    if (lb) {
	data = &d->data(wd).area(DecorationData::Left);
	tr.setRect(r.x()-lb,r.y(),lb,r.height());
	mask += maskStretch(tr, data, Qt::Vertical);
    }

    // right
    if (rb) {
	data = &d->data(wd).area(DecorationData::Right);
	tr.setRect(r.right()+1,r.y(),rb,r.height());
	mask += maskStretch(tr, data, Qt::Vertical);
    }

    // bottom
    if (bb) {
	data = &d->data(wd).area(DecorationData::Bottom);
	tr.setRect(r.x()-lb, r.bottom()+1, r.width()+lb+rb, bb);
	mask += maskStretch(tr, data, Qt::Horizontal);
    }

    return mask;
}

QString PhoneDecoration::name() const
{
    return qApp->translate( "Decoration", "Phone" );
}

QPixmap PhoneDecoration::icon() const
{
    return QPixmap();
}

void PhoneDecoration::drawStretch(QPainter *p, const QRect &r, const DecorationBorderData *data,
				const QBrush &b, Qt::Orientation orient) const
{
    p->fillRect(r, b);
    if (data->pix.isNull())
	return;

    const int *o = data->offs;
    int ss = o[2]-o[1];
    if (orient == Qt::Horizontal) {
	int h = data->pix.height();
	p->drawPixmap(r.x(), r.y(), data->pix, 0, 0, o[1], h);
	int w = r.width() - o[1] - (o[3]-o[2]);
	int x = 0;
	if (ss) {
	    for (; x < w-ss; x+=ss)
		p->drawPixmap(r.x()+o[1]+x, r.y(), data->pix, o[1], 0, ss);
	}
	p->drawPixmap(r.x()+o[1]+x, r.y(), data->pix, o[1], 0, w-x);
	p->drawPixmap(r.x()+r.width()-(o[3]-o[2]), r.y(), data->pix, o[2], 0, o[3]-o[2], h);
    } else {
	int w = data->pix.width();
	p->drawPixmap(r.x(), r.y(), data->pix, 0, 0, w, o[1]);
	int h = r.height() - o[1] - (o[3]-o[2]);
	int y = 0;
	if (ss) {
	    for (; y < h-ss; y+=ss)
		p->drawPixmap(r.x(), r.y()+o[1]+y, data->pix, 0, o[1], w, ss);
	}
	p->drawPixmap(r.x(), r.y()+o[1]+y, data->pix, 0, o[1], w, h-y);
	p->drawPixmap(r.x(), r.y()+r.height()-(o[3]-o[2]), data->pix, 0, o[2], w, o[3]-o[2]);
    }
}

QRegion PhoneDecoration::maskStretch(const QRect &r, const DecorationBorderData *data, Qt::Orientation orient) const
{
    if (data->rgn[0].isEmpty() && data->rgn[1].isEmpty() && data->rgn[2].isEmpty())
	return QRegion(r);

    const int *o = data->offs;
    QRegion mask;
    QRegion trgn;

    if (orient == Qt::Horizontal) {
	trgn = data->rgn[0].isEmpty() ? QRect(0, 0, o[1], data->pix.height()) : data->rgn[0];
	trgn.translate(r.x(), r.y());
	mask = trgn;

	int w = r.width() - o[1] - (o[3]-o[2]);
	if (!data->rgn[1].isNull()) {
	    QRegion tmp(data->rgn[1]);
	    tmp.translate(r.x(), r.y());
	    for (int x = 0; x < w; x+=o[2]-o[1]) {
		trgn += tmp;
		tmp.translate(o[2]-o[1], 0);
	    }
	    trgn &= QRect(r.x()+o[1], r.y(), w, r.height());
	} else {
	    trgn = QRect(r.x()+o[1], r.y(), w, r.height());
	}
	mask += trgn;

	trgn = data->rgn[2].isEmpty() ? QRect(o[2], 0, o[3]-o[2], data->pix.height()) : data->rgn[2];
	trgn.translate(r.x()+r.width()-o[3],r.y());
	mask += trgn;
    } else {
	trgn = data->rgn[0].isEmpty() ? QRect(0, 0, data->pix.width(), o[1]) : data->rgn[0];
	trgn.translate(r.x(), r.y());
	mask = trgn;

	int h = r.height() - o[1] - (o[3]-o[2]);
	if (!data->rgn[1].isNull()) {
	    QRegion tmp(data->rgn[1]);
	    tmp.translate(r.x(), r.y());
	    for (int y = 0; y < h; y+=o[2]-o[1]) {
		trgn += tmp;
		tmp.translate(0, o[2]-o[1]);
	    }
	    trgn &= QRect(r.x(), r.y()+o[1], r.width(), h);
	} else {
	    trgn = QRect(r.x(), r.y()+o[1], r.width(), h);
	}
	mask += trgn;

	trgn = data->rgn[2].isEmpty() ? QRect(0, o[2], data->pix.width(), o[3]-o[2]) : data->rgn[2];
	trgn.translate(r.x(), r.y()+r.height()-o[3]);
	mask += trgn;
    }

    return mask;
}


QRESULT PhoneDecoration::queryInterface( const QUuid &uuid, QUnknownInterface **iface )
{
    *iface = 0;
    if ( uuid == IID_QUnknown )
	*iface = this;
    else if ( uuid == IID_WindowDecoration )
	*iface = this;
    else
	return QS_FALSE;

    (*iface)->addRef();
    return QS_OK;
}
