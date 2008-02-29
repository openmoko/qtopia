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

#include <qdebug.h>
#include <QFile>

#if defined(Q_WS_QWS) || defined(SXE_INSTALLER)
#include <qtransportauth_qws.h>

/* from $QT_DEPOT_PATH/src/gui/embedded/qtransportauth_qws_p.h */
#define QSXE_KEY_LEN 16
#endif

/*!
  \class SxeProgramInfo
  \brief Data transfer object to model a program on disk.

  Used for registration of binaries with the SXE system.

  Registration can happen at run-time of the qtopia device
  or at install-time, when building the qtopia ROM image on
  a development host.

  The field comments below have "eg" examples for the
  following:

  /opt/Qtopia.rom/bin/calculator

     make install on dev host -> $HOME/build/qtopia42/image
     run from on device       -> /opt/Qtopia.rom

  /opt/Qtopia.user/packages/bin/bomber

     installed by packagemanager into above

  The runRoot should be one of the elements which will be
  returned by the Qtopia::installPaths() method at runtime.

  The installRoot is optional and will be empty in the case of
  a binary installed at run-time by the package manager.
*/
struct SxeProgramInfo
{
    SxeProgramInfo() : id( 0 ) { ::memset( key, 0, QSXE_KEY_LEN ); }
    ~SxeProgramInfo() {}
    QString fileName;   // eg calculator, bomber
    QString relPath;    // eg bin, packages/bin
    QString runRoot;    // eg /opt/Qtopia.rom, /opt/Qtopia.user
    QString installRoot; // eg $HOME/build/qtopia/42-phone/image
    QString domain;     // security domains, csv
    unsigned char id;   // program identity
    char key[QSXE_KEY_LEN]; // key

    bool isValid() const;
    QString absolutePath() const;
};

inline void sxeInfoHexstring( char *buf, const unsigned char* key, size_t key_len )
{
    unsigned int i, p;
    for ( i = 0, p = 0; i < key_len; i++, p+=2 )
    {
        unsigned char lo_nibble = key[i] & 0x0f;
        unsigned char hi_nibble = key[i] >> 4;
        buf[p] = (int)hi_nibble > 9 ? hi_nibble-10 + 'A' : hi_nibble + '0';
        buf[p+1] = (int)lo_nibble > 9 ? lo_nibble-10 + 'A' : lo_nibble + '0';
    }
    buf[p] = '\0';
}

inline QDebug operator<<(QDebug debug, const SxeProgramInfo &progInfo)
{
    debug << "Sxe Program Info:" << progInfo.fileName << endl;
    debug << "\trelative path:" << progInfo.relPath << endl;
    debug << "\tinstall to:" << progInfo.installRoot << endl;
    debug << "\trun from:" << progInfo.runRoot << endl;
    debug << "\tSXE domains:" << progInfo.domain << endl;
    debug << "\tSXE id:" << progInfo.id << endl;
    char keydisp[QSXE_KEY_LEN*2+1];
    sxeInfoHexstring( keydisp, (const unsigned char *)progInfo.key, QSXE_KEY_LEN );
    debug << "\tSXE key:" << keydisp << endl;
    return debug;
}

inline QString SxeProgramInfo::absolutePath() const
{
    return ( installRoot.isEmpty() ? runRoot : installRoot ) +
        "/" + relPath + "/" + fileName;
}

inline bool SxeProgramInfo::isValid() const
{
#ifdef SXE_INSTALLER
    if ( installRoot.isEmpty() )
        return false;
#endif
    return !fileName.isEmpty() && !relPath.isEmpty() &&
        !runRoot.isEmpty() && QFile::exists( absolutePath() );
}

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
