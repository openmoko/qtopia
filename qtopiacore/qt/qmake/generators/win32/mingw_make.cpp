/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the qmake application of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "mingw_make.h"
#include "option.h"
#include <qregexp.h>
#include <qdir.h>
#include <stdlib.h>
#include <time.h>

QT_BEGIN_NAMESPACE

MingwMakefileGenerator::MingwMakefileGenerator() : Win32MakefileGenerator(), init_flag(false)
{
    if (Option::shellPath.isEmpty())
        quote = "\"";
    else
        quote = "'";
}

bool MingwMakefileGenerator::isWindowsShell() const
{
#ifdef Q_OS_WIN
    return Option::shellPath.isEmpty();
#else
    return Win32MakefileGenerator::isWindowsShell();
#endif
}

QString MingwMakefileGenerator::escapeDependencyPath(const QString &path) const
{
    QString ret = path;
    ret.remove('\"');
    ret.replace('\\', "/");
    ret.replace(' ', "\\ ");
    return ret;
}

QString MingwMakefileGenerator::getLibTarget()
{
    return QString("lib" + project->first("TARGET") + project->first("TARGET_VERSION_EXT") + ".a");
}

bool MingwMakefileGenerator::findLibraries()
{
    QStringList &l = project->values("QMAKE_LIBS");

    QList<QMakeLocalFileName> dirs;
    {
        QStringList &libpaths = project->values("QMAKE_LIBDIR");
        for(QStringList::Iterator libpathit = libpaths.begin();
            libpathit != libpaths.end(); ++libpathit)
            dirs.append(QMakeLocalFileName((*libpathit)));
    }

    QStringList::Iterator it = l.begin();
    while (it != l.end()) {
        if ((*it).startsWith("-l")) {
            QString steam = (*it).mid(2);
            QString suffix;
            if (!project->isEmpty("QMAKE_" + steam.toUpper() + "_SUFFIX"))
                suffix = project->first("QMAKE_" + steam.toUpper() + "_SUFFIX");
            QString extension;
            for (QList<QMakeLocalFileName>::Iterator dir_it = dirs.begin(); dir_it != dirs.end(); ++dir_it) {
                int ver = findHighestVersion((*dir_it).local(), steam, "dll.a|a");
                if (ver != -1) {
                    extension += QString::number(ver);
                    break;
                }
            }
	        extension += suffix;
	        (*it) += extension;
	    } else if((*it).startsWith("-L")) {
            dirs.append(QMakeLocalFileName((*it).mid(2)));
        }

        ++it;
    }
    return true;
}

bool MingwMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->values("QMAKE_FAILED_REQUIREMENTS").isEmpty()) {
        t << "all clean:" << "\n\t"
          << "@echo \"Some of the required modules ("
          << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
          << "@echo \"Skipped.\"" << endl << endl;
        writeMakeQmake(t);
        return true;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
        if(Option::mkfile::do_stub_makefile) {
            t << "QMAKE    = "        << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") : var("QMAKE_QMAKE")) << endl;
            QStringList &qut = project->values("QMAKE_EXTRA_TARGETS");
            for(QStringList::ConstIterator it = qut.begin(); it != qut.end(); ++it)
                t << *it << " ";
            t << "first all clean install distclean uninstall: qmake" << endl
              << "qmake_all:" << endl;
            writeMakeQmake(t);
            if(project->isEmpty("QMAKE_NOFORCE"))
                t << "FORCE:" << endl << endl;
            return true;
        }
        writeMingwParts(t);
        return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
        writeSubDirs(t);
        return true;
    }
    return false;
 }

void createLdObjectScriptFile(const QString &fileName, const QStringList &objList)
{
    QString filePath = Option::output_dir + QDir::separator() + fileName;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream t(&file);
        t << "INPUT(" << endl;
        for (QStringList::ConstIterator it = objList.constBegin(); it != objList.constEnd(); ++it) {
            if (QDir::isRelativePath(*it))
		t << "./" << *it << endl;
	    else
		t << *it << endl;
        }
        t << ");" << endl;
	t.flush();
        file.close();
    }
}

