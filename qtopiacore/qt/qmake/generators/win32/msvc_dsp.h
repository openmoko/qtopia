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

#ifndef MSVC_DSP_H
#define MSVC_DSP_H

#include "winmakefile.h"

class FolderGroup;

class DspMakefileGenerator : public Win32MakefileGenerator
{
    bool init_flag;
    bool writeDspHeader(QTextStream &);
    bool writeDspParts(QTextStream &);
    bool writeFileGroup(QTextStream &t, const QStringList &listNames, const QString &group, const QString &filter);
    void writeSubFileGroup(QTextStream &t, FolderGroup *folder);
    bool writeBuildstepForFile(QTextStream &t, const QString &file, const QString &listName);
    static bool writeDspConfig(QTextStream &t, DspMakefileGenerator *config);
    static QString writeBuildstepForFileForConfig(const QString &file, const QString &listName, DspMakefileGenerator *config);
    QString configName(DspMakefileGenerator * config);

    bool writeMakefile(QTextStream &);
    bool writeProjectMakefile();
    void init();

public:
    DspMakefileGenerator();
    ~DspMakefileGenerator();

    bool openOutput(QFile &file, const QString &build) const;
    bool hasBuiltinCompiler(const QString &filename) const;

protected:
    virtual bool doDepends() const { return false; } //never necesary
    virtual void processSources() { filterIncludedFiles("SOURCES"); filterIncludedFiles("GENERATED_SOURCES"); }
    virtual QString replaceExtraCompilerVariables(const QString &, const QStringList &, const QStringList &);
    inline QString replaceExtraCompilerVariables(const QString &val, const QString &in, const QString &out)
    { return MakefileGenerator::replaceExtraCompilerVariables(val, in, out); }
    virtual bool supportsMetaBuild() { return true; }
    virtual bool supportsMergedBuilds() { return true; }
    virtual bool mergeBuildProject(MakefileGenerator *other);
    virtual void processPrlVariable(const QString &, const QStringList &);
    virtual bool findLibraries();

    bool usePCH;
    QString precompH, namePCH,
            precompObj, precompPch;

    QString platform;

    struct BuildStep {
        BuildStep() {}
        BuildStep &operator<<(const BuildStep &other) {
            deps << other.deps;
            buildStep += other.buildStep;
            buildName += other.buildName;
            buildOutputs += other.buildOutputs;
            return *this;
        }

        QStringList deps;
        QString buildStep;
        QString buildName;
        QStringList buildOutputs;
    };
    QMap<QString, BuildStep> swappedBuildSteps;

    // Holds all configurations for glue (merged) project
    QList<DspMakefileGenerator*> mergedProjects;
};

inline DspMakefileGenerator::~DspMakefileGenerator()
{ }

inline bool DspMakefileGenerator::findLibraries()
{ return Win32MakefileGenerator::findLibraries("MSVCDSP_LIBS"); }

#endif // MSVC_DSP_H
