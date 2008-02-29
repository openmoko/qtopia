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

#include "qsettings.h"
#include <private/qsettings_p.h>
#include "qt_windows.h"
#include "qregexp.h"

#ifndef QT_NO_SETTINGS

static bool settingsTryUser = TRUE;
static bool settingsTryLocal = TRUE;
static QString *settingsBasePath = 0;

void Q_EXPORT qt_setSettingsTryUser( bool tryUser )
{
    settingsTryUser = tryUser;
}

void Q_EXPORT qt_setSettingsTryLocal( bool tryLocal )
{
    settingsTryLocal = tryLocal;
}

void Q_EXPORT qt_setSettingsBasePath( const QString &base )
{
    if ( settingsBasePath ) {
	qWarning( "qt_setSettingsBasePath has to be called without any settings object being instantiated!" );
	return;
    }
    settingsBasePath = new QString( base );
}

class QSettingsSysPrivate
{
public:
    QSettingsSysPrivate( QSettingsPrivate *priv );
    ~QSettingsSysPrivate();

    HKEY user;
    HKEY local;

    QString folder( const QString& );
    QString entry( const QString& );

    QString validateKey( const QString &key );

    bool writeKey( const QString &key, const QByteArray &value, ulong type );
    QByteArray readKey( const QString &key, ulong &type );
    HKEY readKeyHelper( HKEY root, const QString &folder, const QString &entry, ulong &size );

    HKEY openKey( const QString &key, bool write, bool remove = FALSE );

    QStringList paths;

private:
    QSettingsPrivate *d;

    static uint refCount;
};

uint QSettingsSysPrivate::refCount = 0;

QSettingsSysPrivate::QSettingsSysPrivate( QSettingsPrivate *priv )
    : d( priv )
{
    paths.append( "" );
    if ( !settingsBasePath ) {
	settingsBasePath = new QString("Software");
    }
    refCount++;
    local = 0;
    user  = 0 ;

    LONG res;
    if ( settingsTryLocal ) {
	QT_WA( {
	    res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
	} , {
	    res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_ALL_ACCESS, &local );
	} );

	if ( res != ERROR_SUCCESS ) {
	    QT_WA( {
		res = RegOpenKeyExW( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
	    } , {
		res = RegOpenKeyExA( HKEY_LOCAL_MACHINE, NULL, 0, KEY_READ, &local );
	    } );
	    if ( res != ERROR_SUCCESS ) {
		local = NULL;
	    }
	}
    }

    if ( settingsTryUser ) {
	QT_WA( {
	    res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
	} , {
	    res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_ALL_ACCESS, &user );
	} );

	if ( res != ERROR_SUCCESS ) {
	    QT_WA( {
		res = RegOpenKeyExW( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
	    } , {
		res = RegOpenKeyExA( HKEY_CURRENT_USER, NULL, 0, KEY_READ, &user );
	    } );
	    if ( res != ERROR_SUCCESS ) {
		user = NULL;
	    }
	}
    }

#if defined(QT_CHECK_STATE)
    if ( !local && !user )
	qSystemWarning( "Error opening registry!", res );
#endif
}

QSettingsSysPrivate::~QSettingsSysPrivate()
{
    LONG res;
    if ( local ) {
	res = RegCloseKey( local );
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning( "Error closing local machine!", res );
#endif
    }
    if ( user ) {
	res = RegCloseKey( user );
#if defined(QT_CHECK_STATE)
	if ( res != ERROR_SUCCESS )
	    qSystemWarning( "Error closing current user!", res );
#endif
    }

    // Make sure that we only delete the base path if no one else is using it anymore
    if (refCount > 0) {
	refCount--;

	if (refCount == 0) {
	    delete settingsBasePath;
	    settingsBasePath = 0;
	}
    }
}

QString QSettingsSysPrivate::validateKey( const QString &key )
{
    if ( key.isEmpty() )
	return key;

    QString newKey = key;
    newKey = newKey.replace( QRegExp( "[/]+" ), "\\" );

    if ( newKey[0] != '\\' )
	newKey = "\\" + newKey;
    if ( newKey[(int)newKey.length() - 1] == '\\' )
	newKey = newKey.left( newKey.length() - 1 );

    return newKey;
}

