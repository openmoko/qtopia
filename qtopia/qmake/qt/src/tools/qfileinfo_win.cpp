/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#include "qplatformdefs.h"

#include "qlibrary.h"
#include "qfileinfo.h"
#include "qfiledefs_p.h"
#include "qdatetime.h"
#include "qdir.h"
//#include "qapplication.h"

#ifdef QT_THREAD_SUPPORT
#  include <private/qmutexpool_p.h>
#endif // QT_THREAD_SUPPORT

#include <windows.h>
#include <direct.h>
#include <objbase.h>
#include <shlobj.h>
#include <initguid.h>
#include <ctype.h>
#include <limits.h>


#include <accctrl.h>
#define SECURITY_WIN32
#include <security.h>


typedef DWORD (WINAPI *PtrGetNamedSecurityInfoW)(LPWSTR, SE_OBJECT_TYPE, SECURITY_INFORMATION, PSID*, PSID*, PACL*, PACL*, PSECURITY_DESCRIPTOR*);
static PtrGetNamedSecurityInfoW ptrGetNamedSecurityInfoW = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrLookupAccountSidW)(LPCWSTR, PSID, LPWSTR, LPDWORD, LPWSTR, LPDWORD, PSID_NAME_USE);
static PtrLookupAccountSidW ptrLookupAccountSidW = 0;
typedef DECLSPEC_IMPORT BOOL (WINAPI *PtrAllocateAndInitializeSid)(PSID_IDENTIFIER_AUTHORITY, BYTE, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, DWORD, PSID*);
static PtrAllocateAndInitializeSid ptrAllocateAndInitializeSid = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithSidW)(PTRUSTEE_W, PSID);
static PtrBuildTrusteeWithSidW ptrBuildTrusteeWithSidW = 0;
typedef VOID (WINAPI *PtrBuildTrusteeWithNameW)(PTRUSTEE_W, unsigned short*);
static PtrBuildTrusteeWithNameW ptrBuildTrusteeWithNameW = 0;
typedef DWORD (WINAPI *PtrGetEffectiveRightsFromAclW)(PACL, PTRUSTEE_W, OUT PACCESS_MASK);
static PtrGetEffectiveRightsFromAclW ptrGetEffectiveRightsFromAclW = 0;
typedef DECLSPEC_IMPORT PVOID (WINAPI *PtrFreeSid)(PSID);
static PtrFreeSid ptrFreeSid = 0;

static TRUSTEE_W currentUserTrusteeW;



