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

#ifndef CONFIG_H
#define CONFIG_H

#include <qtopia/qpeglobal.h>

// ##### could use QSettings with Qt 3.0
#include <qmap.h>
#include <qstringlist.h>
#include <qcstring.h>

#include <qstring.h>
#include <qtopia/qtopiawinexport.h>

typedef QMap< QString, QString > ConfigGroup;
typedef QMap< QString, ConfigGroup> ConfigGroupMap;

class ConfigPrivate;
class QTextStream;
class QTOPIA_EXPORT Config
{
public:

    enum Domain { File, User };
    Config( const QString &name, Domain domain=User );
#ifdef QTOPIA_DESKTOP
    Config( QTextStream &s, Domain domain=User );
#endif
    ~Config();

    static long timeStamp( const QString &name, Domain domain=User );

    bool operator == ( const Config & other ) const { return (filename == other.filename); }
    bool operator != ( const Config & other ) const { return (filename != other.filename); }

    bool isValid() const;
    bool hasKey( const QString &key ) const;

    void setGroup( const QString &gname );
    void writeEntry( const QString &key, const char* value );
    void writeEntry( const QString &key, const QString &value );
    void writeEntryCrypt( const QString &key, const QString &value );
    void writeEntry( const QString &key, int num );
#ifdef Q_HAS_BOOL_TYPE
    void writeEntry( const QString &key, bool b );
#endif
    void writeEntry( const QString &key, const QStringList &lst, const QChar &sep );
    void writeEntry( const QString &key, const QStringList &lst ); // Qtopia 2
    void writeEntry( const QString &key, const QByteArray& byteArray); // libqtopia
    void removeEntry( const QString &key );

    // libqtopia...
    QString readEntry( const QString &key, const QString &deflt = QString::null ) const;
    QString readEntryCrypt( const QString &key, const QString &deflt = QString::null ) const;
    QString readEntryDirect( const QString &key, const QString &deflt = QString::null ) const;
    int readNumEntry( const QString &key, int deflt = -1 ) const;
    bool readBoolEntry( const QString &key, bool deflt = FALSE ) const;
    QStringList readListEntry( const QString &key, const QChar &sep ) const;
    QStringList readListEntry( const QString &key ) const; // Qtopia 2
    QByteArray readByteArrayEntry(const QString& key) const;
    QByteArray readByteArrayEntry(const QString& key, const QByteArray& deflt) const;
    // ... end.

    // For compatibility, non-const versions.
    QString readEntry( const QString &key, const QString &deflt = QString::null );
    QString readEntryCrypt( const QString &key, const QString &deflt = QString::null );
    QString readEntryDirect( const QString &key, const QString &deflt = QString::null );
    int readNumEntry( const QString &key, int deflt = -1 );
    bool readBoolEntry( const QString &key, bool deflt = FALSE );
    QStringList readListEntry( const QString &key, const QChar &sep );

    void clearGroup();
    void removeGroup(); // Qtopia 2
    void removeGroup(const QString&); // Qtopia 2
    QStringList allGroups() const; // Qtopia 2

    void write( const QString &fn = QString::null );

protected:
    void read();
    bool parse( const QString &line );

    QMap< QString, ConfigGroup > groups;
    ConfigGroupMap::Iterator git;
    QString filename;
    QString lang;
    QString glang;
    bool changed;
    ConfigPrivate *d;
    static QString configFilename(const QString& name, Domain);

private:
    /* This cannot be made public or protected because of binary compat issues */
    void read( QTextStream &s );
};

#endif