QString QSettingsSysPrivate::folder( const QString &key )
{
    QString k = validateKey( key );
    Q_ASSERT(settingsBasePath);
    return *settingsBasePath + k.left( k.findRev( "\\" ) );
}

QString QSettingsSysPrivate::entry( const QString &key )
{
    QString k = validateKey( key );
    return k.right( k.length() - k.findRev( "\\" ) - 1 );
}

HKEY QSettingsSysPrivate::openKey( const QString &key, bool write, bool remove )
{
    QString f = folder( key );

    HKEY handle = 0;
    LONG res = ERROR_FILE_NOT_FOUND;

    // if we write and there is a user specific setting, overwrite that
    if ( (write||remove) && user ) {
	QT_WA( {
	    if ( remove )
		res = RegOpenKeyExW( user, (TCHAR*)f.ucs2(), 0, KEY_ALL_ACCESS, &handle );
	    else
		res = RegOpenKeyExW( user, (TCHAR*)f.ucs2(), 0, KEY_WRITE, &handle );
	} , {
	    if ( remove )
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	    else
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_WRITE, &handle );
	} );
    }

    wchar_t empty_t[] = L""; // workaround for Borland
    if ( res != ERROR_SUCCESS && local && d->globalScope ) {
	QT_WA( {
	    if ( write && !remove )
		res = RegCreateKeyExW( local, (TCHAR*)f.ucs2(), 0, empty_t, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else if ( !write && !remove )
		res = RegOpenKeyExW( local, (TCHAR*)f.ucs2(), 0, KEY_READ, &handle );
	    else
		res = RegOpenKeyExW( local, (TCHAR*)f.ucs2(), 0, KEY_ALL_ACCESS, &handle );
	} , {
	    if ( write && !remove )
		res = RegCreateKeyExA( local, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else if ( !write && !remove )
		res = RegOpenKeyExA( local, f.local8Bit(), 0, KEY_READ, &handle );
	    else
		res = RegOpenKeyExA( local, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	} );
    }
    if ( !handle && user ) {
	QT_WA( {
	    if ( write && !remove )
		res = RegCreateKeyExW( user, (TCHAR*)f.ucs2(), 0, empty_t, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else if ( !write && !remove )
		res = RegOpenKeyExW( user, (TCHAR*)f.ucs2(), 0, KEY_READ, &handle );
	    else
		res = RegOpenKeyExW( user, (TCHAR*)f.ucs2(), 0, KEY_ALL_ACCESS, &handle );
	} , {
	    if ( write && !remove )
		res = RegCreateKeyExA( user, f.local8Bit(), 0, "", REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &handle, NULL );
	    else if ( !write && !remove )
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_READ, &handle );
	    else
		res = RegOpenKeyExA( user, f.local8Bit(), 0, KEY_ALL_ACCESS, &handle );
	} );
    }
    return handle;
}

bool QSettingsSysPrivate::writeKey( const QString &key, const QByteArray &value, ulong type )
{
    QString e;
    LONG res = ERROR_ACCESS_DENIED;

    HKEY handle = 0;
    for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	QString k = *it + "/" + key;
	e = entry( k );
	handle = openKey( k, TRUE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return FALSE;
    
    if (e == "Default" || e == "." )
	e = "";

    if ( value.size() ) {
	QT_WA( {
	    res = RegSetValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)e.ucs2(), 0, type, (const uchar*)value.data(), value.size() );
	} , {
	    res = RegSetValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), 0, type, (const uchar*)value.data(), value.size() );
	} );
	
	if ( res != ERROR_SUCCESS ) {
#if defined(QT_CHECK_STATE)
	    qSystemWarning( "Couldn't write value " + key, res );
#endif
	    return FALSE;
	}
    }
    
    RegCloseKey( handle );
    return TRUE;
}

HKEY QSettingsSysPrivate::readKeyHelper( HKEY root, const QString &folder, const QString &entry, ulong &size )
{
    HKEY handle;
    LONG res = ERROR_ACCESS_DENIED;
    QT_WA( {
	res = RegOpenKeyExW( root, (TCHAR*)folder.ucs2(), 0, KEY_READ, &handle );
    } , {
	res = RegOpenKeyExA( root, folder.local8Bit(), 0, KEY_READ, &handle );
    } );
    
    if ( res == ERROR_SUCCESS ) {
	QT_WA( {
	    res = RegQueryValueExW( handle, entry.isEmpty() ? 0 : (TCHAR*)entry.ucs2(), NULL, NULL, NULL, &size );
	} , {
	    res = RegQueryValueExA( handle, entry.isEmpty() ? (const char*)0 : (const char*)entry.local8Bit(), NULL, NULL, NULL, &size );
	} );
    }
    if ( res != ERROR_SUCCESS ) {
	RegCloseKey( handle );
	size = 0;
	handle = 0;
    }

    return handle;
}

