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

#include "qkbdvr41xx_qws.h"

#if !defined(QT_NO_QWS_KEYBOARD) && !defined(QT_NO_QWS_KBD_VR41XX)

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <errno.h>

#include <qsocketnotifier.h>

class QWSVr41xxKbPrivate : public QObject
{
    Q_OBJECT
public:
    QWSVr41xxKbPrivate(QWSVr41xxKeyboardHandler *h, const QString&);
    virtual ~QWSVr41xxKbPrivate();

    bool isOpen() { return buttonFD > 0; }

private slots:
    void readKeyboardData();

private:
    QString terminalName;
    int buttonFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
    QWSVr41xxKeyboardHandler *handler;
};

QWSVr41xxKeyboardHandler::QWSVr41xxKeyboardHandler(const QString &device)
{
    d = new QWSVr41xxKbPrivate(this, device);
}

QWSVr41xxKeyboardHandler::~QWSVr41xxKeyboardHandler()
{
    delete d;
}

QWSVr41xxKbPrivate::QWSVr41xxKbPrivate(QWSVr41xxKeyboardHandler *h, const QString &device) : handler(h)
{
    terminalName = device;
    if (terminalName.isEmpty())
        terminalName = "/dev/buttons";
    buttonFD = -1;
    notifier = 0;

    buttonFD = open(terminalName.toLatin1().constData(), O_RDWR | O_NDELAY, 0);;
    if (buttonFD < 0) {
        qWarning("Cannot open %s\n", qPrintable(terminalName));
        return;
    }

    if (buttonFD >= 0) {
        notifier = new QSocketNotifier(buttonFD, QSocketNotifier::Read, this);
        connect(notifier, SIGNAL(activated(int)),this,
                SLOT(readKeyboardData()));
    }

    kbdBufferLen = 80;
    kbdBuffer = new unsigned char [kbdBufferLen];
    kbdIdx = 0;
}

QWSVr41xxKbPrivate::~QWSVr41xxKbPrivate()
{
    if (buttonFD > 0) {
        ::close(buttonFD);
        buttonFD = -1;
    }
    delete notifier;
    notifier = 0;
    delete [] kbdBuffer;
}

void QWSVr41xxKbPrivate::readKeyboardData()
{
    int n = 0;
    do {
        n  = read(buttonFD, kbdBuffer+kbdIdx, kbdBufferLen - kbdIdx);
        if (n > 0)
            kbdIdx += n;
    } while (n > 0);

    int idx = 0;
    while (kbdIdx - idx >= 2) {
        unsigned char *next = kbdBuffer + idx;
        unsigned short *code = (unsigned short *)next;
        int keycode = Qt::Key_unknown;
        switch ((*code) & 0x0fff) {
            case 0x7:
                keycode = Qt::Key_Up;
                break;
            case 0x9:
                keycode = Qt::Key_Right;
                break;
            case 0x8:
                keycode = Qt::Key_Down;
                break;
            case 0xa:
                keycode = Qt::Key_Left;
                break;
            case 0x3:
                keycode = Qt::Key_Up;
                break;
            case 0x4:
                keycode = Qt::Key_Down;
                break;
            case 0x1:
                keycode = Qt::Key_Return;
                break;
            case 0x2:
                keycode = Qt::Key_F4;
                break;
            default:
                qDebug("Unrecognised key sequence %d", (int)code);
        }
        if ((*code) & 0x8000)
            handler->processKeyEvent(0, keycode, 0, false, false);
        else
            handler->processKeyEvent(0, keycode, 0, true, false);
        idx += 2;
    }

    int surplus = kbdIdx - idx;
    for (int i = 0; i < surplus; i++)
        kbdBuffer[i] = kbdBuffer[idx+i];
    kbdIdx = surplus;
}

#include "qkbdvr41xx_qws.moc"

#endif // QT_NO_QWS_KBD_VR41XX

