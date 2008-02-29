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

#ifndef QWSCOMMAND_QWS_P_H
#define QWSCOMMAND_QWS_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// When reading commands "off the wire" in the server, the rawLen is read
// and then that many bytes are allocated.  If the rawLen is corrupted (or
// the protocol is being attacked) too many bytes can be allocated.  Set
// a hard limit here for security.
#define MAX_COMMAND_SIZE (16 * 1024)

#include <QtCore/qbytearray.h>
#include <QtGui/qwsutils_qws.h>
#include <QtGui/qfont.h>
#include <QtCore/qdatastream.h>
#include <QtCore/qvariant.h>
#include <QtCore/qrect.h>
#include <QtGui/qregion.h>
#include <QtCore/qvector.h>
#include <QtCore/qvarlengtharray.h>
#include <QtGui/qwsevent_qws.h>
#include "qwsprotocolitem_qws.h"

QT_MODULE(Gui)

#define QTE_PIPE "QtEmbedded-%1"

class QRect;

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command(QIODevice *socket, int type, char *simpleData, int simpleLen, char *rawData, int rawLen);
bool qws_read_command(QIODevice *socket, char *&simpleData, int &simpleLen, char *&rawData, int &rawLen, int &bytesRead);
#endif

struct QWSCommand : QWSProtocolItem
{
    QWSCommand(int t, int len, char *ptr) : QWSProtocolItem(t,len,ptr) {}

    enum Type {
        Unknown = 0,
        Create,
        Destroy,
        Region,
        RegionMove,
        RegionDestroy,
        SetProperty,
        AddProperty,
        RemoveProperty,
        GetProperty,
        SetSelectionOwner,
        ConvertSelection,
        RequestFocus,
        ChangeAltitude,
        SetOpacity,
        DefineCursor,
        SelectCursor,
        PositionCursor,
        GrabMouse,
        PlaySound,
        QCopRegisterChannel,
        QCopSend,
        RegionName,
        Identify,
        GrabKeyboard,
        RepaintRegion,
        IMMouse,
        IMUpdate,
        IMResponse,
        Embed
    };
    static QWSCommand *factory(int type);
};

const char *qws_getCommandTypeString( QWSCommand::Type tp );

#ifndef QT_NO_DEBUG
class QDebug;
QDebug &operator<<(QDebug &dbg, QWSCommand::Type tp);
#endif // QT_NO_DEBUG

/*********************************************************************
 *
 * Commands
 *
 *********************************************************************/

struct QWSIdentifyCommand : public QWSCommand
{
    QWSIdentifyCommand() :
        QWSCommand(QWSCommand::Identify,
                   sizeof(simpleData), reinterpret_cast<char *>(&simpleData))
    {
        simpleData.idLen = 0;
        simpleData.idLock = -1;
    }

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        if ( simpleData.idLen > MAX_COMMAND_SIZE )
        {
            qWarning( "Identify command - name length %d - too big!", simpleData.idLen );
            simpleData.idLen = MAX_COMMAND_SIZE;
        }
        id = QString(reinterpret_cast<const QChar*>(d), simpleData.idLen);
    }

    void setId(const QString& i, int lock)
    {
        id = i;
        simpleData.idLen = id.length();
        simpleData.idLock = lock;
        setData(reinterpret_cast<const char*>(id.unicode()), simpleData.idLen*2, true);
    }

    struct SimpleData {
        int idLen;
        int idLock;
    } simpleData;
    QString id;
};

struct QWSCreateCommand : public QWSCommand
{
    QWSCreateCommand(int n = 1) :
        QWSCommand(QWSCommand::Create, sizeof(count),
                   reinterpret_cast<char *>(&count)), count(n) {}
    int count;
};

struct QWSRegionNameCommand : public QWSCommand
{
    QWSRegionNameCommand() :
        QWSCommand(QWSCommand::RegionName,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        if ( simpleData.nameLen > MAX_COMMAND_SIZE )
        {
            qWarning( "region name command - name length too big!" );
            simpleData.nameLen = MAX_COMMAND_SIZE;
        }
        if ( simpleData.captionLen > MAX_COMMAND_SIZE )
        {
            qWarning( "region name command - caption length too big!" );
            simpleData.captionLen = MAX_COMMAND_SIZE;
        }
        name = QString(reinterpret_cast<const QChar*>(d), simpleData.nameLen/2);
        d += simpleData.nameLen;
        caption = QString(reinterpret_cast<const QChar*>(d), simpleData.captionLen/2);
    }

