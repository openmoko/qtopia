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

#include "meta.h"
#include "project.h"
#include "option.h"
#include <qdir.h>

QMakeMetaInfo::QMakeMetaInfo()
{
    
}


bool
QMakeMetaInfo::readLib(const QString &lib)
{
    clear();
    QString meta_file = findLib(lib);
    if(!meta_file.isNull()) {
	if(meta_file.endsWith(Option::pkgcfg_ext)) {
	    if(readPkgCfgFile(meta_file)) {
		meta_type = "pkgcfg";
		return TRUE;
	    }
	    return FALSE;
	} else if(meta_file.endsWith(Option::libtool_ext)) {
	    if(readLibtoolFile(meta_file)) {
		meta_type = "libtool";
		return TRUE;
	    }
	    return FALSE;
	} else if(meta_file.endsWith(Option::prl_ext)) {
	    QMakeProject proj;
	    if(!proj.read(Option::fixPathToLocalOS(meta_file), 
			  QDir::currentDirPath(), QMakeProject::ReadProFile))
		return FALSE;
	    meta_type = "qmake";
	    vars = proj.variables();
	    return TRUE;
	} else {
	    warn_msg(WarnLogic, "QMakeMetaInfo: unknown file format for %s", meta_file.latin1());
	}
    }
    return FALSE;
}


void
QMakeMetaInfo::clear()
{
    vars.clear();
}


QString
QMakeMetaInfo::findLib(const QString &lib)
{
    QString ret = QString::null;
    QString extns[] = { Option::prl_ext, /*Option::pkgcfg_ext, Option::libtool_ext,*/ QString::null };
    for(int extn = 0; !extns[extn].isNull(); extn++) {
	if(lib.endsWith(extns[extn]))
	    ret = QFile::exists(lib) ? lib : QString::null;
    }
    if(ret.isNull()) {
	for(int extn = 0; !extns[extn].isNull(); extn++) {
	    if(QFile::exists(lib + extns[extn])) {
		ret = lib + extns[extn];
		break;
	    }
	}
    }
    if(ret.isNull())
	debug_msg(2, "QMakeMetaInfo: Cannot find info file for %s", lib.latin1());
    else
	debug_msg(2, "QMakeMetaInfo: Found info file %s for %s", ret.latin1(), lib.latin1());
    return ret;
}


bool
QMakeMetaInfo::readLibtoolFile(const QString &f)
{
    /* I can just run the .la through the .pro parser since they are compatible.. */
    QMakeProject proj;
    if(!proj.read(Option::fixPathToLocalOS(f), QDir::currentDirPath(), QMakeProject::ReadProFile))
	return FALSE;
    QString dirf = Option::fixPathToTargetOS(f).section(Option::dir_sep, 0, -2);
    if(dirf == f)
	dirf = "";
    else if(!dirf.isEmpty() && !dirf.endsWith(Option::output_dir))
	dirf += Option::dir_sep;
    QMap<QString, QStringList> &v = proj.variables();
    for(QMap<QString, QStringList>::Iterator it = v.begin(); it != v.end(); ++it) {
	QStringList lst = it.data();
	if(lst.count() == 1 && (lst.first().startsWith("'") || lst.first().startsWith("\"")) &&
	   lst.first().endsWith(QString(lst.first()[0])))
	    lst = lst.first().mid(1, lst.first().length() - 2);
	if(!vars.contains("QMAKE_PRL_TARGET") &&
	   (it.key() == "dlname" || it.key() == "library_names" || it.key() == "old_library")) {
	    QString dir = v["libdir"].first();
	    if((dir.startsWith("'") || dir.startsWith("\"")) && dir.endsWith(QString(dir[0])))
		dir = dir.mid(1, dir.length() - 2);
	    dir = dir.stripWhiteSpace();
	    if(!dir.isEmpty() && !dir.endsWith(Option::dir_sep))
		dir += Option::dir_sep;
	    if(lst.count() == 1)
		lst = QStringList::split(" ", lst.first());
	    for(QStringList::Iterator lst_it = lst.begin(); lst_it != lst.end(); ++lst_it) {
		bool found = FALSE;
		QString dirs[] = { "", dir, dirf, dirf + ".libs" + QDir::separator(), "(term)" };
		for(int i = 0; !found && dirs[i] != "(term)"; i++) {
		    if(QFile::exists(dirs[i] + (*lst_it))) {
			QString targ = dirs[i] + (*lst_it);
			if(QDir::isRelativePath(targ)) 
			    targ.prepend(QDir::currentDirPath() + QDir::separator());
			vars["QMAKE_PRL_TARGET"] << targ;
			found = TRUE;
		    }
		}
		if(found)
 		    break;
	    }
	} else if(it.key() == "dependency_libs") {
	    if(lst.count() == 1) {
		QString dep = lst.first();
		if((dep.startsWith("'") || dep.startsWith("\"")) && dep.endsWith(QString(dep[0])))
		    dep = dep.mid(1, dep.length() - 2);
		lst = QStringList::split(" ", dep.stripWhiteSpace());
	    }
	    QMakeProject *conf = NULL;
	    for(QStringList::Iterator lit = lst.begin(); lit != lst.end(); ++lit) {
		if((*lit).startsWith("-R")) {
		    if(!conf) {
			conf = new QMakeProject;
			conf->read(QMakeProject::ReadAll ^ QMakeProject::ReadProFile);
		    }
		    if(!conf->isEmpty("QMAKE_RPATH"))
			(*lit) = conf->first("QMAKE_RPATH") + (*lit).mid(2);
		}
	    }
	    if(conf)
		delete conf;
	    vars["QMAKE_PRL_LIBS"] += lst;
	}
    }
    return TRUE;
}

bool
QMakeMetaInfo::readPkgCfgFile(const QString &f)
{
    fprintf(stderr, "Must implement reading in pkg-config files (%s)!!!\n", f.latin1());
    return FALSE;
}
