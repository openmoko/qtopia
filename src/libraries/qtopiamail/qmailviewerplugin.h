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

#ifndef QMAILVIEWERPLUGIN_H
#define QMAILVIEWERPLUGIN_H

#include <QString>
#include <QIcon>
#include <qfactoryinterface.h>

#include <qtopiaglobal.h>
#include <qmailviewer.h>

struct QTOPIAMAIL_EXPORT QMailViewerPluginInterface : public QFactoryInterface
{
    virtual QString key() const = 0;

    virtual bool isSupported( QMailViewerFactory::ContentType type ) const = 0;
    virtual QMailViewerInterface* create( QWidget* parent ) = 0;
};

#define QMailViewerPluginInterface_iid "com.trolltech.Qtopia.Qtopiamail.QMailViewerPluginInterface"
Q_DECLARE_INTERFACE(QMailViewerPluginInterface, QMailViewerPluginInterface_iid)

class QTOPIAMAIL_EXPORT QMailViewerPlugin : public QObject, public QMailViewerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMailViewerPluginInterface:QFactoryInterface)

public:
    QMailViewerPlugin();
    ~QMailViewerPlugin();

    virtual QStringList keys() const;
};

#endif /* QMAILVIEWERPLUGIN_H */
