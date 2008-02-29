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

#ifndef POLICY_H
#define POLICY_H

#include <QStringList>
#include <QCache>
#include <QHash>
#include <QMap>
#include <QMutex>

#include <qtopiaglobal.h>

#include <qtransportauth_qws.h>

#include <time.h>

class QTOPIASECURITY_EXPORT SXEPolicyManager : public QObject
{
    Q_OBJECT
public:
    static SXEPolicyManager *getInstance();
    ~SXEPolicyManager();
    QStringList findPolicy( unsigned char progId );
    QString findRequest( QString request, QStringList prof );
public slots:
    void policyCheck( QTransportAuth::Data &, const QString & );
    void resetDateCheck();
private:
    SXEPolicyManager();
    bool readProfiles();
    QString checkWildcards( const QString &, const QStringList & );
    QMultiHash<QString,QString> requestHash;
    QCache<unsigned char,QStringList> policyCache;
    QMap<QString,QString> wildcards;
    bool checkDate;
    QMutex policyMutex;
};

#endif
