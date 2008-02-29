/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qplatformdefs.h"

#ifndef QT_NO_QWS_VNC

#include "qtimer.h"
#include "qwindowsystem_qws.h"
#include "qscreenvnc_qws.h"
#include <private/qsharedmemory_p.h>
#include <stdlib.h>
#include <qdebug.h>
#include <qpolygon.h>
#include <qstringlist.h>
#include <qregexp.h>
#include "qwsdisplay_qws.h"

//#define QT_QWS_VNC_DEBUG

extern QString qws_qtePipeFilename();

#define MAP_TILE_SIZE            16
#define MAP_WIDTH            1280/MAP_TILE_SIZE
#define MAP_HEIGHT            1024/MAP_TILE_SIZE
#define UPDATE_FREQUENCY    40

class QVirtualScreen : public QScreen
{
public:
    QVirtualScreen(int displayId);
    ~QVirtualScreen();

    bool initDevice();
    bool connect(const QString &displaySpec);
    void disconnect();
    void shutdownDevice();
    void setMode(int,int,int) {}

private:
#ifndef QT_NO_QWS_MULTIPROCESS
    QSharedMemory *shm;
#endif
};

QVirtualScreen::QVirtualScreen(int displayId)
    : QScreen(displayId)
{
#ifndef QT_NO_QWS_MULTIPROCESS
    shm = 0;
#endif
}

QVirtualScreen::~QVirtualScreen()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    delete shm;
#else
    delete[] QScreen::data;
    QScreen::data = 0;
#endif
}

bool QVirtualScreen::initDevice()
{
    if (d == 4) {
        screencols = 16;
        int val = 0;
        for (int idx = 0; idx < 16; idx++, val += 17) {
            screenclut[idx]=qRgb(val, val, val);
        }
    }
    return true;
}

bool QVirtualScreen::connect(const QString &displayspec)
{
    Q_UNUSED(displayspec);

    d = qgetenv("QWS_DEPTH").toInt();
    if (!d)
        d = 16;

    QByteArray str = qgetenv("QWS_SIZE");
    if(!str.isEmpty()) {
        sscanf(str.constData(), "%dx%d", &w, &h);
        dw = w;
        dh = h;
    } else {
        dw = w = 640;
        dh = h = 480;
    }

    const QStringList args = displayspec.split(QLatin1Char(':'),
                                               QString::SkipEmptyParts);

    QRegExp depthRegexp("^depth=(\\d+)$");
    if (args.indexOf(depthRegexp) != -1) {
        d = depthRegexp.cap(1).toInt();
    }

    QRegExp sizeRegexp("^size=(\\d+)x(\\d+)$");
    if (args.indexOf(sizeRegexp) != -1) {
        dw = w = sizeRegexp.cap(1).toInt();
        dh = h = sizeRegexp.cap(2).toInt();
    }

    const int dpi = 72;
    physWidth = qRound(dw * 25.4 / dpi);
    physHeight = qRound(dh * 25.4 / dpi);

    lstep = (dw * d + 7) / 8;
    size = h * lstep;
    mapsize = size;

    QWSServer::setDefaultMouse("None");
    QWSServer::setDefaultKeyboard("None");

#ifndef QT_NO_QWS_MULTIPROCESS
    shm = new QSharedMemory(size, qws_qtePipeFilename(), displayId);
    if (!shm->create())
        qDebug("QVNCScreen could not create shared memory");
    if (!shm->attach())
        qDebug("QVNCScreen could not attach to shared memory");
    QScreen::data = reinterpret_cast<uchar*>(shm->base());
#else
    QScreen::data = new uchar[size];
#endif
    return true;
}

void QVirtualScreen::disconnect()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (shm)
        shm->detach();
#endif
}

void QVirtualScreen::shutdownDevice()
{
#ifndef QT_NO_QWS_MULTIPROCESS
    if (shm)
        shm->destroy();
#endif
}

struct QVNCHeader
{
    bool dirty;
    uchar map[MAP_HEIGHT][MAP_WIDTH];
};

//===========================================================================

class QRfbRect
{
public:
    QRfbRect() {}
    QRfbRect(quint16 _x, quint16 _y, quint16 _w, quint16 _h) {
        x = _x; y = _y; w = _w; h = _h;
    }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint16 x;
    quint16 y;
    quint16 w;
    quint16 h;
};

class QRfbPixelFormat
{
public:
    static int size() { return 16; }

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

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
    ~QRfbServerInit() { delete[] name; }

    int size() const { return QRfbPixelFormat::size() + 8 + strlen(name); }
    void setName(const char *n);

    void read(QTcpSocket *s);
    void write(QTcpSocket *s);

    quint16 width;
    quint16 height;
    QRfbPixelFormat format;
    char *name;
};

class QRfbSetEncodings
{
public:
    bool read(QTcpSocket *s);

    quint16 count;
};

class QRfbFrameBufferUpdateRequest
{
public:
    bool read(QTcpSocket *s);

    char incremental;
    QRfbRect rect;
};

class QRfbKeyEvent
{
public:
    bool read(QTcpSocket *s);

    char down;
    int  keycode;
    int  unicode;
};

class QRfbPointerEvent
{
public:
    bool read(QTcpSocket *s);