    void setName(const QString& n, const QString &c)
    {
        name = n;
        caption = c;
        int l = simpleData.nameLen = name.length()*2;
        l += simpleData.captionLen = caption.length()*2;
        char *d = new char[l];
        memcpy(d, name.unicode(), simpleData.nameLen);
        memcpy(d+simpleData.nameLen, caption.unicode(), simpleData.captionLen);
        setData(d, l, true);
        delete[] d;
    }

    struct SimpleData {
        int windowid;
        int nameLen;
        int captionLen;
    } simpleData;
    QString name;
    QString caption;
};

struct QWSRegionCommand : public QWSCommand
{
    QWSRegionCommand() :
        QWSCommand(QWSCommand::Region, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);

        char *ptr = rawDataPtr;

        region.setRects(reinterpret_cast<QRect*>(ptr), simpleData.nrectangles);
        ptr += simpleData.nrectangles * sizeof(QRect);

        surfaceKey = QString(reinterpret_cast<QChar*>(ptr),
                             simpleData.surfacekeylength);
        ptr += simpleData.surfacekeylength * sizeof(QChar);

        surfaceData = QByteArray(ptr, simpleData.surfacedatalength);
    }

    void setData(int id, const QString &key, const QByteArray &data,
                 const QRegion &reg)
    {
        surfaceKey = key;
        surfaceData = data;
        region = reg;

        const QVector<QRect> rects = reg.rects();

        simpleData.windowid = id;
        simpleData.surfacekeylength = key.size();
        simpleData.surfacedatalength = data.size();
        simpleData.nrectangles = rects.count();

        QVarLengthArray<char, 256> buffer;
        buffer.append(reinterpret_cast<const char*>(rects.constData()),
                      rects.count() * sizeof(QRect));
        buffer.append(reinterpret_cast<const char*>(key.constData()),
                      key.size() * sizeof(QChar));
        buffer.append(data, data.size());

        QWSCommand::setData(buffer.constData(), buffer.size(), true);
    }

    /* XXX this will pad out in a compiler dependent way,
       should move nrectangles to before windowtype, and
       add reserved bytes.
       Symptom will be valgrind reported uninitialized memory usage
       */
    struct SimpleData {
        int windowid;
        int surfacekeylength;
        int surfacedatalength;
        int nrectangles;
    } simpleData;

    QString surfaceKey;
    QByteArray surfaceData;
    QRegion region;
};

struct QWSSetOpacityCommand : public QWSCommand
{
    QWSSetOpacityCommand() :
        QWSCommand(QWSCommand::SetOpacity, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        uchar opacity;
    } simpleData;
};

struct QWSRegionMoveCommand : public QWSCommand
{
    QWSRegionMoveCommand() :
        QWSCommand(QWSCommand::RegionMove, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int dx;
        int dy;
    } simpleData;

};

struct QWSRegionDestroyCommand : public QWSCommand
{
    QWSRegionDestroyCommand() :
        QWSCommand(QWSCommand::RegionDestroy, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
    } simpleData;

};

struct QWSRequestFocusCommand : public QWSCommand
{
    QWSRequestFocusCommand() :
        QWSCommand(QWSCommand::RequestFocus, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int flag;
    } simpleData;
};

struct QWSChangeAltitudeCommand : public QWSCommand
{
    QWSChangeAltitudeCommand() :
        QWSCommand(QWSCommand::ChangeAltitude, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    typedef enum Altitude {
        Lower = -1,
        Raise = 0,
        StaysOnTop = 1
    };

    struct SimpleData {
        int windowid;
        Altitude altitude;
        bool fixed;
    } simpleData;

};


struct QWSAddPropertyCommand : public QWSCommand
{
    QWSAddPropertyCommand() :
        QWSCommand(QWSCommand::AddProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSSetPropertyCommand : public QWSCommand
{
    QWSSetPropertyCommand() :
        QWSCommand(QWSCommand::SetProperty, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) { data = 0; }

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        data = rawDataPtr;
    }

    struct SimpleData {
        int windowid, property, mode;
    } simpleData;

    char *data;
};

struct QWSRepaintRegionCommand : public QWSCommand
{
    QWSRepaintRegionCommand() :
        QWSCommand(QWSCommand::RepaintRegion, sizeof(simpleData),
                    reinterpret_cast<char*>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        rectangles = reinterpret_cast<QRect *>(rawDataPtr);
    }