QByteArray QSettingsSysPrivate::readKey( const QString &key, ulong &type )
{
    HKEY handle = 0;
    ulong size = 0;
    QString e;
    type = REG_NONE;

    if ( user ) {
	for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	    if ( handle ) {
		RegCloseKey( handle );
		handle = 0;
	    }
	    QString k = *it + "/" + key;
	    QString f = folder( k );
	    e = entry( k );
	    if ( e == "Default" || e == "." )
		e = "";

	    handle = readKeyHelper( user, f, e, size );

	    if ( !handle )
		size = 0;
	    else if ( size )
		break;
	}
    }

    if ( !size && local ) {
	for ( QStringList::Iterator it = paths.fromLast(); it != paths.end(); --it ) {
	    if ( handle ) {
		RegCloseKey( handle );
		handle = 0;
	    }

	    QString k = *it + "/" + key;
	    QString f = folder( k );
	    e = entry( k );
	    if ( e == "Default" || e == "." )
		e = "";

	    handle = readKeyHelper( local, f, e, size );

	    if ( !handle )
		size = 0;
	    else if ( size )
		break;
	}
    }

    if ( !size ) {	
	if ( handle )
	    RegCloseKey( handle );
	return QByteArray();
    }

    uchar* data = new uchar[ size ];
    QT_WA( {
	RegQueryValueExW( handle, e.isEmpty() ? 0 : (TCHAR*)e.ucs2(), NULL, &type, data, &size );
    } , {
	RegQueryValueExA( handle, e.isEmpty() ? (const char*)0 : (const char*)e.local8Bit(), NULL, &type, data, &size );
    } );

    QByteArray result;
    result.setRawData( (const char*)data, size );
    RegCloseKey( handle );

    return result;
}

void QSettingsPrivate::sysInit()
{
    sysd = new QSettingsSysPrivate( this );
}

void QSettingsPrivate::sysClear()
{
    delete sysd;
}

bool QSettingsPrivate::sysSync()
{
    if ( sysd->local )
	RegFlushKey( sysd->local );
    if ( sysd->user )
	RegFlushKey( sysd->user );
    return TRUE;
}

bool QSettingsPrivate::sysReadBoolEntry(const QString &key, bool def, bool *ok ) const
{
    return sysReadNumEntry( key, def, ok );
}

double QSettingsPrivate::sysReadDoubleEntry( const QString &key, double def, bool *ok ) const
{
    Q_ASSERT(sysd);

    if ( ok )
	*ok = FALSE;
    ulong type;
    QByteArray array = sysd->readKey( key, type );
    if ( type != REG_BINARY ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;
	return def;
    }
    if ( array.size() != sizeof(double) )
	return def;

    if ( ok )
	*ok = TRUE;

    double res = 0;
    char* data = (char*)&res;
    for ( int i = 0; i < sizeof(double); ++i )
	data[i] = array[ i ];

    char *adata = array.data();
    array.resetRawData( adata, array.size() );
    delete[] adata;
    return res;
}

int QSettingsPrivate::sysReadNumEntry(const QString &key, int def, bool *ok ) const
{
    Q_ASSERT(sysd);

    if ( ok )
	*ok = FALSE;
    ulong type;
    QByteArray array = sysd->readKey( key, type );
    if ( type != REG_DWORD ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;

	return def;
    }

    if ( array.size() != sizeof(int) )
	return def;

    if ( ok )
	*ok = TRUE;

    int res = 0;
    char* data = (char*)&res;
    for ( int i = 0; i < sizeof(int); ++i )
	data[i] = array[ i ];

    char *adata = array.data();
    array.resetRawData( adata, array.size() );
    delete[] adata;
    return res;
}

