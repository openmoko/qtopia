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

#include "msvc_nmake.h"
#include "option.h"
#include <qregexp.h>
#include <qdict.h>
#include <qdir.h>
#include <stdlib.h>
#include <time.h>


NmakeMakefileGenerator::NmakeMakefileGenerator(QMakeProject *p) : Win32MakefileGenerator(p), init_flag(FALSE)
{

}

bool
NmakeMakefileGenerator::writeMakefile(QTextStream &t)
{
    writeHeader(t);
    if(!project->variables()["QMAKE_FAILED_REQUIREMENTS"].isEmpty()) {
	t << "all clean:" << "\n\t"
	  << "@echo \"Some of the required modules ("
	  << var("QMAKE_FAILED_REQUIREMENTS") << ") are not available.\"" << "\n\t"
	  << "@echo \"Skipped.\"" << endl << endl;
	writeMakeQmake(t);
	return TRUE;
    }

    if(project->first("TEMPLATE") == "app" ||
       project->first("TEMPLATE") == "lib") {
	writeNmakeParts(t);
	return MakefileGenerator::writeMakefile(t);
    }
    else if(project->first("TEMPLATE") == "subdirs") {
	writeSubDirs(t);
	return TRUE;
    }
    return FALSE;
}

void
NmakeMakefileGenerator::writeNmakeParts(QTextStream &t)
{
    t << "####### Compiler, tools and options" << endl << endl;
    t << "CC		=	" << var("QMAKE_CC") << endl;
    t << "CXX		=	" << var("QMAKE_CXX") << endl;
    t << "LEX		= " << var("QMAKE_LEX") << endl;
    t << "YACC		= " << var("QMAKE_YACC") << endl;
    t << "CFLAGS	=	" << var("QMAKE_CFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      <<  varGlue("DEFINES","-D"," -D","") << endl;
    t << "CXXFLAGS	=	" << var("QMAKE_CXXFLAGS") << " "
      << varGlue("PRL_EXPORT_DEFINES","-D"," -D","") << " "
      << varGlue("DEFINES","-D"," -D","") << endl;
    t << "LEXFLAGS	=" << var("QMAKE_LEXFLAGS") << endl;
    t << "YACCFLAGS	=" << var("QMAKE_YACCFLAGS") << endl;

    t << "INCPATH	=	";
    QStringList &incs = project->variables()["INCLUDEPATH"];
    for(QStringList::Iterator incit = incs.begin(); incit != incs.end(); ++incit) {
	QString inc = (*incit);
	inc.replace(QRegExp("\\\\$"), "\\\\");
	inc.replace("\"", "");
	t << " -I\"" << inc << "\"";
    }
    t << " -I\"" << specdir() << "\""
      << endl;
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "LINK	=	" << var("QMAKE_LINK") << endl;
	t << "LFLAGS	=	" << var("QMAKE_LFLAGS");
	if ( !project->variables()["QMAKE_LIBDIR"].isEmpty() )
	  t << " " << varGlue("QMAKE_LIBDIR","/LIBPATH:\"","\" /LIBPATH:\"","\"");
	t << endl;
	t << "LIBS	=	";
	QStringList &libs = project->variables()["QMAKE_LIBS"];
	for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit) {
	    QString lib = (*libit);
	    lib.replace(QRegExp("\\\\$"), "\\\\");
	    lib.replace(QRegExp("\""), "");
	    t << " \"" << lib << "\"";
	}
	t << endl;
    }
    else {
	t << "LIB	=	" << var("QMAKE_LIB") << endl;
    }
    t << "MOC		=	" << (project->isEmpty("QMAKE_MOC") ? QString("moc") :
			      Option::fixPathToTargetOS(var("QMAKE_MOC"), FALSE)) << endl;
    t << "UIC		=	" << (project->isEmpty("QMAKE_UIC") ? QString("uic") :
			      Option::fixPathToTargetOS(var("QMAKE_UIC"), FALSE)) << endl;
    t << "QMAKE		=	" << (project->isEmpty("QMAKE_QMAKE") ? QString("qmake") :
			      Option::fixPathToTargetOS(var("QMAKE_QMAKE"), FALSE)) << endl;
    t << "IDC		=	" << (project->isEmpty("QMAKE_IDC") ? QString("idc") :
			      Option::fixPathToTargetOS(var("QMAKE_IDC"), FALSE)) << endl;
    t << "IDL		=	" << (project->isEmpty("QMAKE_IDL") ? QString("midl") :
			      Option::fixPathToTargetOS(var("QMAKE_IDL"), FALSE)) << endl;
    t << "ZIP		=	" << var("QMAKE_ZIP") << endl;
    t << "COPY		=       " << var("QMAKE_COPY") << endl;
    t << "COPY_FILE	=       " << var("QMAKE_COPY_FILE") << endl;
    t << "COPY_DIR	=       " << var("QMAKE_COPY_DIR") << endl;
    t << "DEL_FILE	=       " << var("QMAKE_DEL_FILE") << endl;
    t << "DEL_DIR	=       " << var("QMAKE_DEL_DIR") << endl;
    t << "MOVE		=       " << var("QMAKE_MOVE") << endl;
    t << "CHK_DIR_EXISTS =	" << var("QMAKE_CHK_DIR_EXISTS") << endl;
    t << "MKDIR		=	" << var("QMAKE_MKDIR") << endl;
    t << endl;

    t << "####### Files" << endl << endl;
    t << "TRANSLATABLES = " << varList("TRANSLATABLES") << endl;
    t << "HEADERS =	" << varList("HEADERS") << endl;
    t << "SOURCES =	" << varList("SOURCES") << endl;
    t << "OBJECTS =	" << varList("OBJECTS") << endl;
    t << "FORMS =	" << varList("FORMS") << endl;
    t << "UICDECLS =	" << varList("UICDECLS") << endl;
    t << "UICIMPLS =	" << varList("UICIMPLS") << endl;
    t << "SRCMOC	=	" << varList("SRCMOC") << endl;
    t << "OBJMOC	=	" << varList("OBJMOC") << endl;
    t << "DIST	=	" << varList("DISTFILES") << endl;
    t << "TARGET	=	";
    if( !project->variables()[ "DESTDIR" ].isEmpty() )
	t << varGlue("TARGET",project->first("DESTDIR"),"",project->first("TARGET_EXT"));
    else
	t << project->variables()[ "TARGET" ].first() << project->variables()[ "TARGET_EXT" ].first();
    t << endl;
    t << "INSTALL_TARGET	= ";
    t << project->variables()[ "TARGET" ].first() << project->variables()[ "TARGET_EXT" ].first();
    t << endl;

    t << "####### Implicit rules" << endl << endl;
    t << ".SUFFIXES: .c";
    QStringList::Iterator cppit;
    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	t << " " << (*cppit);
    t << endl << endl;
    if(!project->isActiveConfig("no_batch")) {
	// Batchmode doesn't use the non implicit rules QMAKE_RUN_CXX & QMAKE_RUN_CC
	project->variables().remove("QMAKE_RUN_CXX");
	project->variables().remove("QMAKE_RUN_CC");

	QDict<void> source_directories_cache;
	QStringList source_directories;
	if(!project->isEmpty("MOC_DIR")) {
	    source_directories << project->first("MOC_DIR");
	    source_directories_cache.insert(project->first("MOC_DIR"), (void*)1);
        }
	if(!project->isEmpty("UI_SOURCES_DIR")) {
	    source_directories << project->first("UI_SOURCES_DIR");
	    source_directories_cache.insert(project->first("UI_SOURCES_DIR"), (void*)1);
	} else if(!project->isEmpty("UI_DIR")) {
	    source_directories << project->first("UI_DIR");
	    source_directories_cache.insert(project->first("UI_DIR"), (void*)1);
        }
	QString srcs[] = { QString("SOURCES"), QString("UICIMPLS"), QString("SRCMOC"), QString::null };
	for(int x = 0; !srcs[x].isNull(); x++) {
	    QStringList &l = project->variables()[srcs[x]];
	    for(QStringList::Iterator sit = l.begin(); sit != l.end(); ++sit) {
		QString sep = "\\";
		if((*sit).find(sep) == -1)
		    sep = "/";
		QString dir = (*sit).section(sep, 0, -2);
		if(dir != "." && !dir.isEmpty() && !source_directories_cache[dir]){
		    source_directories << dir;
		    source_directories_cache.insert(dir, (void*)1);
		}
	    }
	}
	source_directories << "."; // put dot rules last

	for(QStringList::Iterator it = source_directories.begin();
            it != source_directories.end(); ++it) {
	    if((*it).isEmpty())
		continue;
	    for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
		t << "{" << (*it) << "}" << (*cppit) << "{" << var("OBJECTS_DIR") << "}" << Option::obj_ext << "::\n\t"
		  << var("QMAKE_RUN_CXX_IMP_BATCH").replace( QRegExp( "\\$@" ), var("OBJECTS_DIR") ) << endl << "\t$<" << endl << "<<" << endl << endl;
	    t << "{" << (*it) << "}" << ".c{" << var("OBJECTS_DIR") << "}" << Option::obj_ext << "::\n\t"
	      << var("QMAKE_RUN_CC_IMP_BATCH").replace( QRegExp( "\\$@" ), var("OBJECTS_DIR") ) << endl << "\t$<" << endl << "<<" << endl << endl;
	}
    } else {
	for(cppit = Option::cpp_ext.begin(); cppit != Option::cpp_ext.end(); ++cppit)
	    t << (*cppit) << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CXX_IMP") << endl << endl;
	t << ".c" << Option::obj_ext << ":\n\t" << var("QMAKE_RUN_CC_IMP") << endl << endl;
    }

    t << "####### Build rules" << endl << endl;
    t << "all: " << fileFixify(Option::output.name()) << " " << varGlue("ALL_DEPS"," "," "," ") << "$(TARGET)" << endl << endl;
    t << "$(TARGET): " << var("PRE_TARGETDEPS") << " $(UICDECLS) $(OBJECTS) $(OBJMOC) "
      << var("POST_TARGETDEPS");
    if(!project->variables()["QMAKE_APP_OR_DLL"].isEmpty()) {
	t << "\n\t" << "$(LINK) $(LFLAGS) /OUT:$(TARGET) @<< " << "\n\t  "
	  << "$(OBJECTS) $(OBJMOC) $(LIBS)";
    } else {
	t << "\n\t" << "$(LIB) /OUT:$(TARGET) @<<" << "\n\t  "
	  << "$(OBJECTS) $(OBJMOC)";
    }
    t << endl << "<<" << endl;
    if ( !project->variables()["QMAKE_POST_LINK"].isEmpty() )
	t << "\t" << var( "QMAKE_POST_LINK" ) << endl;
    if(project->isActiveConfig("dll") && !project->variables()["DLLDESTDIR"].isEmpty()) {
	QStringList dlldirs = project->variables()["DLLDESTDIR"];
	for ( QStringList::Iterator dlldir = dlldirs.begin(); dlldir != dlldirs.end(); ++dlldir ) {
	    t << "\n\t" << "-$(COPY_FILE) $(TARGET) " << *dlldir;
	}
    }
    QString targetfilename = project->variables()["TARGET"].first();
    if(project->isActiveConfig("activeqt")) {
	QString version = project->variables()["VERSION"].first();
	if ( version.isEmpty() )
	    version = "1.0";

	if ( project->isActiveConfig("dll")) {
	    t << "\n\t" << ("-$(IDC) $(TARGET) /idl tmp\\" + targetfilename + ".idl -version " + version);
	    t << "\n\t" << ("-$(IDL) tmp\\" + targetfilename + ".idl /nologo /o tmp\\" + targetfilename + ".midl /tlb tmp\\" + targetfilename + ".tlb /iid tmp\\dump.midl /dlldata tmp\\dump.midl /cstub tmp\\dump.midl /header tmp\\dump.midl /proxy tmp\\dump.midl /sstub tmp\\dump.midl");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /tlb tmp\\" + targetfilename + ".tlb");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /regserver" );
	} else {
	    t << "\n\t" << ("-$(TARGET) -dumpidl tmp\\" + targetfilename + ".idl -version " + version);
	    t << "\n\t" << ("-$(IDL) tmp\\" + targetfilename + ".idl /nologo /o tmp\\" + targetfilename + ".midl /tlb tmp\\" + targetfilename + ".tlb /iid tmp\\dump.midl /dlldata tmp\\dump.midl /cstub tmp\\dump.midl /header tmp\\dump.midl /proxy tmp\\dump.midl /sstub tmp\\dump.midl");
	    t << "\n\t" << ("-$(IDC) $(TARGET) /tlb tmp\\" + targetfilename + ".tlb");
	    t << "\n\t" << "-$(TARGET) -regserver";
	}
    }
    t << endl << endl;

    if(!project->variables()["RC_FILE"].isEmpty()) {
	t << var("RES_FILE") << ": " << var("RC_FILE") << "\n\t"
	  << var("QMAKE_RC") << " " << var("RC_FILE") << endl << endl;
    }

    t << "mocables: $(SRCMOC)" << endl
      << "uicables: $(UICIMPLS) $(UICDECLS)" << endl << endl;

    writeMakeQmake(t);

    QStringList dist_files = Option::mkfile::project_files;
    if(!project->isEmpty("QMAKE_INTERNAL_INCLUDED_FILES"))
	dist_files += project->variables()["QMAKE_INTERNAL_INCLUDED_FILES"];
    if(!project->isEmpty("TRANSLATIONS"))
	dist_files << var("TRANSLATIONS");
    if(!project->isEmpty("FORMS")) {
	QStringList &forms = project->variables()["FORMS"];
	for(QStringList::Iterator formit = forms.begin(); formit != forms.end(); ++formit) {
	    QString ui_h = fileFixify((*formit) + Option::h_ext.first());
	    if(QFile::exists(ui_h) )
		dist_files << ui_h;
	}
    }
    t << "dist:" << "\n\t"
      << "$(ZIP) " << var("QMAKE_ORIG_TARGET") << ".zip " << "$(SOURCES) $(HEADERS) $(DIST) $(FORMS) "
      << dist_files.join(" ") << " " << var("TRANSLATIONS") << " " << var("IMAGES") << endl << endl;

    t << "uiclean:"
      << varGlue("UICDECLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("UICIMPLS" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","") << endl;

    t << "mocclean:"
      << varGlue("SRCMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("OBJMOC" ,"\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","") << endl;

    t << "clean: uiclean mocclean"
      << varGlue("OBJECTS","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","")
      << varGlue("QMAKE_CLEAN","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n")
      << varGlue("CLEAN_FILES","\n\t-$(DEL_FILE) ","\n\t-$(DEL_FILE) ","\n");
    if ( project->isActiveConfig("activeqt")) {
	t << ("\n\t-$(DEL_FILE) tmp\\" + targetfilename + ".*");
	t << "\n\t-$(DEL_FILE) tmp\\dump.*";
    }
    if(!project->isEmpty("IMAGES"))
	t << varGlue("QMAKE_IMAGE_COLLECTION", "\n\t-$(DEL_FILE) ", "\n\t-$(DEL_FILE) ", "");
    t << endl;

    // user defined targets
    QStringList &qut = project->variables()["QMAKE_EXTRA_WIN_TARGETS"];
    for(QStringList::Iterator it = qut.begin(); it != qut.end(); ++it) {
	QString targ = var((*it) + ".target"),
		 cmd = var((*it) + ".commands"), deps;
	if(targ.isEmpty())
	    targ = (*it);
	QStringList &deplist = project->variables()[(*it) + ".depends"];
	for(QStringList::Iterator dep_it = deplist.begin(); dep_it != deplist.end(); ++dep_it) {
	    QString dep = var((*dep_it) + ".target");
	    if(dep.isEmpty())
		dep = (*dep_it);
	    deps += " " + dep;
	}
	if(!project->variables()["QMAKE_NOFORCE"].isEmpty() && 
	   project->variables()[(*it) + ".CONFIG"].findIndex("phony") != -1)
	    deps += QString(" ") + "FORCE";
	t << "\n\n" << targ << ":" << deps << "\n\t"
	  << cmd;
    }
    t << endl << endl;

    if(project->variables()["QMAKE_NOFORCE"].isEmpty())
	t << "FORCE:" << endl << endl;

    t << "distclean: clean"
      << "\n\t-$(DEL_FILE) $(TARGET)"
      << endl << endl;
}


