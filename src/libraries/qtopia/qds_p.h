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

#ifndef QDS_P_H
#define QDS_P_H

// Qtopia includes
#include <Qtopia>

// Qt includes
#include <QString>

namespace QDS
{
    static const QString SERVICE_TAG        = "QDS"; // No tr
    static const QString PROVIDER_CHANNEL   = "QPE/Application/"; // No tr
    static const QString RESPONSE_CHANNEL   = "QDSResponse"; // No tr
    static const QString SERVICES_DIR_END   = "qds"; // No tr
    static const QString SERVICES_DIR       = Qtopia::qtopiaDir() +
                                              "etc/" +
                                              SERVICES_DIR_END; // No tr

    static const int SERVERING_HEARTBEAT_PERIOD = 1000;
    static const int REQUEST_TIMEOUT            = 20000;
}

#endif //QDS__P_H
