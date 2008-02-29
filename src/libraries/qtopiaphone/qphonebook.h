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
#ifndef QPHONEBOOK_H
#define QPHONEBOOK_H

#include <qcomminterface.h>
#include <qtopiaipcmarshal.h>
#include <qtelephonynamespace.h>

#include <qstringlist.h>
#include <qlist.h>

class QTOPIAPHONE_EXPORT QPhoneBookEntry
{
public:
    QPhoneBookEntry() { _index = 0; }
    ~QPhoneBookEntry() {}

    //methods to satisfy qvaluelist
    QPhoneBookEntry( const QPhoneBookEntry &other )
    {
        _index = other._index;
        _number = other._number;
        _text = other._text;
    }

    QPhoneBookEntry &operator=( const QPhoneBookEntry &other )
    {
        _index = other._index;
        _number = other._number;
        _text = other._text;
        return *this;
    }

    bool operator==( const QPhoneBookEntry &other )
    {
        return _index == other._index;
    }

    uint index() const { return _index; }
    void setIndex( uint value ) { _index = value; }

    QString number() const { return _number; }
    void setNumber( const QString& value ) { _number = value; }

    QString text() const { return _text; }
    void setText( const QString& value ) { _text = value; }

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    uint _index;
    QString _number;
    QString _text;
};

class QPhoneBookLimitsPrivate;

class QTOPIAPHONE_EXPORT QPhoneBookLimits
{
public:
    QPhoneBookLimits();
    QPhoneBookLimits( const QPhoneBookLimits& other );
    ~QPhoneBookLimits();

    bool isNull() const;

    QPhoneBookLimits& operator=( const QPhoneBookLimits& other );

    uint numberLength() const;
    void setNumberLength( uint value );

    uint textLength() const;
    void setTextLength( uint value );

    uint firstIndex() const;
    void setFirstIndex( uint value );

    uint lastIndex() const;
    void setLastIndex( uint value );

    uint used() const;
    void setUsed( uint value );

    template <typename Stream> void serialize(Stream &stream) const;
    template <typename Stream> void deserialize(Stream &stream);

private:
    QPhoneBookLimitsPrivate *d;
};

class QTOPIAPHONE_EXPORT QPhoneBook : public QCommInterface
{
    Q_OBJECT
public:
    explicit QPhoneBook( const QString& service = QString(), QObject *parent = 0,
                         QCommInterface::Mode mode = Client );
    ~QPhoneBook();

public slots:
    virtual void getEntries( const QString& store = "SM" );
    virtual void add( const QPhoneBookEntry& entry, const QString& store = "SM", bool flush = true );
    virtual void remove( uint index, const QString& store = "SM", bool flush = true );
    virtual void update( const QPhoneBookEntry& entry, const QString& store = "SM", bool flush = true );
    virtual void flush( const QString& store = "SM" );
    virtual void setPassword( const QString& store, const QString& password );
    virtual void clearPassword( const QString& store );
    virtual void requestLimits( const QString& store = "SM" );
    virtual void requestFixedDialingState();
    virtual void setFixedDialingState( bool enabled, const QString& pin2 );

public:
    QStringList storages();

signals:
    void entries( const QString& store, const QList<QPhoneBookEntry>& list );
    void limits( const QString& store, const QPhoneBookLimits& value );
    void fixedDialingState( bool enabled );
    void setFixedDialingStateResult( QTelephony::Result result );
    void ready();
};

Q_DECLARE_USER_METATYPE(QPhoneBookEntry)
Q_DECLARE_USER_METATYPE_NO_OPERATORS(QList<QPhoneBookEntry>)
Q_DECLARE_USER_METATYPE(QPhoneBookLimits)

#endif // QPHONEBOOK_H
