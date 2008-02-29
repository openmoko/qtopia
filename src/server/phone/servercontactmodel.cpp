/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "servercontactmodel.h"
#include <QSettings>

ServerContactModel::ServerContactModel()
    : QContactModel()
{
    readSettings();
}

void ServerContactModel::readSettings()
{
    QSettings config( "Trolltech", "Contacts" );

    // load SIM/No SIM settings.
    config.beginGroup( "default" );
    if (config.contains("SelectedSources/size")) {
        int count = config.beginReadArray("SelectedSources");
        QSet<QPimSource> set;
        for(int i = 0; i < count; ++i) {
            config.setArrayIndex(i);
            QPimSource s;
            s.context = QUuid(config.value("context").toString());
            s.identity = config.value("identity").toString();
            set.insert(s);
        }
        config.endArray();
        setVisibleSources(set);
    }
}

ServerContactModel::~ServerContactModel()
{
}

ServerContactModel *ServerContactModel::mInstance = 0;

ServerContactModel *ServerContactModel::instance()
{
    if (!mInstance)
        mInstance = new ServerContactModel();

    mInstance->readSettings();
    return mInstance;
}
