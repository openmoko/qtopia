/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
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
*/
void Config::writeEntry( const QString &key, const QByteArray& byteArray)
{
  writeEntry(key, QString(Global::encodeBase64(byteArray)));
}

/*!
  Returns the QByteArray stored using \a key.  Returns an empty array if
  no matching key is found.
*/
QByteArray Config::readByteArrayEntry(const QString& key) const
{
  QByteArray empty;
  return readByteArrayEntry(key, empty);
}

/*!
  Returns the QByteArray stored using \a key.  Returns \a dflt if
  no matching key is found.
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
