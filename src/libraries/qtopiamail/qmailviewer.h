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

#ifndef QMAILVIEWER_H
#define QMAILVIEWER_H

#include <QObject>
#include <QString>
#include <QVariant>

#include <qtopiaglobal.h>

class QMenu;
class QMailMessage;
class QUrl;
class QWidget;

// The interface for objects able to view mail messages
class QTOPIAMAIL_EXPORT QMailViewerInterface : public QObject
{
    Q_OBJECT

public:
    QMailViewerInterface( QWidget* parent = 0 );
    virtual ~QMailViewerInterface();

    virtual QWidget *widget() const = 0;

    virtual void scrollToAnchor(const QString& link);

    virtual void addActions(QMenu* menu) const;

public slots:
    virtual bool setMessage(const QMailMessage& mail) = 0;
    virtual void setResource(const QUrl& name, QVariant value);
    virtual void clear() = 0;

signals:
    void anchorClicked(const QUrl &link);
    void finished();
};

class QTOPIAMAIL_EXPORT QMailViewerFactory
{
public:
    enum ContentType
    {
        StaticContent = 0,
        SmilContent = 1,
        AnyContent = 2
    };

    // Yield the ID for each interface supporting the supplied type, where the
    // value is interpreted as a ContentType value
    static QStringList keys( ContentType type = AnyContent );

    // Yield the default ID for the supplied type
    static QString defaultKey( ContentType type = AnyContent );

    // Use the interface identified by the supplied ID to create a viewer
    static QMailViewerInterface *create( const QString &key, QWidget *parent = 0 );
};

#endif // QMAILVIEWER_H