void createArObjectScriptFile(const QString &fileName, const QString &target, const QStringList &objList)
{
    QString filePath = Option::output_dir + QDir::separator() + fileName;
    QFile file(filePath);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream t(&file);
        t << "CREATE " << target << endl;
        for (QStringList::ConstIterator it = objList.constBegin(); it != objList.constEnd(); ++it) {
            if (QDir::isRelativePath(*it))
		t << "ADDMOD " << *it << endl;
	    else
		t << *it << endl;
        }
        t << "SAVE" << endl;
	t.flush();
        file.close();
    }
}

void MingwMakefileGenerator::writeMingwParts(QTextStream &t)
{
    writeStandardParts(t);

    if (!preCompHeaderOut.isEmpty()) {
	QString header = project->first("PRECOMPILED_HEADER");
	QString cHeader = preCompHeaderOut + Option::dir_sep + "c";
	t << escapeDependencyPath(cHeader) << ": " << escapeDependencyPath(header) << " "
          << escapeDependencyPaths(findDependencies(header)).join(" \\\n\t\t")
	  << "\n\t" << mkdir_p_asstring(preCompHeaderOut)
	  << "\n\t" << "$(CC) -x c-header -c $(CFLAGS) $(INCPATH) -o " << cHeader << " " << header
          << endl << endl;
	QString cppHeader = preCompHeaderOut + Option::dir_sep + "c++";
	t << escapeDependencyPath(cppHeader) << ": " << escapeDependencyPath(header) << " "
          << escapeDependencyPaths(findDependencies(header)).join(" \\\n\t\t")
	  << "\n\t" << mkdir_p_asstring(preCompHeaderOut)
	  << "\n\t" << "$(CXX) -x c++-header -c $(CXXFLAGS) $(INCPATH) -o " << cppHeader << " " << header
          << endl << endl;
    }
}

void MingwMakefileGenerator::init()
{
    if(init_flag)
        return;
    init_flag = true;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
        project->values("QMAKE_APP_FLAG").append("1");
    else if(project->first("TEMPLATE") == "lib")
        project->values("QMAKE_LIB_FLAG").append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
        MakefileGenerator::init();
        if(project->isEmpty("QMAKE_COPY_FILE"))
            project->values("QMAKE_COPY_FILE").append("$(COPY)");
        if(project->isEmpty("QMAKE_COPY_DIR"))
            project->values("QMAKE_COPY_DIR").append("xcopy /s /q /y /i");
        if(project->isEmpty("QMAKE_INSTALL_FILE"))
            project->values("QMAKE_INSTALL_FILE").append("$(COPY_FILE)");
        if(project->isEmpty("QMAKE_INSTALL_PROGRAM"))
            project->values("QMAKE_INSTALL_PROGRAM").append("$(COPY_FILE)");
        if(project->isEmpty("QMAKE_INSTALL_DIR"))
            project->values("QMAKE_INSTALL_DIR").append("$(COPY_DIR)");
        if(project->values("MAKEFILE").isEmpty())
            project->values("MAKEFILE").append("Makefile");
        if(project->values("QMAKE_QMAKE").isEmpty())
            project->values("QMAKE_QMAKE").append("qmake");
        return;
    }

    project->values("TARGET_PRL").append(project->first("TARGET"));

    processVars();

    if (!project->values("RES_FILE").isEmpty()) {
        project->values("QMAKE_LIBS") += escapeFilePaths(project->values("RES_FILE"));
    }

    // LIBS defined in Profile comes first for gcc
    project->values("QMAKE_LIBS") += escapeFilePaths(project->values("LIBS"));

    QString targetfilename = project->values("TARGET").first();
    QStringList &configs = project->values("CONFIG");

    if(project->isActiveConfig("qt_dll"))
        if(configs.indexOf("qt") == -1)
            configs.append("qt");

    if(project->isActiveConfig("dll")) {
        QString destDir = "";
        if(!project->first("DESTDIR").isEmpty())
            destDir = Option::fixPathToTargetOS(project->first("DESTDIR") + Option::dir_sep, false, false);
        project->values("MINGW_IMPORT_LIB").prepend(destDir + "lib" + project->first("TARGET")
                                                         + project->first("TARGET_VERSION_EXT") + ".a");
	project->values("QMAKE_LFLAGS").append(QString("-Wl,--out-implib,") + project->first("MINGW_IMPORT_LIB"));
    }

    if(!project->values("DEF_FILE").isEmpty())
        project->values("QMAKE_LFLAGS").append(QString("-Wl,") + project->first("DEF_FILE"));

    MakefileGenerator::init();

    // precomp
    if (!project->first("PRECOMPILED_HEADER").isEmpty()
        && project->isActiveConfig("precompile_header")) {
        QString preCompHeader = var("PRECOMPILED_DIR")
		         + QFileInfo(project->first("PRECOMPILED_HEADER")).fileName();
	preCompHeaderOut = preCompHeader + ".gch";
	project->values("QMAKE_CLEAN").append(preCompHeaderOut + Option::dir_sep + "c");
	project->values("QMAKE_CLEAN").append(preCompHeaderOut + Option::dir_sep + "c++");

	project->values("QMAKE_RUN_CC").clear();
	project->values("QMAKE_RUN_CC").append("$(CC) -c -include " + preCompHeader +
                                                    " $(CFLAGS) $(INCPATH) -o $obj $src");
        project->values("QMAKE_RUN_CC_IMP").clear();
	project->values("QMAKE_RUN_CC_IMP").append("$(CC)  -c -include " + preCompHeader +
                                                        " $(CFLAGS) $(INCPATH) -o $@ $<");
        project->values("QMAKE_RUN_CXX").clear();
	project->values("QMAKE_RUN_CXX").append("$(CXX) -c -include " + preCompHeader +
                                                     " $(CXXFLAGS) $(INCPATH) -o $obj $src");
        project->values("QMAKE_RUN_CXX_IMP").clear();
	project->values("QMAKE_RUN_CXX_IMP").append("$(CXX) -c -include " + preCompHeader +
                                                         " $(CXXFLAGS) $(INCPATH) -o $@ $<");
    }

    if(project->isActiveConfig("dll")) {
        project->values("QMAKE_CLEAN").append(project->first("MINGW_IMPORT_LIB"));
    }
}

