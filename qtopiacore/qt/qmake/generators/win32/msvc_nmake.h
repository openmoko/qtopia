/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef MSVC_NMAKE_H
#define MSVC_NMAKE_H

#include "winmakefile.h"

class NmakeMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeNmakeParts(QTextStream &);
    void writeLibDirPart(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void writeImplicitRulesPart(QTextStream &t);
    void writeBuildRulesPart(QTextStream &t);
    void init();

protected:
    virtual QStringList &findDependencies(const QString &file);
    QString var(const QString &value);
    QString precompH, precompObj, precompPch;
    bool usePCH;

public:
    NmakeMakefileGenerator();
    ~NmakeMakefileGenerator();

};

inline NmakeMakefileGenerator::~NmakeMakefileGenerator()
{ }

#endif // MSVC_NMAKE_H
