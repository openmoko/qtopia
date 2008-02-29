/****************************************************************************
** $Id: qt/src/kernel/qgfxvoodoodefs_qws.h   2.3.12   edited 2005-10-27 $
**
** Voodoo defines
**
** Created : 940721
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

#ifndef QT_H
#include <qglobal.h>
#endif // QT_H

#define VOODOOSTATUS (0x000)
#define INTCTRL (0x004+0x0100000)
#define CLIP0MIN (0x008+0x0100000)
#define CLIP0MAX (0x00c+0x0100000)
#define DSTBASEADDR (0x010+0x0100000)
#define DSTFORMAT (0x014+0x0100000)
#define SRCCOLORKEYMIN (0x018+0x0100000)
#define SRCCOLORKEYMAX (0x01c+0x0100000)
#define DSTCOLORKEYMIN (0x020+0x0100000)
#define DSTCOLORKEYMAX (0x024+0x0100000)
#define BRESERROR0 (0x028+0x0100000)
#define BRESERROR1 (0x02c+0x0100000)
#define ROP (0x030+0x0100000)
#define SRCBASEADDR (0x034+0x0100000)
#define COMMANDEXTRA (0x038+0x0100000)
#define LINESTIPPLE (0x03c+0x0100000)
#define LINESTYLE (0x040+0x0100000)
#define PATTERN0ALIAS (0x044+0x0100000)
#define PATTERN1ALIAS (0x048+0x0100000)
#define CLIP1MIN (0x04c+0x0100000)
#define CLIP1MAX (0x050+0x0100000)
#define SRCFORMAT (0x054+0x0100000)
#define SRCSIZE (0x058+0x0100000)
#define SRCXY (0x05c+0x0100000)
#define COLORBACK (0x060+0x0100000)
#define COLORFORE (0x064+0x0100000)
#define DSTSIZE (0x068+0x0100000)
#define DSTXY (0x06c+0x0100000)
#define COMMAND (0x070+0x0100000)
#define LAUNCHAREA (0x080+0x0100000)
#define COLORPATTERN (0x100+0x0100000)

#define VIDPROCCFG 0x5c
#define HWCURC0 0x68
#define HWCURC1 0x6c
#define HWCURPATADDR 0x60
#define HWCURLOC 0x64