    uint buttons;
    quint16 x;
    quint16 y;
};

class QRfbClientCutText
{
public:
    bool read(QTcpSocket *s);

    quint32 length;
};

class QVNCServer;

#define QT_NO_QWS_MULTIPROCESS

class QVNCScreenPrivate
{
public:
    QVNCScreenPrivate(QVNCScreen *parent)
        : vncServer(0), subscreen(0), q(parent) {}
    ~QVNCScreenPrivate();

    void configure();

    QVNCHeader hdr;
    QVNCServer *vncServer;
    QScreen *subscreen;
    QVNCScreen *q;
};

QVNCScreenPrivate::~QVNCScreenPrivate()
{
    delete subscreen;
}

void QVNCScreenPrivate::configure()
{
    q->d = subscreen->depth();
    q->w = subscreen->width();
    q->h = subscreen->height();
    q->dw = subscreen->deviceWidth();
    q->dh = subscreen->deviceHeight();
    q->lstep = subscreen->linestep();
    q->data = subscreen->base();
    q->physWidth = subscreen->physicalWidth();
    q->physHeight = subscreen->physicalHeight();

    q->setOffset(subscreen->offset());
}

class QVNCServer : public QObject
{
    Q_OBJECT
public:
    QVNCServer(QVNCScreen *screen);
    QVNCServer(QVNCScreen *screen, int id);
    ~QVNCServer();

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
    bool checkFill(const uchar *data, int numPixels);
    int getPixel(uchar **);
    void sendHextile();
    void sendRaw();
    inline QVNCHeader* header() { return &qvnc_screen->d_ptr->hdr; }

private slots:
    void newConnection();
    void readClient();
    void checkUpdate();
    void discardClient();

private:
    void init(uint port);
    enum ClientState { Protocol, Init, Connected };
    QTimer *timer;
    QTcpServer *serverSocket;
    QTcpSocket *client;
    ClientState state;
    quint8 msgType;
    bool handleMsg;
    QRfbPixelFormat pixelFormat;
    Qt::KeyboardModifiers keymod;
    int encodingsPending;
    int cutTextPending;
    bool supportHextile;
    bool wantUpdate;
    int nibble;
    bool sameEndian;
    QVNCScreen *qvnc_screen;
};

//===========================================================================

static const struct {
    int keysym;
    int keycode;
} keyMap[] = {
    { 0xff08, Qt::Key_Backspace },
    { 0xff09, Qt::Key_Tab       },
    { 0xff0d, Qt::Key_Return    },
    { 0xff1b, Qt::Key_Escape    },
    { 0xff63, Qt::Key_Insert    },
    { 0xffff, Qt::Key_Delete    },
    { 0xff50, Qt::Key_Home      },
    { 0xff57, Qt::Key_End       },
    { 0xff55, Qt::Key_PageUp    },
    { 0xff56, Qt::Key_PageDown  },
    { 0xff51, Qt::Key_Left      },
    { 0xff52, Qt::Key_Up        },
    { 0xff53, Qt::Key_Right     },
    { 0xff54, Qt::Key_Down      },
    { 0xffbe, Qt::Key_F1        },
    { 0xffbf, Qt::Key_F2        },
    { 0xffc0, Qt::Key_F3        },
    { 0xffc1, Qt::Key_F4        },
    { 0xffc2, Qt::Key_F5        },
    { 0xffc3, Qt::Key_F6        },
    { 0xffc4, Qt::Key_F7        },
    { 0xffc5, Qt::Key_F8        },
    { 0xffc6, Qt::Key_F9        },
    { 0xffc7, Qt::Key_F10       },
    { 0xffc8, Qt::Key_F11       },
    { 0xffc9, Qt::Key_F12       },
    { 0xffe1, Qt::Key_Shift     },
    { 0xffe2, Qt::Key_Shift     },
    { 0xffe3, Qt::Key_Control   },
    { 0xffe4, Qt::Key_Control   },
    { 0xffe7, Qt::Key_Meta      },
    { 0xffe8, Qt::Key_Meta      },
    { 0xffe9, Qt::Key_Alt       },
    { 0xffea, Qt::Key_Alt       },
    { 0, 0 }
};

void QRfbRect::read(QTcpSocket *s)
{
    quint16 buf[4];
    s->read((char*)buf, 8);
    x = ntohs(buf[0]);
    y = ntohs(buf[1]);
    w = ntohs(buf[2]);
    h = ntohs(buf[3]);
}

void QRfbRect::write(QTcpSocket *s)
{
    quint16 buf[4];
    buf[0] = htons(x);
    buf[1] = htons(y);
    buf[2] = htons(w);
    buf[3] = htons(h);
    s->write((char*)buf, 8);
}

void QRfbPixelFormat::read(QTcpSocket *s)
{
    char buf[16];
    s->read(buf, 16);
    bitsPerPixel = buf[0];
    depth = buf[1];
    bigEndian = buf[2];
    trueColor = buf[3];

    quint16 a = ntohs(*(quint16 *)(buf + 4));
    redBits = 0;
    while (a) { a >>= 1; redBits++; }

    a = ntohs(*(quint16 *)(buf + 6));
    greenBits = 0;
    while (a) { a >>= 1; greenBits++; }

    a = ntohs(*(quint16 *)(buf + 8));
    blueBits = 0;
    while (a) { a >>= 1; blueBits++; }

    redShift = buf[10];
    greenShift = buf[11];
    blueShift = buf[12];
}

