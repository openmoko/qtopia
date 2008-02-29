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

#include <qtopia/global.h>
#include <qtopia/config.h>

/*!
  Writes a (\a key, \a byteArray) entry to the current group.  The byteArray is stored as
  a base64 encoded string.

  First availability: Qtopia 1.6
*/
void Config::writeEntry( const QString &key, const QByteArray& byteArray)
{
  writeEntry(key, QString(Global::encodeBase64(byteArray)));
}

/*!
  Returns the QByteArray stored using \a key.  Returns an empty array if
  no matching key is found.

  First availability: Qtopia 1.6
*/
QByteArray Config::readByteArrayEntry(const QString& key) const
{
  QByteArray empty;
  return readByteArrayEntry(key, empty);
}

/*!
  Returns the QByteArray stored using \a key.  Returns \a dflt if
  no matching key is found.

  First availability: Qtopia 1.6
*/
QByteArray Config::readByteArrayEntry(const QString& key, const QByteArray& dflt) const
{
  QString s = readEntry(key);
  if (s.isEmpty())
    return dflt;
  return Global::decodeBase64(s.utf8());
}


QString Config::readEntry( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntry(key,deflt); }
QString Config::readEntryCrypt( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntryCrypt(key,deflt); }
QString Config::readEntryDirect( const QString &key, const QString &deflt ) const
{ return ((Config*)this)->readEntryDirect(key,deflt); }
int Config::readNumEntry( const QString &key, int deflt ) const
{ return ((Config*)this)->readNumEntry(key,deflt); }
bool Config::readBoolEntry( const QString &key, bool deflt ) const
{ return ((Config*)this)->readBoolEntry(key,deflt); }
QStringList Config::readListEntry( const QString &key, const QChar &sep ) const
{ return ((Config*)this)->readListEntry(key,sep); }
