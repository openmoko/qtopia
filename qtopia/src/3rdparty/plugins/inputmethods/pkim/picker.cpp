/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
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
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "picker.h"
#include <qtopia/qcopenvelope_qws.h>
#include <qtopia/config.h>
#include <qtopia/qpeapplication.h>
#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qtimer.h>
#include <qstyle.h>

/* hack to let us into the functions, as it has no data, cast to parent is safe */
class PickerScrollBar : public QScrollBar
{
public:
    PickerScrollBar( QWidget *parent, const char *name=0 )
	: QScrollBar( parent, name ) {}
    PickerScrollBar( Orientation o, QWidget *parent, const char *name=0 )
	: QScrollBar( o, parent, name ) {}
    PickerScrollBar( int minValue, int maxValue, int LineStep, int PageStep,
		int value, Orientation o,
		QWidget *parent, const char *name=0 )
	: QScrollBar( minValue, maxValue, LineStep, PageStep, value,
		o, parent, name) {}

    void passMousePressEvent(QMouseEvent *e)
    {
	QScrollBar::mousePressEvent(e);
    }
    void passMouseMoveEvent(QMouseEvent *e)
    {
	QScrollBar::mouseMoveEvent(e);
    }
    void passMouseReleaseEvent(QMouseEvent *e)
    {
	QScrollBar::mouseReleaseEvent(e);
    }
};

Picker::Picker(QWidget *parent, const char *name, WFlags f)
    : QScrollView(parent, name, f | WStyle_Customize | WStyle_Tool | WStyle_StaysOnTop ),
      grabber(None), nCols(1), nRows(1), cHeight(10), cWidth(10),
      selRow(0), selCol(0), havePress(FALSE)
{
    appFont = qApp->font();
    viewport()->setBackgroundMode( QWidget::PaletteBase );
    setMargin(2);
}

void Picker::setNumRows(int n)
{
    nRows = n;
    updateContentsSize();
}

void Picker::setNumCols(int n)
{
    nCols = n;
    updateContentsSize();
}

void Picker::setCellHeight(int h)
{
    cHeight = h;
    updateContentsSize();
}

void Picker::setCellWidth(int w)
{
    cWidth = w;
    updateContentsSize();
}

void Picker::updateContentsSize()
{
    resizeContents(cWidth*nCols, cHeight*nRows);
    QSize sBarE = style().scrollBarExtent();

    int w = qApp->desktop()->width() - sBarE.width() - 2*frameWidth();
    int h = qApp->desktop()->height() - sBarE.height() - 2*frameWidth();
    h = QMIN(qApp->desktop()->height()/2, h); // height is special.

    bool needHScroll = contentsWidth() > w;
    bool needVScroll = contentsHeight() > h;

    if (needHScroll && !needVScroll) {
	h = contentsHeight()+sBarE.height();
    } else if (needVScroll && !needHScroll) {
	w = contentsWidth()+sBarE.width();
    } else if (!needVScroll && !needHScroll) {
	w = contentsWidth();
	h = contentsHeight();
    }

    resize(w+frameWidth()*2, h+frameWidth()*2);
}

Picker::~Picker()
{
}

// do press and release for proper click handling.
void Picker::contentsMouseReleaseEvent(QMouseEvent *e) 
{
    int row, col;
    col = e->pos().x() / cWidth;
    row = e->pos().y() / cHeight;

    hide();
    if (row >= 0 && row < nRows && col >= 0 && col < nCols)
	emit cellClicked(row, col);
}