void QRfbPixelFormat::write(QTcpSocket *s)
{
    char buf[16];
    buf[0] = bitsPerPixel;
    buf[1] = depth;
    buf[2] = bigEndian;
    buf[3] = trueColor;

    quint16 a = 0;
    for (int i = 0; i < redBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 4) = htons(a);

    a = 0;
    for (int i = 0; i < greenBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 6) = htons(a);

    a = 0;
    for (int i = 0; i < blueBits; i++) a = (a << 1) | 1;
    *(quint16 *)(buf + 8) = htons(a);

    buf[10] = redShift;
    buf[11] = greenShift;
    buf[12] = blueShift;
    s->write(buf, 16);
}


void QRfbServerInit::setName(const char *n)
{
    delete[] name;
    name = new char [strlen(n) + 1];
    strcpy(name, n);
}

void QRfbServerInit::read(QTcpSocket *s)
{
    s->read((char *)&width, 2);
    width = ntohs(width);
    s->read((char *)&height, 2);
    height = ntohs(height);
    format.read(s);

    quint32 len;
    s->read((char *)&len, 4);
    len = ntohl(len);

    name = new char [len + 1];
    s->read(name, len);
    name[len] = '\0';
}

void QRfbServerInit::write(QTcpSocket *s)
{
    quint16 t = htons(width);
    s->write((char *)&t, 2);
    t = htons(height);
    s->write((char *)&t, 2);
    format.write(s);
    quint32 len = strlen(name);
    len = htonl(len);
    s->write((char *)&len, 4);
    s->write(name, strlen(name));
}

bool QRfbSetEncodings::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 3)
        return false;

    char tmp;
    s->read(&tmp, 1);        // padding
    s->read((char *)&count, 2);
    count = ntohs(count);

    return true;
}

bool QRfbFrameBufferUpdateRequest::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 9)
        return false;

    s->read(&incremental, 1);
    rect.read(s);

    return true;
}

bool QRfbKeyEvent::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 7)
        return false;

    s->read(&down, 1);
    quint16 tmp;
    s->read((char *)&tmp, 2);  // padding

    quint32 key;
    s->read((char *)&key, 4);
    key = ntohl(key);

    unicode = 0;
    keycode = 0;
    int i = 0;
    while (keyMap[i].keysym && !keycode) {
        if (keyMap[i].keysym == (int)key)
            keycode = keyMap[i].keycode;
        i++;
    }
    if (!keycode) {
        if (key <= 0xff) {
            unicode = key;
            if (key >= 'a' && key <= 'z')
                keycode = Qt::Key_A + key - 'a';
            else if (key >= ' ' && key <= '~')
                keycode = Qt::Key_Space + key - ' ';
        }
    }

    return true;
}

bool QRfbPointerEvent::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 5)
        return false;

    char buttonMask;
    s->read(&buttonMask, 1);
    buttons = 0;
    if (buttonMask & 1)
        buttons |= Qt::LeftButton;
    if (buttonMask & 2)
        buttons |= Qt::MidButton;
    if (buttonMask & 4)
        buttons |= Qt::RightButton;

    quint16 tmp;
    s->read((char *)&tmp, 2);
    x = ntohs(tmp);
    s->read((char *)&tmp, 2);
    y = ntohs(tmp);

    return true;
}

bool QRfbClientCutText::read(QTcpSocket *s)
{
    if (s->bytesAvailable() < 7)
        return false;

    char tmp[3];
    s->read(tmp, 3);        // padding
    s->read((char *)&length, 4);
    length = ntohl(length);

    return true;
}

//===========================================================================

QVNCServer::QVNCServer(QVNCScreen *screen)
    : qvnc_screen(screen)
{
    init(5900);
}

QVNCServer::QVNCServer(QVNCScreen *screen, int id)
    : qvnc_screen(screen)
{
    init(5900 + id);
}

void QVNCServer::init(uint port)
{
    handleMsg = false;
    client = 0;
    encodingsPending = 0;
    cutTextPending = 0;
    keymod = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(checkUpdate()));
    serverSocket = new QTcpServer(this);
    if (!serverSocket->listen(QHostAddress::Any, port))
        qDebug() << "QVNCServer could not connect:" << serverSocket->errorString();
    else
        qDebug("QVNCServer created on port %d", port);

    connect(serverSocket, SIGNAL(newConnection()), this, SLOT(newConnection()));
}

QVNCServer::~QVNCServer()
{
    discardClient();
    delete client;
}

void QVNCServer::newConnection()
{
    if (client)
        delete client;

    client = serverSocket->nextPendingConnection();
    connect(client,SIGNAL(readyRead()),this,SLOT(readClient()));
    connect(client,SIGNAL(disconnected()),this,SLOT(discardClient()));
    handleMsg = false;
    encodingsPending = 0;
    cutTextPending = 0;
    supportHextile = false;
    wantUpdate = false;
    timer->start(UPDATE_FREQUENCY);

    // send protocol version
    char *proto = "RFB 003.003\n";
    client->write(proto, 12);
    state = Protocol;
}

