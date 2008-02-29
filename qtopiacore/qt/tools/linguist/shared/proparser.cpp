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

#include "proparser.h"
#include "profileevaluator.h"
#include <stdio.h>
#include <proreader.h>

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QStringList>

// assumes that the list is sorted (or more correctly, that all equal elements are grouped together).
void removeDuplicates(QStringList *strings, bool alreadySorted /*= true*/)
{
    if (!alreadySorted) {
        strings->sort();
    }
    QString prev;
    QStringList::iterator it = strings->begin();
    while (it != strings->end()) {
        if (*it == prev) {
            it = strings->erase(it);
        }else{
            prev = *it;
            ++it;
        }
    }
}

bool evaluateProFile(const QString &fileName, bool verbose,QMap<QByteArray, QStringList> *varMap)
{
    bool ok = true;

    QStringList sourceFiles;
    QString codecForTr;
    QString codecForSource;
    QStringList tsFileNames;

    ProReader pr;
    ProFileTranslationsScanner *visitor = new ProFileTranslationsScanner(verbose);
    QFileInfo fi(fileName);
    QDir rootPath;
    ok = fi.exists();
    if (ok) {
        rootPath.setPath(fi.absolutePath());
        ProFile *pro = visitor->queryProFile(fi.absoluteFilePath());
        if (!pro)
            ok = false;
        else
            ok = pro->Accept(visitor);
    }
    if (ok) {
        if (visitor->templateType() == ProFileEvaluator::TT_Subdirs) {
            QString oldPath = QDir::currentPath();
            QFileInfo fi(fileName);
            QDir::setCurrent(fi.absolutePath());
            QStringList subdirs = visitor->values("SUBDIRS");
            for (int is = 0; is < subdirs.count() && ok; ++is) {
                QString subdir = subdirs[is];
                QDir dir( subdir );
                QStringList profiles = dir.entryList(QStringList() << "*.pro");
                if (profiles.count()) {
                    ProReader subreader;
                    ProFileTranslationsScanner *subvisitor = new ProFileTranslationsScanner(verbose);
                    QString profile = subdir + QLatin1Char('/') + profiles[0];
                    fi.setFile(profile);
                    ProFile *pro = subreader.read(fi.absoluteFilePath());
                    QString tmpPath = QDir::currentPath();
                    ok = pro->Accept(subvisitor);
                    if (ok) {
                        sourceFiles += subvisitor->absFileNames(QLatin1String("SOURCES"));
                        sourceFiles += subvisitor->absFileNames(QLatin1String("HEADERS"));

                        QStringList forms = subvisitor->absFileNames(QLatin1String("INTERFACES"))
                            + subvisitor->absFileNames(QLatin1String("FORMS"))
                            + subvisitor->absFileNames(QLatin1String("FORMS3"));
                        sourceFiles << forms;
                    }
                    delete subvisitor;
                }
            }
            QDir::setCurrent(oldPath);
        } else {
            // app/lib template
            sourceFiles += visitor->absFileNames(QLatin1String("SOURCES"));
            sourceFiles += visitor->absFileNames(QLatin1String("HEADERS"));

            tsFileNames << visitor->values("TRANSLATIONS");

            QStringList trcodec = visitor->values(QLatin1String("CODEC"))
                + visitor->values(QLatin1String("DEFAULTCODEC"))
                + visitor->values(QLatin1String("CODECFORTR"));
            if (!trcodec.isEmpty())
                codecForTr = trcodec.last().toLatin1();

            QStringList srccodec = visitor->values(QLatin1String("CODECFORSRC"));
            if (!srccodec.isEmpty()) 
                codecForSource = srccodec.last().toLatin1();
            
            QStringList forms = visitor->absFileNames(QLatin1String("INTERFACES"))
                + visitor->absFileNames(QLatin1String("FORMS"))
                + visitor->absFileNames(QLatin1String("FORMS3"));
            sourceFiles << forms;
        }
    }
    if (ok) {
        removeDuplicates(&sourceFiles, false);
        removeDuplicates(&tsFileNames, false);

        varMap->insert("SOURCES", sourceFiles);
        varMap->insert("CODECFORTR", QStringList() << codecForTr);
        varMap->insert("CODECFORSRC", QStringList() << codecForSource);
        varMap->insert("TRANSLATIONS", tsFileNames);
    }
    return ok;

}
