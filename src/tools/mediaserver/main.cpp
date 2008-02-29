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

#include <qtopiaapplication.h>

#include "mediaserver.h"
#include "qtopiamediaprovider.h"
#include "qsoundprovider.h"

using namespace mediaserver;

QSXE_APP_KEY
int main(int argc, char** argv)
{
    QSXE_SET_APP_KEY(argv[0])

    QtopiaApplication   app(argc, argv);

    MediaServer         server(argc, argv);

    QtopiaMediaProvider qmp(server.mediaEngine());
    QSoundProvider      qsp(server.mediaEngine());

    app.registerRunningTask("MediaServer", &server);

    return app.exec();
}

