/****************************************************************************
** $Id: qt/src/kernel/qsound_qws.cpp   2.3.12   edited 2005-10-27 $
**
** Implementation of QSound class and QAuServer internal class
**
** Created : 000117
**
** Copyright (C) 1999-2000 Trolltech AS.  All rights reserved.
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

#include "qapplication.h"

#ifndef QT_NO_SOUND

#include "qsound.h"
#include "qpaintdevice.h"
#include "qwsdisplay_qws.h"
#include "qsoundqss_qws.h"

class QAuBucket {
public:
    QAuBucket(const QString& s) : name(s)
    {
	mSid = ++nextSid;
    }
    QString name;
    int sid() const { return mSid; }
private:
    int mSid;
    static int nextSid;
};

int QAuBucket::nextSid = 0;

class QAuServerQWS : public QAuServer {
public:
    QAuServerQWS(QObject* parent);
    ~QAuServerQWS();

    void play(QAuBucket* b)
    {
	client->play(b->sid(), b->name);
    }

    QAuBucket* newBucket(const QString& s)
    {
	return new QAuBucket(s);
    }

    void deleteBucket(QAuBucket* b)
    {
	delete b;
    }

    bool okay() { return TRUE; }

private:
    QWSSoundClient *client;
};

QAuServerQWS::QAuServerQWS(QObject* parent) :
    QAuServer(parent,"qauserverqws")
{
    client = new QWSSoundClient(this);
}

QAuServerQWS::~QAuServerQWS()
{
}

QAuServer* qt_new_audio_server()
{
    return new QAuServerQWS(qApp);
}

#endif // QT_NO_SOUND
