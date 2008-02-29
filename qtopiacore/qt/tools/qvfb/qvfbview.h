/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QVFBVIEW_H
#define QVFBVIEW_H

//#ifdef QT_NO_OPENGL
#include <QWidget>
//#else
//#define QVFB_USE_GLWIDGET
//#include <QGLWidget>
//#endif

class QImage;
class QTimer;
class QAnimationWriter;
struct QVFbHeader;
class QVFbViewProtocol;

class QVFbView :
#ifdef QVFB_USE_GLWIDGET
    public QGLWidget
#else
    public QWidget
#endif
{
    Q_OBJECT
public:
    enum Rotation { Rot0, Rot90, Rot180, Rot270 };
    QVFbView( int id, int w, int h, int d, Rotation r, QWidget *parent = 0);
    virtual ~QVFbView();

    int displayId() const;
    int displayWidth() const;
    int displayHeight() const;
    int displayDepth() const;
    Rotation displayRotation() const;

    bool touchScreenEmulation() const { return emulateTouchscreen; }
    bool lcdScreenEmulation() const { return emulateLcdScreen; }
    int rate() { return refreshRate; }
    bool animating() const { return !!animation; }
    QImage image() const;

    void setGamma(double gr, double gg, double gb);
    double gammaRed() const { return gred; }
    double gammaGreen() const { return ggreen; }
    double gammaBlue() const { return gblue; }
    void getGamma(int i, QRgb& rgb);
    void skinKeyPressEvent( int code, const QString& text, bool autorep=FALSE );
    void skinKeyReleaseEvent( int code, const QString& text, bool autorep=FALSE );
    void skinMouseEvent( QMouseEvent *e );

    double zoomH() const { return hzm; }
    double zoomV() const { return vzm; }

    QSize sizeHint() const;
    void setRate(int);

public slots:
    void setTouchscreenEmulation( bool );
    void setLcdScreenEmulation( bool );
    void setZoom( double, double );
    void startAnimation( const QString& );
    void stopAnimation();

protected slots:
    void refreshDisplay(const QRect &);

protected:
    QImage getBuffer( const QRect &r, int &leading ) const;
    void drawScreen();
    void sendMouseData( const QPoint &pos, int buttons, int wheel );
    void sendKeyboardData( QString unicode, int keycode, int modifiers,
			   bool press, bool repeat );
    //virtual bool eventFilter( QObject *obj, QEvent *e );
    virtual void paintEvent( QPaintEvent *pe );
    virtual void contextMenuEvent( QContextMenuEvent *e );
    virtual void mousePressEvent( QMouseEvent *e );
    virtual void mouseDoubleClickEvent( QMouseEvent *e );
    virtual void mouseReleaseEvent( QMouseEvent *e );
    virtual void mouseMoveEvent( QMouseEvent *e );
    virtual void wheelEvent( QWheelEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    void setDirty( const QRect& );
    int viewdepth; // "faked" depth
    int rsh;
    int gsh;
    int bsh;
    int rmax;
    int gmax;
    int bmax;
    int contentsWidth;
    int contentsHeight;
    double gred, ggreen, gblue;
    QRgb* gammatable;

    int refreshRate;
    QAnimationWriter *animation;
    double hzm,vzm;
    QVFbViewProtocol *mView;
    bool emulateTouchscreen;
    bool emulateLcdScreen;
    Rotation rotation;
};

#endif
