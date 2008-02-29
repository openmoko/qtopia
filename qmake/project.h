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

#ifndef __PROJECT_H__
#define __PROJECT_H__

#include <qstringlist.h>
#include <qstring.h>
#include <qmap.h>

class QMakeProperty;

class QMakeProject
{
    enum TestStatus { TestNone, TestFound, TestSeek } test_status;
    int scope_block, scope_flag;

    QString pfile, cfile;
    QMakeProperty *prop;
    QMap<QString, QStringList> vars, base_vars, cache;
    bool parse(const QString &text, QMap<QString, QStringList> &place);
    bool doProjectTest(const QString &func, const QString &params, QMap<QString, QStringList> &place);
    bool doProjectTest(const QString &func, QStringList args, QMap<QString, QStringList> &place);
    bool doProjectCheckReqs(const QStringList &deps, QMap<QString, QStringList> &place);
    QString doVariableReplace(QString &str, const QMap<QString, QStringList> &place);

public:
    QMakeProject();
    QMakeProject(QMakeProperty *);

    enum { ReadCache=0x01, ReadConf=0x02, ReadCmdLine=0x04, ReadProFile=0x08, ReadPostFiles=0x10, ReadAll=0xFF };
    bool read(const QString &project, const QString &pwd, uchar cmd=ReadAll);
    bool read(uchar cmd=ReadAll);

    QString projectFile();
    QString configFile();

    bool isEmpty(const QString &v);
    QStringList &values(const QString &v);
    QString first(const QString &v);
    QMap<QString, QStringList> &variables();
    bool isActiveConfig(const QString &x, bool regex=FALSE, QMap<QString, QStringList> *place=NULL);

protected:
    friend class MakefileGenerator;
    bool read(const QString &file, QMap<QString, QStringList> &place);

};

inline QString QMakeProject::projectFile()
{
#if defined(Q_CC_SUN) && (__SUNPRO_CC == 0x500) || defined(Q_CC_HP)
    // workaround for Sun WorkShop 5.0 bug fixed in Forte 6
    if (pfile == "-")
	return QString("(stdin)");
    else
	return pfile;
#else
    return pfile == "-" ? QString("(stdin)") : pfile;
#endif
}

inline QString QMakeProject::configFile()
{ return cfile; }

inline bool QMakeProject::isEmpty(const QString &v)
{ return !vars.contains(v) || vars[v].isEmpty(); }

inline QStringList &QMakeProject::values(const QString &v)
{ return vars[v]; }

inline QString QMakeProject::first(const QString &v)
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

inline QMap<QString, QStringList> &QMakeProject::variables()
{ return vars; }

#endif /* __PROJECT_H__ */
