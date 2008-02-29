/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
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
