/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MINGW_MAKE_H
#define MINGW_MAKE_H

#include "winmakefile.h"

class MingwMakefileGenerator : public Win32MakefileGenerator
{
public:
    MingwMakefileGenerator();
    ~MingwMakefileGenerator();
private:
    void writeMingwParts(QTextStream &);
    void writeLibsPart(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void writeObjectsPart(QTextStream &t);
    void writeBuildRulesPart(QTextStream &t);
    void writeRcFilePart(QTextStream &t);
    void init();
    void processPrlVariable(const QString &var, const QStringList &l);

    QStringList &findDependencies(const QString &file);
    
    QString preCompHeaderOut;

    virtual bool findLibraries();
    void fixTargetExt();

    bool init_flag;
    QString objectsLinkLine;
};

inline MingwMakefileGenerator::~MingwMakefileGenerator()
{ }

#endif // MINGW_MAKE_H
