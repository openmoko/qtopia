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

#ifndef QDL_LINK_H
#define QDL_LINK_H

// Qtopia includes
#include <Qtopia>
#include <qtopiaipcadaptor.h>
#include <qtopiaipcmarshal.h>
#include <QMimeType>

// Forward class declarations
class QDLLinkPrivate;
class QDSData;

// ============================================================================
//
// QDLLink
//
// ============================================================================

class QTOPIA_EXPORT QDLLink
{
public:
    QDLLink();
    QDLLink( const QString &service,
             const QByteArray &data,
             const QString &description,
             const QString &icon );
    QDLLink( const QDLLink &other );
    explicit QDLLink( const QDSData& dataObject );

    ~QDLLink();

    // Operators
    QDLLink &operator=( const QDLLink &other );

    // Access
    static QMimeType mimeType();
    static QMimeType listMimeType();
    bool isNull() const;
    bool isBroken() const;

    QString service() const;
    QByteArray data() const;
    QString description() const;
    QString icon() const;
    QDSData toQDSData() const;

    void activate() const;

    // Modification
    void setService( const QString &service );
    void setData( const QByteArray &data );
    void setDescription( const QString &description );
    void setIcon( const QString &icon );
    void setBroken( const bool broken = true );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDLLinkPrivate* d;
};

// Macros
Q_DECLARE_USER_METATYPE(QDLLink)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QDLLink>)

#endif //QDL_LINK_H