void
NmakeMakefileGenerator::init()
{
    if(init_flag)
	return;
    init_flag = TRUE;

    /* this should probably not be here, but I'm using it to wrap the .t files */
    if(project->first("TEMPLATE") == "app")
	project->variables()["QMAKE_APP_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "lib")
	project->variables()["QMAKE_LIB_FLAG"].append("1");
    else if(project->first("TEMPLATE") == "subdirs") {
	MakefileGenerator::init();
	if(project->variables()["MAKEFILE"].isEmpty())
	    project->variables()["MAKEFILE"].append("Makefile");
	if(project->variables()["QMAKE"].isEmpty())
	    project->variables()["QMAKE"].append("qmake");
	return;
    }

    if( project->isEmpty("QMAKE_COPY_FILE") )
	project->variables()["QMAKE_COPY_FILE"].append( "$(COPY)" );
    if( project->isEmpty("QMAKE_COPY_DIR") )
	project->variables()["QMAKE_COPY_DIR"].append( "$(COPY)" );

    bool is_qt = (project->first("TARGET") == "qt"QTDLL_POSTFIX || project->first("TARGET") == "qt-mt"QTDLL_POSTFIX);
    project->variables()["QMAKE_ORIG_TARGET"] = project->variables()["TARGET"];

    QString targetfilename = project->variables()["TARGET"].first();
    QStringList &configs = project->variables()["CONFIG"];
    if (project->isActiveConfig("qt") && project->isActiveConfig("shared"))
	project->variables()["DEFINES"].append("QT_DLL");
    if (project->isActiveConfig("qt_dll"))
	if(configs.findIndex("qt") == -1) configs.append("qt");
    if ( project->isActiveConfig("qtopia") ) {
	if(configs.findIndex("qtopialib") == -1)
	    configs.append("qtopialib");
	if(configs.findIndex("qtopiainc") == -1)
	    configs.append("qtopiainc");
    }
    if ( project->isActiveConfig("qt") ) {
	if ( project->isActiveConfig( "plugin" ) ) {
	    project->variables()["CONFIG"].append("dll");
	    if(project->isActiveConfig("qt"))
		project->variables()["DEFINES"].append("QT_PLUGIN");
	}
	if ( (project->variables()["DEFINES"].findIndex("QT_NODLL") == -1) &&
         ((project->variables()["DEFINES"].findIndex("QT_MAKEDLL") != -1 ||
           project->variables()["DEFINES"].findIndex("QT_DLL") != -1) ||
          (getenv("QT_DLL") && !getenv("QT_NODLL"))) ) {
	    project->variables()["QMAKE_QT_DLL"].append("1");
	    if ( is_qt && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() )
		project->variables()["CONFIG"].append("dll");
	}
	if ( project->isActiveConfig("thread") )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_THREAD_SUPPORT");
	if ( project->isActiveConfig("accessibility" ) )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_ACCESSIBILITY_SUPPORT");
	if ( project->isActiveConfig("tablet") )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_TABLET_SUPPORT");
    }
    if ( project->isActiveConfig("dll") || !project->variables()["QMAKE_APP_FLAG"].isEmpty() ) {
	project->variables()["CONFIG"].remove("staticlib");
	project->variables()["QMAKE_APP_OR_DLL"].append("1");
    } else {
	project->variables()["CONFIG"].append("staticlib");
    }
    if ( project->isActiveConfig("warn_off") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_OFF"];
    } else if ( project->isActiveConfig("warn_on") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_WARN_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_WARN_ON"];
    }
    if ( project->isActiveConfig("debug") ) {
        if ( project->isActiveConfig("thread") ) {
	    // use the DLL RT even here
	    if ( project->variables()["DEFINES"].contains("QT_DLL") ) {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DLLDBG"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DLLDBG"];
	    } else {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DBG"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DBG"];
	    }
	}
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_DEBUG"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_DEBUG"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_DEBUG"];
    } else {
	if ( project->isActiveConfig("thread") ) {
	    if ( project->variables()["DEFINES"].contains("QT_DLL") ) {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT_DLL"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT_DLL"];
	    } else {
		project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_MT"];
		project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_MT"];
	    }
	}
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RELEASE"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RELEASE"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_RELEASE"];
    }
    if ( project->isActiveConfig("thread") && !project->variables()["DEFINES"].contains("QT_DLL")
	&& !is_qt && project->first("TARGET") != "qtmain") {
	project->variables()["QMAKE_LFLAGS"].append("/NODEFAULTLIB:\"libc\"");
    }

    if ( !project->variables()["QMAKE_INCDIR"].isEmpty())
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR"];
    if ( project->isActiveConfig("qt") || project->isActiveConfig("opengl") )
	project->variables()["CONFIG"].append("windows");
    if ( project->isActiveConfig("qtopiainc") )
	project->variables()["INCLUDEPATH"] += project->variables()["QMAKE_INCDIR_QTOPIA"];
    if ( project->isActiveConfig("qtopialib") ) {
	if(!project->isEmpty("QMAKE_LIBDIR_QTOPIA"))
	    project->variables()["QMAKE_LIBDIR"] += project->variables()["QMAKE_LIBDIR_QTOPIA"];
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QTOPIA"];
    }
    if ( project->isActiveConfig("qt") ) {
	project->variables()["CONFIG"].append("moc");
	project->variables()["INCLUDEPATH"] +=	project->variables()["QMAKE_INCDIR_QT"];
	project->variables()["QMAKE_LIBDIR"] += project->variables()["QMAKE_LIBDIR_QT"];
	if ( !project->isActiveConfig("debug") )
	    project->variables()[is_qt ? "PRL_EXPORT_DEFINES" : "DEFINES"].append("QT_NO_DEBUG");
	if ( is_qt && !project->variables()["QMAKE_LIB_FLAG"].isEmpty() ) {
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty()) {
		project->variables()["DEFINES"].append("QT_MAKEDLL");
		project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_QT_DLL"];
	    }
	} else {
	    if(project->isActiveConfig("thread"))
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_THREAD"];
	    else
		project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT"];
	    if ( !project->variables()["QMAKE_QT_DLL"].isEmpty() ) {
		int hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt");
		if ( hver == -1 )
		    hver = findHighestVersion(project->first("QMAKE_LIBDIR_QT"), "qt-mt");
		if(hver != -1) {
		    QString ver;
		    ver.sprintf("qt%s" QTDLL_POSTFIX "%d.lib", (project->isActiveConfig("thread") ? "-mt" : ""), hver);
		    QStringList &libs = project->variables()["QMAKE_LIBS"];
		    for(QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit)
			(*libit).replace(QRegExp("qt(-mt)?\\.lib"), ver);
		}
	    }
	    if ( project->isActiveConfig( "activeqt" ) ) {
		project->variables().remove("QMAKE_LIBS_QT_ENTRY");
		project->variables()["QMAKE_LIBS_QT_ENTRY"] = "qaxserver.lib";
		if ( project->isActiveConfig( "dll" ) )
		    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_QT_ENTRY"];
	    }
	    if ( !project->isActiveConfig("dll") && !project->isActiveConfig("plugin") ) {
		project->variables()["QMAKE_LIBS"] +=project->variables()["QMAKE_LIBS_QT_ENTRY"];
	    }
	}
    }
    if ( project->isActiveConfig("opengl") ) {
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_OPENGL"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_OPENGL"];
    }
    if ( project->isActiveConfig("dll") ) {
	project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CXXFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_LFLAGS_CONSOLE_DLL"];
	project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["QMAKE_LFLAGS_WINDOWS_DLL"];
	if ( !project->variables()["QMAKE_LIB_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(
		QStringList::split('.',project->first("VERSION")).join("") + ".dll");
	} else {
	    project->variables()["TARGET_EXT"].append(".dll");
	}
    } else {
	project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CFLAGS_CONSOLE"];
	project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_CXXFLAGS_CONSOLE"];
	project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"] = project->variables()["QMAKE_LFLAGS_CONSOLE"];
	project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"] = project->variables()["QMAKE_LFLAGS_WINDOWS"];
	if ( !project->variables()["QMAKE_APP_FLAG"].isEmpty()) {
	    project->variables()["TARGET_EXT"].append(".exe");
	} else {
	    project->variables()["TARGET_EXT"].append(".lib");
	}
    }
    if ( project->isActiveConfig("windows") ) {
	if ( project->isActiveConfig("console") ) {
	    project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"];
	    project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_CONSOLE"];
	} else {
	    project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_WINDOWS_ANY"];
	}
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_WINDOWS"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_LFLAGS"] += project->variables()["QMAKE_LFLAGS_CONSOLE_ANY"];
	project->variables()["QMAKE_LIBS"] += project->variables()["QMAKE_LIBS_CONSOLE"];
    }
    if ( project->isActiveConfig("stl") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_STL_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_STL_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_STL_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_STL_OFF"];
    }
    if ( project->isActiveConfig("exceptions") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_EXCEPTIONS_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_EXCEPTIONS_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_EXCEPTIONS_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_EXCEPTIONS_OFF"];
    }
    if ( project->isActiveConfig("rtti") ) {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_ON"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_ON"];
    } else {
	project->variables()["QMAKE_CFLAGS"] += project->variables()["QMAKE_CFLAGS_RTTI_OFF"];
	project->variables()["QMAKE_CXXFLAGS"] += project->variables()["QMAKE_CXXFLAGS_RTTI_OFF"];
    }


    if ( project->isActiveConfig("moc") )
	setMocAware(TRUE);
    project->variables()["QMAKE_LIBS"] += project->variables()["LIBS"];

    QStringList &libs = project->variables()["QMAKE_LIBS"];
    for ( QStringList::Iterator libit = libs.begin(); libit != libs.end(); ++libit ) {
	if (  (*libit).startsWith( "-l" ) ) {
	    (*libit) = (*libit).mid( 2 ) + ".lib";
	} else if ( (*libit).startsWith( "-L" ) ) {
	    project->variables()["QMAKE_LIBDIR"] += (*libit).mid(2);
	    libit = libs.remove( libit );
	}
    }

    project->variables()["QMAKE_FILETAGS"] += QStringList::split(' ',
	"HEADERS SOURCES DEF_FILE RC_FILE TARGET QMAKE_LIBS DESTDIR DLLDESTDIR INCLUDEPATH");
    QStringList &l = project->variables()["QMAKE_FILETAGS"];
    for(QStringList::Iterator it = l.begin(); it != l.end(); ++it) {
	QStringList &gdmf = project->variables()[(*it)];
	for(QStringList::Iterator inner = gdmf.begin(); inner != gdmf.end(); ++inner)
	    (*inner) = Option::fixPathToTargetOS((*inner), FALSE);
    }

    if ( !project->variables()["DEF_FILE"].isEmpty() )
	project->variables()["QMAKE_LFLAGS"].append(QString("/DEF:") + project->first("DEF_FILE"));
    if(!project->isActiveConfig("incremental"))
	project->variables()["QMAKE_LFLAGS"].append(QString("/incremental:no"));

    if ( !project->variables()["VERSION"].isEmpty() ) {
	QString version = project->variables()["VERSION"][0];
	int firstDot = version.find( "." );
	QString major = version.left( firstDot );
	QString minor = version.right( version.length() - firstDot - 1 );
	minor.replace( ".", "" );
	project->variables()["QMAKE_LFLAGS"].append( "/VERSION:" + major + "." + minor );
    }
    if ( !project->variables()["RC_FILE"].isEmpty()) {
	if ( !project->variables()["RES_FILE"].isEmpty()) {
	    fprintf(stderr, "Both .rc and .res file specified.\n");
	    fprintf(stderr, "Please specify one of them, not both.");
	    exit(666);
	}
	project->variables()["RES_FILE"] = project->variables()["RC_FILE"];
	project->variables()["RES_FILE"].first().replace(".rc",".res");
	project->variables()["POST_TARGETDEPS"] += project->variables()["RES_FILE"];
    }
    if ( !project->variables()["RES_FILE"].isEmpty())
	project->variables()["QMAKE_LIBS"] += project->variables()["RES_FILE"];

    MakefileGenerator::init();
    if ( !project->variables()["VERSION"].isEmpty()) {
	QStringList l = QStringList::split('.', project->first("VERSION"));
	project->variables()["VER_MAJ"].append(l[0]);
	project->variables()["VER_MIN"].append(l[1]);
    }

    QString version = QStringList::split('.', project->first("VERSION")).join("");
    if(project->isActiveConfig("dll")) {
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".exp");
    }
    if(project->isActiveConfig("debug")) {
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".pdb");
	project->variables()["QMAKE_CLEAN"].append(project->first("DESTDIR") + project->first("TARGET") + version + ".ilk");
	project->variables()["QMAKE_CLEAN"].append("vc*.pdb");
    }
}

