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

#ifndef PBUILDER_PBX_H
#define PBUILDER_PBX_H

#include "unixmake.h"

QT_BEGIN_NAMESPACE

class ProjectBuilderMakefileGenerator : public UnixMakefileGenerator
{
    bool writingUnixMakefileGenerator;
    QString pbx_dir;
    int pbuilderVersion() const;
    bool writeSubDirs(QTextStream &);
    bool writeMakeParts(QTextStream &);
    bool writeMakefile(QTextStream &);

    QString pbxbuild();
    QMap<QString, QString> keys;
    QString keyFor(const QString &file);
    QString findProgram(const QString &prog);
    QString fixForOutput(const QString &file);
    QStringList fixListForOutput(const QString &where);
    int     reftypeForFile(const QString &where);
    QString projectSuffix() const;
    enum { SettingsAsList=0x01, SettingsNoQuote=0x02 };
    inline QString writeSettings(QString var, QString val, int flags=0, int indent_level=0)
    { Q_UNUSED(indent_level); return writeSettings(var, QStringList(val), flags); }
    QString writeSettings(QString var, QStringList vals, int flags=0, int indent_level=0);

public:
    ProjectBuilderMakefileGenerator();
    ~ProjectBuilderMakefileGenerator();

    virtual bool supportsMetaBuild() { return false; }
    virtual bool openOutput(QFile &, const QString &) const;
protected:
    virtual QString escapeFilePath(const QString &path) const;
    bool doPrecompiledHeaders() const { return false; }
    virtual bool doDepends() const { return false; } //never necesary
};

inline ProjectBuilderMakefileGenerator::~ProjectBuilderMakefileGenerator()
{ }

QT_END_NAMESPACE

#endif // PBUILDER_PBX_H