QString QSettingsPrivate::sysReadEntry(const QString &key, const QString &def, bool *ok ) const
{
    if ( ok ) // no, everything is not ok
	*ok = FALSE;

    ulong type;
    QByteArray array = sysd->readKey( key, type );
    if ( type != REG_SZ ) {
	char *data = array.data();
	array.resetRawData( data, array.size() );
	delete[] data;
	return def;
    }

    if ( ok )
	*ok = TRUE;
    QString result = QString::null;

    QT_WA( {
	int s = array.size();
	for ( int i = 0; i < s; i+=2 ) {
	    QChar c( array[ i ], array[ i+1 ] );
	    if( !c.isNull() )
		result+=c;
	}
    } , {
	result = QString::fromLocal8Bit( array );
    } );
    
    if ( array.size() == 2 && result.isNull() )
	result = "";

    char *data = array.data();
    array.resetRawData( data, array.size() );
    delete[] data;

    return result;
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, bool value )
{
    return sysWriteEntry( key, (int)value );
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, double value )
{
    Q_ASSERT(sysd);

    QByteArray array( sizeof(double) );
    const char *data = (char*)&value;

    for ( int i = 0; i < sizeof(double); ++i )
	array[i] = data[i];

    return sysd->writeKey( key, array, REG_BINARY );
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, int value )
{
    Q_ASSERT(sysd);

    QByteArray array( sizeof(int) );
    const char* data = (char*)&value;
    for ( int i = 0; i < sizeof(int ); ++i )
	array[i] = data[i];

    return sysd->writeKey( key, array, REG_DWORD );
}

bool QSettingsPrivate::sysWriteEntry( const QString &key, const QString &value )
{
    Q_ASSERT(sysd);

    QByteArray array( 0 );
    QT_WA( {
	array.resize( value.length() * 2 + 2 );
	const QChar *data = value.unicode();
	int i;
	for ( i = 0; i < (int)value.length(); ++i ) {
	    array[ 2*i ] = data[ i ].cell();
	    array[ (2*i)+1 ] = data[ i ].row();
	}

	array[ (2*i) ] = 0;
	array[ (2*i)+1 ] = 0;
    } , {
	array.resize( value.length() );
	array = value.local8Bit();
    } );

    return sysd->writeKey( key, array, REG_SZ );
}

bool QSettingsPrivate::sysRemoveEntry( const QString &key )
{
    Q_ASSERT(sysd);

    QString e;
    LONG res;

    HKEY handle = 0;
    for ( QStringList::Iterator it = sysd->paths.fromLast(); it != sysd->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key : *it + "/" + key;
	handle = sysd->openKey( k, FALSE, TRUE );
	e = sysd->entry( k );
	if ( handle )
	    break;
    }
    if ( !handle )
	return TRUE;
    if ( e == "Default" || e == "." )
	e = "";
    QT_WA( {
	res = RegDeleteValueW( handle, (TCHAR*)e.ucs2() );
    } , {
	res = RegDeleteValueA( handle, e.local8Bit() );
    } );

    if ( res != ERROR_SUCCESS && res != ERROR_FILE_NOT_FOUND ) {
#if defined(QT_CHECK_STATE)
	qSystemWarning( "Error deleting value " + key, res );
#endif
	return FALSE;
    }
    char vname[2];
    DWORD vnamesz = 1;
    FILETIME lastWrite;
#ifdef Q_OS_TEMP
    LONG res2 = RegEnumValue( handle, 0, (LPWSTR)vname, &vnamesz, NULL, NULL, NULL, NULL );
    LONG res3 = RegEnumKeyEx( handle, 0, (LPWSTR)vname, &vnamesz, NULL, NULL, NULL, &lastWrite );
#else
    LONG res2 = RegEnumValueA( handle, 0, vname, &vnamesz, NULL, NULL, NULL, NULL );
    LONG res3 = RegEnumKeyExA( handle, 0, vname, &vnamesz, NULL, NULL, NULL, &lastWrite );
#endif
    if ( res2 == ERROR_NO_MORE_ITEMS && res3 == ERROR_NO_MORE_ITEMS )
#ifdef Q_OS_TEMP
	RegDeleteKeyW( handle, L"" );
#else
    RegDeleteKeyA( handle, "" );
#endif
    else
	RegCloseKey( handle );
    return TRUE;
}

