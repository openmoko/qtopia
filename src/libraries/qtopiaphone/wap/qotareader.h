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
#ifndef QOTAREADER_H
#define QOTAREADER_H

#include <qwbxmlreader.h>

#include <QMap>
#include <QVariant>
#include <qtopianetworkinterface.h>

class QOtaCharacteristic;
class QOtaCharacteristicList;
class QOtaCharacteristicPrivate;

typedef QMap<QString, QString> QOtaParameters;
typedef QMap<QString, QList<QOtaCharacteristic> > QOtaCharacteristicListBase;

class QTOPIAPHONE_EXPORT QOtaCharacteristic
{
public:
    QOtaCharacteristic();
    QOtaCharacteristic( const QOtaCharacteristic& c );
    ~QOtaCharacteristic();

    void clear();

    const QString& type() const;
    void setType( const QString& type );

    const QOtaParameters& parms() const;
    void addParm( const QString& name, const QString& value );

    const QOtaCharacteristicList& children() const;
    void addChild( QOtaCharacteristic& child );

    QOtaCharacteristic& operator=( const QOtaCharacteristic& c );

private:
    QOtaCharacteristicPrivate *d;
};


class QTOPIAPHONE_EXPORT QOtaCharacteristicList : public QOtaCharacteristicListBase
{
public:
    QOtaCharacteristicList() {}
    QOtaCharacteristicList( const QOtaCharacteristicList& list )
        : QOtaCharacteristicListBase( list ) {}
    ~QOtaCharacteristicList() {}

    QtopiaNetworkProperties toConfig() const;

    QString parameter( const QString& name1 ) const;
    QString parameter( const QString& name1, const QString& name2 ) const;
    QString parameter( const QString& name1, const QString& name2,
                       const QString& name3 ) const;
    QString parameter( const QString& name1, const QString& name2,
                       const QString& name3, const QString& name4 ) const;

    QString appParameter( const QString& app, const QString& name1 ) const;
    QString appParameter( const QString& app, const QString& name1,
                          const QString& name2 ) const;
    QString appParameter( const QString& app, const QString& name1,
                          const QString& name2, const QString& name3 ) const;

private:
    const QOtaParameters *section( const QString& name1 ) const;
    const QOtaParameters *section
        ( const QString& name1, const QString& name2 ) const;
    const QOtaParameters *section
        ( const QString& name1, const QString& name2,
          const QString& name3 ) const;

    const QOtaCharacteristic *appSection( const QString& app ) const;
    const QOtaParameters *appSection
        ( const QString& app, const QString& name1 ) const;
    const QOtaParameters *appSection
        ( const QString& app, const QString& name1,
          const QString& name2 ) const;

    static void add( QtopiaNetworkProperties& cfg,
                     const QString& name, const QString& value );

};


class QTOPIAPHONE_EXPORT QOtaReader : public QWbXmlReader
{
public:
    enum QOtaType { Nokia, Wap };

    QOtaReader( QOtaType type=Nokia );
    virtual ~QOtaReader();

    // Parse a binary-WBXML characteristics stream.
    QOtaCharacteristicList *parseCharacteristics( const QByteArray& input );
    QOtaCharacteristicList *parseCharacteristics( QIODevice& input );

    // Parse a plain-XML characteristics stream.
    static QOtaCharacteristicList *parseCharacteristics
        ( QXmlReader *reader, const QXmlInputSource& input );
};


#endif // QOTAREADER_H
