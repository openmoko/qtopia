/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qvfbview.h"
#include "qvfbshmem.h"
#include "qvfbmmap.h"

#include "qanimationwriter.h"
#include <QApplication>
#include <QPainter>
#include <QImage>
#include <QBitmap>
#include <QTimer>
#include <QMatrix>
#include <QPaintEvent>
#include <QScrollArea>
#include <QFile>
#include <QDebug>

#include <stdlib.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <fcntl.h>
#include <errno.h>
#include <math.h>

extern int qvfb_protocol;

QVFbView::QVFbView( int id, int w, int h, int d, Rotation r, QWidget *parent )
#ifdef QVFB_USE_GLWIDGET
    : QGLWidget( parent ),
#else
    : QWidget( parent ),
#endif
    viewdepth(d), rsh(0), gsh(0), bsh(0), rmax(15), gmax(15), bmax(15),
    contentsWidth(w), contentsHeight(h), gred(1.0), ggreen(1.0), gblue(1.0),
    gammatable(0), refreshRate(30), animation(0),
    hzm(1.0), vzm(1.0), mView(0),
    emulateTouchscreen(false), emulateLcdScreen(false), rotation(r)
{
    int _w = ( rotation & 0x1 ) ? h : w;
    int _h = ( rotation & 0x1 ) ? w : h;

    switch(qvfb_protocol) {
        default:
        case 0:
            mView = new QShMemViewProtocol(id, QSize(_w, _h), d, this);
            break;
        case 1:
            mView = new QMMapViewProtocol(id, QSize(_w, _h), d, this);
            break;
    }

    connect(mView, SIGNAL(displayDataChanged(const QRect &)),
            SLOT(refreshDisplay(const QRect &)));

    setAttribute(Qt::WA_PaintOnScreen, true);
    setMouseTracking( true );
    setFocusPolicy( Qt::StrongFocus );
    setAttribute( Qt::WA_NoSystemBackground );

    resize( contentsWidth, contentsHeight );

    setGamma(1.0,1.0,1.0);
    mView->setRate( 30 );
}

QVFbView::~QVFbView()
{
    stopAnimation();
    sendKeyboardData( 0, 0, 0, true, false ); // magic die key
}

QSize QVFbView::sizeHint() const
{
    return QSize(contentsWidth, contentsHeight);
}

void QVFbView::setRate(int i)
{
    mView->setRate(i);
}

void QVFbView::setGamma(double gr, double gg, double gb)
{
    if ( viewdepth < 12 )
	return; // not implemented

    gred=gr; ggreen=gg; gblue=gb;

    switch ( viewdepth ) {
      case 12:
	rsh = 12;
	gsh = 7;
	bsh = 1;
	rmax = 15;
	gmax = 15;
	bmax = 15;
	break;
      case 16:
	rsh = 11;
	gsh = 5;
	bsh = 0;
	rmax = 31;
	gmax = 63;
	bmax = 31;
	break;
      case 18:
        rsh = 12;
        gsh = 6;
        bsh = 0;
        rmax = 63;
        gmax = 63;
        bmax = 63;
        break;
      case 24:
      case 32:
	rsh = 16;
	gsh = 8;
	bsh = 0;
	rmax = 255;
	gmax = 255;
	bmax = 255;
    }
    int mm = qMax(rmax,qMax(gmax,bmax))+1;
    if ( gammatable )
	delete [] gammatable;
    gammatable = new QRgb[mm];
    for (int i=0; i<mm; i++) {
	int r = int(pow(i,gr)*255/rmax);
	int g = int(pow(i,gg)*255/gmax);
	int b = int(pow(i,gb)*255/bmax);
	if ( r > 255 ) r = 255;
	if ( g > 255 ) g = 255;
	if ( b > 255 ) b = 255;
	gammatable[i] = qRgb(r,g,b);
//qDebug("%d: %d,%d,%d",i,r,g,b);
    }

    mView->flushChanges();
}

void QVFbView::getGamma(int i, QRgb& rgb)
{
    if ( i > 255 ) i = 255;
    if ( i < 0 ) i = 0;
    rgb = qRgb(qRed(gammatable[i*rmax/255]),
               qGreen(gammatable[i*rmax/255]),
               qBlue(gammatable[i*rmax/255]));
}

int QVFbView::displayId() const
{
    return mView->id();
}

int QVFbView::displayWidth() const
{
    return ( (int)rotation & 0x01 ) ? mView->height() : mView->width();
}

int QVFbView::displayHeight() const
{
    return ( (int)rotation & 0x01 ) ? mView->width(): mView->height();
}

int QVFbView::displayDepth() const
{
    return viewdepth;
}