void QVNCServer::readClient()
{
    switch (state) {
        case Protocol:
            if (client->bytesAvailable() >= 12) {
                char proto[13];
                client->read(proto, 12);
                proto[12] = '\0';
                qDebug("Client protocol version %s", proto);
                // No authentication
                quint32 auth = htonl(1);
                client->write((char *) &auth, sizeof(auth));
                state = Init;
            }
            break;

        case Init:
            if (client->bytesAvailable() >= 1) {
                quint8 shared;
                client->read((char *) &shared, 1);

                // Server Init msg
                QRfbServerInit sim;
                QRfbPixelFormat &format = sim.format;
                switch (qvnc_screen->depth()) {
                    case 32:
                        format.bitsPerPixel = 32;
                        format.depth = 32;
                        format.bigEndian = 0;
                        format.trueColor = true;
                        format.redBits = 8;
                        format.greenBits = 8;
                        format.blueBits = 8;
                        format.redShift = 16;
                        format.greenShift = 8;
                        format.blueShift = 0;
                        break;

                    case 16:
                        format.bitsPerPixel = 16;
                        format.depth = 16;
                        format.bigEndian = 0;
                        format.trueColor = true;
                        format.redBits = 5;
                        format.greenBits = 6;
                        format.blueBits = 5;
                        format.redShift = 11;
                        format.greenShift = 5;
                        format.blueShift = 0;
                        break;

                    case 8:
                        format.bitsPerPixel = 8;
                        format.depth = 8;
                        format.bigEndian = 0;
                        format.trueColor = false;
                        format.redBits = 0;
                        format.greenBits = 0;
                        format.blueBits = 0;
                        format.redShift = 0;
                        format.greenShift = 0;
                        format.blueShift = 0;
                        break;

                    case 4:
                        format.bitsPerPixel = 8;
                        format.depth = 8;
                        format.bigEndian = 0;
                        format.trueColor = false;
                        format.redBits = 0;
                        format.greenBits = 0;
                        format.blueBits = 0;
                        format.redShift = 0;
                        format.greenShift = 0;
                        format.blueShift = 0;
                        break;

                    default:
                        qDebug("QVNC cannot drive depth %d", qvnc_screen->depth());
                        discardClient();
                        return;
                }
                sim.width = qvnc_screen->deviceWidth();
                sim.height = qvnc_screen->deviceHeight();
                sim.setName("Qtopia Core VNC Server");
                sim.write(client);
                state = Connected;
            }
            break;

        case Connected:
            do {
                if (!handleMsg) {
                    client->read((char *)&msgType, 1);
                    handleMsg = true;
                }
                if (handleMsg) {
                    switch (msgType ) {
                        case SetPixelFormat:
                            setPixelFormat();
                            break;
                        case FixColourMapEntries:
                            qDebug("Not supported: FixColourMapEntries");
                            handleMsg = false;
                            break;
                        case SetEncodings:
                            setEncodings();
                            break;
                        case FramebufferUpdateRequest:
                            frameBufferUpdateRequest();
                            break;
                        case KeyEvent:
                            keyEvent();
                            break;
                        case PointerEvent:
                            pointerEvent();
                            break;
                        case ClientCutText:
                            clientCutText();
                            break;
                        default:
                            qDebug("Unknown message type: %d", (int)msgType);
                            handleMsg = false;
                    }
                }
            } while (!handleMsg && client->bytesAvailable());
            break;
    }
}

void QVNCServer::setPixelFormat()
{
    if (client->bytesAvailable() >= 19) {
        char buf[3];
        client->read(buf, 3); // just padding
        pixelFormat.read(client);
#ifdef QT_QWS_VNC_DEBUG
        qDebug("Want format: %d %d %d %d %d %d %d %d %d %d",
            int(pixelFormat.bitsPerPixel),
            int(pixelFormat.depth),
            int(pixelFormat.bigEndian),
            int(pixelFormat.trueColor),
            int(pixelFormat.redBits),
            int(pixelFormat.greenBits),
            int(pixelFormat.blueBits),
            int(pixelFormat.redShift),
            int(pixelFormat.greenShift),
            int(pixelFormat.blueShift));
#endif
        if (pixelFormat.bitsPerPixel != 16 && pixelFormat.bitsPerPixel != 32) {
            qDebug("Cannot handle %d bpp client", pixelFormat.bitsPerPixel);
            discardClient();
        }
        handleMsg = false;
        sameEndian = (QSysInfo::ByteOrder == QSysInfo::BigEndian) == !!pixelFormat.bigEndian;
    }
}

void QVNCServer::setEncodings()
{
    QRfbSetEncodings enc;

    if (!encodingsPending && enc.read(client)) {
        encodingsPending = enc.count;
        if (!encodingsPending)
            handleMsg = false;
    }

    if (encodingsPending && (unsigned)client->bytesAvailable() >=
                                encodingsPending * sizeof(quint32)) {
        for (int i = 0; i < encodingsPending; i++) {
            quint32 enc;
            client->read((char *)&enc, sizeof(quint32));
            enc = ntohl(enc);
            if (enc == 5)
                supportHextile = true;
#ifdef QT_QWS_VNC_DEBUG
            qDebug("Can do %d", enc);
#endif
        }
        handleMsg = false;
        encodingsPending = 0;
    }
}

