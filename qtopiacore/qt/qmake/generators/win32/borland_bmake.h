/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_COMMERCIAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef BORLAND_BMAKE_H
#define BORLAND_BMAKE_H

#include "winmakefile.h"

class BorlandMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    void writeBorlandParts(QTextStream &);
    void writeBuildRulesPart(QTextStream &t);
    void writeCleanParts(QTextStream &t);
    bool writeMakefile(QTextStream &);
    void init();

public:
    BorlandMakefileGenerator();
    ~BorlandMakefileGenerator();
};

inline BorlandMakefileGenerator::~BorlandMakefileGenerator()
{ }

#endif // BORLAND_BMAKE_H