QVFbView::Rotation QVFbView::displayRotation() const
{
    return rotation;
}

void QVFbView::setZoom( double hz, double vz )
{
    if ( hzm != hz || vzm != vz ) {
	hzm = hz;
	vzm = vz;
        mView->flushChanges();

        contentsWidth = int(displayWidth()*hz);
        contentsHeight = int(displayHeight()*vz);
        resize(contentsWidth, contentsHeight);

	updateGeometry();
	qApp->sendPostedEvents();
	topLevelWidget()->adjustSize();
	update();
    }
}

static QRect mapToDevice( const QRect &r, const QSize &s, QVFbView::Rotation rotation )
{
    int x1 = r.x();
    int y1 = r.y();
    int x2 = r.right();
    int y2 = r.bottom();
    int w = s.width();
    int h = s.height();
    switch ( rotation ) {
	case QVFbView::Rot90:
	    return QRect(
                    QPoint(y1, w - x1 - 1),
                    QPoint(y2, w - x2 - 1) );
	case QVFbView::Rot180:
	    return QRect(
                    QPoint(w - x1 - 1, h - y1 - 1),
                    QPoint(w - x2 - 1, h - y2 - 1) );
	case QVFbView::Rot270:
	    return QRect(
                    QPoint(h - y1 - 1, x1),
                    QPoint(h - y2 - 1, x2) );
	default:
	    break;
    }
    return r;
}

void QVFbView::sendMouseData( const QPoint &pos, int buttons, int wheel )
{
    QPoint p = mapToDevice( QRect(pos,QSize(1,1)), QSize(displayWidth(), displayHeight()), rotation ).topLeft();
    mView->sendMouseData(p, buttons, wheel);
}

void QVFbView::sendKeyboardData( QString unicode, int keycode, int modifiers,
				 bool press, bool repeat )
{
    mView->sendKeyboardData(unicode, keycode, modifiers, press, repeat);
}

void QVFbView::refreshDisplay(const QRect &r)
{
    if ( animation ) {
	    if ( r.isEmpty() ) {
		animation->appendBlankFrame();
	    } else {
		int l;
		QImage img = getBuffer( r, l );
		animation->appendFrame(img,QPoint(r.x(),r.y()));
	    }
    }
    if ( !r.isNull() )
	repaint();
}

