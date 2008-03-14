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

#include "obexclientwindow.h"
#include "obexquoteserver.h"
#include <qtopiaapplication.h>

/*
    This demonstrates the basics of using Qtopia's OBEX libraries to set up
    an OBEX server and then connect OBEX clients to that server.

    For this example, the OBEX sessions are run over TCP using QTcpSocket,
    but they could have used any transport provided by a subclass of
    QIODevice. For example, Qtopia's QBluetoothRfcommSocket class could have
    been used to run the OBEX sessions over Bluetooth instead.

    More example code is provided in the QObexClientSession and
    QObexServerSession class documentation.
*/

QSXE_APP_KEY
int main( int argc, char **argv )
{
    QSXE_SET_APP_KEY(argv[0])
    QtopiaApplication app( argc, argv );

    ObexQuoteServer server;
    if (server.run()) {
        ObexClientWindow win(server.serverAddress(), server.serverPort());
        win.show();
    } else {
        qWarning("Unable to run OBEX example server");
    }

    return app.exec();
}
