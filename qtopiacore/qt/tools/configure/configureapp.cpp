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

#include "configureapp.h"
#include "environment.h"
#ifdef COMMERCIAL_VERSION
#  include "tools.h"
#endif

#include <QDate>
#include <qdir.h>
#include <qfileinfo.h>
#include <qtextstream.h>
#include <qregexp.h>
#include <qhash.h>

#include <iostream>
#include <windows.h>
#include <conio.h>

std::ostream &operator<<( std::ostream &s, const QString &val ) {
    s << val.toLocal8Bit().data();
    return s;
}


using namespace std;

// Macros to simplify options marking
#define MARK_OPTION(x,y) ( dictionary[ #x ] == #y ? "*" : " " )


bool writeToFile(const char* text, const QString &filename)
{
    QByteArray symFile(text);
    QFile file(filename);
    QDir dir(QFileInfo(file).absoluteDir());
    if (!dir.exists())
        dir.mkdir(dir.absolutePath());
    if (!file.open(QFile::WriteOnly)) {
        cout << "Couldn't write to " << qPrintable(filename) << ": " << qPrintable(file.errorString())
             << endl;
        return false;
    }
    file.write(symFile);
    return true;
}

Configure::Configure( int& argc, char** argv )
{
    // Default values for indentation
    optionIndent = 4;
    descIndent   = 25;
    outputWidth  = 0;
    // Get console buffer output width
    CONSOLE_SCREEN_BUFFER_INFO info;
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    if (GetConsoleScreenBufferInfo(hStdout, &info))
        outputWidth = info.dwSize.X - 1;
    outputWidth = qMin(outputWidth, 79); // Anything wider gets unreadable
    if (outputWidth < 35) // Insanely small, just use 79
        outputWidth = 79;
    int i;

    /*
    ** Set up the initial state, the default
    */
    dictionary[ "CONFIGCMD" ] = argv[ 0 ];

    for ( i = 1; i < argc; i++ )
	configCmdLine += argv[ i ];


    // Get the path to the executable
    QFileInfo sourcePath;
    QT_WA({
        unsigned short module_name[256];
        GetModuleFileNameW(0, reinterpret_cast<wchar_t *>(module_name), sizeof(module_name));
        sourcePath = QString::fromUtf16(module_name);
    }, {
        char module_name[256];
        GetModuleFileNameA(0, module_name, sizeof(module_name));
        sourcePath = QString::fromLocal8Bit(module_name);
    });
    qtDir = sourcePath.absolutePath();
    // Only use the lines below if configure resides in QTDIR/bin
    //QDir srcDir(sourcePath.absolutePath() + "/..");
    //qtDir = QDir::toNativeSeparators( srcDir.absolutePath() );
    QString installPath = QString("C:\\Qt\\%1").arg(QT_VERSION_STR);

    dictionary[ "QT_SOURCE_TREE" ]    = QDir::toNativeSeparators( qtDir );
    dictionary[ "QT_BUILD_TREE" ]     = QDir::toNativeSeparators( qtDir );
    dictionary[ "QT_INSTALL_PREFIX" ] = QDir::toNativeSeparators( qtDir ); //installPath;

    dictionary[ "QMAKESPEC" ] = getenv( "QMAKESPEC" );
    if (dictionary[ "QMAKESPEC" ].size() == 0) {
        dictionary[ "QMAKESPEC" ] = Environment::detectQMakeSpec();
        dictionary[ "QMAKESPEC_FROM" ] = "detected";
    } else {
        dictionary[ "QMAKESPEC_FROM" ] = "env";
    }


    dictionary[ "ARCHITECTURE" ]    = "windows";
    dictionary[ "QCONFIG" ]	    = "full";
    dictionary[ "EMBEDDED" ]	    = "no";
    dictionary[ "BUILD_QMAKE" ]	    = "yes";
    dictionary[ "DSPFILES" ]	    = "yes";
    dictionary[ "VCPROJFILES" ]	    = "yes";
    dictionary[ "QMAKE_INTERNAL" ]  = "no";
    dictionary[ "FAST" ]            = "no";
    dictionary[ "NOPROCESS" ]	    = "no";
    dictionary[ "STL" ]		    = "yes";
    dictionary[ "EXCEPTIONS" ]	    = "yes";
    dictionary[ "RTTI" ]	    = "yes";

    QString version;
    QFile profile(qtDir + "/src/qbase.pri");
    if (profile.open(QFile::ReadOnly)) {
	QTextStream read(&profile);
	QRegExp version_regexp("^\\s*VERSION=(.*)$");
	QString line;
	while (!read.atEnd()) {
	    line = read.readLine();
	    if (version_regexp.exactMatch(line)) {
		version = version_regexp.cap(1).trimmed();
		if (!version.isEmpty())
		    break;
	    }
	}
	profile.close();
    }

    if (version.isEmpty())
	version = QString("%1.%2.%3").arg(QT_VERSION>>16).arg(((QT_VERSION>>8)&0xff)).arg(QT_VERSION&0xff);

    dictionary[ "VERSION" ]	    = version;
    dictionary[ "REDO" ]	    = "no";
    dictionary[ "DEPENDENCIES" ]    = "no";

    dictionary[ "BUILD" ]	    = "debug";
    dictionary[ "BUILDALL" ]	    = "auto"; // Means yes, but not explicitly

    dictionary[ "SHARED" ]	    = "yes";

    dictionary[ "ZLIB" ]	    = "auto";

    dictionary[ "GIF" ]		    = "auto";
    dictionary[ "JPEG" ]	    = "auto";
    dictionary[ "PNG" ]		    = "auto";
    dictionary[ "MNG" ]		    = "auto";
    dictionary[ "LIBJPEG" ]	    = "auto";
    dictionary[ "LIBPNG" ]	    = "auto";
    dictionary[ "LIBMNG" ]	    = "auto";

    dictionary[ "QT3SUPPORT" ]	    = "yes";
    dictionary[ "ACCESSIBILITY" ]   = "yes";
    dictionary[ "OPENGL" ]	    = "yes";
    dictionary[ "IPV6" ]            = "yes"; // Always, dynamicly loaded

    dictionary[ "STYLE_WINDOWS" ]   = "yes";
    dictionary[ "STYLE_WINDOWSXP" ] = "auto";
    dictionary[ "STYLE_PLASTIQUE" ] = "yes";
    dictionary[ "STYLE_CLEANLOOKS" ] = "yes";
    dictionary[ "STYLE_POCKETPC" ]  = "no";
    dictionary[ "STYLE_MOTIF" ]     = "yes";
    dictionary[ "STYLE_CDE" ]	    = "yes";

    dictionary[ "SQL_MYSQL" ]	    = "no";
    dictionary[ "SQL_ODBC" ]	    = "no";
    dictionary[ "SQL_OCI" ]	    = "no";
    dictionary[ "SQL_PSQL" ]	    = "no";
    dictionary[ "SQL_TDS" ]	    = "no";
    dictionary[ "SQL_DB2" ]	    = "no";
    dictionary[ "SQL_SQLITE" ]	    = "yes";
    dictionary[ "SQL_SQLITE2" ]	    = "no";
    dictionary[ "SQL_IBASE" ]	    = "no";

    QString tmp = dictionary[ "QMAKESPEC" ];
    tmp = tmp.mid( tmp.lastIndexOf( "\\" ) + 1 );
    dictionary[ "QMAKESPEC" ] = tmp;
}

Configure::~Configure()
{
    for (int i=0; i<3; ++i) {
	QList<MakeItem*> items = makeList[i];
	for (int j=0; j<items.size(); ++j)
	    delete items[j];
    }
}

// We could use QDir::homePath() + "/.qt-license", but
// that will only look in the first of $HOME,$USERPROFILE
// or $HOMEDRIVE$HOMEPATH. So, here we try'em all to be
// more forgiving for the end user..
QString Configure::firstLicensePath()
{
    QStringList allPaths;
    allPaths << "./.qt-license"
             << QString::fromLocal8Bit(getenv("HOME")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("USERPROFILE")) + "/.qt-license"
             << QString::fromLocal8Bit(getenv("HOMEDRIVE")) + QString::fromLocal8Bit(getenv("HOMEPATH")) + "/.qt-license";
    for (int i = 0; i< allPaths.count(); ++i)
        if (QFile::exists(allPaths.at(i)))
            return allPaths.at(i);
    return QString();
}


// #### somehow I get a compiler error about vc++ reaching the nesting limit without
// undefining the ansi for scoping.
#ifdef for
#undef for
#endif

void Configure::parseCmdLine()
{
    int argCount = configCmdLine.size();
    int i = 0;
#if !defined(EVAL)
    if (argCount < 1) // skip rest if no arguments
	;
    else if( configCmdLine.at(i) == "-redo" ) {
	configCmdLine.clear();
	dictionary[ "REDO" ] = "yes";
	reloadCmdLine();
    }
    else if( configCmdLine.at(i) == "-loadconfig" ) {
	++i;
        if (i != argCount) {
            dictionary[ "REDO" ] = "yes";
	    dictionary[ "CUSTOMCONFIG" ] = "_" + configCmdLine.at(i);
	    configCmdLine.clear();
	    reloadCmdLine();
        } else {
            dictionary[ "HELP" ] = "yes";
        }
    }
#endif

    for( ; i<configCmdLine.size(); ++i ) {
	if( configCmdLine.at(i) == "-help"
            || configCmdLine.at(i) == "-h"
            || configCmdLine.at(i) == "-?" )
	    dictionary[ "HELP" ] = "yes";

#if !defined(EVAL)
	else if( configCmdLine.at(i) == "-qconfig" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "QCONFIG" ] = configCmdLine.at(i);
	}

        else if ( configCmdLine.at(i) == "-buildkey" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "USER_BUILD_KEY" ] = configCmdLine.at(i);
        }

	else if( configCmdLine.at(i) == "-release" ) {
	    dictionary[ "BUILD" ] = "release";
	    if (dictionary[ "BUILDALL" ] == "auto")
	        dictionary[ "BUILDALL" ] = "no";
	} else if( configCmdLine.at(i) == "-debug" ) {
            dictionary[ "BUILD" ] = "debug";
	    if (dictionary[ "BUILDALL" ] == "auto")
	        dictionary[ "BUILDALL" ] = "no";
	} else if( configCmdLine.at(i) == "-debug-and-release" )
	    dictionary[ "BUILDALL" ] = "yes";

	else if( configCmdLine.at(i) == "-shared" )
	    dictionary[ "SHARED" ] = "yes";
	else if( configCmdLine.at(i) == "-static" )
	    dictionary[ "SHARED" ] = "no";

#endif

	else if( configCmdLine.at(i) == "-platform" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "QMAKESPEC" ] = configCmdLine.at(i);
            dictionary[ "QMAKESPEC_FROM" ] = "commandline";
	} else if( configCmdLine.at(i) == "-arch" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "ARCHITECTURE" ] = configCmdLine.at(i);
	}

