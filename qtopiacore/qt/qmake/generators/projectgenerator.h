/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROJECTGENERATOR_H
#define PROJECTGENERATOR_H

#include "makefile.h"

class ProjectGenerator : public MakefileGenerator
{
    bool init_flag;
    bool addFile(QString);
    bool addConfig(const QString &, bool add=true);
    QString getWritableVar(const QString &, bool fixPath=true);
    QString fixPathToQmake(const QString &file);
protected:
    virtual void init();
    virtual bool writeMakefile(QTextStream &);
public:
    ProjectGenerator();
    ~ProjectGenerator();
    virtual bool supportsMetaBuild() { return false; }
    virtual bool openOutput(QFile &, const QString &) const;
};

inline ProjectGenerator::~ProjectGenerator()
{ }

#endif // PROJECTGENERATOR_H
