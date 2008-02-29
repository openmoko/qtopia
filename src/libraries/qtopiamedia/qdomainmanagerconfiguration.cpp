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

#include <QValueSpaceItem>

#include "qdomainmanagerconfiguration.h"


class QDomainManagerConfigurationPrivate : public QObject
{
    Q_OBJECT

public:
    QDomainManagerConfigurationPrivate(QDomainManagerConfiguration* config):
        m_config(config)
    {
        m_configStatus = new QValueSpaceItem("/Media/Domain", this);

        connect(m_configStatus, SIGNAL(contentsChanged()),
                this,  SLOT(contentsChanged()));
    }

    ~QDomainManagerConfigurationPrivate()
    {
        delete m_configStatus;
    }

private slots:
    void contentsChanged()
    {
        m_config->activeDomainChanged(qVariantValue<QAudioDomainList>(m_configStatus->value("ActiveDomains")),
                                      qVariantValue<QAudioDomainList>(m_configStatus->value("InactiveDomains")));
    }

private:
    QValueSpaceItem*                m_configStatus;
    QDomainManagerConfiguration*    m_config;
};


QDomainManagerConfiguration::QDomainManagerConfiguration(QObject* parent):
    QObject(parent)
{
    d = new QDomainManagerConfigurationPrivate(this);
}

QDomainManagerConfiguration::~QDomainManagerConfiguration()
{
    delete d;
}


#include "qdomainmanagerconfiguration.moc"

