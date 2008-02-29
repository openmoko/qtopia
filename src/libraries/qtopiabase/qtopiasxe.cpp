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

#include "qtopiasxe.h"

/*!
  \class SxeProgramInfo
  \mainclass
  \brief The SxeProgramInfo class is a data transfer object that models a program on disk.

  Used for registration of binaries with the SXE system.

  Registration can happen at run-time of the qtopia device
  or at install-time, when building the qtopia ROM image on
  a development host.

  For example, consider the following binaries installed onto
  a device:
  \code
  /opt/Qtopia.rom/bin/calculator
  /opt/Qtopia.user/packages/bin/bomber
  \endcode

  These examples are referenced in the documentation of public variables
  below.  The table below summarizes the difference between "make install"
  time and run time.

  \table
  \header
    \o install root on dev host
    \o run root on device
  \row
    \o $HOME/build/qtopia42/image
    \o /opt/Qtopia.rom
  \endtable

  The runRoot must be one of the elements which will be
  returned by the Qtopia::installPaths() method at runtime.

  The installRoot is optional and will be empty in the case of
  a binary installed at run-time by the package manager.
*/

/*!

  \variable SxeProgramInfo::fileName
    the binaries file name, eg calculator, bomber
*/

/*!

  \variable SxeProgramInfo::relPath
    for example \c bin, \c packages/bin
*/

/*!

  \variable SxeProgramInfo::runRoot
    for example \c /opt/Qtopia.rom, \c /opt/Qtopia.user
*/

/*!

  \variable SxeProgramInfo::installRoot
    for example \c {$HOME/build/qtopia/42-phone/image}
*/

/*!

  \variable SxeProgramInfo::domain
    security domains, a list of comma seperated values
*/

/*!

  \variable SxeProgramInfo::id
    SXE program identity
*/

/*!

  \variable SxeProgramInfo::key
    SXE shared secret key
*/

/*!
  \fn SxeProgramInfo::SxeProgramInfo()

  Construct a new SxeProgramInfo
*/

/*!
  \fn SxeProgramInfo::~SxeProgramInfo()

  Destroy this SxeProgramInfo object
*/

#if !defined(QT_NO_SXE) || defined(SXE_INSTALLER)
void sxeInfoHexstring( char *buf, const unsigned char* key, size_t key_len )
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

/*!
  \relates SxeProgramInfo

  Sends the SxeProgramInfo \a progInfo to the \a debug stream.
*/
QDebug operator<<(QDebug debug, const SxeProgramInfo &progInfo)
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

/*!
  Return the QString of the absolute path to the binary for this SxeProgramInfo object.

  This is simply the concatenation of installRoot, relPath and fileName.  If the installRoot
  is null, the runRoot is used instead.
*/
QString SxeProgramInfo::absolutePath() const
{
    return ( installRoot.isEmpty() ? runRoot : installRoot ) +
        "/" + relPath + "/" + fileName;
}

/*!
  Return true if the SxeProgramInfo object represents a valid binary for SXE registration,
  and otherwise returns false.

  The binary is valid if all of the components of absolutePath() above are non-empty, and
  the resulting path is to a file which exists.
*/
bool SxeProgramInfo::isValid() const
{
#ifdef SXE_INSTALLER
    if ( installRoot.isEmpty() )
        return false;
#endif
    return !fileName.isEmpty() && !relPath.isEmpty() &&
        !runRoot.isEmpty() && QFile::exists( absolutePath() );
}

#endif
