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
    sizeof(int) + sizeof(int) + (item).simpleLen + (item).rawLen

QT_END_HEADER

#endif // QWSPROTOCOLITEM_QWS_H