void QVNCServer::frameBufferUpdateRequest()
{
    QRfbFrameBufferUpdateRequest ev;

    if (ev.read(client)) {
        if (!ev.incremental) {
            QWSDisplay::grab(true);
            QRect r(ev.rect.x, ev.rect.y, ev.rect.w, ev.rect.h);
            qvnc_screen->setDirty(r.translated(qvnc_screen->offset()));
            QWSDisplay::ungrab();
        }
        wantUpdate = true;
        checkUpdate();
        handleMsg = false;
    }
}

void QVNCServer::pointerEvent()
{
    QRfbPointerEvent ev;
    if (ev.read(client)) {
        const QPoint offset = qvnc_screen->offset();
        QWSServer::sendMouseEvent(offset + QPoint(ev.x, ev.y), ev.buttons);
        handleMsg = false;
    }
}

void QVNCServer::keyEvent()
{
    QRfbKeyEvent ev;

    if (ev.read(client)) {
        if (ev.keycode == Qt::Key_Shift)
            keymod = ev.down ? keymod | Qt::ShiftModifier :
                               keymod & ~Qt::ShiftModifier;
        else if (ev.keycode == Qt::Key_Control)
            keymod = ev.down ? keymod | Qt::ControlModifier :
                               keymod & ~Qt::ControlModifier;
        else if (ev.keycode == Qt::Key_Alt)
            keymod = ev.down ? keymod | Qt::AltModifier :
                               keymod & ~Qt::AltModifier;
        if (ev.unicode || ev.keycode)
            QWSServer::sendKeyEvent(ev.unicode, ev.keycode, keymod, ev.down, false);
        handleMsg = false;
    }
}

void QVNCServer::clientCutText()
{
    QRfbClientCutText ev;

    if (ev.read(client)) {
        cutTextPending = ev.length;
        if (!cutTextPending)
            handleMsg = false;
    }

    if (cutTextPending && client->bytesAvailable() >= cutTextPending) {
        char *text = new char [cutTextPending+1];
        client->read(text, cutTextPending);
        delete [] text;
        cutTextPending = 0;
        handleMsg = false;
    }
}

bool QVNCServer::checkFill(const uchar *data, int numPixels)
{
    if (qvnc_screen->depth() == 8) {
        if (*data != *(data+1) || *data != *(data+2) || *data != *(data+3))
            return false;
        quint32 pixels = *((quint32 *)data);
        data += 4;
        for (int i = 1; i < numPixels/4; i++) {
            if (pixels != *((quint32*)data))
                return false;
            data += 4;
        }
    } else if (qvnc_screen->depth() == 16) {
        if (*((ushort *)data) != *(((ushort *)data)+1))
            return false;
        quint32 pixels = *((quint32 *)data);
        data += 4;
        for (int i = 1; i < numPixels/2; i++) {
            if (pixels != *((quint32 *)data))
                return false;
            data += 4;
        }
    } else if (qvnc_screen->depth() == 32) {
        quint32 pixel = *((quint32 *)data);
        data += 4;
        for (int i = 1; i < numPixels; i++) {
            if (pixel != *((quint32 *)data))
                return false;
            data += 4;
        }
    } else if (qvnc_screen->depth() == 4) {
        uchar pixel = *data++;
        for (int i = 2; i < numPixels/2; i++) {
            if (pixel != *data)
                return false;
            data++;
        }
    }

    return true;
}