QImage QVFbView::getBuffer( const QRect &r, int &leading ) const
{
    static QByteArray buffer;

    const int requiredSize = r.width() * r.height() * 4;

    switch ( viewdepth ) {
    case 12:
    case 16: {
        if (requiredSize > buffer.size())
            buffer.resize(requiredSize);
        uchar *b = reinterpret_cast<uchar*>(buffer.data());
	QImage img(b, r.width(), r.height(), QImage::Format_RGB32);
	const int rsh = viewdepth == 12 ? 12 : 11;
	const int gsh = viewdepth == 12 ? 7 : 5;
	const int bsh = viewdepth == 12 ? 1 : 0;
	const int rmax = viewdepth == 12 ? 15 : 31;
	const int gmax = viewdepth == 12 ? 15 : 63;
	const int bmax = viewdepth == 12 ? 15 : 31;
	for ( int row = 0; row < r.height(); row++ ) {
	    QRgb *dptr = (QRgb*)img.scanLine( row );
	    ushort *sptr = (ushort*)(mView->data() + (r.y()+row)*mView->linestep());
	    sptr += r.x();
	    for ( int col=0; col < r.width(); col++ ) {
		ushort s = *sptr++;
		*dptr++ = qRgb(qRed(gammatable[(s>>rsh)&rmax]),qGreen(gammatable[(s>>gsh)&gmax]),qBlue(gammatable[(s>>bsh)&bmax]));
		//*dptr++ = qRgb(((s>>rsh)&rmax)*255/rmax,((s>>gsh)&gmax)*255/gmax,((s>>bsh)&bmax)*255/bmax);
	    }
	}
	leading = 0;
	return img;
    }
    case 4: {
        if (requiredSize > buffer.size())
            buffer.resize(requiredSize);
        uchar *b = reinterpret_cast<uchar*>(buffer.data());
	QImage img(b, r.width(), r.height(), QImage::Format_Indexed8);
        //img.setColorTable(mView->clut());
	for ( int row = 0; row < r.height(); row++ ) {
	    unsigned char *dptr = img.scanLine( row );
	    const unsigned char *sptr = mView->data() + (r.y()+row)*mView->linestep();
	    sptr += r.x()/2;
	    int col = 0;
	    if ( r.x() & 1 ) {
		*dptr++ = *sptr++ >> 4;
		col++;
	    }
	    for ( ; col < r.width()-1; col+=2 ) {
		unsigned char s = *sptr++;
		*dptr++ = s & 0x0f;
		*dptr++ = s >> 4;
	    }
	    if ( !(r.right() & 1) )
		*dptr = *sptr & 0x0f;
	}
	leading = 0;
	return img;
    }
    case 18: {
         // packed into 24 bpp
        if (requiredSize > buffer.size())
            buffer.resize(requiredSize);
        uchar *b = reinterpret_cast<uchar*>(buffer.data());
	QImage img(b, r.width(), r.height(), QImage::Format_RGB32);
        const int rsh = 12;
        const int gsh = 6;
        const int bsh = 0;
        const int rmax = 63;
        const int gmax = 63;
        const int bmax = 63;
	for ( int row = 0; row < r.height(); row++ ) {
	    QRgb *dptr = (QRgb*)img.scanLine( row );
            uchar *sptr = (uchar*)(mView->data() + (r.y()+row)*mView->linestep());
	    sptr += r.x()*3;
	    for ( int col=0; col < r.width(); col++ ) {
                uint s = *(reinterpret_cast<uint*>(sptr));
                s &= 0x00ffffff;
                sptr += 3;
                *dptr++ = qRgb(qRed(gammatable[(s>>rsh)&rmax]),qGreen(gammatable[(s>>gsh)&gmax]),qBlue(gammatable[(s>>bsh)&bmax]));
	    }
	}
	leading = 0;
	return img;
     }
    case 24: {
        static unsigned char *imgData = 0;
        if (!imgData) {
            int bpl = mView->width() *4;
            imgData = new unsigned char[bpl * mView->height()];
        }
        QImage img(imgData, r.width(), r.height(), QImage::Format_RGB32);
        for (int row = 0; row < r.height(); ++row) {
            uchar *dptr = img.scanLine(row);
            const uchar *sptr = mView->data() + (r.y() + row) * mView->linestep();
            sptr += r.x() * 3;
            for (int col = 0; col < r.width(); ++col) {
                *dptr++ = *sptr++;
                *dptr++ = *sptr++;
                *dptr++ = *sptr++;
                dptr++;
            }
        }
        leading = 0;
        return img;
    }
    case 32: {
	leading = r.x();
	return QImage( mView->data() + r.y() * mView->linestep(),
                       mView->width(), r.height(), QImage::Format_RGB32 );
    }
    case 8: {
        leading = r.x();
        QImage img( mView->data() + r.y() * mView->linestep(),
                    mView->width(), r.height(), QImage::Format_Indexed8 );
        img.setColorTable(mView->clut());
        return img;
    }
    case 1: {
	leading = r.x();
	return QImage( mView->data() + r.y() * mView->linestep(),
                       mView->width(), r.height(), QImage::Format_MonoLSB );
    }
    }
    return QImage();
}

static int findMultiple(int start, double m, int limit, int step)
{
    int r = start;
    while (r != limit) {
	if ( int(int(r * m)/m) == r )
	    break;
	r += step;
    }
    return r;
}

