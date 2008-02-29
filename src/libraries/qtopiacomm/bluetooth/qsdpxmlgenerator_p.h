/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __QSDPXMLGENERATOR_P_H__
#define __QSDPXMLGENERATOR_P_H__

#include <QObject>
#include <qtopiaglobal.h>

class QBluetoothSdpRecord;
class QIODevice;

class QTOPIACOMM_EXPORT QSdpXmlGenerator
{
public:
    static void generate(const QBluetoothSdpRecord &record, QIODevice *device);
};

#endif