    struct SimpleData {
        int windowid;
        bool opaque;
        int nrectangles;
    } simpleData;

    QRect * rectangles;

};

struct QWSRemovePropertyCommand : public QWSCommand
{
    QWSRemovePropertyCommand() :
        QWSCommand(QWSCommand::RemoveProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSGetPropertyCommand : public QWSCommand
{
    QWSGetPropertyCommand() :
        QWSCommand(QWSCommand::GetProperty, sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid, property;
    } simpleData;

};

struct QWSSetSelectionOwnerCommand : public QWSCommand
{
    QWSSetSelectionOwnerCommand() :
        QWSCommand(QWSCommand::SetSelectionOwner,
                    sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int hour, minute, sec, ms; // time
    } simpleData;

};

struct QWSConvertSelectionCommand : public QWSCommand
{
    QWSConvertSelectionCommand() :
        QWSCommand(QWSCommand::ConvertSelection,
                    sizeof(simpleData), reinterpret_cast<char*>(&simpleData)) {}

    struct SimpleData {
        int requestor; // requestor window of the selection
        int selection; // property on requestor into which the selection should be stored
        int mimeTypes; // property ion requestor in which the mimetypes, in which the selection may be, are stored
    } simpleData;

};

struct QWSDefineCursorCommand : public QWSCommand
{
    QWSDefineCursorCommand() :
        QWSCommand(QWSCommand::DefineCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem = true) {
        QWSCommand::setData(d, len, allocateMem);
        data = reinterpret_cast<unsigned char *>(rawDataPtr);
    }

    struct SimpleData {
        int width;
        int height;
        int hotX;
        int hotY;
        int id;
    } simpleData;

    unsigned char *data;
};

struct QWSSelectCursorCommand : public QWSCommand
{
    QWSSelectCursorCommand() :
        QWSCommand(QWSCommand::SelectCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int id;
    } simpleData;
};

struct QWSPositionCursorCommand : public QWSCommand
{
    QWSPositionCursorCommand() :
        QWSCommand(QWSCommand::PositionCursor,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int newX;
        int newY;
    } simpleData;
};

struct QWSGrabMouseCommand : public QWSCommand
{
    QWSGrabMouseCommand() :
        QWSCommand(QWSCommand::GrabMouse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        bool grab;  // grab or ungrab?
    } simpleData;
};

struct QWSGrabKeyboardCommand : public QWSCommand
{
    QWSGrabKeyboardCommand() :
        QWSCommand(QWSCommand::GrabKeyboard,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        bool grab;  // grab or ungrab?
    } simpleData;
};

#ifndef QT_NO_SOUND
struct QWSPlaySoundCommand : public QWSCommand
{
    QWSPlaySoundCommand() :
        QWSCommand(QWSCommand::PlaySound,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        filename = QString(reinterpret_cast<QChar*>(rawDataPtr),len/2);
    }
    void setFileName(const QString& n)
    {
        setData(reinterpret_cast<const char*>(n.unicode()), n.length()*2, true);
    }

    struct SimpleData {
        int windowid;
    } simpleData;
    QString filename;
};
#endif


#ifndef QT_NO_COP
struct QWSQCopRegisterChannelCommand : public QWSCommand
{
    QWSQCopRegisterChannelCommand() :
        QWSCommand(QWSCommand::QCopRegisterChannel,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        if ( simpleData.chLen > MAX_COMMAND_SIZE )
        {
            qWarning( "Command channel name too large!" );
            simpleData.chLen = MAX_COMMAND_SIZE;
        }
        channel = QString(reinterpret_cast<const QChar*>(d), simpleData.chLen);
    }

    void setChannel(const QString& n)
    {
        channel = n;
        simpleData.chLen = channel.length();
        setData(reinterpret_cast<const char*>(channel.unicode()), simpleData.chLen*2, true);
    }

    struct SimpleData {
        int chLen;
    } simpleData;
    QString channel;
};

struct QWSQCopSendCommand : public QWSCommand
{
    QWSQCopSendCommand() :
        QWSCommand(QWSCommand::QCopSend,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);
        const QChar *cd = reinterpret_cast<const QChar*>(d);
        channel = QString(cd,simpleData.clen); cd += simpleData.clen;
        message = QString(cd,simpleData.mlen);
        d += simpleData.clen*sizeof(QChar) + simpleData.mlen*sizeof(QChar);
        data = QByteArray(d, simpleData.dlen);
    }

    void setMessage(const QString &c, const QString &m,
                     const QByteArray &data)
    {
        simpleData.clen = c.length();
        simpleData.mlen = m.length();
        simpleData.dlen = data.size();
        int l = simpleData.clen*sizeof(QChar);
        l += simpleData.mlen*sizeof(QChar);
        l += simpleData.dlen;
        char *tmp = new char[l];
        char *d = tmp;
        memcpy(d, c.unicode(), simpleData.clen*sizeof(QChar));
        d += simpleData.clen*sizeof(QChar);
        memcpy(d, m.unicode(), simpleData.mlen*sizeof(QChar));
        d += simpleData.mlen*sizeof(QChar);
        memcpy(d, data.data(), simpleData.dlen);
        setData(tmp, l, true);
        delete[] tmp;
    }

    struct SimpleData {
        int clen;
        int mlen;
        int dlen;
    } simpleData;
    QString channel;
    QString message;
    QByteArray data;
};

#endif


#ifndef QT_NO_QWS_INPUTMETHODS

struct QWSIMMouseCommand : public QWSCommand
{
    QWSIMMouseCommand() :
        QWSCommand(QWSCommand::IMMouse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int state;
        int index;
    } simpleData;
};


struct QWSIMResponseCommand : public QWSCommand
{
    QWSIMResponseCommand() :
        QWSCommand(QWSCommand::IMResponse,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    void setData(const char *d, int len, bool allocateMem) {
        QWSCommand::setData(d, len, allocateMem);

        QByteArray tmp = QByteArray::fromRawData(d, len);
        QDataStream s(tmp);
        s >> result;
    }

    void setResult(const QVariant & v)
    {
        QByteArray tmp;
        QDataStream s(&tmp, QIODevice::WriteOnly);
        s << v;
        setData(tmp.data(), tmp.size(), true);
    }

    struct SimpleData {
        int windowid;
        int property;
    } simpleData;

    QVariant result;
};

struct QWSIMUpdateCommand: public QWSCommand
{
    QWSIMUpdateCommand() :
        QWSCommand(QWSCommand::IMUpdate,
                    sizeof(simpleData), reinterpret_cast<char *>(&simpleData)) {}

