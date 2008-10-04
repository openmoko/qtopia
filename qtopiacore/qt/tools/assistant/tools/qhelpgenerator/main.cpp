/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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

#include "../shared/helpgenerator.h"

#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QCoreApplication>

#include <private/qhelpprojectdata_p.h>

QT_USE_NAMESPACE

int main(int argc, char *argv[])
{
    QString error;
    QString arg;    
    QString compressedFile;
    QString projectFile;
    QString basePath;
    bool showHelp = false;
    bool showVersion = false;

    for (int i=1; i<argc; ++i) {
        arg = QString::fromLocal8Bit(argv[i]);    
        if (arg == QLatin1String("-o")) {
            if (++i < argc) {
                QFileInfo fi(QString::fromLocal8Bit(argv[i]));
                compressedFile = fi.absoluteFilePath();                
            } else {
                error = QObject::tr("Missing output file name!");
            }
        } else if (arg == QLatin1String("-v")) {
            showVersion = true;
        } else if (arg == QLatin1String("-h")) {
            showHelp = true;
        } else {
            QFileInfo fi(arg);
            projectFile = fi.absoluteFilePath();
            basePath = fi.absolutePath();
        }
    }

    if (showVersion) {
        fprintf(stdout, "Qt Help Generator version 1.0 (Qt %s)\n", QT_VERSION_STR);
        return 0;
    }

    if (projectFile.isEmpty() && !showHelp)
        error = QObject::tr("Missing Qt help project file!");

    QString help = QObject::tr("\nUsage:\n\n"
        "qhelpgenerator <help-project-file> [options]\n\n"
        "  -o <compressed-file>   Generates a Qt compressed help\n"
        "                         file called <compressed-file>.\n"
        "                         If this option is not specified\n"
        "                         a default name will be used.\n"
        "  -v                     Displays the version of \n"
        "                         qhelpgenerator.\n\n");

    if (showHelp) {
        fprintf(stdout, "%s", qPrintable(help));
        return 0;
    }else if (!error.isEmpty()) {
        fprintf(stderr, "%s\n\n%s", qPrintable(error), qPrintable(help));
        return -1;
    }

    QFile file(projectFile);
    if (!file.open(QIODevice::ReadOnly)) {
        fprintf(stderr, "Could not open %s!\n", qPrintable(projectFile));
        return -1;
    }

    if (compressedFile.isEmpty()) {
        QFileInfo fi(projectFile);
        compressedFile = basePath + QDir::separator()
            + fi.baseName() + QLatin1String(".qch");
    } else {
        // check if the output dir exists -- create if it doesn't
        QFileInfo fi(compressedFile);
        QDir parentDir = fi.dir();
        if (!parentDir.exists()) {
            if (!parentDir.mkpath(".")) {
                fprintf(stderr, "Could not create output directory: %s\n",
                        qPrintable(parentDir.path()));
            }
        }
    }

    QHelpProjectData *helpData = new QHelpProjectData();
    if (!helpData->readData(projectFile)) {
        fprintf(stderr, "%s\n", qPrintable(helpData->errorMessage()));
        return -1;
    }
    
    QCoreApplication app(argc, argv);
    HelpGenerator generator;
    bool success = generator.generate(helpData, compressedFile);
    delete helpData;
    if (!success) {
        fprintf(stderr, "%s\n", qPrintable(generator.error()));
        return -1;
    }
    return 0;
}
