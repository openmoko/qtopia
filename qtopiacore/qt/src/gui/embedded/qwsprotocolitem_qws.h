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

#ifndef QWSPROTOCOLITEM_QWS_H
#define QWSPROTOCOLITEM_QWS_H

/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/

#include <QtCore/qglobal.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QIODevice;

struct QWSProtocolItem
{
    // ctor - dtor
    QWSProtocolItem(int t, int len, char *ptr) : type(t),
        simpleLen(len), rawLen(-1), deleteRaw(false), simpleDataPtr(ptr),
        rawDataPtr(0), bytesRead(0) { }
    virtual ~QWSProtocolItem();

    // data
    int type;
    int simpleLen;
    int rawLen;
    bool deleteRaw;

    // functions
#ifndef QT_NO_QWS_MULTIPROCESS
    void write(QIODevice *s);
    bool read(QIODevice *s);
#endif
    void copyFrom(const QWSProtocolItem *item);

    virtual void setData(const char *data, int len, bool allocateMem = true);

    char *simpleDataPtr;
    char *rawDataPtr;
    // temp variables
    int bytesRead;
};

// This should probably be a method on QWSProtocolItem, but this way avoids
// changing the API of this apparently public header
// size = (int)type + (int)rawLenSize + simpleLen + rawLen
#define QWS_PROTOCOL_ITEM_SIZE( item ) \
    (2 * sizeof(int)) + ((item).simpleDataPtr ? (item).simpleLen : 0) + ((item).rawDataPtr ? (item).rawLen : 0)

QT_END_HEADER

#endif // QWSPROTOCOLITEM_QWS_H