    struct SimpleData {
        int windowid;
        int type;
        int widgetid;
    } simpleData;
};

#endif

#ifndef QT_NO_QWSEMBEDWIDGET
struct QWSEmbedCommand : public QWSCommand
{
    QWSEmbedCommand() : QWSCommand(QWSCommand::Embed,
                                   sizeof(simpleData),
                                   reinterpret_cast<char*>(&simpleData))
    {}

    void setData(const char *d, int len, bool allocateMem = true)
    {
        QWSCommand::setData(d, len, allocateMem);
        region.setRects(reinterpret_cast<QRect*>(rawDataPtr),
                        simpleData.rects);
    }

    void setData(WId embedder, WId embedded, QWSEmbedEvent::Type type,
                 const QRegion reg = QRegion())
    {
        simpleData.embedder = embedder;
        simpleData.embedded = embedded;
        simpleData.type = type;

        region = reg;
        const QVector<QRect> rects = reg.rects();
        simpleData.rects = rects.count();

        QWSCommand::setData(reinterpret_cast<const char*>(rects.constData()),
                            rects.count() * sizeof(QRect));
    }

    struct {
        WId embedder;
        WId embedded;
        QWSEmbedEvent::Type type;
        int rects;
    } simpleData;

    QRegion region;
};
#endif // QT_NO_QWSEMBEDWIDGET

#endif // QWSCOMMAND_QWS_P_H
