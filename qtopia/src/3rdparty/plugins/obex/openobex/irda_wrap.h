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
#ifndef IRDA_WRAP_H
#define IRDA_WRAP_H

#ifdef _WIN32
#define _WIN32_WINNT

#include <af_irda.h>
struct irda_device_list {DEVICELIST;};
struct irda_device_info {IRDA_DEVICE_INFO;};
struct sockaddr_irda {SOCKADDR_IRDA;};
#define sir_name irdaServiceName
#define sir_family irdaAddressFamily

#else /* _WIN32 */

#include <irda.h>

#endif /* _WIN32 */

#endif /* IRDA_WRAP_H */
