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

#ifndef QDIALOPTIONS_H
#define QDIALOPTIONS_H

#include <quniqueid.h>

class QDialOptionsPrivate;

class QTOPIAPHONE_EXPORT QDialOptions
{
public:
    QDialOptions();
    QDialOptions( const QDialOptions& other );
    ~QDialOptions();

    QDialOptions& operator=( const QDialOptions& other );

    QString number() const;
    void setNumber( const QString& value );

    enum CallerId
    {
        DefaultCallerId,
        SendCallerId,
        SuppressCallerId
    };

    QDialOptions::CallerId callerId() const;
    void setCallerId( QDialOptions::CallerId value );

    bool closedUserGroup() const;
    void setClosedUserGroup( bool value );

    QUniqueId contact() const;
    void setContact( const QUniqueId& value );

    int speed() const;
    void setSpeed( int value );

    int gsmSpeed() const;
    void setGsmSpeed( int value );

    enum Bearer
    {
        DataCircuitAsyncUDI,
        DataCircuitSyncUDI,
        PadAccessUDI,
        PacketAccessUDI,
        DataCircuitAsyncRDI,
        DataCircuitSyncRDI,
        PadAccessRDI,
        PacketAccessRDI
    };

    QDialOptions::Bearer bearer() const;
    void setBearer( QDialOptions::Bearer value );

    enum TransparentMode
    {
        Transparent,
        NonTransparent,
        TransparentPreferred,
        NonTransparentPreferred
    };

    QDialOptions::TransparentMode transparentMode() const;
    void setTransparentMode( QDialOptions::TransparentMode value );

    QVariant extensionOption
        ( const QString& name, const QVariant& def = QVariant() ) const;
    void setExtensionOption( const QString& name, const QVariant& value );

    bool useIpModule() const;
    void setUseIpModule( bool value );

    QString ipProgramName() const;
    void setIpProgramName( const QString& value );

    QStringList ipArgs() const;
    void setIpArgs( const QStringList& value );

    QString ipConnectScript() const;
    void setIpConnectScript( const QString& filename );

    QString ipDisconnectScript() const;
    void setIpDisconnectScript( const QString& filename );

    bool ipDemandDialing() const;
    void setIpDemandDialing( bool value );

    int contextId() const;
    void setContextId( int value );

    QString pdpType() const;
    void setPdpType( const QString& value );

    QString apn() const;
    void setApn( const QString& value );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QDialOptionsPrivate *d;
};

Q_DECLARE_USER_METATYPE(QDialOptions)

#endif // QDIALOPTIONS_H