void MingwMakefileGenerator::fixTargetExt()
{
    if (project->isActiveConfig("staticlib") && project->first("TEMPLATE") == "lib") {
        project->values("TARGET_EXT").append(".a");
        project->values("QMAKE_LFLAGS").append("-static");
        project->values("TARGET").first() =  "lib" + project->first("TARGET");
    } else {
        Win32MakefileGenerator::fixTargetExt();
    }
}

void MingwMakefileGenerator::writeIncPart(QTextStream &t)
{
    t << "INCPATH       = ";

    QStringList &incs = project->values("INCLUDEPATH");
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
        QString inc = (*incit);
        inc.replace(QRegExp("\\\\$"), "");
        inc.replace(QRegExp("\""), "");
        t << "-I" << quote << inc << quote << " ";
    }
    t << "-I" << quote << specdir() << quote
      << endl;
}

void MingwMakefileGenerator::writeLibsPart(QTextStream &t)
{
    if(project->isActiveConfig("staticlib") && project->first("TEMPLATE") == "lib") {
        t << "LIB        =        " << var("QMAKE_LIB") << endl;
    } else {
        t << "LINK        =        " << var("QMAKE_LINK") << endl;
        t << "LFLAGS        =        " << var("QMAKE_LFLAGS") << endl;
        t << "LIBS        =        ";
        if(!project->values("QMAKE_LIBDIR").isEmpty())
            writeLibDirPart(t);
        t << var("QMAKE_LIBS").replace(QRegExp("(\\slib|^lib)")," -l") << endl;
    }
}

void MingwMakefileGenerator::writeLibDirPart(QTextStream &t)
{
    QStringList libDirs = project->values("QMAKE_LIBDIR");
    for (int i = 0; i < libDirs.size(); ++i)
        libDirs[i].remove("\"");
    t << valGlue(libDirs,"-L"+quote,quote+" -L" +quote,quote) << " ";
}

void MingwMakefileGenerator::writeObjectsPart(QTextStream &t)
{
    if (project->values("OBJECTS").count() < var("QMAKE_LINK_OBJECT_MAX").toInt()) {
        objectsLinkLine = "$(OBJECTS)";
    } else if (project->isActiveConfig("staticlib") && project->first("TEMPLATE") == "lib") {
	QString ar_script_file = var("QMAKE_LINK_OBJECT_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ar_script_file += "." + var("BUILD_NAME");
	}
	createArObjectScriptFile(ar_script_file, var("DEST_TARGET"), project->values("OBJECTS"));
        objectsLinkLine = "ar -M < " + ar_script_file;
    } else {
        QString ld_script_file = var("QMAKE_LINK_OBJECT_SCRIPT") + "." + var("TARGET");
	if (!var("BUILD_NAME").isEmpty()) {
	    ld_script_file += "." + var("BUILD_NAME");
	}
	createLdObjectScriptFile(ld_script_file, project->values("OBJECTS"));
        objectsLinkLine = ld_script_file;
    }
    Win32MakefileGenerator::writeObjectsPart(t);
}

