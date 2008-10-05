/****************************************************************************
**
** This file is part of the Qt Extended Opensource Package.
**
** Copyright (C) 2008 Trolltech ASA.
**
** Contact: Qt Extended Information (info@qtextended.org)
**
** This file may be used under the terms of the GNU General Public License
** version 2.0 as published by the Free Software Foundation and appearing
** in the file LICENSE.GPL included in the packaging of this file.
**
** Please review the following information to ensure GNU General Public
** Licensing requirements will be met:
**     http://www.fsf.org/licensing/licenses/info/GPLv2.html.
**
**
****************************************************************************/

#ifndef QSERIALIODEVICE_H
#define QSERIALIODEVICE_H

#include <qtopiaglobal.h>

#include <qobject.h>
#include <qiodevice.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qprocess.h>


class QPseudoTtyProcess;
class QAtChat;

class QTOPIACOMM_EXPORT QSerialIODevice : public QIODevice
{
    Q_OBJECT
    friend class QPseudoTtyProcess;
public:
    explicit QSerialIODevice( QObject *parent = 0 );
    ~QSerialIODevice();

    bool isSequential() const;

    virtual int rate() const;
    virtual bool dtr() const = 0;
    virtual void setDtr( bool value ) = 0;
    virtual bool dsr() const = 0;
    virtual bool carrier() const = 0;
    virtual bool setCarrier( bool value );
    virtual bool rts() const = 0;
    virtual void setRts( bool value ) = 0;
    virtual bool cts() const = 0;

    virtual void discard() = 0;

    virtual bool waitForReady() const;

    virtual QProcess *run( const QStringList& arguments, bool addPPPdOptions );

    virtual QAtChat *atchat();

    virtual void abortDial();

    virtual bool isValid() const;

signals:
    void dsrChanged( bool value );
    void carrierChanged( bool value );
    void ctsChanged( bool value );
    void ready();

protected slots:
    void internalReadyRead();

private:
    QPseudoTtyProcess *process;
    QAtChat *chat;
};


class QTOPIACOMM_EXPORT QNullSerialIODevice : public QSerialIODevice
{
    Q_OBJECT
public:
    explicit QNullSerialIODevice( QObject *parent = 0 );
    ~QNullSerialIODevice();

    bool open( OpenMode mode );
    void close();
    qint64 bytesAvailable() const;
    int rate() const;
    bool dtr() const;
    void setDtr( bool value );
    bool dsr() const;
    bool carrier() const;
    bool rts() const;
    void setRts( bool value );
    bool cts() const;
    void discard();
    bool isValid() const;

protected:
    qint64 readData( char *data, qint64 maxlen );
    qint64 writeData( const char *data, qint64 len );
};

#endif
