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

#ifndef QMAILCOMPOSERPLUGIN_H
#define QMAILCOMPOSERPLUGIN_H

#include <QString>
#include <QIcon>
#include <qfactoryinterface.h>

#include <qmailmessage.h>
#include <qtopiaglobal.h>

class QMailComposerInterface;

struct QTOPIAMAIL_EXPORT QMailComposerPluginInterface : public QFactoryInterface
{
    virtual bool isSupported( QMailMessage::MessageType type ) const = 0;
    virtual QMailComposerInterface* create( QWidget* parent ) = 0;

    virtual QString key() const = 0;
    virtual QMailMessage::MessageType messageType() const = 0;

    virtual QString name() const = 0;
    virtual QString displayName() const = 0;
    virtual QIcon displayIcon() const = 0;
};

#define QMailComposerPluginInterface_iid "com.trolltech.Qtopia.Qtopiamail.QMailComposerPluginInterface"
Q_DECLARE_INTERFACE(QMailComposerPluginInterface, QMailComposerPluginInterface_iid)

class QTOPIAMAIL_EXPORT QMailComposerPlugin : public QObject, public QMailComposerPluginInterface
{
    Q_OBJECT
    Q_INTERFACES(QMailComposerPluginInterface:QFactoryInterface)

public:
    QMailComposerPlugin();
    ~QMailComposerPlugin();

    virtual QStringList keys() const;

    virtual bool isSupported( QMailMessage::MessageType type ) const;
};

#endif /* QMAILCOMPOSERPLUGIN_H */