#if !defined(EVAL)
	else if( configCmdLine.at(i) == "-no-zlib" ) {
	    dictionary[ "ZLIB" ] = "no";
	    dictionary[ "PNG" ] = "no";
	    dictionary[ "LIBPNG" ] = "no";
	    dictionary[ "MNG" ] = "no";
	    dictionary[ "LIBMNG" ] = "no";
	} else if( configCmdLine.at(i) == "-qt-zlib" ) {
	    dictionary[ "ZLIB" ] = "qt";
	} else if( configCmdLine.at(i) == "-system-zlib" ) {
	    dictionary[ "ZLIB" ] = "system";
	}

        // Image formats --------------------------------------------
	else if( configCmdLine.at(i) == "-no-gif" )
	    dictionary[ "GIF" ] = "no";
	else if( configCmdLine.at(i) == "-qt-gif" )
	    dictionary[ "GIF" ] = "plugin";

        else if( configCmdLine.at(i) == "-no-libjpeg" ) {
	    dictionary[ "JPEG" ] = "no";
	    dictionary[ "LIBJPEG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libjpeg" ) {
	    dictionary[ "JPEG" ] = "plugin";
	    dictionary[ "LIBJPEG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libjpeg" ) {
	    dictionary[ "JPEG" ] = "plugin";
	    dictionary[ "LIBJPEG" ] = "system";
        }

        else if( configCmdLine.at(i) == "-no-libpng" ) {
	    dictionary[ "PNG" ] = "no";
	    dictionary[ "LIBPNG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libpng" ) {
	    dictionary[ "PNG" ] = "qt";
	    dictionary[ "LIBPNG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libpng" ) {
	    dictionary[ "PNG" ] = "qt";
	    dictionary[ "LIBPNG" ] = "system";
        }

        else if( configCmdLine.at(i) == "-no-libmng" ) {
	    dictionary[ "MNG" ] = "no";
	    dictionary[ "LIBMNG" ] = "no";
        } else if( configCmdLine.at(i) == "-qt-libmng" ) {
	    dictionary[ "MNG" ] = "qt";
	    dictionary[ "LIBMNG" ] = "qt";
        } else if( configCmdLine.at(i) == "-system-libmng" ) {
	    dictionary[ "MNG" ] = "qt";
	    dictionary[ "LIBMNG" ] = "system";
        }

        // Styles ---------------------------------------------------
	else if( configCmdLine.at(i) == "-qt-style-pocketpc" )
	    dictionary[ "STYLE_POCKETPC" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-pocketpc" )
	    dictionary[ "STYLE_POCKETPC" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-windows" )
	    dictionary[ "STYLE_WINDOWS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-windowsxp" )
	    dictionary[ "STYLE_WINDOWSXP" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-windowsxp" )
	    dictionary[ "STYLE_WINDOWSXP" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-plastique" )
	    dictionary[ "STYLE_PLASTIQUE" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-plastique" )
	    dictionary[ "STYLE_PLASTIQUE" ] = "no";

        else if( configCmdLine.at(i) == "-qt-style-cleanlooks" )
	    dictionary[ "STYLE_CLEANLOOKS" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-cleanlooks" )
	    dictionary[ "STYLE_CLEANLOOKS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-motif" )
	    dictionary[ "STYLE_MOTIF" ] = "no";

	else if( configCmdLine.at(i) == "-qt-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "yes";
	else if( configCmdLine.at(i) == "-no-style-cde" )
	    dictionary[ "STYLE_CDE" ] = "no";

        // Qt 3 Support ---------------------------------------------
        else if( configCmdLine.at(i) == "-no-qt3support" )
            dictionary[ "QT3SUPPORT" ] = "no";

        // Databases ------------------------------------------------
	else if( configCmdLine.at(i) == "-qt-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-mysql" )
	    dictionary[ "SQL_MYSQL" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-odbc" )
	    dictionary[ "SQL_ODBC" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-oci" )
	    dictionary[ "SQL_OCI" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-psql" )
	    dictionary[ "SQL_PSQL" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-tds" )
	    dictionary[ "SQL_TDS" ] = "no";

	else if( configCmdLine.at(i) == "-qt-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "yes";
	else if( configCmdLine.at(i) == "-plugin-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "plugin";
	else if( configCmdLine.at(i) == "-no-sql-db2" )
	    dictionary[ "SQL_DB2" ] = "no";

        else if( configCmdLine.at(i) == "-qt-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-sqlite" )
            dictionary[ "SQL_SQLITE" ] = "no";

        else if( configCmdLine.at(i) == "-qt-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-sqlite2" )
            dictionary[ "SQL_SQLITE2" ] = "no";

        else if( configCmdLine.at(i) == "-qt-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "yes";
        else if( configCmdLine.at(i) == "-plugin-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "plugin";
        else if( configCmdLine.at(i) == "-no-sql-ibase" )
            dictionary[ "SQL_IBASE" ] = "no";
#endif
        // IDE project generation -----------------------------------
	else if( configCmdLine.at(i) == "-no-dsp" )
	    dictionary[ "DSPFILES" ] = "no";
	else if( configCmdLine.at(i) == "-dsp" )
	    dictionary[ "DSPFILES" ] = "yes";

	else if( configCmdLine.at(i) == "-no-vcp" )
	    dictionary[ "VCPFILES" ] = "no";
	else if( configCmdLine.at(i) == "-vcp" )
	    dictionary[ "VCPFILES" ] = "yes";

	else if( configCmdLine.at(i) == "-no-vcproj" )
	    dictionary[ "VCPROJFILES" ] = "no";
	else if( configCmdLine.at(i) == "-vcproj" )
	    dictionary[ "VCPROJFILES" ] = "yes";

#if !defined(EVAL)
        // Others ---------------------------------------------------
        else if (configCmdLine.at(i) == "-fast" )
            dictionary[ "FAST" ] = "yes";
        else if (configCmdLine.at(i) == "-no-fast" )
            dictionary[ "FAST" ] = "no";

	else if( configCmdLine.at(i) == "-stl" )
	    dictionary[ "STL" ] = "yes";
	else if( configCmdLine.at(i) == "-no-stl" )
	    dictionary[ "STL" ] = "no";

	else if ( configCmdLine.at(i) == "-exceptions" )
	    dictionary[ "EXCEPTIONS" ] = "yes";
	else if ( configCmdLine.at(i) == "-no-exceptions" )
	    dictionary[ "EXCEPTIONS" ] = "no";

	else if ( configCmdLine.at(i) == "-rtti" )
	    dictionary[ "RTTI" ] = "yes";
	else if ( configCmdLine.at(i) == "-no-rtti" )
	    dictionary[ "RTTI" ] = "no";

	else if( configCmdLine.at(i) == "-accessibility" )
	    dictionary[ "ACCESSIBILITY" ] = "yes";
        else if( configCmdLine.at(i) == "-no-accessibility" ) {
	    dictionary[ "ACCESSIBILITY" ] = "no";
            cout << "Setting accessibility to NO" << endl;
        }

	else if( configCmdLine.at(i) == "-internal" )
	    dictionary[ "QMAKE_INTERNAL" ] = "yes";

	else if( configCmdLine.at(i) == "-no-qmake" )
	    dictionary[ "BUILD_QMAKE" ] = "no";
	else if( configCmdLine.at(i) == "-qmake" )
	    dictionary[ "BUILD_QMAKE" ] = "yes";

	else if( configCmdLine.at(i) == "-dont-process" )
	    dictionary[ "NOPROCESS" ] = "yes";
	else if( configCmdLine.at(i) == "-process" )
	    dictionary[ "NOPROCESS" ] = "no";

	else if( configCmdLine.at(i) == "-no-qmake-deps" )
	    dictionary[ "DEPENDENCIES" ] = "no";
	else if( configCmdLine.at(i) == "-qmake-deps" )
	    dictionary[ "DEPENDENCIES" ] = "yes";

	else if( configCmdLine.at(i) == "-D" ) {
	    ++i;
	    if (i==argCount)
		break;
            qmakeDefines += configCmdLine.at(i);
        } else if( configCmdLine.at(i) == "-I" ) {
	    ++i;
	    if (i==argCount)
		break;
	    qmakeIncludes += configCmdLine.at(i);
	} else if( configCmdLine.at(i) == "-L" ) {
	    ++i;
	    if (i==argCount)
		break;
            QFileInfo check(configCmdLine.at(i));
            if (!check.isDir()) {
                cout << "Argument passed to -L option is not a directory path. Did you mean the -l option?" << endl;
                dictionary[ "DONE" ] = "error";
                break;
            }
	    qmakeLibs += QString("-L" + configCmdLine.at(i));
	} else if( configCmdLine.at(i) == "-l" ) {
	    ++i;
	    if (i==argCount)
		break;
	    qmakeLibs += QString("-l" + configCmdLine.at(i));
	}

        else if( ( configCmdLine.at(i) == "-override-version" ) || ( configCmdLine.at(i) == "-version-override" ) ){
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "VERSION" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-saveconfig" ) {
	    ++i;
	    if (i==argCount)
		break;
	    dictionary[ "CUSTOMCONFIG" ] = "_" + configCmdLine.at(i);
	}

        else if (configCmdLine.at(i) == "-confirm-license") {
            dictionary["LICENSE_CONFIRMED"] = "yes";
        }

        // Directories ----------------------------------------------
	else if( configCmdLine.at(i) == "-prefix" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_PREFIX" ] = configCmdLine.at(i);
	}

        else if( configCmdLine.at(i) == "-bindir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_BINS" ] = configCmdLine.at(i);
	}

        else if( configCmdLine.at(i) == "-libdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_LIBS" ] = configCmdLine.at(i);
	}

        else if( configCmdLine.at(i) == "-docdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_DOCS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-headerdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_HEADERS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-plugindir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_PLUGINS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-datadir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_DATA" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-translationdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_TRANSLATIONS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-examplesdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_EXAMPLES" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-demosdir" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "QT_INSTALL_DEMOS" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i) == "-make" ) {
	    ++i;
	    if(i==argCount)
		break;
	    dictionary[ "MAKE" ] = configCmdLine.at(i);
	}

	else if( configCmdLine.at(i).indexOf( QRegExp( "^-(en|dis)able-" ) ) != -1 ) {
	    // Scan to see if any specific modules and drivers are enabled or disabled
	    for( QStringList::Iterator module = modules.begin(); module != modules.end(); ++module ) {
		if( configCmdLine.at(i) == QString( "-enable-" ) + (*module) ) {
		    enabledModules += (*module);
		    break;
		}
		else if( configCmdLine.at(i) == QString( "-disable-" ) + (*module) ) {
		    disabledModules += (*module);
		    break;
		}
	    }
	}

	else {
	    dictionary[ "HELP" ] = "yes";
	    cout << "Unknown option " << configCmdLine.at(i) << endl;
	    break;
	}

#endif
    }

    // Ensure that QMAKESPEC exists in the mkspecs folder
    QDir mkspec_dir = qtDir + "\\mkspecs";
    QStringList mkspecs = mkspec_dir.entryList(QDir::AllDirs | QDir::NoDotAndDotDot);

    if (dictionary["QMAKESPEC"].toLower() == "features"
        || !mkspecs.contains(dictionary["QMAKESPEC"], Qt::CaseInsensitive)) {
        dictionary[ "HELP" ] = "yes";
        if (dictionary ["QMAKESPEC_FROM"] == "commandline") {
            cout << "Invalid option \"" << dictionary["QMAKESPEC"] << "\" for -platform." << endl;
        } else if (dictionary ["QMAKESPEC_FROM"] == "env") {
            cout << "QMAKESPEC environment variable is set to \"" << dictionary["QMAKESPEC"]
                 << "\" which is not a supported platform" << endl;
        } else { // was autodetected from environment
            cout << "Unable to detect the platform from environment. Use -platform command line"
                    "argument or set the QMAKESPEC environment variable and run configure again" << endl;
        }
        cout << "See the README file for a list of supported operating systems and compilers." << endl;
    } else {
        if( dictionary[ "QMAKESPEC" ].endsWith( "-msvc" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( ".net" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( "-icc" ) ||
            dictionary[ "QMAKESPEC" ].endsWith( "-msvc2005" )) {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "nmake";
            dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32";
        } else if ( dictionary[ "QMAKESPEC" ] == QString( "win32-g++" ) ) {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "mingw32-make";
            dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32-g++";
        } else {
            if ( dictionary[ "MAKE" ].isEmpty() ) dictionary[ "MAKE" ] = "make";
            dictionary[ "QMAKEMAKEFILE" ] = "Makefile.win32";
        }
    }

#if !defined(EVAL)
    for( QStringList::Iterator dis = disabledModules.begin(); dis != disabledModules.end(); ++dis ) {
	modules.removeAll( (*dis) );
    }
    for( QStringList::Iterator ena = enabledModules.begin(); ena != enabledModules.end(); ++ena ) {
	if( modules.indexOf( (*ena) ) == -1 )
	    modules += (*ena);
    }
    qtConfig += modules;

    for( QStringList::Iterator it = disabledModules.begin(); it != disabledModules.end(); ++it )
	qtConfig.removeAll(*it);

    if( ( dictionary[ "REDO" ] != "yes" ) && ( dictionary[ "HELP" ] != "yes" ) )
	saveCmdLine();
#endif
}

#if !defined(EVAL)
void Configure::validateArgs()
{
    QStringList configs;
    // Validate the specified config

    allConfigs = QStringList() << "minimal" << "small" <<  "medium" << "large" << "full";

    QStringList::Iterator config;
    for( config = allConfigs.begin(); config != allConfigs.end(); ++config ) {
	configs += (*config) + "-config";
	if( (*config) == dictionary[ "QCONFIG" ] )
	    break;
    }
    if( config == allConfigs.end() ) {
	dictionary[ "HELP" ] = "yes";
	cout << "No such configuration \"" << qPrintable(dictionary[ "QCONFIG" ]) << "\"" << endl ;
    }
    else
	qmakeConfig += configs;
}
#endif


// Output helper functions --------------------------------[ Start ]-
/*!
    Determines the length of a string token.
*/
static int tokenLength(const char *str)
{
    if (*str == 0)
        return 0;

    const char *nextToken = strpbrk(str, " _/\n\r");
    if (nextToken == str || !nextToken)
        return 1;

    return int(nextToken - str);
}

/*!
    Prints out a string which starts at position \a startingAt, and
    indents each wrapped line with \a wrapIndent characters.
    The wrap point is set to the console width, unless that width
    cannot be determined, or is too small.
*/
void Configure::desc(const char *description, int startingAt, int wrapIndent)
{
    int linePos = startingAt;

    bool firstLine = true;
    const char *nextToken = description;
    while (*nextToken) {
        int nextTokenLen = tokenLength(nextToken);
        if (*nextToken == '\n'                         // Wrap on newline, duh
            || (linePos + nextTokenLen > outputWidth)) // Wrap at outputWidth
        {
            printf("\n");
            linePos = 0;
            firstLine = false;
            if (*nextToken == '\n')
                ++nextToken;
            continue;
        }
        if (!firstLine && linePos < wrapIndent) {  // Indent to wrapIndent
            printf("%*s", wrapIndent , "");
            linePos = wrapIndent;
            if (*nextToken == ' ') {
                ++nextToken;
                continue;
            }
        }
        printf("%.*s", nextTokenLen, nextToken);
        linePos += nextTokenLen;
        nextToken += nextTokenLen;
    }
}

/*!
    Prints out an option with its description wrapped at the
    description starting point. If \a skipIndent is true, the
    indentation to the option is not outputted (used by marked option
    version of desc()). Extra spaces between option and its
    description is filled with\a fillChar, if there's available
    space.
*/
void Configure::desc(const char *option, const char *description, bool skipIndent, char fillChar)
{
    if (!skipIndent)
        printf("%*s", optionIndent, "");

    int remaining  = descIndent - optionIndent - strlen(option);
    int wrapIndent = descIndent + qMax(0, 1 - remaining);
    printf("%s", option);

    if (remaining > 2) {
        printf(" "); // Space in front
        for (int i = remaining; i > 2; --i)
            printf("%c", fillChar); // Fill, if available space
    }
    printf(" "); // Space between option and description

    desc(description, wrapIndent, wrapIndent);
    printf("\n");
}

/*!
    Same as above, except it also marks an option with an '*', if
    the option is default action.
*/
void Configure::desc(const char *mark_option, const char *mark, const char *option, const char *description, char fillChar)
{
    const QString markedAs = dictionary.value(mark_option);
    if (markedAs == "auto" && markedAs == mark) // both "auto", always => +
        printf(" +  ");
    else if (markedAs == "auto")                // setting marked as "auto" and option is default => +
        printf(" %c  " , (defaultTo(mark_option) == QLatin1String(mark))? '+' : ' ');
    else if (QLatin1String(mark) == "auto")     // description marked as "auto" and option is available => +
        printf(" %c  " , checkAvailability(mark_option) ? '+' : ' ');
    else                                        // None are "auto", (markedAs == mark) => *
        printf(" %c  " , markedAs == QLatin1String(mark) ? '*' : ' ');

    desc(option, description, true, fillChar);
}

// Output helper functions ---------------------------------[ Stop ]-


bool Configure::displayHelp()
{
    if( dictionary[ "HELP" ] == "yes" ) {
	desc("Usage: configure [-buildkey <key>]\n"
//	desc("Usage: configure [-prefix dir] [-bindir <dir>] [-libdir <dir>]\n"
//	            "[-docdir <dir>] [-headerdir <dir>] [-plugindir <dir>]\n"
//	            "[-datadir <dir>] [-translationdir <dir>]\n"
//                    "[-examplesdir <dir>] [-demosdir <dir>][-buildkey <key>]\n"
	            "[-release] [-debug] [-debug-and-release] [-shared] [-static]\n"
	            "[-no-fast] [-fast] [-no-exceptions] [-exceptions]\n"
	            "[-no-accessibility] [-accessibility] [-no-rtti] [-rtti]\n"
	            "[-no-stl] [-stl] [-no-sql-<driver>] [-qt-sql-<driver>]\n"
	            "[-plugin-sql-<driver>] [-arch <arch>] [-platform <spec>]\n"
	            "[-D <define>] [-I <includepath>] [-L <librarypath>]\n"
                    "[-help] [-no-dsp] [-dsp] [-no-vcproj] [-vcproj]\n"
                    "[-no-qmake] [-qmake] [-dont-process] [-process]\n"
                    "[-no-style-<style>] [-qt-style-<style>] [-redo]\n"
                    "[-saveconfig <config>] [-loadconfig <config>] [-no-zlib]\n"
                    "[-qt-zlib] [-system-zlib] [-no-gif] [-qt-gif] [-no-libpng]\n"
                    "[-qt-libpng] [-system-libpng] [-no-libjpeg] [-qt-libjpeg]\n"
                    "[-system-libjpeg] [-no-libmng] [-qt-libmng] [-system-libmng]\n"
                    "[-no-qt3support]\n\n", 0, 7);

        desc("Installation options:\n\n");
#if !defined(EVAL)
/*
        desc(" These are optional, but you may specify install directories.\n\n", 0, 1);

        desc(                   "-prefix dir",          "This will install everything relative to dir\n(default $QT_INSTALL_PREFIX)\n");

        desc(" You may use these to separate different parts of the install:\n\n", 0, 1);

	desc(                   "-bindir <dir>",        "Executables will be installed to dir\n(default PREFIX/bin)");
	desc(                   "-libdir <dir>",        "Libraries will be installed to dir\n(default PREFIX/lib)");
	desc(                   "-docdir <dir>",        "Documentation will be installed to dir\n(default PREFIX/doc)");
	desc(                   "-headerdir <dir>",     "Headers will be installed to dir\n(default PREFIX/include)");
	desc(                   "-plugindir <dir>",     "Plugins will be installed to dir\n(default PREFIX/plugins)");
	desc(                   "-datadir <dir>",       "Data used by Qt programs will be installed to dir\n(default PREFIX)");
	desc(                   "-translationdir <dir>","Translations of Qt programs will be installed to dir\n(default PREFIX/translations)\n");
	desc(                   "-examplesdir <dir>",   "Examples will be installed to dir\n(default PREFIX/examples)");
	desc(                   "-demosdir <dir>",      "Demos will be installed to dir\n(default PREFIX/demos)");
*/
        desc(" You may use these options to turn on strict plugin loading:\n\n", 0, 1);

	desc(                   "-buildkey <key>",      "Build the Qt library and plugins using the specified <key>.  "
                                                        "When the library loads plugins, it will only load those that have a matching <key>.\n");

        desc("Configure options:\n\n");

        desc(" The defaults (*) are usually acceptable. A plus (+) denotes a default value"
             " that needs to be evaluated. If the evaluation succeeds, the feature is"
             " included. Here is a short explanation of each option:\n\n", 0, 1);

        desc("BUILD", "release","-release",             "Compile and link Qt with debugging turned off.");
        desc("BUILD", "debug",  "-debug",               "Compile and link Qt with debugging turned on.");
        desc("BUILDALL", "yes", "-debug-and-release",   "Compile and link two Qt libraries, with and without debugging turned on.\n");

        desc("SHARED", "yes",   "-shared",              "Create and use shared Qt libraries.");
        desc("SHARED", "no",    "-static",              "Create and use static Qt libraries.\n");

        desc("FAST", "no",      "-no-fast",             "Configure Qt normally by generating Makefiles for all project files.");
        desc("FAST", "yes",     "-fast",                "Configure Qt quickly by generating Makefiles only for library and "
                                                        "subdirectory targets.  All other Makefiles are created as wrappers "
                                                        "which will in turn run qmake\n");

        desc("EXCEPTIONS", "no", "-no-exceptions",      "Disable exceptions on platforms that support it.");
        desc("EXCEPTIONS", "yes","-exceptions",         "Enable exceptions on platforms that support it.\n");

        desc("ACCESSIBILITY", "no",  "-no-accessibility", "Do not compile Windows Active Accessibilit support.");
        desc("ACCESSIBILITY", "yes", "-accessibility",    "Compile Windows Active Accessibilit support.\n");

        desc("STL", "no",       "-no-stl",              "Do not compile STL support.");
        desc("STL", "yes",      "-stl",                 "Compile STL support.\n");

        desc(                   "-no-sql-<driver>",     "Disable SQL <driver> entirely, by default none are turned on.");
        desc(                   "-qt-sql-<driver>",     "Enable a SQL <driver> in the Qt Library.");
        desc(                   "-plugin-sql-<driver>", "Enable SQL <driver> as a plugin to be linked to at run time.\n"
                                                        "Available values for <driver>:");
        desc("SQL_MYSQL", "auto", "",                   "  mysql", ' ');
        desc("SQL_PSQL", "auto", "",                    "  psql", ' ');
        desc("SQL_OCI", "auto", "",                     "  oci", ' ');
        desc("SQL_ODBC", "auto", "",                    "  odbc", ' ');
        desc("SQL_TDS", "auto", "",                     "  tds", ' ');
        desc("SQL_DB2", "auto", "",                     "  db2", ' ');
        desc("SQL_SQLITE", "auto", "",                  "  sqlite", ' ');
        desc("SQL_SQLITE2", "auto", "",                 "  sqlite2", ' ');
        desc("SQL_IBASE", "auto", "",                   "  ibase", ' ');
        desc(                   "",                     "(drivers marked with a '+' have been detected as available on this system)\n", false, ' ');

        desc("QT3SUPPORT", "no","-no-qt3support",       "Disables the Qt 3 support functionality.\n");

#endif
        desc(                   "-platform <spec>",     "The operating system and compiler you are building on.\n(default %QMAKESPEC%)\n");
        desc(                   "",                     "See the README file for a list of supported operating systems and compilers.\n", false, ' ');

#if !defined(EVAL)
        desc(                   "-D <define>",          "Add an explicit define to the preprocessor.");
        desc(                   "-I <includepath>",     "Add an explicit include path.");
        desc(                   "-L <librarypath>",     "Add an explicit library path.");
        desc(                   "-l <libraryname>",     "Add an explicit library name, residing in a librarypath.\n");
#endif

        desc(                   "-help, -h, -?",        "Display this information.\n");

#if !defined(EVAL)
        // 3rd party stuff options go below here --------------------------------------------------------------------------------
        desc("Third Party Libraries:\n\n");

        desc("ZLIB", "no",      "-no-zlib",             "Do not compile in ZLIB support. Implies -no-libpng.");
        desc("ZLIB", "qt",      "-qt-zlib",             "Use the zlib bundled with Qt.");
        desc("ZLIB", "system",  "-system-zlib",         "Use zlib from the operating system.\nSee http://www.gzip.org/zlib\n");

        desc("GIF", "no",       "-no-gif",              "Do not compile the plugin for GIF reading support.");
        desc("GIF", "plugin",   "-qt-gif",              "Compile the plugin for GIF reading support.\nSee also src/plugins/imageformats/gif/qgifhandler.h\n");

        desc("LIBPNG", "no",    "-no-libpng",           "Do not compile in PNG support.");
        desc("LIBPNG", "qt",    "-qt-libpng",           "Use the libpng bundled with Qt.");
        desc("LIBPNG", "system","-system-libpng",       "Use libpng from the operating system.\nSee http://www.libpng.org/pub/png\n");

        desc("LIBMNG", "no",    "-no-libmng",           "Do not compile in MNG support.");
        desc("LIBMNG", "qt",    "-qt-libmng",           "Use the libmng bundled with Qt.");
        desc("LIBMNG", "system","-system-libmng",       "Use libmng from the operating system.\nSee See http://www.libmng.com\n");

        desc("LIBJPEG", "no",    "-no-libjpeg",         "Do not compile the plugin for JPEG support.");
        desc("LIBJPEG", "qt",    "-qt-libjpeg",         "Use the libjpeg bundled with Qt.");
        desc("LIBJPEG", "system","-system-libjpeg",     "Use libjpeg from the operating system.\nSee http://www.ijg.org\n");

#endif
        // Qt\Windows only options go below here --------------------------------------------------------------------------------
        desc("Qt/Windows only:\n\n");

        desc("DSPFILES", "no",  "-no-dsp",              "Do not generate VC++ .dsp files.");
        desc("DSPFILES", "yes", "-dsp",                 "Generate VC++ .dsp files, only if spec \"win32-msvc\".\n");

        desc("VCPROJFILES", "no", "-no-vcproj",         "Do not generate VC++ .vcproj files.");
        desc("VCPROJFILES", "yes", "-vcproj",           "Generate VC++ .vcproj files, only if platform \"win32-msvc.net\".\n");

#if !defined(EVAL)
        desc("BUILD_QMAKE", "no", "-no-qmake",          "Do not compile qmake.");
        desc("BUILD_QMAKE", "yes", "-qmake",            "Compile qmake.\n");

        desc("NOPROCESS", "yes", "-dont-process",        "Do not generate Makefiles/Project files. This will override -no-fast if specified.");
        desc("NOPROCESS", "no",  "-process",             "Generate Makefiles/Project files.\n");

        desc("RTTI", "no",      "-no-rtti",             "Do not compile runtime type information.");
        desc("RTTI", "yes",     "-rtti",                "Compile runtime type information.\n");

        desc(                   "-arch <arch>",         "Specify an architecture.\n"
                                                        "Available values for <arch>:");
        desc("ARCHITECTURE","windows",       "",        "  windows", ' ');
        desc("ARCHITECTURE","boundschecker", "",        "  boundschecker\n", ' ');

        desc(                   "-no-style-<style>",    "Disable <style> entirely.");
        desc(                   "-qt-style-<style>",    "Enable <style> in the Qt Library.\nAvailable styles: ");

        desc("STYLE_WINDOWS", "yes", "",                "  windows", ' ');
        desc("STYLE_WINDOWSXP", "auto", "",             "  windowsxp", ' ');
        desc("STYLE_PLASTIQUE", "yes", "",              "  plastique", ' ');
        desc("STYLE_CLEANLOOKS", "yes", "",             "  cleanlooks", ' ');
        desc("STYLE_MOTIF", "yes", "",                  "  motif", ' ');
        desc("STYLE_CDE", "yes", "",                    "  cde\n", ' ');

/*      We do not support -qconfig on Windows yet

        desc(                   "-qconfig <local>",     "Use src/tools/qconfig-local.h rather than the default.\nPossible values for local:");
	for (int i=0; i<allConfigs.size(); ++i)
	    desc(               "",                     qPrintable(QString("  %1").arg(allConfigs.at(i))), false, ' ');
        printf("\n");
*/
#endif
        desc(                   "-loadconfig <config>", "Run configure with the parameters from file configure_<config>.cache.");
        desc(                   "-saveconfig <config>", "Run configure and save the parameters in file configure_<config>.cache.");
        desc(                   "-redo",                "Run configure with the same parameters as last time.\n");
	return true;
    }
    return false;
}

bool Configure::findFileInPaths(const QString &fileName, const QStringList &paths)
{
    QDir d;
    for( QStringList::ConstIterator it = paths.begin(); it != paths.end(); ++it ) {
        // Remove any leading or trailing ", this is commonly used in the environment
        // variables
        QString path = (*it);
        if ( path.startsWith( "\"" ) )
            path = path.right( path.length() - 1 );
        if ( path.endsWith( "\"" ) )
	    path = path.left( path.length() - 1 );
        if( d.exists( path + QDir::separator() + fileName ) )
	    return true;
    }
    return false;
}

bool Configure::findFile( const QString &fileName )
{
    QString file = fileName.toLower();
    QStringList paths;
#if defined(Q_OS_WIN32)
    QRegExp splitReg("[;,]");
#else
    QRegExp splitReg("[:]");
#endif
    if (file.endsWith(".h"))
        paths = QString::fromLocal8Bit(getenv("INCLUDE")).split(splitReg, QString::SkipEmptyParts);
    else if ( file.endsWith( ".lib" ) )
        paths = QString::fromLocal8Bit(getenv("LIB")).split(splitReg, QString::SkipEmptyParts);
    else
        paths = QString::fromLocal8Bit(getenv("PATH")).split(splitReg, QString::SkipEmptyParts);
    return findFileInPaths(file, paths);
}

/*!
    Default value for options marked as "auto" if the test passes.
    (Used both by the autoDetection() below, and the desc() function
    to mark (+) the default option of autodetecting options.
*/
QString Configure::defaultTo(const QString &option)
{
    // We prefer using the system version of the 3rd party libs
    if (option == "ZLIB"
        || option == "LIBJPEG"
        || option == "LIBPNG"
        || option == "LIBMNG")
        return "system";

    // We want PNG built-in
    if (option == "PNG")
        return "qt";

    // The JPEG image library can only be a plugin
    if (option == "JPEG"
        || option == "MNG")
        return "plugin";

    // GIF off by default
    if (option == "GIF")
        return "no"; // We cannot have this on by default yet

    // By default we do not want to compile OCI driver when compiling with
    // MinGW, due to lack of such support from Oracle. It prob. wont work.
    // (Customer may force the use though)
    if (dictionary["QMAKESPEC"].endsWith("-g++")
        && option == "SQL_OCI")
        return "no";

    if (option == "SQL_MYSQL"
        || option == "SQL_MYSQL"
        || option == "SQL_ODBC"
        || option == "SQL_OCI"
        || option == "SQL_PSQL"
        || option == "SQL_TDS"
        || option == "SQL_DB2"
        || option == "SQL_SQLITE"
        || option == "SQL_SQLITE2"
        || option == "SQL_IBASE")
        return "plugin";

    return "yes";
}

/*!
    Checks the system for the availability of a feature.
    Returns true if the feature is available, else false.
*/
bool Configure::checkAvailability(const QString &part)
{
    bool available = false;
    if (part == "STYLE_WINDOWSXP")
        available = (dictionary.value("QMAKESPEC") == "win32-g++" || findFile("uxtheme.h"));

    else if (part == "ZLIB")
        available = findFile("zlib.h");

    else if (part == "LIBJPEG")
        available = findFile("jpeglib.h");
    else if (part == "LIBPNG")
        available = findFile("png.h");
    else if (part == "LIBMNG")
        available = findFile("libmng.h");

    else if (part == "SQL_MYSQL")
        available = findFile("mysql.h") && findFile("libmySQL.lib");
    else if (part == "SQL_ODBC")
        available = findFile("sql.h") && findFile("sqlext.h") && findFile("odbc32.lib");
    else if (part == "SQL_OCI")
        available = findFile("oci.h") && findFile("oci.lib");
    else if (part == "SQL_PSQL")
        available = findFile("libpq-fe.h") && findFile("libpq.lib") && findFile("ws2_32.lib") && findFile("advapi32.lib");
    else if (part == "SQL_TDS")
        available = findFile("sybfront.h") && findFile("sybdb.h") && findFile("ntwdblib.lib");
    else if (part == "SQL_DB2")
        available = findFile("sqlcli.h") && findFile("sqlcli1.h") && findFile("db2cli.lib");
    else if (part == "SQL_SQLITE")
        available = true; // Built in, we have a fork
    else if (part == "SQL_SQLITE2")
        available = findFile("sqlite.h") && findFile("sqlite.lib");
    else if (part == "SQL_IBASE")
        available = findFile("ibase.h") && (findFile("gds32_ms.lib") || findFile("gds32.lib"));

    return available;
}

/*
    Autodetect options marked as "auto".
*/
void Configure::autoDetection()
{
    // Style detection
    if (dictionary["STYLE_WINDOWSXP"] == "auto")
        dictionary["STYLE_WINDOWSXP"] = checkAvailability("STYLE_WINDOWSXP") ? defaultTo("STYLE_WINDOWSXP") : "no";

    // Compression detection
    if (dictionary["ZLIB"] == "auto")
        dictionary["ZLIB"] =  checkAvailability("ZLIB") ? defaultTo("ZLIB") : "qt";

    // Image format detection
    if (dictionary["GIF"] == "auto")
        dictionary["GIF"] = defaultTo("GIF");
    if (dictionary["JPEG"] == "auto")
        dictionary["JPEG"] = defaultTo("JPEG");
    if (dictionary["PNG"] == "auto")
        dictionary["PNG"] = defaultTo("PNG");
    if (dictionary["MNG"] == "auto")
        dictionary["MNG"] = defaultTo("MNG");

    if (dictionary["LIBJPEG"] == "auto")
        dictionary["LIBJPEG"] = checkAvailability("LIBJPEG") ? defaultTo("LIBJPEG") : "qt";
    if (dictionary["LIBPNG"] == "auto")
        dictionary["LIBPNG"] = checkAvailability("LIBPNG") ? defaultTo("LIBPNG") : "qt";
    if (dictionary["LIBMNG"] == "auto")
        dictionary["LIBMNG"] = checkAvailability("LIBMNG") ? defaultTo("LIBMNG") : "qt";

    // SQL detection (not on by default)
    if (dictionary["SQL_MYSQL"] == "auto")
        dictionary["SQL_MYSQL"] = checkAvailability("SQL_MYSQL") ? defaultTo("SQL_MYSQL") : "no";
    if (dictionary["SQL_ODBC"] == "auto")
        dictionary["SQL_ODBC"] = checkAvailability("SQL_ODBC") ? defaultTo("SQL_ODBC") : "no";
    if (dictionary["SQL_OCI"] == "auto")
        dictionary["SQL_OCI"] = checkAvailability("SQL_OCI") ? defaultTo("SQL_OCI") : "no";
    if (dictionary["SQL_PSQL"] == "auto")
        dictionary["SQL_PSQL"] = checkAvailability("SQL_PSQL") ? defaultTo("SQL_PSQL") : "no";
    if (dictionary["SQL_TDS"] == "auto")
        dictionary["SQL_TDS"] = checkAvailability("SQL_TDS") ? defaultTo("SQL_TDS") : "no";
    if (dictionary["SQL_DB2"] == "auto")
        dictionary["SQL_DB2"] = checkAvailability("SQL_DB2") ? defaultTo("SQL_DB2") : "no";
    if (dictionary["SQL_SQLITE"] == "auto")
        dictionary["SQL_SQLITE"] = checkAvailability("SQL_SQLITE") ? defaultTo("SQL_SQLITE") : "no";
    if (dictionary["SQL_SQLITE2"] == "auto")
        dictionary["SQL_SQLITE2"] = checkAvailability("SQL_SQLITE2") ? defaultTo("SQL_SQLITE2") : "no";
    if (dictionary["SQL_IBASE"] == "auto")
        dictionary["SQL_IBASE"] = checkAvailability("SQL_IBASE") ? defaultTo("SQL_IBASE") : "no";

    // Mark all unknown "auto" to the default value..
    for (QMap<QString,QString>::iterator i = dictionary.begin(); i != dictionary.end(); ++i)
        if (i.value() == "auto")
            i.value() = defaultTo(i.key());
}

bool Configure::verifyConfiguration()
{
    if (dictionary["QMAKESPEC"].endsWith("-g++")
        && dictionary["SQL_OCI"] != "no") {
        cout << "WARNING: Qt does not support compiling the Oracle database driver with" << endl
             << "MinGW, due to lack of such support from Oracle. Consider disabling the" << endl
             << "Oracle driver, as the current build will most likely fail." << endl;
        cout << "(Press any key to continue..)";
        if(_getch() == 3) // _Any_ keypress w/no echo(eat <Enter> for stdout)
            exit(0);      // Exit cleanly for Ctrl+C
    }

    return true;
}

/*
 Things that affect the Qt API/ABI:
   Options:
     minimal-config small-config medium-config large-config full-config

   Options:
     debug release
     stl

 Things that do not affect the Qt API/ABI:
     system-jpeg no-jpeg jpeg
     system-mng no-mng mng
     system-png no-png png
     system-zlib no-zlib zlib
     no-gif gif
     dll staticlib

     internal
     nocrosscompiler
     GNUmake
     largefile
     nis
     nas
     tablet
     ipv6

     X11     : x11sm xinerama xcursor xfixes xrandr xrender fontconfig xkb
     Embedded: embedded freetype
*/
void Configure::generateBuildKey()
{
    QString spec = dictionary["QMAKESPEC"];

    QString compiler = "msvc"; // ICC is compatible
    if (spec.endsWith("-g++"))
        compiler = "mingw";
    else if (spec.endsWith("-borland"))
        compiler = "borland";

    // Build options which changes the Qt API/ABI
    QStringList build_options;
    if (!dictionary["QCONFIG"].isEmpty())
        build_options += dictionary["QCONFIG"] + "-config ";
    if (dictionary["STL"] == "no")
        build_options += "no-stl";
    build_options.sort();

    // Sorted defines that start with QT_NO_
    QStringList build_defines = qmakeDefines.filter(QRegExp("^QT_NO_"));
    build_defines.sort();

    // Build up the QT_BUILD_KEY ifdef
    QString buildKey = "QT_BUILD_KEY \"";
    if (!dictionary["USER_BUILD_KEY"].isEmpty())
        buildKey += dictionary["USER_BUILD_KEY"] + " ";

    QString build32Key = buildKey + "Windows " + compiler + " %1 " + build_options.join(" ") + " " + build_defines.join(" ");
    QString build64Key = buildKey + "Windows x64 " + compiler + " %1 " + build_options.join(" ") + " " + build_defines.join(" ");
    build32Key = build32Key.simplified();
    build64Key = build64Key.simplified();
    build32Key.prepend("#  define ");
    build64Key.prepend("#  define ");

    QString buildkey = // Debug builds
                       "#if (defined(_DEBUG) || defined(DEBUG))\n"
                       "# if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))\n"
                       + build64Key.arg("debug") + "\"\n"
                       "# else\n"
                       + build32Key.arg("debug") + "\"\n"
                       "# endif\n"
                       "#else\n"
                       // Release builds
                       "# if (defined(WIN64) || defined(_WIN64) || defined(__WIN64__))\n"
                       + build64Key.arg("release") + "\"\n"
                       "# else\n"
                       + build32Key.arg("release") + "\"\n"
                       "# endif\n"
                       "#endif\n";

    dictionary["BUILD_KEY"] = buildkey;
}

void Configure::generateOutputVars()
{
    // Generate variables for output
    // Build key ----------------------------------------------------
    if ( dictionary.contains("BUILD_KEY") ) {
        qmakeVars += dictionary.value("BUILD_KEY");
    }

    QString build = dictionary[ "BUILD" ];
    bool buildAll = (dictionary[ "BUILDALL" ] == "yes");
    if ( build == "debug") {
        if (buildAll)
            qtConfig += "release";
        qtConfig += "debug";
    } else if (build == "release") {
        if (buildAll)
            qtConfig += "debug";
        qtConfig += "release";
    }

    // Compression --------------------------------------------------
    if ( dictionary[ "ZLIB" ] == "no" )
	qtConfig += "no-zlib";
    if( dictionary[ "ZLIB" ] == "qt" )
	qtConfig += "zlib";
    else if( dictionary[ "ZLIB" ] == "system" )
	qtConfig += "system-zlib";

    // Image formates -----------------------------------------------
    if( dictionary[ "GIF" ] == "no" )
	qtConfig += "no-gif";
    else if( dictionary[ "GIF" ] == "plugin" )
	qmakeFormatPlugins += "gif";

    if( dictionary[ "JPEG" ] == "no" )
	qtConfig += "no-jpeg";
    else if( dictionary[ "JPEG" ] == "plugin" )
	qmakeFormatPlugins += "jpeg";
    if( dictionary[ "LIBJPEG" ] == "system" )
	qtConfig += "system-jpeg";

    if( dictionary[ "PNG" ] == "no" )
	qtConfig += "no-png";
    else if( dictionary[ "PNG" ] == "qt" )
	qtConfig += "png";
    if( dictionary[ "LIBPNG" ] == "system" )
	qtConfig += "system-png";

    if( dictionary[ "MNG" ] == "no" )
	qtConfig += "no-mng";
    else if( dictionary[ "MNG" ] == "qt" )
	qtConfig += "mng";
    if( dictionary[ "LIBMNG" ] == "system" )
	qtConfig += "system-mng";

    // Styles -------------------------------------------------------
    if ( dictionary[ "STYLE_WINDOWS" ] == "yes" )
	qmakeStyles += "windows";

    if ( dictionary[ "STYLE_PLASTIQUE" ] == "yes" )
	qmakeStyles += "plastique";

    if ( dictionary[ "STYLE_CLEANLOOKS" ] == "yes" )
	qmakeStyles += "cleanlooks";

    if ( dictionary[ "STYLE_WINDOWSXP" ] == "yes" )
	qmakeStyles += "windowsxp";

    if ( dictionary[ "STYLE_MOTIF" ] == "yes" )
	qmakeStyles += "motif";

    if ( dictionary[ "STYLE_SGI" ] == "yes" )
	qmakeStyles += "sgi";

    if ( dictionary[ "STYLE_POCKETPC" ] == "yes" )
	qmakeStyles += "pocketpc";

    if ( dictionary[ "STYLE_CDE" ] == "yes" )
	qmakeStyles += "cde";

    // Databases ----------------------------------------------------
    if ( dictionary[ "SQL_MYSQL" ] == "yes" )
	qmakeSql += "mysql";
    else if ( dictionary[ "SQL_MYSQL" ] == "plugin" )
	qmakeSqlPlugins += "mysql";

    if ( dictionary[ "SQL_ODBC" ] == "yes" )
	qmakeSql += "odbc";
    else if ( dictionary[ "SQL_ODBC" ] == "plugin" )
	qmakeSqlPlugins += "odbc";

    if ( dictionary[ "SQL_OCI" ] == "yes" )
	qmakeSql += "oci";
    else if ( dictionary[ "SQL_OCI" ] == "plugin" )
	qmakeSqlPlugins += "oci";

    if ( dictionary[ "SQL_PSQL" ] == "yes" )
	qmakeSql += "psql";
    else if ( dictionary[ "SQL_PSQL" ] == "plugin" )
	qmakeSqlPlugins += "psql";

    if ( dictionary[ "SQL_TDS" ] == "yes" )
	qmakeSql += "tds";
    else if ( dictionary[ "SQL_TDS" ] == "plugin" )
	qmakeSqlPlugins += "tds";

    if ( dictionary[ "SQL_DB2" ] == "yes" )
	qmakeSql += "db2";
    else if ( dictionary[ "SQL_DB2" ] == "plugin" )
	qmakeSqlPlugins += "db2";

    if ( dictionary[ "SQL_SQLITE" ] == "yes" )
        qmakeSql += "sqlite";
    else if ( dictionary[ "SQL_SQLITE" ] == "plugin" )
        qmakeSqlPlugins += "sqlite";

    if ( dictionary[ "SQL_SQLITE2" ] == "yes" )
        qmakeSql += "sqlite2";
    else if ( dictionary[ "SQL_SQLITE2" ] == "plugin" )
        qmakeSqlPlugins += "sqlite2";

    if ( dictionary[ "SQL_IBASE" ] == "yes" )
        qmakeSql += "ibase";
    else if ( dictionary[ "SQL_IBASE" ] == "plugin" )
        qmakeSqlPlugins += "ibase";

    // Other options ------------------------------------------------
    if( dictionary[ "BUILDALL" ] == "yes" ) {
        qmakeConfig += "build_all";
    }
    qmakeConfig += dictionary[ "BUILD" ];
    dictionary[ "QMAKE_OUTDIR" ] = dictionary[ "BUILD" ];

    if ( dictionary[ "SHARED" ] == "yes" ) {
	QString version = dictionary[ "VERSION" ];
	if (!version.isEmpty()) {
            qmakeVars += "QMAKE_QT_VERSION_OVERRIDE = " + version.left(version.indexOf("."));
            version.remove(QLatin1Char('.'));
        }
	dictionary[ "QMAKE_OUTDIR" ] += "_shared";
    } else {
	dictionary[ "QMAKE_OUTDIR" ] += "_static";
    }

    if( dictionary[ "ACCESSIBILITY" ] == "yes" )
	qtConfig += "accessibility";

    if( !qmakeLibs.isEmpty() )
        qmakeVars += "LIBS           += " + qmakeLibs.join( " " );

    if (dictionary[ "QT3SUPPORT" ] == "yes")
        qtConfig += "qt3support";

    if (dictionary[ "OPENGL" ] == "yes")
        qtConfig += "opengl";

    if (dictionary["IPV6"] == "yes")
        qtConfig += "ipv6";
    else if (dictionary["IPV6"] == "no")
        qtConfig += "no-ipv6";

    // Directories and settings for .qmake.cache --------------------

    // if QT_INSTALL_* have not been specified on commandline, define them now from QT_INSTALL_PREFIX
    if( !dictionary[ "QT_INSTALL_DOCS" ].size() )
	dictionary[ "QT_INSTALL_DOCS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/doc" );
    if( !dictionary[ "QT_INSTALL_HEADERS" ].size() )
	dictionary[ "QT_INSTALL_HEADERS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/include" );
    if( !dictionary[ "QT_INSTALL_LIBS" ].size() )
	dictionary[ "QT_INSTALL_LIBS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/lib" );
    if( !dictionary[ "QT_INSTALL_BINS" ].size() )
	dictionary[ "QT_INSTALL_BINS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/bin" );
    if( !dictionary[ "QT_INSTALL_PLUGINS" ].size() )
	dictionary[ "QT_INSTALL_PLUGINS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/plugins" );
    if( !dictionary[ "QT_INSTALL_DATA" ].size() )
	dictionary[ "QT_INSTALL_DATA" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] );
    if( !dictionary[ "QT_INSTALL_TRANSLATIONS" ].size() )
	dictionary[ "QT_INSTALL_TRANSLATIONS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/translations" );
    if( !dictionary[ "QT_INSTALL_EXAMPLES" ].size() )
	dictionary[ "QT_INSTALL_EXAMPLES" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/examples");
    if( !dictionary[ "QT_INSTALL_DEMOS" ].size() )
	dictionary[ "QT_INSTALL_DEMOS" ] = QDir::toNativeSeparators( dictionary[ "QT_INSTALL_PREFIX" ] + "/demos" );

    qmakeVars += QString("OBJECTS_DIR     = ") + QDir::toNativeSeparators( "tmp/obj/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString("MOC_DIR         = ") + QDir::toNativeSeparators( "tmp/moc/" + dictionary[ "QMAKE_OUTDIR" ] );
    qmakeVars += QString("RCC_DIR         = ") + QDir::toNativeSeparators("tmp/rcc/" + dictionary["QMAKE_OUTDIR"]);

    if (!qmakeDefines.isEmpty())
        qmakeVars += QString("DEFINES        += ") + qmakeDefines.join( " " );
    if (!qmakeIncludes.isEmpty())
        qmakeVars += QString("INCLUDEPATH    += ") + qmakeIncludes.join( " " );
    if (!qmakeSql.isEmpty())
        qmakeVars += QString("sql-drivers    += ") + qmakeSql.join( " " );
    if (!qmakeSqlPlugins.isEmpty())
        qmakeVars += QString("sql-plugins    += ") + qmakeSqlPlugins.join( " " );
    if (!qmakeStyles.isEmpty())
        qmakeVars += QString("styles         += ") + qmakeStyles.join( " " );
    if (!qmakeStylePlugins.isEmpty())
        qmakeVars += QString("style-plugins  += ") + qmakeStylePlugins.join( " " );
    if (!qmakeFormatPlugins.isEmpty())
        qmakeVars += QString("imageformat-plugins += ") + qmakeFormatPlugins.join( " " );

    if( !dictionary[ "QMAKESPEC" ].length() ) {
	cout << "Configure could not detect your compiler. QMAKESPEC must either" << endl
             << "be defined as an environment variable, or specified as an" << endl
             << "argument with -platform" << endl;
	dictionary[ "HELP" ] = "yes";

	QStringList winPlatforms;
	QDir mkspecsDir( dictionary[ "QT_SOURCE_TREE" ] + "\\mkspecs" );
	const QFileInfoList &specsList = mkspecsDir.entryInfoList();
	for(int i = 0; i < specsList.size(); ++i) {
    	    const QFileInfo &fi = specsList.at(i);
	    if( fi.fileName().left( 5 ) == "win32" ) {
		winPlatforms += fi.fileName();
	    }
	}
	cout << "Available platforms are: " << qPrintable(winPlatforms.join( ", " )) << endl;
	dictionary[ "DONE" ] = "error";
    }
}

#if !defined(EVAL)
void Configure::generateCachefile()
{
    // Generate .qmake.cache
    QFile cacheFile( dictionary[ "QT_SOURCE_TREE" ] + "\\.qmake.cache" );
    if( cacheFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
	QTextStream cacheStream( &cacheFile );
        for( QStringList::Iterator var = qmakeVars.begin(); var != qmakeVars.end(); ++var ) {
	    cacheStream << (*var) << endl;
	}
	cacheStream << "CONFIG         += " << qmakeConfig.join( " " ) << " incremental create_prl link_prl depend_includepath" << endl;
	QString mkspec_path=dictionary[ "QT_SOURCE_TREE" ] + "\\mkspecs\\" + dictionary[ "QMAKESPEC" ];
	if(QFile::exists(mkspec_path))
	    cacheStream << "QMAKESPEC       = " << mkspec_path << endl;
	else
	    cacheStream << "QMAKESPEC       = " << dictionary[ "QMAKESPEC" ] << endl;
        cacheStream << "ARCH            = " << dictionary[ "ARCHITECTURE" ] << endl;
	cacheStream << "QT_BUILD_TREE   = " << dictionary[ "QT_SOURCE_TREE" ] << endl;
	cacheStream << "QT_SOURCE_TREE  = " << dictionary[ "QT_SOURCE_TREE" ] << endl;

        if (dictionary["QT_EDITION"] != "QT_EDITION_OPENSOURCE")
            cacheStream << "DEFINES        *= QT_EDITION=QT_EDITION_DESKTOP" << endl;

	//so that we can build without an install first (which would be impossible)
	cacheStream << "QMAKE_MOC       = $$QT_BUILD_TREE/bin/moc.exe" << endl;
	cacheStream << "QMAKE_UIC       = $$QT_BUILD_TREE/bin/uic.exe" << endl;
	cacheStream << "QMAKE_UIC3      = $$QT_BUILD_TREE/bin/uic3.exe" << endl;
	cacheStream << "QMAKE_RCC       = $$QT_BUILD_TREE/bin/rcc.exe" << endl;
	cacheStream << "QMAKE_DUMPCPP   = $$QT_BUILD_TREE/bin/dumpcpp.exe" << endl;
	cacheStream << "QMAKE_INCDIR_QT = $$QT_BUILD_TREE/include" << endl;
	cacheStream << "QMAKE_LIBDIR_QT = $$QT_BUILD_TREE/lib" << endl;
        cacheStream.flush();
	cacheFile.close();
    }
    QFile configFile( dictionary[ "QT_SOURCE_TREE" ] + "\\mkspecs\\qconfig.pri" );
    if( configFile.open( QFile::WriteOnly | QFile::Text ) ) { // Truncates any existing file.
	QTextStream configStream( &configFile );
	configStream << "CONFIG+= ";
	configStream << dictionary[ "BUILD" ];
	if( dictionary[ "SHARED" ] == "yes" )
	    configStream << " shared";
        else
	    configStream << " static";

	if( dictionary[ "STL" ] == "yes" )
	    configStream << " stl";
	if ( dictionary[ "EXCEPTIONS" ] == "yes" )
	    configStream << " exceptions";
	if ( dictionary[ "RTTI" ] == "yes" )
	    configStream << " rtti";
	configStream << endl;
        configStream << "QT_CONFIG += " << qtConfig.join(" ") << endl;
        if (dictionary["QT_EDITION"].contains("OPENSOURCE"))
            configStream << "QT_EDITION = " << QLatin1String("OpenSource");
        else
            configStream << "QT_EDITION = " << dictionary["EDITION"];

        configStream.flush();
	configFile.close();
    }
}
#endif

QString Configure::addDefine(QString def)
{
    QString result, defNeg, defD = def;

    defD.replace(QRegExp("=.*"), "");
    def.replace(QRegExp("="), " ");

    if(def.startsWith("QT_NO_")) {
        defNeg = defD;
        defNeg.replace("QT_NO_", "QT_");
    } else if(def.startsWith("QT_")) {
        defNeg = defD;
        defNeg.replace("QT_", "QT_NO_");
    }

    if (defNeg.isEmpty()) {
        result = "#ifndef $DEFD\n"
                 "# define $DEF\n"
                 "#endif\n\n";
    } else {
        result = "#if defined($DEFD) && defined($DEFNEG)\n"
                 "# undef $DEFD\n"
                 "#elif !defined($DEFD)\n"
                 "# define $DEF\n"
                 "#endif\n\n";
    }
    result.replace("$DEFNEG", defNeg);
    result.replace("$DEFD", defD);
    result.replace("$DEF", def);
    return result;
}

#if !defined(EVAL)
void Configure::generateConfigfiles()
{
    QString outDir(dictionary[ "QT_SOURCE_TREE" ] + "/src/corelib/global");
    QString outName( outDir + "/qconfig.h" );

    ::SetFileAttributesA( outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    QFile::remove( outName );
    QFile outFile( outName );

    if(outFile.open(QFile::WriteOnly | QFile::Text)) {
	outStream.setDevice(&outFile);

	if( dictionary[ "QCONFIG" ] == "full" ) {
	    outStream << "/* Everything */" << endl;
	} else {
	    QString configName( "qconfig-" + dictionary[ "QCONFIG" ] + ".h" );
	    outStream << "/* Copied from " << configName << "*/" << endl;
            outStream << "#ifndef QT_BOOTSTRAPPED" << endl;
	    QFile inFile( dictionary[ "QT_SOURCE_TREE" ] + "/src/corelib/global/" + configName );
	    if( inFile.open( QFile::ReadOnly ) ) {
		QByteArray buffer = inFile.readAll();
		outFile.write( buffer.constData(), buffer.size() );
		inFile.close();
	    }
            outStream << "#endif // QT_BOOTSTRAPPED" << endl;
	}
        outStream << endl;

        if( dictionary[ "SHARED" ] == "yes" ) {
            outStream << "#ifndef QT_DLL" << endl;
            outStream << "#define QT_DLL" << endl;
            outStream << "#endif" << endl;
        }
	outStream << endl;
	outStream << "/* License information */" << endl;
	outStream << "#define QT_PRODUCT_LICENSEE \"" << licenseInfo[ "LICENSEE" ] << "\"" << endl;
	outStream << "#define QT_PRODUCT_LICENSE \"" << dictionary[ "EDITION" ] << "\"" << endl;
        outStream << endl;
        outStream << "// Qt Edition" << endl;
        outStream << "#ifndef QT_EDITION" << endl;
        outStream << "#  define QT_EDITION " << dictionary["QT_EDITION"] << endl;
        outStream << "#endif" << endl;
        outStream << endl;
        outStream << dictionary["BUILD_KEY"];
        outStream << endl;
    if (dictionary["EDITION"] == "Trolltech") {
        outStream << "/* Used for example to export symbols for the certain autotests*/" << endl;
        outStream << "#define QT_BUILD_INTERNAL" << endl;
        outStream << endl;
    }
	outStream << "/* Machine byte-order */" << endl;
	outStream << "#define Q_BIG_ENDIAN 4321" << endl;
	outStream << "#define Q_LITTLE_ENDIAN 1234" << endl;
	if ( QSysInfo::ByteOrder == QSysInfo::BigEndian )
	    outStream << "#define Q_BYTE_ORDER Q_BIG_ENDIAN" << endl;
	else
	    outStream << "#define Q_BYTE_ORDER Q_LITTLE_ENDIAN" << endl;

	outStream << endl << "// Compile time features" << endl;
        outStream << "#define QT_ARCH_" << dictionary["ARCHITECTURE"].toUpper() << endl;
        QStringList qconfigList;
        if(dictionary["STL"] == "no")                qconfigList += "QT_NO_STL";
        if(dictionary["STYLE_WINDOWS"] != "yes")     qconfigList += "QT_NO_STYLE_WINDOWS";
        if(dictionary["STYLE_PLASTIQUE"] != "yes")   qconfigList += "QT_NO_STYLE_PLASTIQUE";
        if(dictionary["STYLE_CLEANLOOKS"] != "yes")   qconfigList += "QT_NO_STYLE_CLEANLOOKS";
	if(dictionary["STYLE_WINDOWSXP"] != "yes")   qconfigList += "QT_NO_STYLE_WINDOWSXP";
	if(dictionary["STYLE_MOTIF"] != "yes")       qconfigList += "QT_NO_STYLE_MOTIF";
	if(dictionary["STYLE_CDE"] != "yes")         qconfigList += "QT_NO_STYLE_CDE";

        if(dictionary["GIF"] == "yes")              qconfigList += "QT_BUILTIN_GIF_READER=1";
        if(dictionary["PNG"] == "no")               qconfigList += "QT_NO_IMAGEFORMAT_PNG";
        if(dictionary["MNG"] == "no")               qconfigList += "QT_NO_IMAGEFORMAT_MNG";
        if(dictionary["JPEG"] == "no")              qconfigList += "QT_NO_IMAGEFORMAT_JPEG";
        if(dictionary["ZLIB"] == "no") {
            qconfigList += "QT_NO_ZLIB";
            qconfigList += "QT_NO_COMPRESS";
        }

        if(dictionary["QT3SUPPORT"] == "no")        qconfigList += "QT_NO_QT3SUPPORT";
        if(dictionary["ACCESSIBILITY"] == "no")     qconfigList += "QT_NO_ACCESSIBILITY";
        if(dictionary["EXCEPTIONS"] == "no")        qconfigList += "QT_NO_EXCEPTIONS";
        if(dictionary["OPENGL"] == "no")            qconfigList += "QT_NO_OPENGL";
        if(dictionary["IPV6"] == "no")              qconfigList += "QT_NO_IPV6";

        if(dictionary["SQL_MYSQL"] == "yes")        qconfigList += "QT_SQL_MYSQL";
        if(dictionary["SQL_ODBC"] == "yes")         qconfigList += "QT_SQL_ODBC";
        if(dictionary["SQL_OCI"] == "yes")          qconfigList += "QT_SQL_OCI";
        if(dictionary["SQL_PSQL"] == "yes")         qconfigList += "QT_SQL_PSQL";
        if(dictionary["SQL_TDS"] == "yes")          qconfigList += "QT_SQL_TDS";
        if(dictionary["SQL_DB2"] == "yes")          qconfigList += "QT_SQL_DB2";
        if(dictionary["SQL_SQLITE"] == "yes")       qconfigList += "QT_SQL_SQLITE";
        if(dictionary["SQL_SQLITE2"] == "yes")      qconfigList += "QT_SQL_SQLITE2";
        if(dictionary["SQL_IBASE"] == "yes")        qconfigList += "QT_SQL_IBASE";

        qconfigList.sort();
        for (int i = 0; i < qconfigList.count(); ++i)
            outStream << addDefine(qconfigList.at(i));

        outStream.flush();
	outFile.close();
        if (!writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n",    qtDir + "/include/QtCore/qconfig.h")
            || !writeToFile("#include \"../../src/corelib/global/qconfig.h\"\n", qtDir + "/include/Qt/qconfig.h")) {
            dictionary["DONE"] = "error";
            return;
        }
    }

    QString archFile = qtDir + "/src/corelib/arch/qatomic_" + dictionary["ARCHITECTURE"].toLower() + ".h";
    QFileInfo archInfo(archFile);
    if (!archInfo.exists()) {
	qDebug("Architecture file %s does not exist!", qPrintable(archFile) );
        dictionary[ "DONE" ] = "error";
        return;
    }
    QDir archhelper;
    archhelper.mkdir(qtDir + "/include/QtCore/arch");
    if (!CopyFileA(archFile.toLocal8Bit(), QString(qtDir + "/include/QtCore/arch/qatomic.h").toLocal8Bit(), FALSE))
	qDebug("Couldn't copy %s to include/arch", qPrintable(archFile) );
    if (!SetFileAttributesA(QString(qtDir + "/include/QtCore/arch/qatomic.h").toLocal8Bit(), FILE_ATTRIBUTE_NORMAL))
	qDebug("Couldn't reset writable file attribute for qatomic.h");

    // Create qatomic.h "symlinks"
    QString atomicContents = QString("#include \"../../../src/corelib/arch/qatomic_" + dictionary[ "ARCHITECTURE" ].toLower() + ".h\"\n");
    if (!writeToFile(atomicContents.toLocal8Bit(),    qtDir + "/include/QtCore/arch/qatomic.h")
        || !writeToFile(atomicContents.toLocal8Bit(), qtDir + "/include/Qt/arch/qatomic.h")) {
        dictionary[ "DONE" ] = "error";
        return;
    }

    // Copy configured mkspec to default directory, but remove the old one first, if there is any
    QString defSpec = qtDir + "/mkspecs/default";
    QFileInfo defSpecInfo(defSpec);
    if (defSpecInfo.exists()) {
        if (!Environment::rmdir(defSpec)) {
            cout << "Couldn't update default mkspec! Are files in " << qPrintable(defSpec) << " read-only?" << endl;
            dictionary["DONE"] = "error";
            return;
        }
    }

    QString pltSpec = qtDir + "/mkspecs/" + dictionary["QMAKESPEC"];
    if (!Environment::cpdir(pltSpec, defSpec)) {
        cout << "Couldn't update default mkspec! Does " << qPrintable(pltSpec) << " exist?" << endl;
        dictionary["DONE"] = "error";
        return;
    }

    outName = defSpec + "/qmake.conf";
    ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    outFile.setFileName(outName);
    if (outFile.open(QFile::Append | QFile::WriteOnly | QFile::Text)) {
        outStream.setDevice(&outFile);
        outStream << endl << "QMAKESPEC_ORIGINAL=" << pltSpec << endl;
        outStream.flush();
	outFile.close();
    }

    outDir = dictionary[ "QT_SOURCE_TREE" ];

    // Generate the new qconfig.cpp file
    // ### Should go through a qconfig.cpp.new, like the X11/Mac/Qtopia
    // ### one, to avoid unecessary rebuilds, if file hasn't changed
    outName = outDir + "/src/corelib/global/qconfig.cpp";
    ::SetFileAttributesA(outName.toLocal8Bit(), FILE_ATTRIBUTE_NORMAL );
    outFile.setFileName(outName);
    if (outFile.open(QFile::WriteOnly | QFile::Text)) {
        outStream.setDevice(&outFile);

        outStream << "/* Licensed */" << endl
                  << "static const char qt_configure_licensee_str          [512 + 12] = \"qt_lcnsuser=" << licenseInfo["LICENSEE"] << "\";" << endl
                  << "static const char qt_configure_licensed_products_str [512 + 12] = \"qt_lcnsprod=" << dictionary["EDITION"] << "\";" << endl
                  << "static const char qt_configure_prefix_path_str       [512 + 12] = \"qt_prfxpath=" << QString(dictionary["QT_INSTALL_PREFIX"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "static const char qt_configure_documentation_path_str[512 + 12] = \"qt_docspath=" << QString(dictionary["QT_INSTALL_DOCS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_headers_path_str      [512 + 12] = \"qt_hdrspath=" << QString(dictionary["QT_INSTALL_HEADERS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_libraries_path_str    [512 + 12] = \"qt_libspath=" << QString(dictionary["QT_INSTALL_LIBS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_binaries_path_str     [512 + 12] = \"qt_binspath=" << QString(dictionary["QT_INSTALL_BINS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_plugins_path_str      [512 + 12] = \"qt_plugpath=" << QString(dictionary["QT_INSTALL_PLUGINS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_data_path_str         [512 + 12] = \"qt_datapath=" << QString(dictionary["QT_INSTALL_DATA"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_translations_path_str [512 + 12] = \"qt_trnspath=" << QString(dictionary["QT_INSTALL_TRANSLATIONS"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "static const char qt_configure_examples_path_str     [512 + 12] = \"qt_xmplpath=" << QString(dictionary["QT_INSTALL_EXAMPLES"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  << "static const char qt_configure_demos_path_str        [512 + 12] = \"qt_demopath=" << QString(dictionary["QT_INSTALL_DEMOS"]).replace( "\\", "\\\\" ) << "\";"  << endl
                  //<< "static const char qt_configure_settings_path_str [256] = \"qt_stngpath=" << QString(dictionary["QT_INSTALL_SETTINGS"]).replace( "\\", "\\\\" ) << "\";" << endl
                  << "/* strlen( \"qt_lcnsxxxx\" ) == 12 */" << endl
                  << "#define QT_CONFIGURE_LICENSEE qt_configure_licensee_str + 12;" << endl
                  << "#define QT_CONFIGURE_LICENSED_PRODUCTS qt_configure_licensed_products_str + 12;" << endl
                  << "#define QT_CONFIGURE_PREFIX_PATH qt_configure_prefix_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DOCUMENTATION_PATH qt_configure_documentation_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_HEADERS_PATH qt_configure_headers_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_LIBRARIES_PATH qt_configure_libraries_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_BINARIES_PATH qt_configure_binaries_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_PLUGINS_PATH qt_configure_plugins_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DATA_PATH qt_configure_data_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_TRANSLATIONS_PATH qt_configure_translations_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_EXAMPLES_PATH qt_configure_examples_path_str + 12;" << endl
                  << "#define QT_CONFIGURE_DEMOS_PATH qt_configure_demos_path_str + 12;" << endl
                  //<< "#define QT_CONFIGURE_SETTINGS_PATH qt_configure_settings_path_str + 12;" << endl
                  << endl;

        outStream.flush();
        outFile.close();
    }
}
#endif

#if !defined(EVAL)
void Configure::displayConfig()
{
    // Give some feedback
    cout << "Environment:" << endl;
    QString env = QString::fromLocal8Bit(getenv("INCLUDE")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    INCLUDE=\r\n      " << env << endl;
    env = QString::fromLocal8Bit(getenv("LIB")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    LIB=\r\n      " << env << endl;
    env = QString::fromLocal8Bit(getenv("PATH")).replace(QRegExp("[;,]"), "\r\n      ");
    if (env.isEmpty())
	env = "Unset";
    cout << "    PATH=\r\n      " << env << endl;

    if (dictionary["EDITION"] == "Trolltech") {
	cout << "Trolltech license file used (" << dictionary["LICENSE FILE"] << ")" << endl;
    } else if (dictionary["EDITION"] == "OpenSource") {
        cout << "You are licensed to use this software under the terms of the GNU GPL." << endl;
        cout << "See " << dictionary["LICENSE FILE"] << endl << endl;
    } else {
        QString l1 = licenseInfo[ "LICENSEE" ];
        QString l2 = licenseInfo[ "LICENSEID" ];
        QString l3 = dictionary["EDITION"] + ' ' + "Edition";
        QString l4 = licenseInfo[ "EXPIRYDATE" ];
        cout << "Licensee...................." << (l1.isNull() ? "" : l1) << endl;
        cout << "License ID.................." << (l2.isNull() ? "" : l2) << endl;
        cout << "Product license............." << (l3.isNull() ? "" : l3) << endl;
        cout << "Expiry Date................." << (l4.isNull() ? "" : l4) << endl << endl;
    }

    cout << "Configuration:" << endl;
    cout << "    " << qmakeConfig.join( "\r\n    " ) << endl;
    cout << "Qt Configuration:" << endl;
    cout << "    " << qtConfig.join( "\r\n    " ) << endl;
    cout << endl;

    cout << "QMAKESPEC..................." << dictionary[ "QMAKESPEC" ] << " (" << dictionary["QMAKESPEC_FROM"] << ")" << endl;
    cout << "Architecture................" << dictionary[ "ARCHITECTURE" ] << endl;
    cout << "Maketool...................." << dictionary[ "MAKE" ] << endl;
    cout << "Debug symbols..............." << (dictionary[ "BUILD" ] == "debug" ? "yes" : "no") << endl;
    cout << "Accessibility support......." << dictionary[ "ACCESSIBILITY" ] << endl;
    cout << "STL support................." << dictionary[ "STL" ] << endl;
    cout << "Exception support..........." << dictionary[ "EXCEPTIONS" ] << endl;
    cout << "RTTI support................" << dictionary[ "RTTI" ] << endl;
    cout << "OpenGL support.............." << dictionary[ "OPENGL" ] << endl;
    cout << "Qt3 compatibility..........." << dictionary[ "QT3SUPPORT" ] << endl << endl;

    cout << "Third Party Libraries:" << endl;
    cout << "    ZLIB support............" << dictionary[ "ZLIB" ] << endl;
    cout << "    GIF support............." << dictionary[ "GIF" ] << endl;
    cout << "    JPEG support............" << dictionary[ "JPEG" ] << endl;
    cout << "    PNG support............." << dictionary[ "PNG" ] << endl;
    cout << "    MNG support............." << dictionary[ "MNG" ] << endl << endl;

    cout << "Styles:" << endl;
    cout << "    Windows................." << dictionary[ "STYLE_WINDOWS" ] << endl;
    cout << "    Windows XP.............." << dictionary[ "STYLE_WINDOWSXP" ] << endl;
    cout << "    Plastique..............." << dictionary[ "STYLE_PLASTIQUE" ] << endl;
    cout << "    Cleanlooks.............." << dictionary[ "STYLE_CLEANLOOKS" ] << endl;
    cout << "    Motif..................." << dictionary[ "STYLE_MOTIF" ] << endl;
    cout << "    CDE....................." << dictionary[ "STYLE_CDE" ] << endl << endl;
    // Only show the PocketPC style option for CE users

    cout << "Sql Drivers:" << endl;
    cout << "    ODBC...................." << dictionary[ "SQL_ODBC" ] << endl;
    cout << "    MySQL..................." << dictionary[ "SQL_MYSQL" ] << endl;
    cout << "    OCI....................." << dictionary[ "SQL_OCI" ] << endl;
    cout << "    PostgreSQL.............." << dictionary[ "SQL_PSQL" ] << endl;
    cout << "    TDS....................." << dictionary[ "SQL_TDS" ] << endl;
    cout << "    DB2....................." << dictionary[ "SQL_DB2" ] << endl;
    cout << "    SQLite.................." << dictionary[ "SQL_SQLITE" ] << endl;
    cout << "    SQLite2................." << dictionary[ "SQL_SQLITE2" ] << endl;
    cout << "    InterBase..............." << dictionary[ "SQL_IBASE" ] << endl << endl;

    cout << "Sources are in.............." << dictionary[ "QT_SOURCE_TREE" ] << endl;
    cout << "Build is done in............" << dictionary[ "QT_BUILD_TREE" ] << endl;
    cout << "Install prefix.............." << dictionary[ "QT_INSTALL_PREFIX" ] << endl;
    cout << "Headers installed to........" << dictionary[ "QT_INSTALL_HEADERS" ] << endl;
    cout << "Libraries installed to......" << dictionary[ "QT_INSTALL_LIBS" ] << endl;
    cout << "Plugins installed to........" << dictionary[ "QT_INSTALL_PLUGINS" ] << endl;
    cout << "Binaries installed to......." << dictionary[ "QT_INSTALL_BINS" ] << endl;
    cout << "Docs installed to..........." << dictionary[ "QT_INSTALL_DOCS" ] << endl;
    cout << "Data installed to..........." << dictionary[ "QT_INSTALL_DATA" ] << endl;
    cout << "Translations installed to..." << dictionary[ "QT_INSTALL_TRANSLATIONS" ] << endl;
    cout << "Examples installed to......." << dictionary[ "QT_INSTALL_EXAMPLES" ] << endl;
    cout << "Demos installed to.........." << dictionary[ "QT_INSTALL_DEMOS" ] << endl << endl;

    cout << endl;
    if( !qmakeDefines.isEmpty() ) {
	cout << "Defines.....................";
	for( QStringList::Iterator defs = qmakeDefines.begin(); defs != qmakeDefines.end(); ++defs )
	    cout << (*defs) << " ";
	cout << endl;
    }
    if( !qmakeIncludes.isEmpty() ) {
	cout << "Include paths...............";
	for( QStringList::Iterator incs = qmakeIncludes.begin(); incs != qmakeIncludes.end(); ++incs )
	    cout << (*incs) << " ";
	cout << endl;
    }
    if( !qmakeLibs.isEmpty() ) {
	cout << "Additional libraries........";
	for( QStringList::Iterator libs = qmakeLibs.begin(); libs != qmakeLibs.end(); ++libs )
	    cout << (*libs) << " ";
	cout << endl;
    }
    if( dictionary[ "QMAKE_INTERNAL" ] == "yes" ) {
	cout << "Using internal configuration." << endl;
    }
    if( dictionary[ "SHARED" ] == "no" ) {
	cout << "WARNING: Using static linking will disable the use of plugins." << endl;
	cout << "         Make sure you compile ALL needed modules into the library." << endl;
    }
}
#endif

#if !defined(EVAL)
void Configure::buildQmake()
{
    if( dictionary[ "BUILD_QMAKE" ] == "yes" ) {
	QStringList args;

	// Build qmake
	QString pwd = QDir::currentPath();
	QDir::setCurrent( dictionary[ "QT_SOURCE_TREE" ] + "/qmake" );

        args += dictionary[ "MAKE" ];
	args += "-f";
	args += dictionary[ "QMAKEMAKEFILE" ];

        QStringList additionalEnv;
        additionalEnv.append("QMAKESPEC=" + dictionary["QMAKESPEC"]);
        if (dictionary["EDITION"] == "OpenSource" || dictionary["QT_EDITION"].contains("OPENSOURCE"))
            additionalEnv.append("QMAKE_OPENSOURCE_EDITION=yes");

	cout << "Creating qmake..." << endl;
        int exitCode = 0;
        if( exitCode = Environment::execute(args, additionalEnv, QStringList()) ) {
	    args.clear();
	    args += dictionary[ "MAKE" ];
	    args += "-f";
	    args += dictionary[ "QMAKEMAKEFILE" ];
	    args += "clean";
	    if( exitCode = Environment::execute(args, additionalEnv, QStringList()) ) {
		cout << "Cleaning qmake failed, return code " << exitCode << endl << endl;
                dictionary[ "DONE" ] = "error";
	    } else {
		args.clear();
		args += dictionary[ "MAKE" ];
		args += "-f";
		args += dictionary[ "QMAKEMAKEFILE" ];
		if (exitCode = Environment::execute(args, additionalEnv, QStringList())) {
		    cout << "Building qmake failed, return code " << exitCode << endl << endl;
		    dictionary[ "DONE" ] = "error";
		}
	    }
	}
	QDir::setCurrent( pwd );
    }
}
#endif

void Configure::findProjects( const QString& dirName )
{
    if( dictionary[ "NOPROCESS" ] == "no" ) {
	QDir dir( dirName );
	QString entryName;
	int makeListNumber;
	ProjectType qmakeTemplate;

	const QFileInfoList &list = dir.entryInfoList();
	for(int i = 0; i < list.size(); ++i) {
	    const QFileInfo &fi = list.at(i);
	    if( fi.fileName()[ 0 ] != '.' && fi.fileName() != "qmake.pro" ) {
		entryName = dirName + "/" + fi.fileName();
                if(fi.isDir()) {
                    findProjects( entryName );
		} else {
		    if( fi.fileName().right( 4 ) == ".pro" ) {
			qmakeTemplate = projectType( fi.absoluteFilePath() );
			switch ( qmakeTemplate ) {
			case Lib:
			case Subdirs:
			    makeListNumber = 1;
			    break;
			default:
			    makeListNumber = 2;
			    break;
			}
			makeList[makeListNumber].append( new MakeItem(dirName,
								      fi.fileName(),
								      "Makefile",
								      qmakeTemplate ) );
		    }
		}
	    }

	}
    }
}

void Configure::appendMakeItem(int inList, const QString &item)
{
    QString dir;
    if (item != "src")
	dir = "/" + item;
    makeList[inList].append(new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	item + ".pro", "Makefile", Lib ) );
    if( dictionary[ "DSPFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".dsp", Lib ) );
    }
    if( dictionary[ "VCPFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".vcp", Lib ) );
    }
    if( dictionary[ "VCPROJFILES" ] == "yes" ) {
	makeList[inList].append( new MakeItem(dictionary[ "QT_SOURCE_TREE" ] + "/src" + dir,
	    item + ".pro", item + ".vcproj", Lib ) );
    }
}

void Configure::generateMakefiles()
{
    if( dictionary[ "NOPROCESS" ] == "no" ) {
#if !defined(EVAL)
	cout << "Creating makefiles in src..." << endl;
#endif

	QString spec = dictionary[ "QMAKESPEC" ];
	if( spec != "win32-msvc" )
	    dictionary[ "DSPFILES" ] = "no";

	if( spec != "win32-msvc.net" && spec != "win32-msvc2005" )
	    dictionary[ "VCPROJFILES" ] = "no";

	int i = 0;
        QString pwd = QDir::currentPath();
        QString qtDir = QDir::toNativeSeparators(dictionary["QT_SOURCE_TREE"] + "/");
        if (dictionary["FAST"] != "yes") {
            QString dirName;
            bool generate = true;
            bool doDsp = (dictionary["DSPFILES"] == "yes" || dictionary["VCPFILES"] == "yes"
                          || dictionary["VCPROJFILES"] == "yes");
            while (generate) {
                QString qtDir = QDir::toNativeSeparators(dictionary["QT_SOURCE_TREE"] + "/");
                QString pwd = QDir::currentPath();
                QString dirPath = QDir::toNativeSeparators(qtDir + dirName);
                QStringList args;

                args << QDir::toNativeSeparators( qtDir + "/bin/qmake" );

                if (doDsp) {
                    if( dictionary[ "DEPENDENCIES" ] == "no" )
                        args << "-nodepend";
                    args << "-tp" <<  "vc";
                    doDsp = false; // DSP files will be done
                    printf("Generating Visual Studio project files...\n");
                } else {
                    printf("Generating Makefiles...\n");
                    generate = false; // Now Makefiles will be done
                }
                args << "-spec";
                args << dictionary[ "QMAKESPEC" ];
                args << "-r";

                QDir::setCurrent( QDir::toNativeSeparators( dirPath ) );
                if( int exitCode = Environment::execute(args, QStringList(), QStringList()) ) {
                    cout << "Qmake failed, return code " << exitCode  << endl << endl;
                    dictionary[ "DONE" ] = "error";
                }
            }
        } else {
            findProjects(dictionary["QT_SOURCE_TREE"]);
            for ( i=0; i<3; i++ ) {
                for ( int j=0; j<makeList[i].size(); ++j) {
                    MakeItem *it=makeList[i][j];
                    QString dirPath = QDir::toNativeSeparators( it->directory + "/" );
                    QString projectName = dirPath + it->proFile;
                    QString makefileName = dirPath + it->target;
                    QStringList args;

                    args << QDir::toNativeSeparators( qtDir + "/bin/qmake" );
                    args << projectName;
                    args << dictionary[ "QMAKE_ALL_ARGS" ];

                    cout << "For " << qPrintable(projectName) << endl;
                    args << "-o";
                    args << makefileName;
                    args << "-spec";
                    args << dictionary[ "QMAKESPEC" ];

                    QDir::setCurrent( QDir::toNativeSeparators( dirPath ) );

                    QFile file(makefileName);
                    if (!file.open(QFile::WriteOnly)) {
                        printf("failed on dirPath=%s, makefile=%s\n",
                            qPrintable(dirPath), qPrintable(makefileName));
                        continue;
                    }
                    QTextStream txt(&file);
                    txt << "all:\n";
                    txt << "\t" << args.join(" ") << "\n";
                    txt << "\t" << dictionary[ "MAKE" ] << " -f " << makefileName << "\n";
                    txt << "first: all\n";
                    txt << "qmake:\n";
                    txt << "\t" << args.join(" ") << "\n";
                }
            }
        }
        QDir::setCurrent( pwd );
    } else {
	cout << "Processing of project files have been disabled." << endl;
	cout << "Only use this option if you really know what you're doing." << endl << endl;
	dictionary[ "DONE" ] = "yes";
	return;
    }
}

void Configure::showSummary()
{
    QString make = dictionary[ "MAKE" ];
    cout << endl << endl << "Qt is now configured for building. Just run " << qPrintable(make) << "." << endl;
    cout << "To reconfigure, run " << qPrintable(make) << " clean and configure." << endl << endl;
}

Configure::ProjectType Configure::projectType( const QString& proFileName )
{
    QFile proFile( proFileName );
    if( proFile.open( QFile::ReadOnly ) ) {
        QString buffer = proFile.readLine(1024);
	while (!buffer.isEmpty()) {
	    QStringList segments = buffer.split(QRegExp( "\\s" ));
	    QStringList::Iterator it = segments.begin();

	    if(segments.size() >= 3) {
		QString keyword = (*it++);
		QString operation = (*it++);
		QString value = (*it++);

		if( keyword == "TEMPLATE" ) {
		    if( value == "lib" )
			return Lib;
		    else if( value == "subdirs" )
			return Subdirs;
		}
	    }
	    // read next line
	    buffer = proFile.readLine(1024);
	}
	proFile.close();
    }
    // Default to app handling
    return App;
}

#if !defined(EVAL)

bool Configure::showLicense(const QString &licenseFile)
{
    if (dictionary["LICENSE_CONFIRMED"] == "yes") {
        cout << "You have already accepted the terms of the license." << endl << endl;
        return true;
    }

    QString theLicense;
    if (dictionary["EDITION"] == "OpenSource") {
        theLicense = "GNU General Public License";
    } else {
        // the first line of the license file tells us which license it is
        QFile file(licenseFile);
        if (!file.open(QFile::ReadOnly)) {
            cout << "Failed to load LICENSE file";
            return false;
        }
        theLicense = file.readLine().trimmed();
    }

    forever {
        char accept = '?';
        cout << "You are licensed to use this software under the terms of" << endl
             << "the " << theLicense << "." << endl
             << endl
             << "Type '?' to view the " << theLicense << "." << endl
             << "Type 'y' to accept this license offer." << endl
             << "Type 'n' to decline this license offer." << endl
             << endl
             << "Do you accept the terms of the license?" << endl;
        cin >> accept;
        accept = tolower(accept);

        if (accept == 'y') {
            return true;
        } else if (accept == 'n') {
            return false;
        } else {
            // Get console line height, to fill the screen properly
            int i = 0, screenHeight = 25; // default
            CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
            HANDLE stdOut = GetStdHandle(STD_OUTPUT_HANDLE);
            if (GetConsoleScreenBufferInfo(stdOut, &consoleInfo))
                screenHeight = consoleInfo.srWindow.Bottom
                             - consoleInfo.srWindow.Top
                             - 1; // Some overlap for context

            // Prompt the license content to the user
            QFile file(licenseFile);
            if (!file.open(QFile::ReadOnly)) {
                cout << "Failed to load LICENSE file";
                return false;
            }
            QStringList licenseContent = QString(file.readAll()).split('\n');
            while(i < licenseContent.size()) {
                cout << licenseContent.at(i) << endl;
                if (++i % screenHeight == 0) {
                    cout << "(Press any key for more..)";
                    if(_getch() == 3) // _Any_ keypress w/no echo(eat <Enter> for stdout)
                        exit(0);      // Exit cleanly for Ctrl+C
                    cout << "\r";     // Overwrite text above
                }
            }
        }
    }
}

void Configure::readLicense()
{
    dictionary["LICENSE FILE"] = dictionary["QT_SOURCE_TREE"] + "/LICENSE.GPL";
    if (QFile::exists(dictionary["LICENSE FILE"])) {
        cout << endl << "This is the Qt/Windows Open Source Edition." << endl;
        licenseInfo["LICENSEE"] = "Open Source";
        dictionary["EDITION"] = "OpenSource";
        dictionary["QT_EDITION"] = "QT_EDITION_OPENSOURCE";
        // Ensure that the right QMAKESPEC is used for the Open Source version
        if (!dictionary["QMAKESPEC"].endsWith("-g++")) {
            cout << "The Qt/Windows Open Source Edition only supports the MinGW compiler." << endl;
            dictionary["DONE"] = "error";
            return;
        }
        cout << endl;
        if (!showLicense(dictionary["LICENSE FILE"])) {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return;
        }
        return;
    }
#ifndef COMMERCIAL_VERSION
    else {
        cout << endl << "Cannot find the GPL license file (" << dictionary["LICENSE FILE"] << ")" << endl;
        dictionary["DONE"] = "error";
    }
#else
    else if (Tools::checkInternalLicense(dictionary)) {
        licenseInfo["LICENSEE"] = dictionary["EDITION"];
        return;
    }

    Tools::checkLicense(dictionary, licenseInfo, firstLicensePath());
    if (dictionary["DONE"] != "error") {
        // give the user some feedback, and prompt for license acceptance
        cout << endl << "This is the Qt/Windows " << dictionary["EDITION"] << " Edition."<< endl << endl;
        if (!showLicense(dictionary["LICENSE FILE"])) {
            cout << "Configuration aborted since license was not accepted";
            dictionary["DONE"] = "error";
            return;
        }
        if (!dictionary.contains("METERED LICENSE"))
            QFile::remove(dictionary["QT_SOURCE_TREE"] + "/bin/qtusagereporter.exe");
    }
#endif // COMMERCIAL_VERSION
}

void Configure::reloadCmdLine()
{
    if( dictionary[ "REDO" ] == "yes" ) {
	QFile inFile( dictionary[ "QT_SOURCE_TREE" ] + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
	if( inFile.open( QFile::ReadOnly ) ) {
	    QTextStream inStream( &inFile );
	    QString buffer;
	    inStream >> buffer;
	    while( buffer.length() ) {
		configCmdLine += buffer;
		inStream >> buffer;
	    }
	    inFile.close();
	}
    }
}

void Configure::saveCmdLine()
{
    if( dictionary[ "REDO" ] != "yes" ) {
	QFile outFile( dictionary[ "QT_SOURCE_TREE" ] + "/configure" + dictionary[ "CUSTOMCONFIG" ] + ".cache" );
	if( outFile.open( QFile::WriteOnly | QFile::Text ) ) {
	    QTextStream outStream( &outFile );
	    for( QStringList::Iterator it = configCmdLine.begin(); it != configCmdLine.end(); ++it ) {
		outStream << (*it) << " " << endl;
	    }
            outStream.flush();
	    outFile.close();
	}
    }
}
#endif // !EVAL

bool Configure::isDone()
{
    return !dictionary["DONE"].isEmpty();
}

bool Configure::isOk()
{
    return (dictionary[ "DONE" ] != "error");
}
