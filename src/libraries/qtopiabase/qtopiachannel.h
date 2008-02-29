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

#ifndef __QTOPIACHANNEL_H__
#define __QTOPIACHANNEL_H__

#include <QObject>
#include <qtopiaglobal.h>

class QtopiaChannel_Private;
class QTOPIABASE_EXPORT QtopiaChannel : public QObject
{
    Q_OBJECT

public:
    explicit QtopiaChannel(const QString& channel, QObject *parent=0);
    virtual ~QtopiaChannel();

    QString channel() const;

    static bool isRegistered(const QString&  channel);
    static bool send(const QString& channel, const QString& msg);
    static bool send(const QString& channel, const QString& msg,
                     const QByteArray &data);
    static bool flush();

signals:
    void received(const QString& msg, const QByteArray &data);

private:
    QtopiaChannel_Private *m_data;
    friend class QtopiaChannel_Private;
};

#endif
