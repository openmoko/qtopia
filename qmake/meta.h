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

#ifndef __META_H__
#define __META_H__

#include <qmap.h>
#include <qstringlist.h>
#include <qstring.h>

class QMakeMetaInfo 
{
    bool readLibtoolFile(const QString &f);
    bool readPkgCfgFile(const QString &f);
    QMap<QString, QStringList> vars;
    QString meta_type;
    void clear();
public:
    QMakeMetaInfo();

    bool readLib(const QString &lib);
    static QString findLib(const QString &lib);
    static bool libExists(const QString &lib);
    QString type() const;

    bool isEmpty(const QString &v);
    QStringList &values(const QString &v);
    QString first(const QString &v);
    QMap<QString, QStringList> &variables();
};

inline bool QMakeMetaInfo::isEmpty(const QString &v)
{ return !vars.contains(v) || vars[v].isEmpty(); }

inline QString QMakeMetaInfo::type() const
{ return meta_type; }

inline QStringList &QMakeMetaInfo::values(const QString &v)
{ return vars[v]; }

inline QString QMakeMetaInfo::first(const QString &v)
{
#if defined(Q_CC_SUN) && (__SUNPRO_CC == 0x500) || defined(Q_CC_HP)
    // workaround for Sun WorkShop 5.0 bug fixed in Forte 6
    if (isEmpty(v))
	return QString("");
    else
	return vars[v].first();
#else
    return isEmpty(v) ? QString("") : vars[v].first();
#endif
}

inline QMap<QString, QStringList> &QMakeMetaInfo::variables()
{ return vars; }

inline bool QMakeMetaInfo::libExists(const QString &lib)
{ return !findLib(lib).isNull(); }

#endif /* __META_H__ */
