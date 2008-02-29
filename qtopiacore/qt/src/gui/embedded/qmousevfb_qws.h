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

#ifndef QMOUSEVFB_QWS_H
#define QMOUSEVFB_QWS_H

#include <QtGui/qmouse_qws.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_QWS_MOUSE_QVFB

class QSocketNotifier;

class QVFbMouseHandler : public QObject, public QWSMouseHandler {
    Q_OBJECT
public:
    QVFbMouseHandler(const QString &driver = QString(),
            const QString &device = QString());
    ~QVFbMouseHandler();

    void resume();
    void suspend();

private:
    int mouseFD;
    int mouseIdx;
    enum {mouseBufSize = 128};
    uchar mouseBuf[mouseBufSize];
    QSocketNotifier *mouseNotifier;

private Q_SLOTS:
    void readMouseData();
};
#endif // QT_NO_QWS_MOUSE_QVFB

QT_END_HEADER

#endif // QMOUSEVFB_QWS_H
