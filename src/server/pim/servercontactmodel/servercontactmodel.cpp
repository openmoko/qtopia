/****************************************************************************
**
** Copyright (C) 2008-2008 TROLLTECH ASA. All rights reserved.
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

#include "servercontactmodel.h"
#include <QSettings>


/*!
    \class ServerContactModel
    \ingroup QtopiaServer
    \brief The ServerContactModel class provides access to a singleton instance of a QContactModel.

    The model extends the QContactModel to read the Trolltech/Contacts settings on construction to include the same sources of contact data as configured in the Qtopia Contacts application.

    This class is part of the Qtopia server and cannot be used by other applications.
*/

/*!
  Constructs a new ServerContactModel.
*/
ServerContactModel::ServerContactModel()
    : QContactModel()
{
    readSettings();
}

/*!
  Reads settings information for the contact model.  This includes
  the set of contact sources as configured in the Contacts application.
*/
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

/*!
  Destroys the ServerContactModel.
  */
ServerContactModel::~ServerContactModel()
{
}

ServerContactModel *ServerContactModel::mInstance = 0;

/*!
  Returns the instance of a ServerContactModel.  If an instance
  has not yet been constructed a new ServerContactModel will be
  constructed.
*/
ServerContactModel *ServerContactModel::instance()
{
    if (!mInstance)
        mInstance = new ServerContactModel();

    mInstance->readSettings();
    return mInstance;
}
