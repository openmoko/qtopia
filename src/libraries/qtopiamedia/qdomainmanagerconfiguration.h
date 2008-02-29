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

#ifndef __QTOPIA_MEDIALIBRARY_DOMAINMANAGERCONFIGURATION_H
#define __QTOPIA_MEDIALIBRARY_DOMAINMANAGERCONFIGURATION_H

#include <QObject>

#include <qtopiaglobal.h>

#include "qaudiodomain.h"


class QDomainManagerConfigurationPrivate;

class QTOPIAMEDIA_EXPORT QDomainManagerConfiguration : public QObject
{
    Q_OBJECT

    friend class QDomainManagerConfigurationPrivate;

public:
    explicit QDomainManagerConfiguration(QObject* parent = 0);
    ~QDomainManagerConfiguration();

signals:
    void activeDomainChanged(QAudioDomainList const& activeDomains,
                             QAudioDomainList const& inactiveDomains);

private:
    QDomainManagerConfigurationPrivate* d;
};

#endif  // __QTOPIA_MEDIALIBRARY_DOMAINMANAGERCONFIGURATION_H