int QVNCServer::getPixel(uchar **data)
{
    int r, g, b;

    if (qvnc_screen->depth() == 8) {
        QRgb rgb = qvnc_screen->clut()[**data];
        r = qRed(rgb);
        g = qGreen(rgb);
        b = qBlue(rgb);
        (*data)++;
    } else if (qvnc_screen->depth() == 16) {
        ushort p = *((ushort *)*data);
        r = (p >> 11) & 0x1f;
        g = (p >> 5) & 0x3f;
        b = p & 0x1f;
        r <<= 3;
        g <<= 2;
        b <<= 3;
        *data += 2;
    } else if (qvnc_screen->depth() == 32) {
        uint p = *((uint *)*data);
        r = (p >> 16) & 0xff;
        g = (p >> 8) & 0xff;
        b = p & 0xff;
        *data += 4;
    } else if (qvnc_screen->depth() == 4) {
        if (!nibble) {
            r = ((**data) & 0x0f) << 4;
        } else {
            r = (**data) & 0xf0;
            (*data)++;
        }
        nibble = !nibble;
        g = b = r;
    } else {
        r = g = b = 0;
        qDebug("QVNCServer: don't support %dbpp display", qvnc_screen->depth());
    }

    r >>= (8 - pixelFormat.redBits);
    g >>= (8 - pixelFormat.greenBits);
    b >>= (8 - pixelFormat.blueBits);

    if (sameEndian)
        return (r << pixelFormat.redShift) |
            (g << pixelFormat.greenShift) |
            (b << pixelFormat.blueShift);


    int pixel = (r << pixelFormat.redShift) |
                (g << pixelFormat.greenShift) |
                (b << pixelFormat.blueShift);

    if ( QSysInfo::ByteOrder == QSysInfo::BigEndian ) { // server runs on a big endian system
      if ( pixelFormat.bitsPerPixel == 16 ) {
           if ( pixelFormat.bigEndian ) { // client expects big endian
              pixel = ((pixel & 0x0000ffff) << 16);
           } else { // client expects little endian
              pixel = (((pixel & 0x0000ff00) << 8)  |
                        ((pixel & 0x000000ff) << 24));
           }
       } else if ( pixelFormat.bitsPerPixel == 32 ) {
           if ( !pixelFormat.bigEndian ) { // client expects little endian
               pixel = (((pixel & 0xff000000) >> 24) |
                        ((pixel & 0x00ff0000) >> 8)  |
                        ((pixel & 0x0000ff00) << 8)  |
                        ((pixel & 0x000000ff) << 24));
           }
       } else {
          qDebug( "Cannot handle %d bpp client", pixelFormat.bitsPerPixel );
       }
   } else { // server runs on a little endian system
      if ( pixelFormat.bitsPerPixel == 16 ) {
           if ( pixelFormat.bigEndian ) { // client expects big endian
              pixel = (((pixel & 0xff000000) >> 8) |
                        ((pixel & 0x00ff0000) << 8));
           }
       } else if ( pixelFormat.bitsPerPixel == 32 ) {
           if ( pixelFormat.bigEndian ) { // client expects big endian
               pixel = (((pixel & 0xff000000) >> 24) |
                        ((pixel & 0x00ff0000) >> 8)  |
                        ((pixel & 0x0000ff00) << 8)  |
                        ((pixel & 0x000000ff) << 24));
           }
       } else {
          qDebug( "Cannot handle %d bpp client", pixelFormat.bitsPerPixel );
       }
   }

   return pixel;

}

