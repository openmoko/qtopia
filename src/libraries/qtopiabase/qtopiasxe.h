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

#ifndef QTOPIASXE_H
#define QTOPIASXE_H

#include <qtopiaglobal.h>

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER) || defined (Q_QDOC)

#include <QFile>

#if !defined(QTOPIA_HOST) || defined(SXE_INSTALLER)
#include <qtransportauth_qws.h>

/* from $QT_DEPOT_PATH/src/gui/embedded/qtransportauth_qws_p.h */
#define QSXE_KEY_LEN 16
#endif

#include <qtopiaglobal.h>
#include <QDebug>

class SxeProgramInfoPrivate;

struct QTOPIABASE_EXPORT SxeProgramInfo
{
    SxeProgramInfo();
    ~SxeProgramInfo();
    QString fileName;   // eg calculator, bomber
    QString relPath;    // eg bin, packages/bin
    QString runRoot;    // eg /opt/Qtopia.rom, /opt/Qtopia.user
    QString installRoot; // eg $HOME/build/qtopia/42-phone/image
    QString domain;     // security domains, csv
    unsigned char id;   // program identity
    char key[QSXE_KEY_LEN]; // key

    bool isValid() const;
    QString absolutePath() const;
    bool locateBinary();
    void suid();
private:
    SxeProgramInfoPrivate *d;
};

QTOPIABASE_EXPORT QDebug operator<<(QDebug debug, const SxeProgramInfo &progInfo);

#endif  // QT_NO_SXE

#ifdef SINGLE_EXEC
#define QSXE_KEY_TEMPLATE
#define QSXE_APP_KEY ;
#define QSXE_QL_APP_KEY ;
#define QSXE_SET_QL_KEY(APPNAME) ;
#define QSXE_SET_APP_KEY(APPNAME) ;
#else
QTOPIABASE_EXPORT void checkAndSetProcessKey( const char *key, const char *app );
#define QSXE_KEY_TEMPLATE "XOXOXOauthOXOXOX99"
#define QSXE_APP_KEY char _key[] = QSXE_KEY_TEMPLATE;
#define QSXE_QL_APP_KEY char _ql_key[] = QSXE_KEY_TEMPLATE;
#define QSXE_SET_QL_KEY(APPNAME) checkAndSetProcessKey( _ql_key, APPNAME );
#define QSXE_SET_APP_KEY(APPNAME) checkAndSetProcessKey( _key, APPNAME );
#endif

#endif  // QTOPIASXE_H
