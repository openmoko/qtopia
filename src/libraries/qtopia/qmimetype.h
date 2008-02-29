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
#ifndef QMIMETYPE_H
#define QMIMETYPE_H

#include <qtopiaglobal.h>
#include <qstringlist.h>
#include <qicon.h>
#include <qlist.h>
#include <QMutex>

#include <qdrmcontent.h>
#include <qcontentset.h>

class QStringList;
class QMimeTypeData;

class QTOPIA_EXPORT QMimeType
{
public:
    explicit QMimeType( const QString& ext_or_id );
    explicit QMimeType( const QContent& );

    enum IconType
    {
        Default,
        DrmValid,
        DrmInvalid
    };

    QString id() const;
    QString description() const;
    QIcon icon( IconType iconType = Default ) const;

    QString extension() const;
    QStringList extensions() const;
    QContentList applications() const;
    QContent application() const;

    QDrmRights::Permission permission() const;
    QList<QDrmRights::Permission> permissions() const;

    static QString appsFolderName();
    static void updateApplications();

private:
    static void clear();
    static void registerApp( const QContent& );
    static void loadExtensions();
    static void loadExtensions(const QString&);
    static void initializeAppManager();
    void init( const QString& ext_or_id );
    class Private;
    static Private* d;
    static Private& data();
    static QMimeTypeData* data(const QString& id);
    QString mimeId;
    static QMutex staticsGuardMutex;

    friend class QMimeTypeData;
    friend class QMimeTypeAppManager;
    friend class QtopiaApplication;
};

#endif
