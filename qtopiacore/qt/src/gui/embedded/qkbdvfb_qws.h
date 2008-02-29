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

#ifndef QKBDVFB_QWS_H
#define QKBDVFB_QWS_H

#include <QtGui/qkbd_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_KEYBOARD

#ifndef QT_NO_QWS_KBD_QVFB

class QSocketNotifier;

class QVFbKeyboardHandler : public QObject, public QWSKeyboardHandler
{
    Q_OBJECT
public:
    QVFbKeyboardHandler(const QString &device);
    virtual ~QVFbKeyboardHandler();

private Q_SLOTS:
    void readKeyboardData();

private:
    QString terminalName;
    int kbdFD;
    int kbdIdx;
    int kbdBufferLen;
    unsigned char *kbdBuffer;
    QSocketNotifier *notifier;
};

#endif // QT_NO_QWS_KBD_QVFB

#endif // QT_NO_QWS_KEYBOARD

QT_END_HEADER

#endif // QKBDVFB_QWS_H
