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

#ifndef __MSVC_VCPROJ_H__
#define __MSVC_VCPROJ_H__

#include "winmakefile.h"
#include "msvc_objectmodel.h"

enum target {
    Application,
    SharedLib,
    StaticLib
};

struct QUuid;
class VcprojGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    bool writeVcprojParts(QTextStream &);

    bool writeMakefile(QTextStream &);
    virtual void writeSubDirs(QTextStream &t);
    QString findTemplate(QString file);
    void init();

public:
    VcprojGenerator(QMakeProject *p);
    ~VcprojGenerator();

    QString defaultMakefile() const;
    virtual bool doDepends() const { return FALSE; } //never necesary

protected:
    virtual bool openOutput(QFile &file) const;
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();
    virtual void outputVariables();
    QString fixFilename(QString ofile) const;
    
    void initOld();
    void initProject();
    void initConfiguration();
    void initCompilerTool();
    void initLinkerTool();
    void initLibrarianTool();
    void initIDLTool();
    void initCustomBuildTool();
    void initPreBuildEventTools();
    void initPostBuildEventTools();
    void initPreLinkEventTools();
    void initSourceFiles();
    void initHeaderFiles();
    void initMOCFiles();
    void initUICFiles();
    void initFormsFiles();
    void initTranslationFiles();
    void initLexYaccFiles();
    void initResourceFiles();

    VCProject vcProject;
    target projectTarget;

private:
    QUuid increaseUUID(const QUuid &id);
    friend class VCFilter;
};

inline VcprojGenerator::~VcprojGenerator()
{ }

inline QString VcprojGenerator::defaultMakefile() const
{
    return project->first("TARGET") + project->first("VCPROJ_EXTENSION");
}

inline bool VcprojGenerator::findLibraries()
{ 
    return Win32MakefileGenerator::findLibraries("MSVCVCPROJ_LIBS");
}

#endif /* __MSVC_VCPROJ_H__ */
