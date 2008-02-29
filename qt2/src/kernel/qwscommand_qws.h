/****************************************************************************
** $Id: qt/src/kernel/qwscommand_qws.h   2.3.12   edited 2005-10-27 $
**
** Implementation of Qt/FB central server
**
** Created : 991025
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

#ifndef QWSCOMMAND_H
#define QWSCOMMAND_H

#ifndef QT_H
#include "qwsutils_qws.h"
#include "qfont.h"
#include <qdatastream.h>
#endif // QT_H

#define QTE_PIPE "QtEmbedded-%1"

/*********************************************************************
 *
 * Functions to read/write commands on/from a socket
 *
 *********************************************************************/
#ifndef QT_NO_QWS_MULTIPROCESS
void qws_write_command( QWSSocket *socket, int type, char *simpleData, int simpleLen, char *rawData, int rawLen );
bool qws_read_command( QWSSocket *socket, char *&simpleData, int &simpleLen, char *&rawData, int &rawLen, int &bytesRead );
#endif
/*********************************************************************
 *
 * QWSCommand base class - only use derived classes from that
 *
 *********************************************************************/


struct QWSProtocolItem
{
    // ctor - dtor
    QWSProtocolItem( int t, int len, char *ptr ) : type( t ),
	simpleLen( len ), rawLen( -1 ), deleteRaw(FALSE), simpleDataPtr( ptr ),
	rawDataPtr( 0 ), bytesRead( 0 ) { }
    virtual ~QWSProtocolItem();


    // data
    int type;
    int simpleLen;
    int rawLen;
    bool deleteRaw;

    // functions
#ifndef QT_NO_QWS_MULTIPROCESS
    void write( QWSSocket *s );
    bool read( QWSSocket *s );
#endif
    void copyFrom( const QWSProtocolItem *item );

    virtual void setData( char *data, int len, bool allocateMem = TRUE );

    char *simpleDataPtr;
    char *rawDataPtr;
    // temp variables
    int bytesRead;
};


struct QWSCommand : QWSProtocolItem
{
    QWSCommand( int t, int len, char *ptr ) : QWSProtocolItem(t,len,ptr) {}

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
	SetSelectionOwner, //10
	ConvertSelection,
	RequestFocus,
	ChangeAltitude,
	DefineCursor,
	SelectCursor,
	GrabMouse,
	PlaySound,
	QCopRegisterChannel,
	QCopSend,
	RegionName,
	Identify,
	GrabKeyboard,
	RepaintRegion,
	SetIMFont,
	ResetIM,
	SetIMInfo,
	IMMouse,
	MoveCursor
    };
    static QWSCommand *factory( int type );
};

/*********************************************************************
 *
 * Commands
 *
 *********************************************************************/

struct QWSIdentifyCommand : public QWSCommand
{
    QWSIdentifyCommand() :
	QWSCommand( QWSCommand::Identify,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	id = QString((QChar*)d, simpleData.idLen/2);
    }

    void setId( const QString& i )
    {
	id = i;
	int l = simpleData.idLen = id.length()*2;
	QByteArray ba(l);
	char *d = ba.data();
	memcpy( d, id.unicode(), simpleData.idLen );
	setData( d, l, TRUE );
    }

    struct SimpleData {
	int idLen;
    } simpleData;
    QString id;
};

struct QWSCreateCommand : public QWSCommand
{
    QWSCreateCommand() :
	QWSCommand( QWSCommand::Create, 0, 0 ) {}
};

struct QWSRegionNameCommand : public QWSCommand
{
    QWSRegionNameCommand() :
	QWSCommand( QWSCommand::RegionName,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	name = QString((QChar*)d, simpleData.nameLen/2);
	d += simpleData.nameLen;
	caption = QString((QChar*)d, simpleData.captionLen/2);
    }

