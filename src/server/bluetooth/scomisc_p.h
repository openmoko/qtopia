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

#ifndef __SCOMISC_H__
#define __SCOMISC_H__

#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>

bool bt_sco_set_fd(snd_hwdep_t * handle, int sco_fd);
QByteArray find_btsco_device(const QByteArray &idPref = QByteArray());
#endif

#endif