void Picker::mousePressEvent(QMouseEvent *e)
{
    QPoint gp = e->globalPos();
    QPoint lp = e->pos();
    PickerScrollBar *vsb = (PickerScrollBar *)verticalScrollBar();
    PickerScrollBar *hsb = (PickerScrollBar *)horizontalScrollBar();

    if ( vsb->isVisible() && vsb->frameGeometry().contains(lp) ) {
	QMouseEvent ce(e->type(), vsb->mapFrom(this, lp),
		gp, e->button(), e->state());

	vsb->passMousePressEvent(&ce);
	grabber = VScroll;
    } else if ( hsb->isVisible() && hsb->frameGeometry().contains(lp) ) {
	QMouseEvent ce(e->type(), hsb->mapFrom(this, lp),
		gp, e->button(), e->state());

	hsb->passMousePressEvent(&ce);
	grabber = HScroll;
    } else if (viewport()->frameGeometry().contains(lp)) {
	QPoint vp = viewport()->mapFrom(this, lp);
	QMouseEvent ce(e->type(), viewportToContents(vp),
		e->globalPos(), e->button(), e->state());
	contentsMousePressEvent(&ce);
	grabber = View;
    } else {
	grabber = OutSide;
    }
}

void Picker::mouseMoveEvent(QMouseEvent *e)
{
    QPoint gp = e->globalPos();
    QPoint lp = e->pos();
    PickerScrollBar *vsb = (PickerScrollBar *)verticalScrollBar();
    PickerScrollBar *hsb = (PickerScrollBar *)horizontalScrollBar();

    switch (grabber) {
	default:
	    break;
	case OutSide:
	    if (e->pos().x() < 0 || e->pos().y() < 0
		    || e->pos().x() > width() || e->pos().y() > height())
		grabber = None;
	    break;
	case VScroll:
	    {
		QMouseEvent ce(e->type(), vsb->mapFrom(this, lp),
			gp, e->button(), e->state());

		vsb->passMouseMoveEvent(&ce);
	    }
	    break;
	case HScroll:
	    {
		QMouseEvent ce(e->type(), hsb->mapFrom(this, lp),
			gp, e->button(), e->state());

		hsb->passMouseMoveEvent(&ce);
	    }
	    break;
	case View:
	    {
		QPoint vp = viewport()->mapFrom(this, lp);
		QMouseEvent ce(e->type(), viewportToContents(vp),
			gp, e->button(), e->state());
		contentsMouseMoveEvent(&ce);
	    }
	    break;
    }
}

void Picker::mouseReleaseEvent(QMouseEvent *e)
{
    QPoint gp = e->globalPos();
    QPoint lp = e->pos();
    PickerScrollBar *vsb = (PickerScrollBar *)verticalScrollBar();
    PickerScrollBar *hsb = (PickerScrollBar *)horizontalScrollBar();

    switch (grabber) {
	default:
	    break;
	case OutSide:
	    if (e->pos().x() < 0 || e->pos().y() < 0
		    || e->pos().x() > width() || e->pos().y() > height()) {
		hide();
	    }
	    break;
	case VScroll:
	    {
		QMouseEvent ce(e->type(), vsb->mapFrom(this, lp),
			gp, e->button(), e->state());

		vsb->passMouseReleaseEvent(&ce);
	    }
	    break;
	case HScroll:
	    {
		QMouseEvent ce(e->type(), hsb->mapFrom(this, lp),
			gp, e->button(), e->state());

		hsb->passMouseReleaseEvent(&ce);
	    }
	    break;
	case View:
	    {
		QPoint vp = viewport()->mapFrom(this, lp);
		QMouseEvent ce(e->type(), viewportToContents(vp),
			e->globalPos(), e->button(), e->state());
		contentsMouseReleaseEvent(&ce);
	    }
	    break;
    }
    grabber = None;
}

