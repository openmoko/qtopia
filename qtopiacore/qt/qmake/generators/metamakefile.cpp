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

#include "metamakefile.h"
#include "qregexp.h"
#include "qdir.h"
#include "qdebug.h"
#include "makefile.h"
#include "project.h"
#include "cachekeys.h"

#define BUILDSMETATYPE 1
#define SUBDIRSMETATYPE 2

MetaMakefileGenerator::~MetaMakefileGenerator()
{
    if(own_project)
        delete project;
}

class BuildsMetaMakefileGenerator : public MetaMakefileGenerator
{
    bool init_flag;
private:
    struct Build {
        QString name;
        MakefileGenerator *makefile;
    };
    QList<Build *> makefiles;
    void clearBuilds();
    MakefileGenerator *processBuild(const QString &);

public:

    BuildsMetaMakefileGenerator(QMakeProject *p, bool op) : MetaMakefileGenerator(p, op), init_flag(false) { }
    virtual ~BuildsMetaMakefileGenerator() { clearBuilds(); }

    virtual bool init();
    virtual int type() const { return BUILDSMETATYPE; }
    virtual bool write(const QString &);
};

void
BuildsMetaMakefileGenerator::clearBuilds()
{
    for(int i = 0; i < makefiles.count(); i++) {
        Build *build = makefiles[i];
        if(QMakeProject *p = build->makefile->projectFile()) {
            if(p != project)
                delete p;
        }
        delete build->makefile;
        delete build;
    }
    makefiles.clear();
}

bool
BuildsMetaMakefileGenerator::init()
{
    if(init_flag)
        return false;
    init_flag = true;

    const QStringList &builds = project->variables()["BUILDS"];
    bool use_single_build = builds.isEmpty();
    if(builds.count() > 1 && Option::output.fileName() == "-") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot direct to stdout when using multiple BUILDS.");
    } else if(0 && !use_single_build && project->first("TEMPLATE") == "subdirs") {
        use_single_build = true;
        warn_msg(WarnLogic, "Cannot specify multiple builds with TEMPLATE subdirs.");
    }
    if(!use_single_build) {
        for(int i = 0; i < builds.count(); i++) {
            QString build = builds[i];
            MakefileGenerator *makefile = processBuild(build);
            if(!makefile)
                return false;
            if(!makefile->supportsMetaBuild()) {
                warn_msg(WarnLogic, "QMAKESPEC does not support multiple BUILDS.");
                clearBuilds();
                use_single_build = true;
                break;
            } else {
                Build *b = new Build;
                if(builds.count() != 1)
                    b->name = build;
                b->makefile = makefile;
                makefiles += b;
            }
        }
    }
    if(use_single_build) {
        Build *build = new Build;
        build->makefile = createMakefileGenerator(project, false);
        makefiles += build;
    }
    return true;
}

bool
BuildsMetaMakefileGenerator::write(const QString &oldpwd)
{
    Build *glue = 0;
    if(!makefiles.isEmpty() && !makefiles.first()->name.isNull()) {
        glue = new Build;
        glue->makefile = createMakefileGenerator(project, true);
        makefiles += glue;
    }

    bool ret = true;
    const QString &output_name = Option::output.fileName();
    for(int i = 0; ret && i < makefiles.count(); i++) {
        Option::output.setFileName(output_name);
        Build *build = makefiles[i];

        bool using_stdout = false;
        if(build->makefile && (Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
                               Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT)
           && (!build->makefile->supportsMergedBuilds()
            || (build->makefile->supportsMergedBuilds() && (!glue || build == glue)))) {
            //open output
            if(!(Option::output.isOpen())) {
                if(Option::output.fileName() == "-") {
                    Option::output.setFileName("");
                    Option::output_dir = qmake_getpwd();
                    Option::output.open(stdout, QIODevice::WriteOnly | QIODevice::Text);
                    using_stdout = true;
                } else {
                    if(Option::output.fileName().isEmpty() && Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE)
                        Option::output.setFileName(project->first("QMAKE_MAKEFILE"));
                    Option::output_dir = oldpwd;
                    if(!build->makefile->openOutput(Option::output, build->name)) {
                        fprintf(stderr, "Failure to open file: %s\n",
                                Option::output.fileName().isEmpty() ? "(stdout)" :
                                Option::output.fileName().toLatin1().constData());
                        return false;
                    }
                }
            }
        } else {
           using_stdout = true; //kind of..
        }

        if(!build->makefile) {
            ret = false;
        } else if(build == glue) {
            ret = build->makefile->writeProjectMakefile();
        } else {
            ret = build->makefile->write();
            if (glue && glue->makefile->supportsMergedBuilds())
                ret = glue->makefile->mergeBuildProject(build->makefile);
        }
        if(!using_stdout) {
            Option::output.close();
            if(!ret)
                Option::output.remove();
        }

        // debugging
        if(Option::debug_level) {
            QMap<QString, QStringList> &vars = project->variables();
            for(QMap<QString, QStringList>::Iterator it = vars.begin(); it != vars.end(); ++it) {
                if(!it.key().startsWith(".") && !it.value().isEmpty())
                    debug_msg(1, "%s === %s", it.key().toLatin1().constData(),
                              it.value().join(" :: ").toLatin1().constData());
            }
        }
    }
    return ret;
}