static void resolveLibs()
{
#if !defined(QT_NO_COMPONENT)
    static bool triedResolve = FALSE;
    if ( !triedResolve ) {
	// need to resolve the security info functions

#ifdef QT_THREAD_SUPPORT
	// protect initialization
	QMutexLocker locker( qt_global_mutexpool ?
	    qt_global_mutexpool->get( &triedResolve ) : 0 );
	// check triedResolve again, since another thread may have already
	// done the initialization
	if ( triedResolve ) {
	    // another thread did initialize the security function pointers,
	    // so we shouldn't do it again.
	    return;
	}
#endif

	triedResolve = TRUE;
	if ( qWinVersion() & Qt::WV_NT_based ) {
	    QLibrary lib("advapi32");
	    lib.setAutoUnload( FALSE );

	    ptrGetNamedSecurityInfoW = (PtrGetNamedSecurityInfoW) lib.resolve( "GetNamedSecurityInfoW" );
	    ptrLookupAccountSidW = (PtrLookupAccountSidW) lib.resolve( "LookupAccountSidW" );
	    ptrAllocateAndInitializeSid = (PtrAllocateAndInitializeSid) lib.resolve( "AllocateAndInitializeSid" );
	    ptrBuildTrusteeWithSidW = (PtrBuildTrusteeWithSidW) lib.resolve( "BuildTrusteeWithSidW" );
	    ptrBuildTrusteeWithNameW = (PtrBuildTrusteeWithNameW) lib.resolve( "BuildTrusteeWithNameW" );
	    ptrGetEffectiveRightsFromAclW = (PtrGetEffectiveRightsFromAclW) lib.resolve( "GetEffectiveRightsFromAclW" );
	    ptrFreeSid = (PtrFreeSid) lib.resolve( "FreeSid" );
	    if ( ptrBuildTrusteeWithNameW ) {
		QLibrary versionLib("version");
		typedef DWORD (WINAPI *PtrGetFileVersionInfoSizeW)(LPWSTR lptstrFilename,LPDWORD lpdwHandle);
		PtrGetFileVersionInfoSizeW ptrGetFileVersionInfoSizeW = (PtrGetFileVersionInfoSizeW)versionLib.resolve("GetFileVersionInfoSizeW");
		typedef BOOL (WINAPI *PtrGetFileVersionInfoW)(LPWSTR lptstrFilename,DWORD dwHandle,DWORD dwLen,LPVOID lpData);
		PtrGetFileVersionInfoW ptrGetFileVersionInfoW = (PtrGetFileVersionInfoW)versionLib.resolve("GetFileVersionInfoW");
		typedef BOOL (WINAPI *PtrVerQueryValueW)(const LPVOID pBlock,LPWSTR lpSubBlock,LPVOID *lplpBuffer,PUINT puLen);
		PtrVerQueryValueW ptrVerQueryValueW = (PtrVerQueryValueW)versionLib.resolve("VerQueryValueW");
		if ( ptrGetFileVersionInfoSizeW && ptrGetFileVersionInfoW && ptrVerQueryValueW ) {
		    DWORD fakeHandle;
		    DWORD versionSize = ptrGetFileVersionInfoSizeW( L"secur32.dll", &fakeHandle );
		    if ( versionSize ) {
			LPVOID versionData;
			versionData = malloc(versionSize);
			if ( ptrGetFileVersionInfoW( L"secur32.dll", 0, versionSize, versionData ) ) {
			    UINT puLen;
			    VS_FIXEDFILEINFO *pLocalInfo;
			    if ( ptrVerQueryValueW( versionData, L"\\", (void**)&pLocalInfo, &puLen ) ) {
				WORD wVer1, wVer2, wVer3, wVer4;
				wVer1 = HIWORD(pLocalInfo->dwFileVersionMS);
				wVer2 = LOWORD(pLocalInfo->dwFileVersionMS);
				wVer3 = HIWORD(pLocalInfo->dwFileVersionLS);
				wVer4 = LOWORD(pLocalInfo->dwFileVersionLS);
				// It will not work with secur32.dll version 5.0.2195.2862
				if ( !(wVer1 == 5 && wVer2 == 0 && wVer3 == 2195 && (wVer4 == 2862 || wVer4 == 4587) ) ) {
				    QLibrary userLib("secur32");
				    typedef BOOL (WINAPI *PtrGetUserNameExW)(EXTENDED_NAME_FORMAT nameFormat, ushort* lpBuffer, LPDWORD nSize);
				    PtrGetUserNameExW ptrGetUserNameExW = (PtrGetUserNameExW)userLib.resolve( "GetUserNameExW" );
				    if ( ptrGetUserNameExW ) {
					static TCHAR buffer[258];
					DWORD bufferSize = 257;
					ptrGetUserNameExW( NameSamCompatible, (ushort*)buffer, &bufferSize );
					ptrBuildTrusteeWithNameW( &currentUserTrusteeW, (ushort*)buffer );
				    }
				}
			    }
			}
			free(versionData);
		    }
		}
	    }
	}
    }
#endif // QT_NO_COMPONENT
}