bool Picker::filter(int, int keycode, int,
                            bool isPress, bool)
{
    if (isPress) {
	havePress = TRUE;
    } else if (havePress) {
	havePress = FALSE;
	int oldSelRow = selRow;
	int oldSelCol = selCol;
	switch (keycode) {
#ifdef QTOPIA_PHONE
	    case Key_Back:
		return FALSE;
	    case Key_Select:
		hide();
		emit cellClicked(selRow, selCol);
		return TRUE;
#else 
	    case Key_BackSpace:
	    case Key_Delete:
	    case Key_Escape:
		return FALSE;
	    case Key_Space:
		hide();
		emit cellClicked(selRow, selCol);
		return TRUE;
#endif
	    case Key_Up:
		selRow--;
		if (selRow < 0)
		    selRow = nRows-1;
		break;
	    case Key_Down:
		selRow++;
		if (selRow >= nRows)
		    selRow = 0;
		break;
	    case Key_Left:
		selCol--;
		if (selCol < 0)
		    selCol = nCols-1;
		break;
	    case Key_Right:
		selCol++;
		if (selCol >= nCols)
		    selCol = 0;
		break;
	}

	if (oldSelRow != selRow || oldSelCol != selCol) {
	    repaintContents(oldSelCol*cWidth, oldSelRow*cHeight, cWidth, cHeight);
	    repaintContents(selCol*cWidth, selRow*cHeight, cWidth, cHeight);
	    ensureVisible(selCol*cWidth+cWidth/2,
		    selRow*cHeight+cHeight/2, cWidth/2, cHeight/2);
	}
    }

    return TRUE;
}

void Picker::setMicroFocus( int, int y )
{
    if (y < (qApp->desktop()->height() + appFont.pointSize())/2 + 5)
	move((qApp->desktop()->width()-width())/2, y+2);
    else
	move((qApp->desktop()->width()-width())/2, y-appFont.pointSize()-4-height());
}

void Picker::drawFrame(QPainter *p)
{
    QScrollView::drawFrame(p);
    const QColorGroup &g = colorGroup();
    style().drawFocusRect(p, QRect(frameWidth()-margin(),
		frameWidth()-margin(),width()-frameWidth()*2+2*margin(),
		height()-frameWidth()*2+2*margin()), g, &g.highlight());
}

void Picker::drawContents(QPainter *p, int, int cy, int, int ch)
{
    int row = cy/cHeight;
    int y = row*cHeight;
    while (y < cy+ch && row < nRows) {
	p->drawLine( 0, y, nCols*cWidth, y );
	for ( int i = 0; i < nCols; i++ ) {
	    bool sel = ( row == selRow && i == selCol );
	    // Translate painter and draw the cell
	    int colp = i * cWidth; 
	    int rowp = row * cHeight;
	    p->save();
	    p->translate( colp, rowp );
	    drawCell( p, row, i, sel );
	    p->restore();

	}
	// fill in the little bit of space at the end.
	p->fillRect(nCols*cWidth, y, cWidth, cHeight, colorGroup().base());
	row++;
	y += cHeight;
    }
}

void Picker::showEvent(QShowEvent *ev)
{
    QWidget::showEvent(ev);

#ifdef QTOPIA_PHONE
    // Danger, danger, danger!  Direct manipulation of the context bar.
    int key = Key_Back;
    QString pm("contextbar/back");
    {
	QCopEnvelope e("QPE/ContextBar", "setLabelPixmap(int,QString)");
	e << key;
	e << pm;
    }

    key = Key_Select;
    pm = "contextbar/select";
    {
	QCopEnvelope e("QPE/ContextBar", "setLabelPixmap(int,QString)");
	e << key;
	e << pm;
    }

    key = Key_Context1;
    {
	QCopEnvelope e("QPE/ContextBar", "clearLabel(int)");
	e << key;
    }
    {
	key = 1;
	QCopEnvelope e("QPE/ContextBar", "blockUpdates(int)");
	e << key;
    }
#endif

    selCol = 0;
    selRow = 0;
    setContentsPos(0, 0);
    grabMouse(QCursor());
    grabKeyboard();
}

void Picker::hideEvent(QHideEvent *ev)
{
    int b = 0;
    {
	QCopEnvelope e( "QPE/ContextBar", "blockUpdates(int)");
	e << b;
    }
    {
	QCopEnvelope e( "QPE/System", "updateContextBar()");
    }
    releaseMouse();
    releaseKeyboard();
    QWidget::hideEvent(ev);
}