void MingwMakefileGenerator::writeBuildRulesPart(QTextStream &t)
{
    t << "first: all" << endl;
    t << "all: " << escapeDependencyPath(fileFixify(Option::output.fileName())) << " " << valGlue(escapeDependencyPaths(project->values("ALL_DEPS"))," "," "," ") << " $(DESTDIR_TARGET)" << endl << endl;
    t << "$(DESTDIR_TARGET): " << var("PRE_TARGETDEPS") << " $(OBJECTS) " << var("POST_TARGETDEPS");
    if(!project->isEmpty("QMAKE_PRE_LINK"))
        t << "\n\t" <<var("QMAKE_PRE_LINK");
    if(project->isActiveConfig("staticlib") && project->first("TEMPLATE") == "lib") {
	if (project->values("OBJECTS").count() < var("QMAKE_LINK_OBJECT_MAX").toInt()) {
            t << "\n\t" << "$(LIB) $(DESTDIR_TARGET) " << objectsLinkLine << " " ;
        } else {
            t << "\n\t" << objectsLinkLine << " " ;
        }
    } else {
        t << "\n\t" << "$(LINK) $(LFLAGS) -o $(DESTDIR_TARGET) " << objectsLinkLine << " " << " $(LIBS)";
    }
    if(!project->isEmpty("QMAKE_POST_LINK"))
        t << "\n\t" <<var("QMAKE_POST_LINK");
    t << endl;
}

void MingwMakefileGenerator::writeRcFilePart(QTextStream &t)
{
    const QString rc_file = fileFixify(project->first("RC_FILE"));

    QString incPathStr = fileInfo(rc_file).path();
    if (incPathStr != "." && QDir::isRelativePath(incPathStr))
        incPathStr.prepend("./");

    if (!rc_file.isEmpty()) {
        t << escapeDependencyPath(var("RES_FILE")) << ": " << rc_file << "\n\t"
          << var("QMAKE_RC") << " -i " << rc_file << " -o " << var("RES_FILE") 
          << " --include-dir=" << incPathStr << endl << endl;
    }
}

void MingwMakefileGenerator::processPrlVariable(const QString &var, const QStringList &l)
{
    if (var == "QMAKE_PRL_LIBS") {
        QString where = "QMAKE_LIBS";
        if (!project->isEmpty("QMAKE_INTERNAL_PRL_LIBS"))
            where = project->first("QMAKE_INTERNAL_PRL_LIBS");
        QStringList &out = project->values(where);
        for (QStringList::ConstIterator it = l.begin(); it != l.end(); ++it) {
            out.removeAll((*it));
            out.append((*it));
        }
    } else {
        Win32MakefileGenerator::processPrlVariable(var, l);
    }
}

QStringList &MingwMakefileGenerator::findDependencies(const QString &file)
{
    QStringList &aList = MakefileGenerator::findDependencies(file);
    // Note: The QMAKE_IMAGE_COLLECTION file have all images
    // as dependency, so don't add precompiled header then
    if (file == project->first("QMAKE_IMAGE_COLLECTION")
        || preCompHeaderOut.isEmpty())
        return aList;
    for (QStringList::Iterator it = Option::c_ext.begin(); it != Option::c_ext.end(); ++it) {
        if (file.endsWith(*it)) {
            QString cHeader = preCompHeaderOut + Option::dir_sep + "c";
            if (!aList.contains(cHeader))
                aList += cHeader;
            break;
        }
    }
    for (QStringList::Iterator it = Option::cpp_ext.begin(); it != Option::cpp_ext.end(); ++it) {
        if (file.endsWith(*it)) {
            QString cppHeader = preCompHeaderOut + Option::dir_sep + "c++";
            if (!aList.contains(cppHeader))
                aList += cppHeader;
            break;
        }
    }
    return aList;
}

QT_END_NAMESPACE
