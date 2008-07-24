/***************************************************************************
**
** Copyright (C) 2008 Openmoko Inc.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2 or at your option any later version.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
***************************************************************************/

#include "qemailsupport.h"

#include <qtopiaglobal.h>
#include <qsettings.h>
#include <qdebug.h>
#include <QtopiaFeatures>



namespace QEmailSupport {

static const char * const HAS_EMAIL_ACCOUNT_FEATURE = "hasEmailSupport";

bool _getEmailConf(QSettings *conf, int count) 
{
    int i;
    for (i=0;i< count;i++) {
        conf->beginGroup( "account_" + QString::number(i) );
        if (!conf->value("smtpserver").toString().isEmpty()) {
            return true;
        }
        conf->endGroup();
    }
    return false;
}

QTOPIAMAIL_EXPORT bool updateEmailSupportFeature()
{
    int count;
    QSettings accountconf("Trolltech","qtmail_account");
    accountconf.beginGroup( "accountglobal" );
    count =  accountconf.value("accounts", 0).toInt();
    accountconf.endGroup();
    if(count==0) {
        QtopiaFeatures::removeFeature( HAS_EMAIL_ACCOUNT_FEATURE );
        return false;
    }
    if (_getEmailConf(&accountconf, count)) 
        QtopiaFeatures::setFeature( HAS_EMAIL_ACCOUNT_FEATURE );
    else
        QtopiaFeatures::removeFeature( HAS_EMAIL_ACCOUNT_FEATURE );
    return QtopiaFeatures::hasFeature( HAS_EMAIL_ACCOUNT_FEATURE );
}

}
