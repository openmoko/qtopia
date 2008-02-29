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
#ifndef __MAKEFILE_H__
#define __MAKEFILE_H__

#include "option.h"
#include "project.h"
#include <qtextstream.h>

class MakefileGenerator
{
    QString spec;
    bool init_opath_already, init_already, moc_aware, no_io;
    QStringList createObjectList(const QString &var);
    QString build_args();
    QMap<QString, QString> depHeuristics, depKeyMap, fileFixed;
    QMap<QString, QString> mocablesToMOC, mocablesFromMOC;
    QMap<QString, QStringList> depends;

protected:
    void writeObj(QTextStream &, const QString &obj, const QString &src);
    void writeUicSrc(QTextStream &, const QString &ui);
    void writeMocObj(QTextStream &, const QString &obj, const QString &src);
    void writeMocSrc(QTextStream &, const QString &src);
    void writeLexSrc(QTextStream &, const QString &lex);
    void writeYaccSrc(QTextStream &, const QString &yac);
    void writeInstalls(QTextStream &t, const QString &installs);
    void writeImageObj(QTextStream &t, const QString &obj);
    void writeImageSrc(QTextStream &t, const QString &images);

protected:

    QMakeProject *project;

    class MakefileDependDir {
    public:
	MakefileDependDir(const QString &r, const QString &l) : real_dir(r), local_dir(l) { }
	QString real_dir, local_dir;
    };
    bool generateDependencies(QPtrList<MakefileDependDir> &dirs, const QString &x, bool recurse);

    QString buildArgs();

    QString specdir();
    QString cleanFilePath(const QString &file) const;
    bool generateMocList(const QString &fn);

    QString findMocSource(const QString &moc_file) const;
    QString findMocDestination(const QString &src_file) const;
    QStringList &findDependencies(const QString &file);

    void setNoIO(bool o);
    bool noIO() const;

    void setMocAware(bool o);
    bool mocAware() const;
    void logicWarn(const QString &, const QString &);

    virtual bool doDepends() const { return Option::mkfile::do_deps; }
    bool writeHeader(QTextStream &);
    virtual bool writeMakefile(QTextStream &);
    virtual bool writeMakeQmake(QTextStream &);
    void initOutPaths();
    virtual void init();

    //for installs
    virtual QString defaultInstall(const QString &);

    //for prl
    bool processPrlFile(QString &);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual void processPrlFiles();
    virtual void writePrlFile(QTextStream &);

    //make sure libraries are found
    virtual bool findLibraries();
    virtual QString findDependency(const QString &);

    QString var(const QString &var);
    QString varGlue(const QString &var, const QString &before, const QString &glue, const QString &after);
    QString varList(const QString &var);
    QString val(const QStringList &varList);
    QString valGlue(const QStringList &varList, const QString &before, const QString &glue, const QString &after);
    QString valList(const QStringList &varList);


    QString fileFixify(const QString& file, const QString &out_dir=QString::null, 
		       const QString &in_dir=QString::null, bool force_fix=FALSE, bool canon=TRUE) const;
    QStringList fileFixify(const QStringList& files, const QString &out_dir=QString::null, 
			   const QString &in_dir=QString::null, bool force_fix=FALSE, bool canon=TRUE) const;
public:
    MakefileGenerator(QMakeProject *p);
    virtual ~MakefileGenerator();

    static MakefileGenerator *create(QMakeProject *);
    virtual bool write();
    virtual bool openOutput(QFile &) const;
};

inline QString MakefileGenerator::findMocSource(const QString &moc_file) const
{
    QString tmp = cleanFilePath(moc_file);
    if (mocablesFromMOC.contains(tmp))
	return mocablesFromMOC[tmp];
    else
	return QString("");
}

inline QString MakefileGenerator::findMocDestination(const QString &src_file) const
{
    QString tmp = cleanFilePath(src_file);
    if (mocablesToMOC.contains(tmp))
	return mocablesToMOC[tmp];
    else
	return QString("");
}

inline void MakefileGenerator::setMocAware(bool o)
{ moc_aware = o; }

inline bool MakefileGenerator::mocAware() const
{ return moc_aware; }

inline void MakefileGenerator::setNoIO(bool o)
{ no_io = o; }

inline bool MakefileGenerator::noIO() const
{ return no_io; }

inline QString MakefileGenerator::defaultInstall(const QString &)
{ return QString(""); }

inline bool MakefileGenerator::findLibraries()
{ return TRUE; }

inline QString MakefileGenerator::findDependency(const QString &dep)
{ return QString(""); }

inline MakefileGenerator::~MakefileGenerator()
{ }

QString mkdir_p_asstring(const QString &dir);

#endif /* __MAKEFILE_H__ */
