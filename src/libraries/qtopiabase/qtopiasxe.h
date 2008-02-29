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

#ifndef QTOPIASXE_H
#define QTOPIASXE_H

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER)

#ifdef Q_WS_QWS
#include <qtransportauth_qws.h>
#endif

#define QSXE_KEY_TEMPLATE "XOXOXOauthOXOXOX99"
#define QSXE_APP_KEY char _key[] = QSXE_KEY_TEMPLATE;
#define QSXE_QL_APP_KEY char _ql_key[] = QSXE_KEY_TEMPLATE;
#define QSXE_SET_QL_KEY(APPNAME) QTransportAuth::getInstance()->setProcessKey( _ql_key, APPNAME );
#define QSXE_SET_APP_KEY(APPNAME) QTransportAuth::getInstance()->setProcessKey( _key, APPNAME );

#else

#define QSXE_KEY_TEMPLATE
#define QSXE_APP_KEY
#define QSXE_QL_APP_KEY
#define QSXE_SET_QL_KEY(APPNAME)
#define QSXE_SET_APP_KEY(APPNAME)

#endif  // QT_NO_SXE

#endif  // QTOPIASXE_H