    void setName( const QString& n, const QString &c )
    {
	name = n;
	caption = c;
	int l = simpleData.nameLen = name.length()*2;
	l += simpleData.captionLen = caption.length()*2;
	QByteArray ba(l);
	char *d = ba.data();
	memcpy( d, name.unicode(), simpleData.nameLen );
	memcpy( d+simpleData.nameLen, caption.unicode(), simpleData.captionLen );
	setData( d, l, TRUE );
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
	QWSCommand( QWSCommand::Region, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	rectangles = (QRect*)rawDataPtr;
    }

    struct SimpleData {
	int windowid;
	int nrectangles;
    } simpleData;

    QRect *rectangles;

};

struct QWSRegionMoveCommand : public QWSCommand
{
    QWSRegionMoveCommand() :
	QWSCommand( QWSCommand::RegionMove, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int dx;
	int dy;
    } simpleData;

};

struct QWSRegionDestroyCommand : public QWSCommand
{
    QWSRegionDestroyCommand() :
	QWSCommand( QWSCommand::RegionDestroy, sizeof( simpleData ),
		    (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
    } simpleData;

};

struct QWSRequestFocusCommand : public QWSCommand
{
    QWSRequestFocusCommand() :
	QWSCommand( QWSCommand::RequestFocus, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int flag;
    } simpleData;
};

struct QWSChangeAltitudeCommand : public QWSCommand
{
    QWSChangeAltitudeCommand() :
	QWSCommand( QWSCommand::ChangeAltitude, sizeof( simpleData ), (char*)&simpleData )
	{ memset((char*)&simpleData,0,sizeof(simpleData)); }

