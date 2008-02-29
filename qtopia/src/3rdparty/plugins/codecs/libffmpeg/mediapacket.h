/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS and its licensors.
** All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
** See below for additional copyright and license information
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef MEDIA_PACKET_H
#define MEDIA_PACKET_H


class MediaPacket {
public:
    MediaPacket() {
	av_init_packet(&pkt);
	pkt.size = 0;
	pkt.data = 0;
	eof = true;
    }
    MediaPacket(const AVPacket &avPkt) {
	pkt = avPkt;
	eof = false;
    }
    virtual ~MediaPacket() {
	if ( !eof )
	    av_free_packet(&pkt);
    }

    AVPacket pkt;
    unsigned long fileOffset;
    int64_t msecOffset;
    bool eof;
};


#endif // MEDIA_PACKET_H

