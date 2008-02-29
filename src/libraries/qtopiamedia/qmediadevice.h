/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#ifndef __QTOPIA_MEDIASERVER_MEDIADEVICE_H
#define __QTOPIA_MEDIASERVER_MEDIADEVICE_H


#include <QString>
#include <QVariant>
#include <QIODevice>

#include <qtopiaglobal.h>

class QMediaPipe;

class QTOPIAMEDIA_EXPORT QMediaDevice : public QIODevice
{
public:
    virtual void connectInputPipe(QMediaPipe* inputPipe) = 0;
    virtual void connectOutputPipe(QMediaPipe* outputPipe) = 0;

    virtual void disconnectInputPipe(QMediaPipe* inputPipe) = 0;
    virtual void disconnectOutputPipe(QMediaPipe* outputPipe) = 0;

    virtual void setValue(QString const& name, QVariant const& value) = 0;
    virtual QVariant value(QString const& name) = 0;
};


#endif  // __QTOPIA_MEDIASERVER_MEDIADEVICE_H
