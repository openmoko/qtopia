/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef VIDEO_LOOP_CONTEXT_H
#define VIDEO_LOOP_CONTEXT_H


#include <qdatetime.h>
#include "loopcontextbase.h"
#include "mediaplayerstate.h"
#include "videooutput.h"
#include <qtopia/mediaplayerplugininterface.h>


class VideoLoopContext : public LoopContextBase {
    Q_OBJECT
public:
    VideoLoopContext(QTime *pt, bool t, bool r=false, bool fifo=false, int priority=10) : LoopContextBase(t,r,fifo,priority), playTime(pt) {
    }
    long timerInterval() {
	return (iRate >> 16) - 1; // round down (schedule upto a millisecond early)
    }
    void init() {
	current = 0;
	streamId = 0;
	framesPlayed = 0;
	size = mediaPlayerState->decoder()->videoFrames(streamId);
        rate = (float)mediaPlayerState->decoder()->videoFrameRate(streamId);
	prevFrame = -1;
        qDebug("Frame rate %g total %ld", rate, size); // No tr
	if ( rate <= 1.0 ) {
	    qDebug("Crazy frame rate, resetting to sensible"); // No tr
	    rate = 25;
	}
	// rateNumDen.rateNumerator = 0;
	// rateNumDen.rateDenominator = 0;
	iRate = (int)(65536.0 * 1000.0 / rate);
	invRate = (int)(65536.0 * rate / 1000.0);
    }
    void stats() {
#if 0//1 //ndef NO_DEBUG
	long msecElapsed = playTime->elapsed();
	double fps = 0.0;
	if ( framesPlayed && msecElapsed )
	    fps = framesPlayed * 1000.0 / msecElapsed;
//	fprintf(stderr, "\rframe %i, played: %i, playtime: %i ms, fps: %f - ", current, framesPlayed, playTime->elapsed(), fps);
	qDebug("frame %li, played: %li, playtime: %li ms, fps: %f", current, framesPlayed, msecElapsed, fps);
//	fflush(stdout);
#endif
    }
    bool playFrame() {
	// Enough time has elapsed that it is time to display another frame
	bool moreFrames = (videoOutput) ? videoOutput->playNextVideoFrame() : false;
	framesPlayed++;
	prevFrame = current;
	return moreFrames;
    }
    bool playMore() {
	if (!mediaPlayerState->decoder())
	    return false;
	current = (playTime->elapsed() * invRate) >> 16;
	if ( mediaPlayerState->decoderVersion() == Decoder_1_6 ) {
	    MediaPlayerDecoder_1_6 *decoder = (MediaPlayerDecoder_1_6 *)mediaPlayerState->decoder();
	    if ( current != prevFrame ) {
		// Resync the video with the audio
		if ( decoder->syncAvailable() )
		    decoder->sync();
		return playFrame();
	    }
	} else {
	    if ( prevFrame == -1 || current > prevFrame ) {
		if ( current > prevFrame + 1 )
		    mediaPlayerState->decoder()->videoSetFrame(current, streamId);
		return playFrame();
	    }
	}
	return true;
    }

    long iRate;
    long invRate;
    long prevFrame;
    long framesPlayed;
    QTime *playTime;
};


#endif // VIDEO_LOOP_CONTEXT_H

