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

#ifndef __WINMAKEFILE_H__
#define __WINMAKEFILE_H__

#include "makefile.h"

// In the Qt evaluation and educational version, we have a postfix in the
// library name (e.g. qtmteval301.dll). QTDLL_POSTFIX is used for this.
// A script modifies these lines when building eval/edu version, so be careful
// when changing them.
#ifndef QTDLL_POSTFIX
#define QTDLL_POSTFIX ""
#endif

class Win32MakefileGenerator : public MakefileGenerator
{
protected:
    virtual void writeSubDirs(QTextStream &t);
    int findHighestVersion(const QString &dir, const QString &stem);
    bool findLibraries(const QString &);
    QString findDependency(const QString &);
    virtual bool findLibraries();
    virtual void processPrlFiles();
    virtual QString defaultInstall(const QString &);

public:
    Win32MakefileGenerator(QMakeProject *p);
    ~Win32MakefileGenerator();
};

inline Win32MakefileGenerator::~Win32MakefileGenerator()
{ }

inline bool Win32MakefileGenerator::findLibraries()
{ return findLibraries("QMAKE_LIBS"); }

inline QString Win32MakefileGenerator::defaultInstall(const QString &)
{ return QString(); }


#endif /* __WINMAKEFILE_H__ */