/*
  Send dirty rects using hextile encoding.  We only actually use the Raw
  and BackgroundSpecified subencodings.  The BackgroundSpecified encoding
  is only used to send areas of a single color.
*/
void QVNCServer::sendHextile()
{
    QWSDisplay::grab(true);

    const quint32 encoding = htonl(5); // hextile encoding

    int lineSize = MAP_TILE_SIZE * qvnc_screen->depth() / 8;
    uchar *screendata = new uchar[MAP_TILE_SIZE * lineSize];

    quint16 count = 0;
    int vtiles = (qvnc_screen->deviceHeight()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    int htiles = (qvnc_screen->deviceWidth()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    if (header()->dirty) {
        for (int y = 0; y < vtiles; y++)
            for (int x = 0; x < htiles; x++)
                if (header()->map[y][x])
                    count++;
    }

    char tmp = 0;
    client->write(&tmp, 1); // msg type
    client->write(&tmp, 1); // padding
    count = htons(count);
    client->write((char *)&count, 2);

    if (!header()->dirty) {
        QWSDisplay::ungrab();
        return;
    }

    QRfbRect rect;
    rect.h = MAP_TILE_SIZE;
    for (int y = 0; y < vtiles; y++) {
        rect.y = y * MAP_TILE_SIZE;
        if (rect.y + MAP_TILE_SIZE > qvnc_screen->height())
            rect.h = qvnc_screen->height() - rect.y;
        rect.w = MAP_TILE_SIZE;
        for (int x = 0; x < htiles; x++) {
            if (!header()->map[y][x])
                continue;
            header()->map[y][x] = 0;

            rect.x = x * MAP_TILE_SIZE;
            if (rect.x + MAP_TILE_SIZE > qvnc_screen->deviceWidth())
                rect.w = qvnc_screen->deviceWidth() - rect.x;
            rect.write(client);

            client->write((char *)&encoding, sizeof(encoding));

            // grab screen memory
            uchar *sptr = screendata;
            const uchar *data = qvnc_screen->base() +
                                rect.y * qvnc_screen->linestep() +
                                rect.x * qvnc_screen->depth() / 8;

            for (int i = 0; i < rect.h; i++) {
                memcpy(sptr, data, lineSize);
                sptr += lineSize;
                data += qvnc_screen->linestep();
            }

            sptr = screendata;
            if (checkFill(screendata, rect.w * rect.h)) {
                // This area is a single color
                //qDebug("Send empty block");
                quint8 subenc = 2; // BackgroundSpecified subencoding
                client->write((char *)&subenc, 1);
                int pixel;
                pixel = getPixel(&sptr);
                client->write((char *)&pixel, pixelFormat.bitsPerPixel/8);
            } else {
                const int bytesPerPixel = pixelFormat.bitsPerPixel / 8;
                const int bufferSize = rect.w * rect.h * bytesPerPixel + 1;
                char *buffer = new char[bufferSize];

                buffer[0] = 1; // Raw subencoding

                // convert pixels
                char *b = buffer + 1;
                for (int i = rect.y; i < rect.y+rect.h; i++) {
                    nibble = 0;
                    for (int j = 0; j < rect.w; j++) {
                        const int pixel = getPixel(&sptr);
                        memcpy(b, &pixel, bytesPerPixel);
                        b += bytesPerPixel;
                    }
                }
                client->write(buffer, bufferSize);
                delete[] buffer;
            }
        }
        if (client->state() == QAbstractSocket::UnconnectedState)
            break;
    }
    client->flush();

    header()->dirty = false;

    delete[] screendata;

    QWSDisplay::ungrab();
}

/*
  Send dirty rects as raw data.  The rectangles are merged into larger
  rects before sending.
*/
void QVNCServer::sendRaw()
{
    QWSDisplay::grab(true);

    QRegion rgn;

    int vtiles = (qvnc_screen->deviceHeight()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    int htiles = (qvnc_screen->deviceWidth()+MAP_TILE_SIZE-1)/MAP_TILE_SIZE;
    if (header()->dirty) {
        // make a region from the dirty rects and send the region's merged
        // rects.
        for (int y = 0; y < vtiles; y++)
            for (int x = 0; x < htiles; x++)
                if (header()->map[y][x])
                    rgn += QRect(x*MAP_TILE_SIZE, y*MAP_TILE_SIZE, MAP_TILE_SIZE, MAP_TILE_SIZE);

        rgn &= QRect(0, 0, qvnc_screen->deviceWidth()-1,
                    qvnc_screen->deviceHeight()-1);
    }

    char tmp = 0;
    client->write(&tmp, 1); // msg type
    client->write(&tmp, 1); // padding
    quint16 count = htons(rgn.rects().count());
    client->write((char *)&count, 2);

    if (rgn.rects().count()) {
        for (int idx = 0; idx < rgn.rects().count(); idx++) {
            QRfbRect rect;
            rect.x = rgn.rects()[idx].x();
            rect.y = rgn.rects()[idx].y();
            rect.w = rgn.rects()[idx].width();
            rect.h = rgn.rects()[idx].height();
            rect.write(client);

            quint32 encoding = htonl(0);        // raw encoding
            client->write((char *)&encoding, 4);

            int pixel;
            for (int i = rect.y; i < rect.y+rect.h; i++) {
                uchar *data = qvnc_screen->base() + i * qvnc_screen->linestep() +
                                rect.x * qvnc_screen->depth() / 8;
                nibble = rect.x & 1;
                for (int j = 0; j < rect.w; j++) {
                    pixel = getPixel(&data);
                    client->write((char *)&pixel, pixelFormat.bitsPerPixel/8);
                }
            }
        }
        header()->dirty = false;
        memset(header()->map, 0, MAP_WIDTH*vtiles);
    }

    QWSDisplay::ungrab();
}

void QVNCServer::checkUpdate()
{
    if (wantUpdate && header()->dirty) {
        if (supportHextile)
            sendHextile();
        else
            sendRaw();
        wantUpdate = false;
    }
}

void QVNCServer::discardClient()
{
    timer->stop();
}


//===========================================================================

/*!
    \internal

    \class QVNCScreen
    \ingroup qws

    \brief The QVNCScreen class implements a screen driver for VNC
    servers.

    Note that this class is only available in \l {Qtopia Core}.
    Custom screen drivers can be added by subclassing the
    QScreenDriverPlugin class, using the QScreenDriverFactory class to
    dynamically load the driver into the application.

    The VNC protocol allows you to view and interact with the
    computer's display from anywhere on the network. See the \l {The
    VNC protocol}{VNC protocol} documentation for more details.

    The default implementation of QVNCScreen inherits QLinuxFbScreen,
    but any QScreen subclass, or QScreen itself, can serve as its base
    class. This is easily achieved by manipulating the \c
    VNCSCREEN_BASE definition in the header file.

    \sa QScreen, QScreenDriverPlugin, {Running Applications}
*/

/*!
    \fn QVNCScreen::QVNCScreen(int displayId)

    Constructs a QVNCScreen object. The \a displayId argument
    identifies the Qtopia Core server to connect to.
*/
QVNCScreen::QVNCScreen(int display_id)
    : QScreen(display_id)
{
    d_ptr = new QVNCScreenPrivate(this);
}

/*!
    Destroys this QVNCScreen object.
*/
QVNCScreen::~QVNCScreen()
{
    delete d_ptr;
}

/*!
    \reimp
*/
void QVNCScreen::setDirty(const QRect& rect)
{
    if (rect.isEmpty())
        return;

    const QRect r = rect.translated(-offset());
    d_ptr->hdr.dirty = true;
    int x1 = r.x()/MAP_TILE_SIZE;
    int y1 = r.y()/MAP_TILE_SIZE;
    for (int y = y1; y <= r.bottom()/MAP_TILE_SIZE && y < MAP_HEIGHT; y++)
        for (int x = x1; x <= r.right()/MAP_TILE_SIZE && x < MAP_WIDTH; x++)
            d_ptr->hdr.map[y][x] = 1;

    if (d_ptr->subscreen)
        d_ptr->subscreen->setDirty(rect);
}

static int getDisplayId(const QString &spec)
{
    QRegExp regexp(":(\\d+)\\b");
    if (regexp.lastIndexIn(spec) != -1) {
        const QString capture = regexp.cap(1);
        return capture.toInt();
    }
    return 0;
}

/*!
    \reimp
*/
bool QVNCScreen::connect(const QString &displaySpec)
{
    QString dspec = displaySpec;
    if (dspec.startsWith("VNC:", Qt::CaseInsensitive))
        dspec = dspec.mid(QString("VNC:").size());
    else if (dspec.compare(QLatin1String("VNC"), Qt::CaseInsensitive) == 0)
        dspec = QString();

    const QString displayIdSpec = QString(" :%1").arg(displayId);
    if (dspec.endsWith(displayIdSpec))
        dspec = dspec.left(dspec.size() - displayIdSpec.size());

    QString driver = dspec;
    int colon = driver.indexOf(':');
    if (colon >= 0)
        driver.truncate(colon);

    if (driver.trimmed().isEmpty() || !driver.compare(QLatin1String("virtual"))
        || QRegExp("\\d+").exactMatch(driver))
    {
        d_ptr->subscreen = new QVirtualScreen(displayId);
        d_ptr->subscreen->connect(dspec);
    } else {
        const int id = getDisplayId(dspec);
        d_ptr->subscreen = qt_get_screen(id, dspec.toLatin1().constData());
    }
    d_ptr->configure();

    // XXX
    qt_screen = this;

    return true;
}

/*!
    \reimp
*/
void QVNCScreen::disconnect()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->disconnect();
}

/*!
    \reimp
*/
bool QVNCScreen::initDevice()
{
    if (!d_ptr->subscreen && d == 4) {
        screencols = 16;
        int val = 0;
        for (int idx = 0; idx < 16; idx++, val += 17) {
            screenclut[idx]=qRgb(val, val, val);
        }
    }
    d_ptr->vncServer = new QVNCServer(this, displayId);

    d_ptr->hdr.dirty = false;
    memset(d_ptr->hdr.map, 0, MAP_WIDTH * MAP_HEIGHT);

#ifndef QT_NO_QWS_CURSOR
    QScreenCursor::initSoftwareCursor();
#endif
    if (d_ptr->subscreen)
        return d_ptr->subscreen->initDevice();

    return true;
}

/*!
    \reimp
*/
void QVNCScreen::shutdownDevice()
{
    delete d_ptr->vncServer;
    if (d_ptr->subscreen)
        d_ptr->subscreen->shutdownDevice();
}

/*!
    \reimp
*/
void QVNCScreen::setMode(int w,int h, int d)
{
    if (d_ptr->subscreen) {
        d_ptr->subscreen->setMode(w, h, d);
        d_ptr->configure();
        setDirty(region().boundingRect());
    }
}

/*!
    \reimp
*/
bool QVNCScreen::supportsDepth(int depth) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->supportsDepth(depth);
    return false;
}

/*!
    \reimp
*/
void QVNCScreen::save()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->save();
    QScreen::save();
}

