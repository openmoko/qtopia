/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
** ** This file is part of the tools applications of the Qt Toolkit.
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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.

#ifndef QApplicationArgumentParser_H
#define QApplicationArgumentParser_H

#include <QtCore/QVariant> /* Needed, because we can't forward declare QVariantList. */

QT_BEGIN_HEADER
QT_BEGIN_NAMESPACE

class QApplicationArgument;
class QApplicationArgumentParserPrivate;
class QStringList;
template<typename A, typename B> struct QPair;
template<typename T> class QList;
template<typename Value> class QList;

class QApplicationArgumentParser
{
public:
    enum ExitCode
    {
        Success = 0,
        ParseError = 1
    };

    QApplicationArgumentParser(int argc, char **argv);
    QApplicationArgumentParser(const QStringList &input);
    virtual ~QApplicationArgumentParser();
    void addArgument(const QApplicationArgument &argument);
    void setDeclaredArguments(const QList<QApplicationArgument> &arguments);
    QList<QApplicationArgument> declaredArguments() const;

    int count(const QApplicationArgument &argument) const;
    bool has(const QApplicationArgument &argument) const;

    virtual bool parse();
    ExitCode exitCode() const;
    QVariant value(const QApplicationArgument &argument) const;
    QVariantList values(const QApplicationArgument &argument) const;
    void setApplicationDescription(const QString &description);
    void setApplicationVersion(const QString &version);

protected:
    void setExitCode(ExitCode code);
    void setUsedArguments(const QList<QPair<QApplicationArgument, QVariant> > &arguments);
    QList<QPair<QApplicationArgument, QVariant> > usedArguments() const;
    QStringList input() const;
    virtual QVariant convertToValue(const QApplicationArgument &argument,
                                    const QString &value) const;
    virtual QString typeToName(const QApplicationArgument &argument) const;
    virtual QVariant defaultValue(const QApplicationArgument &argument) const;
    virtual void message(const QString &message) const;

private:
    friend class QApplicationArgumentParserPrivate;
    QApplicationArgumentParserPrivate *d;
    Q_DISABLE_COPY(QApplicationArgumentParser)
};

QT_END_NAMESPACE
QT_END_HEADER
#endif
