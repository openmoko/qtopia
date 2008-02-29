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
#ifndef QMIMETYPEDATA_P_H
#define QMIMETYPEDATA_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qtopia API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include <QContent>

class QMimeTypeDataPrivate;

class QMimeTypeData
{
public:
    QMimeTypeData();
    QMimeTypeData( const QString &id );
    QMimeTypeData( const QMimeTypeData &other );
    ~QMimeTypeData();

    QMimeTypeData &operator =( const QMimeTypeData &other );

    bool operator ==( const QMimeTypeData &other );

    QString id() const;

    QContentList applications() const;

    QContent defaultApplication() const;

    QIcon icon( const QContent &application ) const;
    QIcon validDrmIcon( const QContent &application ) const;
    QIcon invalidDrmIcon( const QContent &application ) const;
    QDrmRights::Permission permission( const QContent &application ) const;

    void addApplication( const QContent &application, const QString &iconFile, QDrmRights::Permission permission );
    void removeApplication( const QContent &application );
    void setDefaultApplication( const QContent &application );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QSharedDataPointer< QMimeTypeDataPrivate > d;
};

Q_DECLARE_USER_METATYPE(QMimeTypeData);

#endif