MakefileGenerator
*BuildsMetaMakefileGenerator::processBuild(const QString &build)
{
    if(project) {
        debug_msg(1, "Meta Generator: Parsing '%s' for build [%s].",
                  project->projectFile().toLatin1().constData(),build.toLatin1().constData());

        //initialize the base
        QMap<QString, QStringList> basevars;
        if(!project->isEmpty(build + ".CONFIG"))
            basevars["CONFIG"] += project->values(build + ".CONFIG");
        basevars["CONFIG"] += build;
        basevars["CONFIG"] += "build_pass";
        basevars["BUILD_PASS"] = QStringList(build);
        QStringList buildname = project->values(build + ".name");
        basevars["BUILD_NAME"] = (buildname.isEmpty() ? QStringList(build) : buildname);

        //create project
        QMakeProject *build_proj = new QMakeProject(project->properities(), basevars);

        //all the user configs must be set again afterwards (for .pro tests and for .prf tests)
        const QStringList old_after_user_config = Option::after_user_configs;
        const QStringList old_user_config = Option::user_configs;
        Option::after_user_configs += basevars["CONFIG"];
        Option::user_configs += basevars["CONFIG"];
        build_proj->read(project->projectFile());
        Option::after_user_configs = old_after_user_config;
        Option::user_configs = old_user_config;

        //done
        return createMakefileGenerator(build_proj);
    }
    return 0;
}

class SubdirsMetaMakefileGenerator : public MetaMakefileGenerator
{
    bool init_flag;
private:
    struct Subdir {
        Subdir() : makefile(0), indent(0) { }
        ~Subdir() { delete makefile; }
        QString input_dir;
        QString output_dir, output_file;
        MetaMakefileGenerator *makefile;
        int indent;
    };
    QList<Subdir *> subs;
    MakefileGenerator *processBuild(const QString &);

public:
    SubdirsMetaMakefileGenerator(QMakeProject *p, bool op) : MetaMakefileGenerator(p, op), init_flag(false) { }
    virtual ~SubdirsMetaMakefileGenerator();

    virtual bool init();
    virtual int type() const { return SUBDIRSMETATYPE; }
    virtual bool write(const QString &);
};

bool
SubdirsMetaMakefileGenerator::init()
{
    if(init_flag)
        return false;
    init_flag = true;

    if(Option::recursive) {
        const QString old_output_dir = QDir::cleanPath(Option::output_dir);
        const QString oldpwd = QDir::cleanPath(qmake_getpwd());
        const QStringList &subdirs = project->values("SUBDIRS");
        static int recurseDepth = -1;
        ++recurseDepth;
        for(int i = 0; i < subdirs.size(); ++i) {
            Subdir *sub = new Subdir;
            sub->indent = recurseDepth;

            QFileInfo subdir(subdirs.at(i));
            if(!project->isEmpty(subdirs.at(i) + ".file"))
                subdir = project->first(subdirs.at(i) + ".file");
            else if(!project->isEmpty(subdirs.at(i) + ".subdir"))
                subdir = project->first(subdirs.at(i) + ".subdir");
            if(subdir.isDir())
                subdir = QFileInfo(subdir.filePath() + "/" + subdir.fileName() + Option::pro_ext);

            //handle sub project
            QMakeProject *sub_proj = new QMakeProject(project->properities());
            for (int ind = 0; ind < sub->indent; ++ind)
                printf(" ");
            sub->input_dir = subdir.absolutePath();
            if(subdir.isRelative() && old_output_dir != oldpwd) {
                sub->output_dir = old_output_dir + "/" + subdir.path();
                printf("Reading %s [%s]\n", subdir.absoluteFilePath().toLatin1().constData(), sub->output_dir.toLatin1().constData());
            } else { //what about shadow builds?
                sub->output_dir = sub->input_dir;
                printf("Reading %s\n", subdir.absoluteFilePath().toLatin1().constData());
            }
            qmake_setpwd(sub->input_dir);
            Option::output_dir = sub->output_dir;
            if(Option::output_dir.at(Option::output_dir.length()-1) != QLatin1Char('/'))
                Option::output_dir += QLatin1Char('/');
            sub_proj->read(subdir.fileName());
            if(!sub_proj->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
                fprintf(stderr, "Project file(%s) not recursed because all requirements not met:\n\t%s\n",
                        subdir.fileName().toLatin1().constData(),
                        sub_proj->values("QMAKE_FAILED_REQUIREMENTS").join(" ").toLatin1().constData());
                delete sub;
                delete sub_proj;
                continue;
            }
            sub->makefile = MetaMakefileGenerator::createMetaGenerator(sub_proj);
            if(0 && sub->makefile->type() == SUBDIRSMETATYPE) {
                subs.append(sub);
            } else {
                const QString &output_name = Option::output.fileName();
                Option::output.setFileName(sub->output_file);
                sub->makefile->write(sub->output_dir);
                delete sub;
                qmakeClearCaches();
                sub = 0;
                Option::output.setFileName(output_name);
            }
            Option::output_dir = old_output_dir;
            qmake_setpwd(oldpwd);

        }
        --recurseDepth;
        Option::output_dir = old_output_dir;
        qmake_setpwd(oldpwd);
    }

    Subdir *self = new Subdir;
    self->input_dir = qmake_getpwd();
    self->output_dir = Option::output_dir;
    self->output_file = Option::output.fileName();
    self->makefile = new BuildsMetaMakefileGenerator(project, false);
    self->makefile->init();
    subs.append(self);
    return true;
}

