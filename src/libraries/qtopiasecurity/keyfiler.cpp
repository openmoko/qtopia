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

#include "keyfiler_p.h"
#ifdef SXE_INSTALLER
#endif

#include <time.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>

#include <qtopiasxe.h>

#include <qtopialog.h>


// "/dev/urandom" has possibly inferior source of randomness, but will not
// block - can switch to "/dev/random" if this causes problems
#define RAND_DEV "/dev/urandom"

    KeyFiler::KeyFiler()
: bin( 0 )
{
}

KeyFiler::~KeyFiler()
{
}

void KeyFiler::setFileName( const QString &fn )
{
    bin.setFileName( fn );
    if ( !bin.open( QIODevice::ReadWrite ))
        qFatal( "Couldnt open %s for SXE install", qPrintable( fn ));
}


/*!
  \internal
  Read some random bytes into \a keybuf.

  If on linux (ie read embedded) the os MUST have /dev/[u]random
  available.
  */
void KeyFiler::randomizeKey( char keybuf[] )
{
    qint64 rc;
    QFile rf( RAND_DEV );
    if ( !rf.open( QIODevice::ReadOnly ))
    {
        time_t curTime = time(0);
        perror( "open " RAND_DEV );
#if defined(Q_OS_LINUX) || defined(_OS_LINUX_)
        qFatal( "Safe execution requires " RAND_DEV );
#endif
        qWarning( "Warning: Using inferior random numbers!" );
        srandom(curTime);  // this is very crappy
        long int r;
        for ( int i = 0; i < QSXE_KEY_LEN; i++ )
        {
            r = random();
            keybuf[i] = (char)r;
        }
    }
    else
    {
        rc = rf.read( keybuf, QSXE_KEY_LEN );
        rf.close();
        if ( rc != QSXE_KEY_LEN )
            qFatal( "read %s: %s", RAND_DEV, strerror(errno) );
    }
}

/*
   Write an authorisation cookie \a ckToWrite into the binary on disk.

   The binary has been successfully opened to the file descriptor
   stored in the member variable fileDescriptor.
   */
bool KeyFiler::scanFile( AuthCookie *ckToWrite )
{
    char *fileBytes;
    char *match_start;
    const char *eof;
    unsigned int fileSize;
    const char *key_pattern = QSXE_KEY_TEMPLATE;
    unsigned int target_len = strlen( key_pattern );

    Q_ASSERT( target_len >= sizeof( ckToWrite ));

    if ( !bin.isOpen() )
        qFatal( "%s not open in scanfile", qPrintable( bin.fileName() ));

    fileSize = bin.size();
    fileBytes = (char*)::mmap( 0, fileSize, PROT_READ|PROT_WRITE, MAP_SHARED, bin.handle(), 0 );
    if ( fileBytes == MAP_FAILED )
    {
        qWarning( "Could not mmap %s : %s", qPrintable( bin.fileName() ), strerror( errno ));
        return false;
    }
    match_start = fileBytes;
    eof = fileBytes + fileSize;
    while (( match_start + target_len ) < eof )
    {
        if ( ::memcmp( match_start, key_pattern, target_len ) == 0 )
        {
            ::memcpy( match_start, ckToWrite, sizeof( AuthCookie ));
            ::munmap( fileBytes, fileSize );
            return true;
        }
        match_start++;
    }
    qLog(SXE) << "\tkey template not found";
    ::munmap( fileBytes, fileSize );
    return false;
}
