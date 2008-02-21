/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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


class QTOPIAMEDIA_EXPORT QMediaDevice : public QIODevice
{
    Q_OBJECT

public:

    struct Info
    {
        enum DataType { Raw, PCM };

        DataType    type;
        union
        {
            struct /* Raw */
            {
                qint64      dataSize;
            };

            struct /* PCM */
            {
                int         frequency;
                int         bitsPerSample;
                int         channels;
                int         volume;
            };
        };
    };

    virtual Info const& dataType() const = 0;

    virtual bool connectToInput(QMediaDevice* input) = 0;
    virtual void disconnectFromInput(QMediaDevice* input) = 0;
};


#endif  // __QTOPIA_MEDIASERVER_MEDIADEVICE_H
