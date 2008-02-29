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

#include "messageviewer.h"
#include <qtopiaapplication.h>

// Comment out this line to use a manual main() function.
// Ensure you also remove CONFIG+=qtopia_main from application.pro if you do this.
#define USE_THE_MAIN_MACROS



#ifdef USE_THE_MAIN_MACROS

QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, MessageViewer)
QTOPIA_MAIN

#else

#ifdef SINGLE_EXEC
QTOPIA_ADD_APPLICATION(QTOPIA_TARGET, exampleapp)
#define MAIN_FUNC main_exampleapp
#else
#define MAIN_FUNC main
#endif

// This is the storage for the SXE key that uniquely identified this applicaiton.
// make will fail without this!
QSXE_APP_KEY

int MAIN_FUNC( int argc, char **argv )
{
    // This is required to load the SXE key into memory
    QSXE_SET_APP_KEY(argv[0]);

    QtopiaApplication a( argc, argv );

    // Set the preferred document system connection type
    QTOPIA_SET_DOCUMENT_SYSTEM_CONNECTION();

    MessageViewer *mw = new MessageViewer();
    a.setMainWidget(mw);
    if ( mw->metaObject()->indexOfSlot("setDocument(QString)") != -1 ) {
        a.showMainDocumentWidget();
    } else {
        a.showMainWidget();
    }
    int rv = a.exec();
    delete mw;
    return rv;
}

#endif

