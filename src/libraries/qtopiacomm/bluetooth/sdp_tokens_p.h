/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef __SDP_TOKENS_H__
#define __SDP_TOKENS_H__

#define SDP_NEWLINE 0x01
#define SDP_SPACE 0x02
#define SDP_TAB 0x03
#define SDP_SEQ 0x04
#define SDP_ALT 0x05
#define SDP_ATTR 0x06

#define SDP_NIL            0x10
#define SDP_BOOL           0x11
#define SDP_UINT8          0x12
#define SDP_INT8           0x13
#define SDP_UINT16         0x14
#define SDP_INT16          0x15
#define SDP_UINT32         0x16
#define SDP_INT32          0x17
#define SDP_UINT64         0x18
#define SDP_INT64          0x19
#define SDP_UINT128        0x1A
#define SDP_INT128         0x1B
#define SDP_UUID16         0x1C
#define SDP_UUID32         0x1D
#define SDP_UUID128        0x1E
#define SDP_TEXT           0x1F
#define SDP_URL            0x21
#define SDP_DATA           0x22

#define SDP_BOOL_VALUE             0x41
#define SDP_HEX8_VALUE             0x42
#define SDP_HEX16_VALUE            0x43
#define SDP_HEX32_VALUE            0x44
#define SDP_HEX64_VALUE            0x45
#define SDP_HEX128_VALUE           0x46
#define SDP_UUID128_VALUE          0x47
#define SDP_STRING_VALUE           0x48
#define SDP_BINARY_DATA_VALUE      0x49

#endif
