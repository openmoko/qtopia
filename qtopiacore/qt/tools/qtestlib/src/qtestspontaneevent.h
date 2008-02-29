/****************************************************************************
**
** Copyright (C) 1992-2008 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef QTESTSPONTANEEVENT_H
#define QTESTSPONTANEEVENT_H

#include <QtCore/qcoreevent.h>

#if 0
// inform syncqt
#pragma qt_no_master_include
#endif

QT_BEGIN_HEADER

#ifndef QTEST_NO_SIZEOF_CHECK
template <int>
class QEventSizeOfChecker
{
private:
    QEventSizeOfChecker() {}
};

template <>
class QEventSizeOfChecker<sizeof(QEvent)>
{
public:
    QEventSizeOfChecker() {}
};
#endif

class QSpontaneKeyEvent
{
public:
    void setSpontaneous() { spont = 1; };
    bool spontaneous() { return spont; };
    virtual void dummyFunc() {  };
    virtual ~QSpontaneKeyEvent() {}

#ifndef QTEST_NO_SIZEOF_CHECK
    inline void ifYouGetCompileErrorHereYouUseWrongQt()
    {
        // this is a static assert in case QEvent changed in Qt
        QEventSizeOfChecker<sizeof(QSpontaneKeyEvent)> dummy;
    }
#endif

protected:
    void *d;
    ushort t;

private:
    ushort posted : 1;
    ushort spont : 1;
    ushort m_accept : 1;
    ushort reserved : 13;
};

QT_END_HEADER

#endif
