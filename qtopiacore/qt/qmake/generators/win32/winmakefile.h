/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the qmake application of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef WINMAKEFILE_H
#define WINMAKEFILE_H

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
public:
    Win32MakefileGenerator();
    ~Win32MakefileGenerator();
protected:
    virtual QString defaultInstall(const QString &);
    virtual void writeCleanParts(QTextStream &t);
    virtual void writeStandardParts(QTextStream &t);
    virtual void writeLibDirPart(QTextStream &t);
    virtual void writeLibsPart(QTextStream &t);
    virtual void writeObjectsPart(QTextStream &t);
    virtual void writeImplicitRulesPart(QTextStream &t);
    virtual void writeBuildRulesPart(QTextStream &);
    virtual QString escapeFilePath(const QString &path) const;

    virtual void writeRcFilePart(QTextStream &t);

    int findHighestVersion(const QString &dir, const QString &stem);
    bool findLibraries(const QString &);
    virtual bool findLibraries();

    virtual void processPrlFiles();
    virtual void processVars();
    virtual void fixTargetExt();
    virtual void processRcFileVar();
    virtual void processFileTagsVar();
};

inline Win32MakefileGenerator::~Win32MakefileGenerator()
{ }

inline bool Win32MakefileGenerator::findLibraries()
{ return findLibraries("QMAKE_LIBS"); }

#endif // WINMAKEFILE_H
