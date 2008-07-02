/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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
#ifndef __QTOPIA_MIDIPLUGIN_H
#define __QTOPIA_MIDIPLUGIN_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qstringlist.h>

#include <qtopiamedia/qmediacodecplugin.h>



class MidiPlugin :
    public QObject,
    public QMediaCodecPlugin
{
    Q_OBJECT
    Q_INTERFACES(QMediaCodecPlugin)

    class MidiPluginPrivate;

public:
    MidiPlugin();
    ~MidiPlugin();

    QString name() const;
    QString comment() const;
    QStringList mimeTypes() const;
    QStringList fileExtensions() const;

    double version() const;

    bool canEncode() const;
    bool canDecode() const;

    QMediaEncoder* encoder(QString const& mimeType);
    QMediaDecoder* decoder(QString const& mimeType);

private:
    MidiPluginPrivate*  d;
};

#endif  // __QTOPIA_MIDIPLUGIN_H
