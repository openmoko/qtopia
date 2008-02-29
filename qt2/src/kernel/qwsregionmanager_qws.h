/****************************************************************************
** $Id: qt/src/kernel/qwsregionmanager_qws.h   2.3.12   edited 2005-10-27 $
**
** Declaration of Qt/Embedded region manager
**
** Created : 000101
**
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
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

#ifndef QT_H
#include "qvector.h"
#include "qregion.h"
#endif // QT_H

class QWSRegionHeader;
class QWSRegionIndex;

class QWSRegionManager
{
public:
    QWSRegionManager( const QString &filename, bool c = TRUE );
    ~QWSRegionManager();

    // for clients
    const int *revision( int idx ) const;
    QRegion region( int idx );

    int find( int id );

    // for server
    int add( int id, QRegion region );
    void set( int idx, QRegion region );
    void remove( int idx );
    void markUpdated( int idx );
    void commit();

private:
    QRect *rects( int offset );
    bool attach( const QString &filename );
    void detach();

private:
    bool client;
    QVector<QRegion> regions;
    QWSRegionHeader *regHdr;
    QWSRegionIndex *regIdx;
    unsigned char *data;
    int shmId;
};

