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

#include <qtopia/config.h>

#include <qfile.h>
#include <qregexp.h>
#ifdef Q_WS_WIN
#include <qdatetime.h>
#include <qfileinfo.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

/*!
  Returns the time stamp for the config identified by \a name.  The
  time stamp represents the time the config was last committed to storage.
  Returns 0 if there is no time stamp available for the config.

  A \a domain can optionally be specified and defaults to User.
  See \l{Config()} for details.

  First availability: Qtopia 2.0
*/
long Config::timeStamp(const QString& name, Domain domain)
{
#ifdef Q_WS_WIN
    // Too slow (many conversions too and from time_t and QDataTime)
    QDateTime epoch;
    epoch.setTime_t(0);
    return epoch.secsTo(QFileInfo(Config::configFilename(name,domain)).lastModified());
#else
    QString fn = Config::configFilename(name,domain);
    struct stat b;
    if (lstat( QFile::encodeName(fn).data(), &b ) == 0)
	return b.st_mtime;
    else 
	return 0;
#endif
}

/*!
  Returns the list of all groups in the Config.

  First availability: Qtopia 2.0
*/
QStringList Config::allGroups() const
{
#if QT_VERSION > 0x030000
    return groups.keys();
#else
    QStringList res;
    QMap<QString,ConfigGroup>::ConstIterator i = groups.begin();
    while (i != groups.end()) {
        res.append(i.key());
        ++i;
    }
    return res;
#endif
}

/*!
  Removes the current group (and all its entries).

  The current group becomes unset.

  First availability: Qtopia 2.0
*/
void Config::removeGroup()
{
    if ( git == groups.end() ) {
	qWarning( "no group set" );
	return;
    }
    groups.remove(git.key());
    git = groups.end();
    changed = TRUE;
}

/*!
  Removes the current group (and all its entries).

  The current group becomes unset.

  First availability: Qtopia 2.0
*/
void Config::removeGroup(const QString& g)
{
    groups.remove(g);
    git = groups.end();
}



/*!
  Writes a (\a key, \a lst) entry to the current group.

  The list is
  separated by the two characters "^e", and "^" withing the strings
  is replaced by "^^", such that the strings may contain any character,
  including "^".

  Null strings are also allowed, and are recorded as "^0" in the string.

  First availability: Qtopia 2.0

  \sa readListEntry()
*/
void Config::writeEntry( const QString &key, const QStringList &lst )
{
    QString s;
    for (QStringList::ConstIterator it=lst.begin(); it!=lst.end(); ++it) {
        QString el = *it;
        if ( el.isNull() ) {
            el = "^0";
        } else {
            el.replace(QRegExp("\\^"), "^^");
        }
        s+=el;
        s+="^e"; // end of element
    }
    writeEntry(key, s);
}

/*!
  Returns the string list entry stored using \a key and with
  the escaped seperator convention described in writeListEntry().

  First availability: Qtopia 2.0
*/
QStringList Config::readListEntry( const QString &key ) const
{
    QString value = readEntry( key, QString::null );
    QStringList l;
    QString s;
    bool esc=FALSE;
    for (int i=0; i<(int)value.length(); i++) {
	if ( esc ) {
	    if ( value[i] == 'e' ) { // end-of-string
		l.append(s);
		s="";
	    } else if ( value[i] == '0' ) { // null string
		s=QString::null;
	    } else {
		s.append(value[i]);
	    }
	    esc = FALSE;
	} else if ( value[i] == '^' ) {
	    esc = TRUE;
	} else {
	    s.append(value[i]);
	    if ( i == (int)value.length()-1 )
		l.append(s);
	}
    }
    return l;
}
