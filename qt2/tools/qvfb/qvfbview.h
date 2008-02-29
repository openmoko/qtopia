/****************************************************************************
**
** Qt/Embedded virtual framebuffer
**
** Created : 20000605
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the Qt GUI Toolkit.
**
** Licensees holding valid Qt Professional Edition licenses may use this
** file in accordance with the Qt Professional Edition License Agreement
** provided with the Qt Professional Edition.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
** information about the Professional Edition licensing.
**
*****************************************************************************/

#include <qscrollview.h>

class QImage;
class QTimer;
class QAnimationWriter;
struct QVFbHeader;

class QVFbView : public QScrollView
{
    Q_OBJECT
public:
    enum Rotation { Rot0, Rot90, Rot180, Rot270 };
    QVFbView( int display_id, int w, int h, int d, Rotation r, bool use_mmap = false, QWidget *parent = 0,
		const char *name = 0, uint wflags = 0 );
    ~QVFbView();

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

public slots:
    void setTouchscreenEmulation( bool );
    void setLcdScreenEmulation( bool );
    void setRate( int );
    void setZoom( double, double );
    void startAnimation( const QString& );
    void stopAnimation();

protected slots:
    void flushChanges();

protected:
    void initLock();
    void lock();
    void unlock();
    QImage getBuffer( const QRect &r, int &leading ) const;
    void drawScreen();
    void sendMouseData( const QPoint &pos, int buttons );
    void sendKeyboardData( int unicode, int keycode, int modifiers,
			   bool press, bool repeat );
    virtual bool eventFilter( QObject *obj, QEvent *e );
    virtual void viewportPaintEvent( QPaintEvent *pe );
    virtual void contentsMousePressEvent( QMouseEvent *e );
    virtual void contentsMouseDoubleClickEvent( QMouseEvent *e );
    virtual void contentsMouseReleaseEvent( QMouseEvent *e );
    virtual void contentsMouseMoveEvent( QMouseEvent *e );
    virtual void keyPressEvent( QKeyEvent *e );
    virtual void keyReleaseEvent( QKeyEvent *e );

private:
    void setDirty( const QRect& );
    int shmId;
    unsigned char *data;
    QVFbHeader *hdr;
    int viewdepth; // "faked" depth
    int rsh;
    int gsh;
    int bsh;
    int rmax;
    int gmax;
    int bmax;
    double gred, ggreen, gblue;
    QRgb* gammatable;
    int lockId;
    QTimer *t_flush;

    bool mmapVfb;
    int vfbFd;
    int dataSize;
    int mouseFd;
    int keyboardFd;
    int refreshRate;
    QString vfbMap;
    QString mousePipe;
    QString keyboardPipe;
    QAnimationWriter *animation;
    int displayid;
    double hzm,vzm;
    bool emulateTouchscreen;
    bool emulateLcdScreen;
    Rotation rotation;
};