QString
NmakeMakefileGenerator::defaultInstall(const QString &t)
{
    if(t != "target" || project->first("TEMPLATE") == "subdirs")
	return QString();

    bool resource = FALSE;
    const QString root = "$(INSTALL_ROOT)";
    QStringList &uninst = project->variables()[t + ".uninstall"];
    QString ret, destdir=project->first("DESTDIR");
    QString targetdir = Option::fixPathToTargetOS(project->first("target.path"), FALSE);
    if(!destdir.isEmpty() && destdir.right(1) != Option::dir_sep)
	destdir += Option::dir_sep;
    targetdir = fileFixify(targetdir);
    if(targetdir.right(1) != Option::dir_sep)
	targetdir += Option::dir_sep;

    QStringList links;
    QString target="$(INSTALL_TARGET)";
    if(project->first("TEMPLATE") == "lib") {
	if(project->isActiveConfig("create_libtool") && !project->isActiveConfig("compile_libtool")) {
	    QString src_lt = var("QMAKE_ORIG_TARGET");
	    int slsh = src_lt.findRev(Option::dir_sep);
	    if(slsh != -1)
		src_lt = src_lt.right(src_lt.length() - slsh);
	    int dot = src_lt.find('.');
	    if(dot != -1)
		src_lt = src_lt.left(dot);
	    src_lt += Option::libtool_ext;
	    src_lt.prepend("lib");
	    QString dst_lt = root + targetdir + src_lt;
	    if(!project->isEmpty("DESTDIR")) {
		src_lt.prepend(var("DESTDIR"));
		src_lt = Option::fixPathToLocalOS(fileFixify(src_lt,
							     QDir::currentDirPath(), Option::output_dir));
	    }
	    if(!ret.isEmpty())
		ret += "\n\t";
	    ret += "-$(COPY_FILE) \"" + src_lt + "\" \"" + dst_lt + "\"";
	    if(!uninst.isEmpty())
		uninst.append("\n\t");
	    uninst.append("-$(DEL_FILE) \"" + dst_lt + "\"");
	}
	if(project->isActiveConfig("create_pc")) {
	    QString src_pc = var("QMAKE_ORIG_TARGET");
	    int slsh = src_pc.findRev(Option::dir_sep);
	    if(slsh != -1)
		src_pc = src_pc.right(src_pc.length() - slsh);
	    int dot = src_pc.find('.');
	    if(dot != -1)
		src_pc = src_pc.left(dot);
	    src_pc += ".pc";
	    QString d = root + targetdir + "pkgconfig" + Option::dir_sep;
	    QString dst_pc = d + src_pc;
	    if(!project->isEmpty("DESTDIR")) {
		src_pc.prepend(var("DESTDIR"));
		src_pc = Option::fixPathToLocalOS(fileFixify(src_pc,
							     QDir::currentDirPath(), Option::output_dir));
	    }
	    if(!ret.isEmpty())
		ret += "\n\t";
	    ret += mkdir_p_asstring(d) + "\n\t";
	    ret += "-$(COPY_FILE) \"" + src_pc + "\" \"" + dst_pc + "\"";
	    if(!uninst.isEmpty())
		uninst.append("\n\t");
	    uninst.append("-$(DEL_FILE) \"" + dst_pc + "\"");
	}
	if ( project->isEmpty("QMAKE_CYGWIN_SHLIB") ) {
	    if ( !project->isActiveConfig("staticlib") && !project->isActiveConfig("plugin") ) {
		if ( project->isEmpty("QMAKE_HPUX_SHLIB") ) {
		    links << "$(TARGET0)" << "$(TARGET1)" << "$(TARGET2)";
		} else {
		    links << "$(TARGET0)";
	        }
	    }
	}
    }

    if(!resource && project->isActiveConfig("compile_libtool")) {
	QString src_targ = target;
	if(src_targ == "$(TARGET)")
	    src_targ = "$(TARGETL)";
	QString dst_dir = fileFixify(targetdir);
	if(QDir::isRelativePath(dst_dir))
	    dst_dir = Option::fixPathToTargetOS(Option::output_dir + Option::dir_sep + dst_dir);
	ret = "-$(LIBTOOL) --mode=install cp \"" + src_targ + "\" \"" + root + dst_dir + "\"";
	uninst.append("-$(LIBTOOL) --mode=uninstall \"" + src_targ + "\"");
    } else {
	QString src_targ = target;
	if(!destdir.isEmpty())
	    src_targ = Option::fixPathToTargetOS(destdir + target, FALSE);
	QString dst_targ = root + fileFixify(targetdir + target, 
					     QDir::currentDirPath(), Option::output_dir);
	if(!ret.isEmpty())
	    ret += "\n\t";
	if(resource)
	    ret += "$(DEL_FILE) -r \"" + dst_targ + "\"" + "\n\t";
	if(!ret.isEmpty())
	    ret += "\n\t";
	ret += QString(resource ? "-$(COPY_DIR)" : "-$(COPY_FILE)") + " \"" +
	       src_targ + "\" \"" + dst_targ + "\"";
	if(!project->isActiveConfig("debug") && !project->isEmpty("QMAKE_STRIP") &&
	   (project->first("TEMPLATE") != "lib" || !project->isActiveConfig("staticlib"))) {
	    ret += "\n\t-" + var("QMAKE_STRIP");
	    if(project->first("TEMPLATE") == "lib" && !project->isEmpty("QMAKE_STRIPFLAGS_LIB"))
		ret += " " + var("QMAKE_STRIPFLAGS_LIB");
	    else if(project->first("TEMPLATE") == "app" && !project->isEmpty("QMAKE_STRIPFLAGS_APP"))
		ret += " " + var("QMAKE_STRIPFLAGS_APP");
	    if(resource)
		ret = " \"" + dst_targ + "/Contents/MacOS/$(QMAKE_TARGET)\"";
	    else
		ret += " \"" + dst_targ + "\"";
	}
	if(!uninst.isEmpty())
	    uninst.append("\n\t");
	if(resource)
	    uninst.append("-$(DEL_FILE) -r \"" + dst_targ + "\"");
	else
	    uninst.append("-$(DEL_FILE) \"" + dst_targ + "\"");
	if(!links.isEmpty()) {
	    for(QStringList::Iterator it = links.begin(); it != links.end(); it++) {
		if(Option::target_mode == Option::TARG_WIN_MODE ||
		   Option::target_mode == Option::TARG_MAC9_MODE) {
		} else if(Option::target_mode == Option::TARG_UNIX_MODE ||
			  Option::target_mode == Option::TARG_MACX_MODE) {
		    QString link = Option::fixPathToTargetOS(destdir + (*it), FALSE);
		    int lslash = link.findRev(Option::dir_sep);
		    if(lslash != -1)
			link = link.right(link.length() - (lslash + 1));
		    QString dst_link = root + targetdir + link;
		    ret += "\n\t-$(SYMLINK) \"$(TARGET)\" \"" + dst_link + "\"";
		    if(!uninst.isEmpty())
			uninst.append("\n\t");
		    uninst.append("-$(DEL_FILE) \"" + dst_link + "\"");
		}
	    }
	}
    }
    return ret;
}