    struct SimpleData {
	int windowid;
	int altitude;
	bool fixed;
    } simpleData;

};


struct QWSAddPropertyCommand : public QWSCommand
{
    QWSAddPropertyCommand() :
	QWSCommand( QWSCommand::AddProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSSetPropertyCommand : public QWSCommand
{
    QWSSetPropertyCommand() :
	QWSCommand( QWSCommand::SetProperty, sizeof( simpleData ),
		    (char*)&simpleData ) { data = 0; }

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	data = rawDataPtr;
    }

    struct SimpleData {
	int windowid, property, mode;
    } simpleData;

    char *data;
};

struct QWSRemovePropertyCommand : public QWSCommand
{
    QWSRemovePropertyCommand() :
	QWSCommand( QWSCommand::RemoveProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSGetPropertyCommand : public QWSCommand
{
    QWSGetPropertyCommand() :
	QWSCommand( QWSCommand::GetProperty, sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid, property;
    } simpleData;

};

struct QWSSetSelectionOwnerCommand : public QWSCommand
{
    QWSSetSelectionOwnerCommand() :
	QWSCommand( QWSCommand::SetSelectionOwner,
		    sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int hour, minute, sec, ms; // time
    } simpleData;

};

struct QWSConvertSelectionCommand : public QWSCommand
{
    QWSConvertSelectionCommand() :
	QWSCommand( QWSCommand::ConvertSelection,
		    sizeof( simpleData ), (char*)&simpleData ) {}

    struct SimpleData {
	int requestor; // requestor window of the selection
	int selection; // property on requestor into which the selection should be stored
	int mimeTypes; // property ion requestor in which the mimetypes, in which the selection may be, are stored
    } simpleData;

};

struct QWSDefineCursorCommand : public QWSCommand
{
    QWSDefineCursorCommand() :
	QWSCommand( QWSCommand::DefineCursor,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem = TRUE ) {
	QWSCommand::setData( d, len, allocateMem );
	data = (unsigned char *)rawDataPtr;
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
	QWSCommand( QWSCommand::SelectCursor,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int id;
    } simpleData;
};

struct QWSMoveCursorCommand : public QWSCommand
{
    QWSMoveCursorCommand() :
	QWSCommand( QWSCommand::MoveCursor,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int x;
	int y;
    } simpleData;
};

struct QWSGrabMouseCommand : public QWSCommand
{
    QWSGrabMouseCommand() :
	QWSCommand( QWSCommand::GrabMouse,
		    sizeof( simpleData ), (char *)&simpleData )
	{ memset((char*)&simpleData,0,sizeof(simpleData)); }

    struct SimpleData {
	int windowid;
	bool grab;  // grab or ungrab?
    } simpleData;
};

struct QWSGrabKeyboardCommand : public QWSCommand
{
    QWSGrabKeyboardCommand() :
	QWSCommand( QWSCommand::GrabKeyboard,
		    sizeof( simpleData ), (char *)&simpleData )
	{ memset((char*)&simpleData,0,sizeof(simpleData)); }

    struct SimpleData {
	int windowid;
	bool grab;  // grab or ungrab?
    } simpleData;
};

#ifndef QT_NO_SOUND
struct QWSPlaySoundCommand : public QWSCommand
{
    QWSPlaySoundCommand() :
	QWSCommand( QWSCommand::PlaySound,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	filename = QString((QChar*)rawDataPtr,len/2);
    }
    void setFileName( const QString& n )
    {
	setData( (char*)n.unicode(), n.length()*2, TRUE );
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
	QWSCommand( QWSCommand::QCopRegisterChannel,
		    sizeof( simpleData ), (char *)&simpleData )
	{ memset((char*)&simpleData,0,sizeof(simpleData)); }

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	channel = QCString( d, len );
    }

    void setChannel( const QCString& n )
    {
	setData( (char*)n.data(), n.length()+1, TRUE );
    }

    struct SimpleData {
	bool unused; // we may need it
    } simpleData;
    QString channel;
};

struct QWSQCopSendCommand : public QWSCommand
{
    QWSQCopSendCommand() :
	QWSCommand( QWSCommand::QCopSend,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );
	channel = QCString( d, simpleData.clen + 1 );
	d += simpleData.clen;
	message = QCString( d, simpleData.mlen + 1 );
	d += simpleData.mlen;
	data.duplicate( d, simpleData.dlen );
    }

    void setMessage( const QCString &c, const QCString &m,
		     const QByteArray &data )
    {
	int l = simpleData.clen = c.length();
	l += simpleData.mlen = m.length();
	l += simpleData.dlen = data.size();
	QByteArray tmp( l );
	char *d = tmp.data();
	memcpy( d, c.data(), simpleData.clen );
	d += simpleData.clen;
	memcpy( d, m.data(), simpleData.mlen );
	d += simpleData.mlen;
	memcpy( d, data.data(), simpleData.dlen );
	setData( (char*)tmp.data(), l, TRUE );
    }

    struct SimpleData {
	int clen;
	int mlen;
	int dlen;
    } simpleData;
    QCString channel;
    QCString message;
    QByteArray data;
};

#endif

#ifndef QT_NO_QWS_IM
struct QWSSetIMInfoCommand : public QWSCommand
{
    QWSSetIMInfoCommand() :
	QWSCommand( QWSCommand::SetIMInfo,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int x;
	int y;
	int x1;
	int y1;
	int w;
	int h;
	bool reset;
    } simpleData;
};

struct QWSIMMouseCommand : public QWSCommand
{
    QWSIMMouseCommand() :
	QWSCommand( QWSCommand::IMMouse,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
	int state;
	int index;
    } simpleData;
};

struct QWSResetIMCommand : public QWSCommand
{
    QWSResetIMCommand() :
	QWSCommand( QWSCommand::ResetIM,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    struct SimpleData {
	int windowid;
    } simpleData;
};

struct QWSSetIMFontCommand : public QWSCommand
{
    QWSSetIMFontCommand() :
	QWSCommand( QWSCommand::SetIMFont,
		    sizeof( simpleData ), (char *)&simpleData ) {}

    void setData( char *d, int len, bool allocateMem ) {
	QWSCommand::setData( d, len, allocateMem );

	QByteArray  tmp;
	tmp.setRawData( d, len );
	QDataStream s( tmp, IO_ReadOnly );
	s >> font;
	tmp.resetRawData( d, len );
    }
    void setFont( const QFont & f )
    {
	QByteArray tmp;
	QDataStream s( tmp, IO_WriteOnly );
	s << f;
	setData( tmp.data(), tmp.size(), TRUE );
    }

    struct SimpleData {
	int windowid;
    } simpleData;
    QFont font;
};



#endif

#endif // QWSCOMMAND_H