static QString currentDirOfDrive( char ch )
{
    QString result;

    QT_WA( {
	TCHAR currentName[PATH_MAX];
	if ( _wgetdcwd( toupper( (uchar) ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
	    result = QString::fromUcs2( (ushort*)currentName );
	}
    } , {
	char currentName[PATH_MAX];
	if ( _getdcwd( toupper( (uchar) ch ) - 'A' + 1, currentName, PATH_MAX ) >= 0 ) {
	    result = QString::fromLocal8Bit(currentName);
	}
    } );
    return result;
}


void QFileInfo::slashify( QString &s )
{
    for (int i=0; i<(int)s.length(); i++) {
	if ( s[i] == '\\' )
	    s[i] = '/';
    }
    if ( s[ (int)s.length() - 1 ] == '/' && s.length() > 3 )
	s.remove( (int)s.length() - 1, 1 );
}

void QFileInfo::makeAbs( QString &s )
{
    if ( s[ 1 ] != ':' && s[ 1 ] != '/' ) {
	s.prepend( ":" );
	s.prepend( _getdrive() + 'A' - 1 );
    }
    if ( s[ 1 ] == ':' && s.length() > 3 && s[ 2 ] != '/' ) {
	QString d = currentDirOfDrive( (char)s[ 0 ].latin1() );
	slashify( d );
	s = d + "/" + s.mid( 2, 0xFFFFFF );
    }
}

extern QCString qt_win95Name(const QString s);

bool QFileInfo::isFile() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_REG : FALSE;
}

bool QFileInfo::isDir() const
{
    if ( !fic || !cache )
	doStat();
    return fic ? (fic->st.st_mode & QT_STAT_MASK) == QT_STAT_DIR : FALSE;
}

bool QFileInfo::isSymLink() const
{
    if ( fn.right( 4 ) == ".lnk" )
        return TRUE;
    else
        return FALSE;
}

QString QFileInfo::readLink() const
{
#if !defined(QT_NO_COMPONENT)
    QString fileLinked;

    QT_WA( {
	IShellLink *psl;                            // pointer to IShellLink i/f
	HRESULT hres;
	WIN32_FIND_DATA wfd;
	TCHAR szGotPath[MAX_PATH];
	// Get pointer to the IShellLink interface.

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
				    IID_IShellLink, (LPVOID *)&psl);

	if (SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
	    IPersistFile *ppf;
	    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
	    if (SUCCEEDED(hres))  {
		hres = ppf->Load( (LPOLESTR)fn.ucs2(), STGM_READ);
		if (SUCCEEDED(hres)) {        // Resolve the link.

		    hres = psl->Resolve(0, SLR_ANY_MATCH);

		    if (SUCCEEDED(hres)) {
			memcpy( szGotPath, (TCHAR*)fn.ucs2(), (fn.length()+1)*sizeof(QChar) );
			hres = psl->GetPath( szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH );
			fileLinked = QString::fromUcs2( (ushort*)szGotPath );
		    }
		}
		ppf->Release();
	    }
	    psl->Release();
	}
    } , {
	IShellLinkA *psl;                            // pointer to IShellLink i/f
	HRESULT hres;
	WIN32_FIND_DATAA wfd;
	QString fileLinked;
	char szGotPath[MAX_PATH];
	// Get pointer to the IShellLink interface.

	hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
				    IID_IShellLinkA, (LPVOID *)&psl);

	if (SUCCEEDED(hres)) {    // Get pointer to the IPersistFile interface.
	    IPersistFile *ppf;
	    hres = psl->QueryInterface(IID_IPersistFile, (LPVOID *)&ppf);
	    if (SUCCEEDED(hres))  {
		hres = ppf->Load( (LPOLESTR)fn.ucs2(), STGM_READ);
		if (SUCCEEDED(hres)) {        // Resolve the link.

		    hres = psl->Resolve(0, SLR_ANY_MATCH);

		    if (SUCCEEDED(hres)) {
			QCString lfn = fn.local8Bit();
			memcpy( szGotPath, lfn.data(), (lfn.length()+1)*sizeof(char) );
			hres = psl->GetPath((char*)szGotPath, MAX_PATH, &wfd, SLGP_SHORTPATH);
			fileLinked = QString::fromLocal8Bit(szGotPath);

		    }
		}
		ppf->Release();
	    }
	    psl->Release();
	}
    } );

    return fileLinked;
#else
    return QString();
#endif // QT_NO_COMPONENT
}

Q_EXPORT int qt_ntfs_permission_lookup = 1;

