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

#include <qmap.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qlist.h>
#include <qtextstream.h>

class MakeItem;

class Configure
{
public:
    Configure( int& argc, char** argv );
    ~Configure();

    void parseCmdLine();
#if !defined(EVAL)
    void validateArgs();
#endif
    bool displayHelp();

    QString defaultTo(const QString &option);
    bool checkAvailability(const QString &part);
    void autoDetection();
    bool verifyConfiguration();

    void generateOutputVars();
#if !defined(EVAL)
    void generateBuildKey();
    void generateCachefile();
    void displayConfig();
    void buildQmake();
#endif
    void generateMakefiles();
    void appendMakeItem(int inList, const QString &item);
#if !defined(EVAL)
    void generateConfigfiles();
#endif
    void showSummary();
    void findProjects( const QString& dirName );
    QString firstLicensePath();

#if !defined(EVAL)
    bool showLicense(const QString &licenseFile);
    void readLicense();
#endif

    QString addDefine(QString def);

    enum ProjectType {
	App,
	Lib,
	Subdirs
    };

    ProjectType projectType( const QString& proFileName );
    bool isDone();
    bool isOk();
private:
    // Our variable dictionaries
    QMap<QString,QString> dictionary;
    QStringList licensedModules;
    QStringList allSqlDrivers;
    QStringList allConfigs;
    QStringList disabledModules;
    QStringList enabledModules;
    QStringList modules;
//    QStringList sqlDrivers;
    QStringList configCmdLine;
    QStringList qmakeConfig;
    QStringList qtConfig;

    QStringList qmakeSql;
    QStringList qmakeSqlPlugins;

    QStringList qmakeStyles;
    QStringList qmakeStylePlugins;

    QStringList qmakeFormatPlugins;

    QStringList qmakeVars;
    QStringList qmakeDefines;
    //  makeList[0] for qt and qtmain
    //  makeList[1] for subdirs and libs
    //  makeList[2] for the rest
    QList<MakeItem*> makeList[3];
    QStringList qmakeIncludes;
    QStringList qmakeLibs;

    QMap<QString,QString> licenseInfo;
    QString outputLine;

    QTextStream outStream;
    QString qtDir;

    // Variables for usage output
    int optionIndent;
    int descIndent;
    int outputWidth;

    static bool findFile(const QString &fileName);
    static bool findFileInPaths(const QString &fileName, const QStringList &paths);
#if !defined(EVAL)
    void reloadCmdLine();
    void saveCmdLine();
#endif

    void desc(const char *description, int startingAt = 0, int wrapIndent = 0);
    void desc(const char *option, const char *description, bool skipIndent = false, char fillChar = '.');
    void desc(const char *mark_option, const char *mark, const char *option, const char *description, char fillChar = '.');
};

class MakeItem
{
public:
    MakeItem( const QString &d, const QString &p, const QString &t, Configure::ProjectType qt )
	: directory( d ),
	  proFile( p ),
	  target( t ),
	  qmakeTemplate( qt )
    { }

    QString directory;
    QString proFile;
    QString target;
    Configure::ProjectType qmakeTemplate;
};