void QVFbView::drawScreen()
{
    QPainter p( this );

    /* later just draw the update */
    QRect r(0, 0, mView->width(), mView->height() );

    if ( int(hzm) != hzm || int(vzm) != vzm ) {
        r.setLeft( findMultiple(r.left(),hzm,0,-1) );
        r.setTop( findMultiple(r.top(),vzm,0,-1) );
        int w = findMultiple(r.width(),hzm,mView->width(),1);
        int h = findMultiple(r.height(),vzm,mView->height(),1);
        r.setRight( r.left()+w-1 );
        r.setBottom( r.top()+h-1 );
    }
    int leading;
    QImage img( getBuffer( r, leading ) );
    QPixmap pm;
    if ( hzm == 1.0 && vzm == 1.0 ) {
        pm = QPixmap::fromImage( img );
    } else if ( emulateLcdScreen && hzm == 3.0 && vzm == 3.0 ) {
        QImage img2( img.width()*3, img.height(), QImage::Format_RGB32 );
        for ( int row = 0; row < img2.height(); row++ ) {
            QRgb *dptr = (QRgb*)img2.scanLine( row );
            QRgb *sptr = (QRgb*)img.scanLine( row );
            for ( int col = 0; col < img.width(); col++ ) {
                QRgb s = *sptr++;
                *dptr++ = qRgb(qRed(s),0,0);
                *dptr++ = qRgb(0,qGreen(s),0);
                *dptr++ = qRgb(0,0,qBlue(s));
            }
        }
        QMatrix m;
        m.scale(1.0, 3.0);
        pm = QPixmap::fromImage( img2 );
        pm = pm.transformed(m);
    } else if ( int(hzm) == hzm && int(vzm) == vzm ) {
        QMatrix m;
        m.scale(hzm,vzm);
        pm = QPixmap::fromImage( img );
        pm = pm.transformed(m);
    } else {
        pm = QPixmap::fromImage( img.scaled(int(img.width()*hzm),int(img.height()*vzm), Qt::IgnoreAspectRatio, Qt::SmoothTransformation) );
    }

    int x1 = r.x();
    int y1 = r.y();
    int leadingX = leading;
    int leadingY = 0;

    // Do the rotation thing
    int rotX1 = mView->width() - x1 - img.width();
    int rotY1 = mView->height() - y1 - img.height();
    int rotLeadingX = (leading) ? mView->width() - leadingX - img.width() : 0;
    int rotLeadingY = 0;
    switch ( rotation ) {
        case Rot0:
            break;
        case Rot90:
            leadingY = leadingX;
            leadingX = rotLeadingY;
            y1 = x1;
            x1 = rotY1;
            break;
        case Rot180:
            leadingX = rotLeadingX;
            leadingY = leadingY;
            x1 = rotX1;
            y1 = rotY1;
            break;
        case Rot270:
            leadingX = leadingY;
            leadingY = rotLeadingX;
            x1 = y1;
            y1 = rotX1;
            break;
        default:
            break;
    }
    x1 = int(x1*hzm);
    y1 = int(y1*vzm);
    leadingX = int(leadingX*hzm);
    leadingY = int(leadingY*vzm);
    if ( rotation != 0 ) {
        QMatrix m;
        m.rotate(rotation * 90.0);
        pm = pm.transformed(m);
    }
    p.setPen( Qt::black );
    p.setBrush( Qt::white );
    p.drawPixmap( x1, y1, pm, leadingX, leadingY, pm.width(), pm.height() );
}

//bool QVFbView::eventFilter( QObject *obj, QEvent *e )
//{
//    if ( obj == this &&
//	 (e->type() == QEvent::FocusIn || e->type() == QEvent::FocusOut) )
//	return true;
//
//    return QWidgetView::eventFilter( obj, e );
//}

void QVFbView::paintEvent( QPaintEvent * /*pe*/ )
{
    /*
    QRect r( pe->rect() );
    r = QRect(int(r.x()/hzm),int(r.y()/vzm),
	    int(r.width()/hzm)+1,int(r.height()/vzm)+1);

    mView->flushChanges();
    */
    drawScreen();
}

void QVFbView::mousePressEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), 0 );
}

void QVFbView::contextMenuEvent( QContextMenuEvent* )
{

}

void QVFbView::mouseDoubleClickEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), 0 );
}

void QVFbView::mouseReleaseEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), 0 );
}

void QVFbView::skinMouseEvent( QMouseEvent *e )
{
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), 0 );
}

void QVFbView::mouseMoveEvent( QMouseEvent *e )
{
    if ( !emulateTouchscreen || (e->buttons() & Qt::MouseButtonMask ) )
	sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), 0 );
}

void QVFbView::wheelEvent( QWheelEvent *e )
{
    if (!e)
        return;
    sendMouseData( QPoint(int(e->x()/hzm),int(e->y()/vzm)), e->buttons(), e->delta());
}

void QVFbView::setTouchscreenEmulation( bool b )
{
    emulateTouchscreen = b;
}

void QVFbView::setLcdScreenEmulation( bool b )
{
    emulateLcdScreen = b;
}

void QVFbView::keyPressEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text(), e->key(),
		     e->modifiers()&(Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier),
		     true, e->isAutoRepeat());
}

void QVFbView::keyReleaseEvent( QKeyEvent *e )
{
    sendKeyboardData(e->text(), e->key(),
		     e->modifiers()&(Qt::ShiftModifier|Qt::ControlModifier|Qt::AltModifier),
		     false, e->isAutoRepeat());
}


QImage QVFbView::image() const
{
    int l;
    QImage r = getBuffer( QRect(0, 0, mView->width(), mView->height()), l ).copy();
    return r;
}

void QVFbView::startAnimation( const QString& filename )
{
    delete animation;
    animation = new QAnimationWriter(filename,"MNG");
    animation->setFrameRate(refreshRate);
    animation->appendFrame(QImage(mView->data(),
                mView->width(), mView->height(), QImage::Format_RGB32));
}

void QVFbView::stopAnimation()
{
    delete animation;
    animation = 0;
}


void QVFbView::skinKeyPressEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyPress,code,0,text,autorep);
    keyPressEvent(&e);
}

void QVFbView::skinKeyReleaseEvent( int code, const QString& text, bool autorep )
{
    QKeyEvent e(QEvent::KeyRelease,code,0,text,autorep);
    keyReleaseEvent(&e);
}