QString QFileInfo::owner() const
{
    if ( ( qt_ntfs_permission_lookup > 0 ) && ( qWinVersion() == Qt::WV_2000 || qWinVersion() == Qt::WV_XP ) ) {
	PSID pOwner = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();
	if ( ptrGetNamedSecurityInfoW && ptrLookupAccountSidW ) {
	    if ( ptrGetNamedSecurityInfoW( (wchar_t*)fn.ucs2(), SE_FILE_OBJECT, OWNER_SECURITY_INFORMATION, &pOwner, NULL, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lowner = 0, ldomain = 0;
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW( NULL, pOwner, NULL, &lowner, NULL, &ldomain, &use );
		TCHAR *owner = new TCHAR[lowner];
		TCHAR *domain = new TCHAR[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidW( NULL, pOwner, (LPWSTR)owner, &lowner, (LPWSTR)domain, &ldomain, &use ) ) {
		    name = qt_winQString(owner);
		}
		LocalFree( pSD );
		delete [] owner;
		delete [] domain;
	    }
	}
	return name;
    } else
	return QString::null;
}

static const uint nobodyID = (uint) -2;

uint QFileInfo::ownerId() const
{
    return nobodyID;
}

QString QFileInfo::group() const
{
    if ( ( qt_ntfs_permission_lookup > 0 ) && ( qWinVersion() == Qt::WV_2000 || qWinVersion() == Qt::WV_XP ) ) {
	PSID pGroup = 0;
	PSECURITY_DESCRIPTOR pSD;
	QString name;
	resolveLibs();

	if ( ptrGetNamedSecurityInfoW && ptrLookupAccountSidW ) {
	    if ( ptrGetNamedSecurityInfoW( (wchar_t*)fn.ucs2(), SE_FILE_OBJECT, GROUP_SECURITY_INFORMATION, NULL, &pGroup, NULL, NULL, &pSD ) == ERROR_SUCCESS ) {
		DWORD lgroup = 0, ldomain = 0;
		SID_NAME_USE use;
		// First call, to determine size of the strings (with '\0').
		ptrLookupAccountSidW( NULL, pGroup, NULL, &lgroup, NULL, &ldomain, &use );
		TCHAR *group = new TCHAR[lgroup];
		TCHAR *domain = new TCHAR[ldomain];
		// Second call, size is without '\0'
		if ( ptrLookupAccountSidW( NULL, pGroup, (LPWSTR)group, &lgroup, (LPWSTR)domain, &ldomain, &use ) ) {
		    name = qt_winQString(group);
		}
		LocalFree( pSD );
		delete [] group;
		delete [] domain;
	    }
	}
	return name;
    } else
	return QString::null;
}

uint QFileInfo::groupId() const
{
    return nobodyID;
}

bool QFileInfo::permission( int p ) const
{
    if ( ( qt_ntfs_permission_lookup > 0 ) && ( qWinVersion() == Qt::WV_2000 || qWinVersion() == Qt::WV_XP ) ) {
	PSID pOwner = 0;
	PSID pGroup = 0;
	PACL pDacl;
	PSECURITY_DESCRIPTOR pSD;
	ACCESS_MASK access_mask;

	enum { ReadMask = 0x00000001, WriteMask = 0x00000002, ExecMask = 0x00000020 };
	resolveLibs();

	if ( ptrGetNamedSecurityInfoW && ptrAllocateAndInitializeSid && ptrBuildTrusteeWithSidW && ptrGetEffectiveRightsFromAclW && ptrFreeSid ) {
	    DWORD res = ptrGetNamedSecurityInfoW( (wchar_t*)fn.ucs2(), SE_FILE_OBJECT,
		OWNER_SECURITY_INFORMATION | GROUP_SECURITY_INFORMATION | DACL_SECURITY_INFORMATION,
		&pOwner, &pGroup, &pDacl, 0, &pSD );

	    if ( res == ERROR_SUCCESS ) {
		TRUSTEE_W trustee;
		if ( p & ( ReadUser | WriteUser | ExeUser) ) {
		    if ( ptrGetEffectiveRightsFromAclW( pDacl, &currentUserTrusteeW, &access_mask ) != ERROR_SUCCESS )
			access_mask = (ACCESS_MASK)-1;
		    if ( ( p & ReadUser ) && !( access_mask & ReadMask )   ||
			( p & WriteUser ) && !( access_mask & WriteMask ) ||
			( p & ExeUser ) && !( access_mask & ExecMask )      ) {
			LocalFree( pSD );
			return FALSE;
		    }
		}
		if ( p & ( ReadOwner | WriteOwner | ExeOwner) ) {
		    ptrBuildTrusteeWithSidW( &trustee, pOwner );
		    if ( ptrGetEffectiveRightsFromAclW( pDacl, &trustee, &access_mask ) != ERROR_SUCCESS )
			access_mask = (ACCESS_MASK)-1;
		    if ( ( p & ReadOwner ) && !( access_mask & ReadMask )   ||
			( p & WriteOwner ) && !( access_mask & WriteMask ) ||
			( p & ExeOwner ) && !( access_mask & ExecMask )      ) {
			LocalFree( pSD );
			return FALSE;
		    }
		}
		if ( p & ( ReadGroup | WriteGroup | ExeGroup) ) {
		    ptrBuildTrusteeWithSidW( &trustee, pGroup );
		    if ( ptrGetEffectiveRightsFromAclW( pDacl, &trustee, &access_mask ) != ERROR_SUCCESS )
			access_mask = (ACCESS_MASK)-1;
		    if ( ( p & ReadGroup ) && !( access_mask & ReadMask )   ||
			( p & WriteGroup ) && !( access_mask & WriteMask ) ||
			( p & ExeGroup ) && !( access_mask & ExecMask )      ) {
			LocalFree( pSD );
			return FALSE;
		    }
		}
		if ( p & ( ReadOther | WriteOther | ExeOther) ) {
		    // Create SID for Everyone (World)
		    SID_IDENTIFIER_AUTHORITY worldAuth = { SECURITY_WORLD_SID_AUTHORITY };
		    PSID pWorld = 0;
		    if ( ptrAllocateAndInitializeSid( &worldAuth, 1, SECURITY_WORLD_RID, 0,0,0,0,0,0,0, &pWorld ) ) {
			ptrBuildTrusteeWithSidW( &trustee, pWorld );
			if ( ptrGetEffectiveRightsFromAclW( pDacl, &trustee, &access_mask ) != ERROR_SUCCESS )
			    access_mask = (ACCESS_MASK)-1; // ###
			if ( ( p & ReadOther ) && !( access_mask & ReadMask )   ||
			    ( p & WriteOther ) && !( access_mask & WriteMask ) ||
			    ( p & ExeOther ) && !( access_mask & ExecMask )      ) {
			    LocalFree( pSD );
			    return FALSE;
			}
		    }
		    ptrFreeSid( pWorld );
		}
		LocalFree( pSD );
	    }
	}
    }
    // just check if it's ReadOnly

    QT_WA( {
	if ( p & ( WriteOwner | WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributes( (TCHAR*)fn.ucs2() );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
    } , {
	if ( p & ( WriteOwner | WriteUser | WriteGroup | WriteOther ) ) {
	    DWORD attr = GetFileAttributesA( fn.local8Bit() );
	    if ( attr & FILE_ATTRIBUTE_READONLY )
		return FALSE;
	}
    } );

    return TRUE;
}

void QFileInfo::doStat() const
{
    if ( fn.isEmpty() )
	return;

    UINT oldmode = SetErrorMode(SEM_FAILCRITICALERRORS);
    QFileInfo *that = ((QFileInfo*)this);	// mutable function
    if ( !that->fic )
	that->fic = new QFileInfoCache;
    QT_STATBUF *b = &that->fic->st;

    int r;
    QT_WA( {
	r = QT_TSTAT((TCHAR*)fn.ucs2(), (QT_STATBUF4TSTAT*)b);
    } , {
	r = QT_STAT(qt_win95Name(fn), b);
    } );
    if ( r!=0 ) {
	bool is_dir=FALSE;
	if ( fn[0] == '/' && fn[1] == '/'
	  || fn[0] == '\\' && fn[1] == '\\' )
	{
	    // UNC - stat doesn't work for all cases (Windows bug)
	    int s = fn.find(fn[0],2);
	    if ( s > 0 ) {
		// "\\server\..."
		s = fn.find(fn[0],s+1);
		if ( s > 0 ) {
		    // "\\server\share\..."
		    if ( fn[s+1] ) {
			// "\\server\share\notfound"
		    } else {
			// "\\server\share\"
			is_dir=TRUE;
		    }
		} else {
		    // "\\server\share"
		    is_dir=TRUE;
		}
	    } else {
		// "\\server"
		is_dir=TRUE;
	    }
	}
	if ( is_dir ) {
	    // looks like a UNC dir, is a dir.
	    memset(b,0,sizeof(*b));
	    b->st_mode = QT_STAT_DIR;
	    b->st_nlink = 1;
	    r = 0;
	}
    }

    if ( r != 0 ) {
	delete that->fic;
	that->fic = 0;
    }

    SetErrorMode(oldmode);
}

QString QFileInfo::dirPath( bool absPath ) const
{
    QString s;
    if ( absPath )
	s = absFilePath();
    else
	s = fn;
    int pos = s.findRev( '/' );
    if ( pos == -1 ) {
	if ( s[ 2 ] == '/' )
	    return s.left( 3 );
	if ( s[ 1 ] == ':' ) {
	    if ( absPath )
		return s.left( 2 ) + "/";
	    return s.left( 2 );
	}
	return QString::fromLatin1(".");
    } else {
	if ( pos == 0 )
	    return QString::fromLatin1( "/" );
	if ( pos == 2 && s[ 1 ] == ':'  && s[ 2 ] == '/')
	    pos++;
	return s.left( pos );
    }
}

QString QFileInfo::fileName() const
{
    int p = fn.findRev( '/' );
    if ( p == -1 ) {
	int p = fn.findRev( ':' );
	if ( p != -1 )
	    return fn.mid( p + 1 );
	return fn;
    } else {
	return fn.mid( p + 1 );
    }
}

/*!
    Returns TRUE if the file is hidden; otherwise returns FALSE.

    On Unix-like operating systems, including Mac OS X, a file is
    hidden if its name begins with ".". On Windows a file is hidden if
    its hidden attribute is set.
*/
bool QFileInfo::isHidden() const
{
    QT_WA( {
	return GetFileAttributesW( (TCHAR*)fn.ucs2() ) & FILE_ATTRIBUTE_HIDDEN;
    } , {
    return GetFileAttributesA( fn.local8Bit() ) & FILE_ATTRIBUTE_HIDDEN;
    } );
}
