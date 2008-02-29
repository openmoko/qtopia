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

#ifndef __CRUXUSURISESSIONBUILDER_H
#define __CRUXUSURISESSIONBUILDER_H


#include <QObject>
#include <QStringList>

#include <qmediasessionbuilder.h>

class QMediaCodecPlugin;

namespace cruxus
{

typedef QMap<QString, QMediaCodecPlugin*>  MediaCodecPluginMap;

class Engine;
class UriSessionBuilderPrivate;

class UriSessionBuilder :
    public QObject,
    public QMediaSessionBuilder
{
    Q_OBJECT

public:
    UriSessionBuilder(Engine* engine,
                      QStringList const& mimeTypes,
                      MediaCodecPluginMap const& pluginExtensionMapping);
    ~UriSessionBuilder();

    QString type() const;
    Attributes const& attributes() const;

    QMediaServerSession* createSession(QMediaSessionRequest sessionRequest);
    void destroySession(QMediaServerSession* serverSession);

private:
    UriSessionBuilderPrivate* d;
};

} // ns cruxus

#endif  // __CRUXUSURISESSIONBUILDER_H

