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

#ifndef __QTOPIA_MEDIASERVER_MEDIADEVICE_H
#define __QTOPIA_MEDIASERVER_MEDIADEVICE_H


#include <qstring.h>
#include <qvariant.h>
#include <qiodevice.h>

class QMediaPipe;

class QMediaDevice : public QIODevice
{
public:
    virtual void setInputPipe(QMediaPipe* inputPipe) = 0;
    virtual void setOutputPipe(QMediaPipe* outputPipe) = 0;

    virtual void setValue(QString const& name, QVariant const& value) = 0;
    virtual QVariant value(QString const& name) = 0;
};


#endif  // __QTOPIA_MEDIASERVER_MEDIADEVICE_H