/*!
    \reimp
*/
void QVNCScreen::restore()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->restore();
    QScreen::restore();
}

/*!
    \reimp
*/
void QVNCScreen::blank(bool on)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->blank(on);
}

/*!
    \reimp
*/
bool QVNCScreen::onCard(const unsigned char *ptr) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr);
    return false;
}

/*!
    \reimp
*/
bool QVNCScreen::onCard(const unsigned char *ptr, ulong &offset) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->onCard(ptr, offset);
    return false;
}

/*!
    \reimp
*/
bool QVNCScreen::isInterlaced() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->isInterlaced();
    return false;
}

/*!
    \reimp
*/
int QVNCScreen::memoryNeeded(const QString &str)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->memoryNeeded(str);
    else
        return QScreen::memoryNeeded(str);
}

/*!
    \reimp
*/
int QVNCScreen::sharedRamSize(void *ptr)
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->sharedRamSize(ptr);
    else
        return QScreen::sharedRamSize(ptr);
}

/*!
    \reimp
*/
void QVNCScreen::haltUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->haltUpdates();
    else
        QScreen::haltUpdates();
}

/*!
    \reimp
*/
void QVNCScreen::resumeUpdates()
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->resumeUpdates();
}

/*!
    \reimp
*/
void QVNCScreen::exposeRegion(QRegion r, int changing)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->exposeRegion(r, changing);
    else
        QScreen::exposeRegion(r, changing);

    const QVector<QRect> rects = (region() & r).rects();
    for (int i = 0; i < rects.size(); ++i)
        setDirty(rects.at(i));
}

/*!
    \reimp
*/
void QVNCScreen::blit(const QImage &img, const QPoint &topLeft, const QRegion &region)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->blit(img, topLeft, region);
    else
        QScreen::blit(img, topLeft, region);
}

/*!
    \reimp
*/
void QVNCScreen::solidFill(const QColor &color, const QRegion &region)
{
    if (d_ptr->subscreen)
        d_ptr->subscreen->solidFill(color, region);
    else
        QScreen::solidFill(color, region);
}

/*!
    \reimp
*/
QWSWindowSurface* QVNCScreen::createSurface(QWidget *widget) const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->createSurface(widget);
    return QScreen::createSurface(widget);
}

/*!
    \reimp
*/
QList<QScreen*> QVNCScreen::subScreens() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->subScreens();
    return QScreen::subScreens();
}

/*!
    \reimp
*/
QRegion QVNCScreen::region() const
{
    if (d_ptr->subscreen)
        return d_ptr->subscreen->region();
    return QScreen::region();
}

#include "qscreenvnc_qws.moc"

#endif // QT_NO_QWS_VNC