QStringList QSettingsPrivate::sysEntryList( const QString &key ) const
{
    Q_ASSERT(sysd);

    QStringList result;

    HKEY handle = 0;
    for ( QStringList::Iterator it = sysd->paths.fromLast(); it != sysd->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key + "/fake" : *it + "/" + key + "/fake";
	handle = sysd->openKey( k, FALSE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return result;

    DWORD count;
    DWORD maxlen;
    
    QT_WA( {
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL );
    } , {
	RegQueryInfoKeyA( handle, NULL, NULL, NULL, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL );
    } );
    maxlen++;
    DWORD index = 0;

    TCHAR *vnameT = new TCHAR[ maxlen ];
    char *vnameA = new char[ maxlen ];
    QString qname;

    DWORD vnamesz = 0;
    LONG res = ERROR_SUCCESS;

    while ( res != ERROR_NO_MORE_ITEMS ) {
	vnamesz = maxlen;
	QT_WA( {
	    res = RegEnumValueW( handle, index, vnameT, &vnamesz, NULL, NULL, NULL, NULL );
	    if ( res == ERROR_SUCCESS )
		qname = QString::fromUcs2( (ushort*)vnameT );
	} , {
	    res = RegEnumValueA( handle, index, vnameA, &vnamesz, NULL, NULL, NULL, NULL );
	    if ( res == ERROR_SUCCESS )
		qname = vnameA;
	} );
	if ( res == ERROR_NO_MORE_ITEMS )
	    break;
	if ( qname.isEmpty() )
	    result.append( "Default" );
	else
	    result.append( qname );
	++index;
    }

    delete [] vnameA;
    delete [] vnameT;

    RegCloseKey( handle );
    return result;
}

QStringList QSettingsPrivate::sysSubkeyList( const QString &key ) const
{
    Q_ASSERT(sysd);

    QStringList result;

    HKEY handle = 0;
    for ( QStringList::Iterator it = sysd->paths.fromLast(); it != sysd->paths.end(); --it ) {
	QString k = (*it).isEmpty() ? key + "/fake" : *it + "/" + key + "/fake";
	handle = sysd->openKey( k, FALSE );
	if ( handle )
	    break;
    }
    if ( !handle )
	return result;

    DWORD count;
    DWORD maxlen;
    QT_WA( {
	RegQueryInfoKeyW( handle, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL, NULL, NULL, NULL );
	maxlen++;
    } , {
	RegQueryInfoKeyA( handle, NULL, NULL, NULL, &count, &maxlen, NULL, NULL, NULL, NULL, NULL, NULL );
    } );

    DWORD index = 0;
    FILETIME lastWrite;

    TCHAR *vnameT = new TCHAR[ maxlen ];
    char *vnameA = new char[ maxlen ];
    QString qname;

    DWORD vnamesz = 0;
    LONG res = ERROR_SUCCESS;

    while ( res != ERROR_NO_MORE_ITEMS ) {
	vnamesz = maxlen;
	QT_WA( {
	    res = RegEnumKeyExW( handle, index, vnameT, &vnamesz, NULL, NULL, NULL, &lastWrite );
	    if ( res == ERROR_SUCCESS )
		qname = QString::fromUcs2( (ushort*)vnameT );
	} , {
	    res = RegEnumKeyExA( handle, index, vnameA, &vnamesz, NULL, NULL, NULL, &lastWrite );
	    if ( res == ERROR_SUCCESS )
		qname = vnameA;
	} );

	if ( res == ERROR_NO_MORE_ITEMS )
	    break;
	result.append( qname );
	++index;
    }

    delete [] vnameA;
    delete [] vnameT;

    RegCloseKey( handle );
    return result;
}

void QSettingsPrivate::sysInsertSearchPath( QSettings::System s, const QString &path )
{
    Q_ASSERT(sysd);

    if ( s != QSettings::Windows || path.isEmpty() )
	return;
    QString p = path;
    if ( p[0] != '/' )
	p = "/" + p;
    sysd->paths.append( p );
}

void QSettingsPrivate::sysRemoveSearchPath( QSettings::System s, const QString &path )
{
    Q_ASSERT(sysd);

    if ( s != QSettings::Windows || path.isEmpty() )
	return;
    QString p = path;
    if ( p[0] != '/' )
	p = "/" + p;
    sysd->paths.remove( p );
}

#endif //QT_NO_SETTINGS
