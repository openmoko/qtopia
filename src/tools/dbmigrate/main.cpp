/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include <QtopiaApplication>
#include <QDebug>
#include "migrateengine.h"
#include <private/qcontent_p.h>

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET,dbmigrate)
#define MAIN_FUNC main_dbmigrate
#else
#define MAIN_FUNC main
#endif

QSXE_APP_KEY
int MAIN_FUNC( int argc, char** argv )
{
    QSXE_SET_APP_KEY(argv[0]);

    int i;

    QStringList args;
    for(i=1; i<argc; i++)
        args.append(argv[i]);
    qLog(DocAPI) << "dbmigrate called with parameters..." << args;
    if(args.contains("--systemupgrade"))
    {
        if(args.contains("--qws"))
        {
            qWarning() << "\"dbmigrate --systemupgrade\" will not work with the --qws command line option\n"
                       << " as it is designed to be run in this mode as a standalone application";
            return -1;
        }
        args.clear();
        QApplication app(argc, argv, false);
        app.setApplicationName( QLatin1String( "dbmigrate" ) );
        if(app.arguments().count() > 2)
            for(i=1; i<app.arguments().count(); i++)
                if(app.arguments().at(i) != "--systemupgrade")
                    args.append(app.arguments().at(i));
        if(QDBMigrationEngine::instance()->doMigrate(args))
            return 0;
        else
            return -1;
    }
    else
    {
        QtopiaApplication app(argc, argv);
        MigrationEngineService service(NULL);
        return app.exec();
    }
}
