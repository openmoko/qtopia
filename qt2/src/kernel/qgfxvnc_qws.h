/*****************************************************************************
** $Id: qt/src/kernel/qgfxvnc_qws.h   2.3.12   edited 2005-10-27 $
**
** Implementation of QGfxvnc (remote frame buffer driver)
** Proof of concept driver only.
** 
** Created : 20000703
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of the kernel module of the Qt GUI Toolkit.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** Licensees holding valid Qt Enterprise Edition or Qt Professional Edition
** licenses for Qt/Embedded may use this file in accordance with the
** Qt Embedded Commercial License Agreement provided with the Software.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/pricing.html or email sales@trolltech.com for
**   information about Qt Commercial License Agreements.
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef QGFXVNC_QWS_H
#define QGFXVNC_QWS_H

#ifndef QT_H
#include <qserversocket.h>
#endif // QT_H

#ifndef QT_NO_QWS_VNC

class QTimer;

class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect( Q_UINT16 _x, Q_UINT16 _y, Q_UINT16 _w, Q_UINT16 _h ) {
	x = _x; y = _y; w = _w; h = _h;
    }

    void read( QSocket *s );
    void write( QSocket *s );

    Q_UINT16 x;
    Q_UINT16 y;
    Q_UINT16 w;
    Q_UINT16 h;
};

class QRfbPixelFormat
{
public:
    static int size() { return 16; }

    void read( QSocket *s );
    void write( QSocket *s );

    int bitsPerPixel;
    int depth;
    bool bigEndian;
    bool trueColor;
    int redBits;
    int greenBits;
    int blueBits;
    int redShift;
    int greenShift;
    int blueShift;
};

class QRfbServerInit
{
public:
    QRfbServerInit() { name = 0; }
    ~QRfbServerInit() { delete name; }

    int size() const { return QRfbPixelFormat::size() + 8 + strlen( name ); }
    void setName( const char *n );

    void read( QSocket *s );
    void write( QSocket *s );

    Q_UINT16 width;
    Q_UINT16 height;
    QRfbPixelFormat format;
    char *name;
};

class QRfbSetEncodings
{
public:
    bool read( QSocket *s );

    Q_UINT16 count;
};

class QRfbFrameBufferUpdateRequest
{
public:
    bool read( QSocket *s );

    char incremental;
    QRfbRect rect;
};

class QRfbKeyEvent
{
public:
    bool read( QSocket *s );

    char down;
    int  keycode;
    int  unicode;
};

class QRfbPointerEvent
{
public:
    bool read( QSocket *s );

    uint buttons;
    Q_UINT16 x;
    Q_UINT16 y;
};

class QRfbClientCutText
{
public:
    bool read( QSocket *s );

    Q_UINT32 length;
};


class QVNCServer : public QServerSocket
{
    Q_OBJECT
public:
    QVNCServer();
    QVNCServer( int id );
    ~QVNCServer();

    virtual void newConnection( int socket );

    enum ClientMsg { SetPixelFormat = 0,
		     FixColourMapEntries = 1,
		     SetEncodings = 2,
		     FramebufferUpdateRequest = 3,
		     KeyEvent = 4,
		     PointerEvent = 5,
		     ClientCutText = 6 };

    enum ServerMsg { FramebufferUpdate = 0,
		     SetColourMapEntries = 1 };

private:
    void setPixelFormat();
    void setEncodings();
    void frameBufferUpdateRequest();
    void pointerEvent();
    void keyEvent();
    void clientCutText();
    bool checkFill( const uchar *data, int numPixels );
    int getPixel( uchar ** );
    void sendHextile();
    void sendRaw();

private slots:
    void readClient();
    void checkUpdate();
    void discardClient();

private:
    enum ClientState { Protocol, Init, Connected };
    QTimer *timer;
    QSocket *client;
    ClientState state;
    Q_UINT8 msgType;
    bool handleMsg;
    QRfbPixelFormat pixelFormat;
    int keymod;
    int encodingsPending;
    int cutTextPending;
    bool supportHextile;
    bool wantUpdate;
};

#endif // QT_NO_QWS_VNC

#endif // QGFXVNC_QWS_H

