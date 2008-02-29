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
#ifndef _FRAMEHOOK_H
#define _FRAMEHOOK_H

/*
 * Prototypes for interface to .so that implement a video processing hook 
 */

#include "avcodec.h"

/* Function must be called 'Configure' */
typedef int (FrameHookConfigure)(void **ctxp, int argc, char *argv[]);
typedef FrameHookConfigure *FrameHookConfigureFn;
extern FrameHookConfigure Configure;

/* Function must be called 'Process' */
typedef void (FrameHookProcess)(void *ctx, struct AVPicture *pict, enum PixelFormat pix_fmt, int width, int height, int64_t pts);
typedef FrameHookProcess *FrameHookProcessFn;
extern FrameHookProcess Process;

/* Function must be called 'Release' */
typedef void (FrameHookRelease)(void *ctx);
typedef FrameHookRelease *FrameHookReleaseFn;
extern FrameHookRelease Release;

extern int frame_hook_add(int argc, char *argv[]);
extern void frame_hook_process(struct AVPicture *pict, enum PixelFormat pix_fmt, int width, int height);
extern void frame_hook_release(void);

#endif