bool
SubdirsMetaMakefileGenerator::write(const QString &passpwd)
{
    bool ret = true;
    const QString &oldpwd = qmake_getpwd();
    const QString &output_dir = Option::output_dir;
    const QString &output_name = Option::output.fileName();
    for(int i = 0; ret && i < subs.count(); i++) {
        const Subdir *sub = subs.at(i);
        qmake_setpwd(subs.at(i)->input_dir);
        Option::output_dir = QFileInfo(subs.at(i)->output_dir).absoluteFilePath();
        if(Option::output_dir.at(Option::output_dir.length()-1) != QLatin1Char('/'))
            Option::output_dir += QLatin1Char('/');
        Option::output.setFileName(subs.at(i)->output_file);
        if(i != subs.count()-1) {
            for (int ind = 0; ind < sub->indent; ++ind)
                printf(" ");
            printf("Writing %s\n", QDir::cleanPath(Option::output_dir+"/"+
                                                   Option::output.fileName()).toLatin1().constData());
        }
        QString writepwd = Option::fixPathToLocalOS(qmake_getpwd());
        if(!writepwd.startsWith(Option::fixPathToLocalOS(passpwd)))
            writepwd = passpwd;
        if(!(ret = subs.at(i)->makefile->write(writepwd)))
            break;
        qmake_setpwd(oldpwd);
    }
    //restore because I'm paranoid
    Option::output.setFileName(output_name);
    Option::output_dir = output_dir;
    return ret;
}

SubdirsMetaMakefileGenerator::~SubdirsMetaMakefileGenerator()
{
    for(int i = 0; i < subs.count(); i++)
        delete subs[i];
    subs.clear();
}

//Factory things
#include "unixmake.h"
#include "mingw_make.h"
#include "projectgenerator.h"
#include "pbuilder_pbx.h"
#ifndef QMAKE_OPENSOURCE_EDITION
# include "msvc_nmake.h"
# include "borland_bmake.h"
# include "msvc_dsp.h"
# include "msvc_vcproj.h"
#endif

MakefileGenerator *
MetaMakefileGenerator::createMakefileGenerator(QMakeProject *proj, bool noIO)
{
    MakefileGenerator *mkfile = NULL;
    if(Option::qmake_mode == Option::QMAKE_GENERATE_PROJECT) {
        mkfile = new ProjectGenerator;
        mkfile->setProjectFile(proj);
        return mkfile;
    }

    QString gen = proj->first("MAKEFILE_GENERATOR");
    if(gen.isEmpty()) {
        fprintf(stderr, "No generator specified in config file: %s\n",
                proj->projectFile().toLatin1().constData());
    } else if(gen == "UNIX") {
        mkfile = new UnixMakefileGenerator;
    } else if(gen == "MINGW") {
        mkfile = new MingwMakefileGenerator;
    } else if(gen == "PROJECTBUILDER" || gen == "XCODE") {
        mkfile = new ProjectBuilderMakefileGenerator;
#ifndef QMAKE_OPENSOURCE_EDITION
    } else if(gen == "MSVC") {
        // Visual Studio =< v6.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new DspMakefileGenerator;
        else
            mkfile = new NmakeMakefileGenerator;
    } else if(gen == "MSVC.NET") {
        // Visual Studio >= v7.0
        if(proj->first("TEMPLATE").indexOf(QRegExp("^vc.*")) != -1)
            mkfile = new VcprojGenerator;
        else
            mkfile = new NmakeMakefileGenerator;
    } else if(gen == "BMAKE") {
        mkfile = new BorlandMakefileGenerator;
#endif
    } else {
        fprintf(stderr, "Unknown generator specified: %s\n", gen.toLatin1().constData());
    }
    if (mkfile) {
        mkfile->setNoIO(noIO);
        mkfile->setProjectFile(proj);
    }
    return mkfile;
}

MetaMakefileGenerator *
MetaMakefileGenerator::createMetaGenerator(QMakeProject *proj, bool op)
{
    MetaMakefileGenerator *ret = 0;
    if((Option::qmake_mode == Option::QMAKE_GENERATE_MAKEFILE ||
        Option::qmake_mode == Option::QMAKE_GENERATE_PRL)) {
        if(proj->first("TEMPLATE").endsWith("subdirs"))
            ret = new SubdirsMetaMakefileGenerator(proj, op);
    }
    if(!ret)
        ret = new BuildsMetaMakefileGenerator(proj, op);
    ret->init();
    return ret;
}
