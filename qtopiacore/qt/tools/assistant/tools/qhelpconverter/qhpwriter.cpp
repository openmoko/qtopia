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

#include <QtCore/QFile>

#include "qhpwriter.h"
#include "adpreader.h"

QT_BEGIN_NAMESPACE

QhpWriter::QhpWriter(const QString &namespaceName,
                     const QString &virtualFolder)
{
    m_namespaceName = namespaceName;
    m_virtualFolder = virtualFolder;
    setAutoFormatting(true);
}

void QhpWriter::setAdpReader(AdpReader *reader)
{
    m_adpReader = reader;
}

void QhpWriter::setFilterAttributes(const QStringList &attributes)
{
    m_filterAttributes = attributes;
}

void QhpWriter::setCustomFilters(const QList<CustomFilter> filters)
{
    m_customFilters = filters;
}

void QhpWriter::setFiles(const QStringList &files)
{
    m_files = files;
}

void QhpWriter::generateIdentifiers(IdentifierPrefix prefix,
                                    const QString prefixString)
{
    m_prefix = prefix;
    m_prefixString = prefixString;
}

bool QhpWriter::writeFile(const QString &fileName)
{
    QFile out(fileName);
    if (!out.open(QIODevice::WriteOnly))
        return false;
    
    setDevice(&out);
    writeStartDocument();
    writeStartElement(QLatin1String("QtHelpProject"));
    writeAttribute(QLatin1String("version"), QLatin1String("1.0"));
    writeTextElement(QLatin1String("namespace"), m_namespaceName);
    writeTextElement(QLatin1String("virtualFolder"), m_virtualFolder);
    writeCustomFilters();
    writeFilterSection();
    writeEndDocument();

    out.close();
    return true;
}

void QhpWriter::writeCustomFilters()
{
    if (!m_customFilters.count())
        return;

    foreach (CustomFilter f, m_customFilters) {
        writeStartElement(QLatin1String("customFilter"));
        writeAttribute(QLatin1String("name"), f.name);
        foreach (QString a, f.filterAttributes)
            writeTextElement(QLatin1String("filterAttribute"), a);
        writeEndElement();
    }
}

void QhpWriter::writeFilterSection()
{
    writeStartElement(QLatin1String("filterSection"));
    foreach (QString a, m_filterAttributes)
        writeTextElement(QLatin1String("filterAttribute"), a);

    writeToc();
    writeKeywords();
    writeFiles();
    writeEndElement();
}

void QhpWriter::writeToc()
{
    QList<ContentItem> lst = m_adpReader->contents();
    if (lst.isEmpty())
        return;
    
    int depth = -1;
    writeStartElement(QLatin1String("toc"));
    foreach (ContentItem i, lst) {
        while (depth-- >= i.depth)
            writeEndElement();
        writeStartElement(QLatin1String("section"));
        writeAttribute(QLatin1String("title"), i.title);
        writeAttribute(QLatin1String("ref"), i.reference);
        depth = i.depth;
    }
    while (depth-- >= -1)
        writeEndElement();
}

void QhpWriter::writeKeywords()
{
    QList<KeywordItem> lst = m_adpReader->keywords();
    if (lst.isEmpty())
        return;

    writeStartElement(QLatin1String("keywords"));
    foreach (KeywordItem i, lst) {
        writeEmptyElement(QLatin1String("keyword"));
        writeAttribute(QLatin1String("name"), i.keyword);
        writeAttribute(QLatin1String("ref"), i.reference);
        if (m_prefix == FilePrefix) {
            QString str = i.reference.mid(
                i.reference.lastIndexOf(QLatin1Char('/'))+1);
            str = str.left(str.lastIndexOf(QLatin1Char('.')));
            writeAttribute(QLatin1String("id"), str + QLatin1String("::") + i.keyword);
        } else if (m_prefix == GlobalPrefix) {
            writeAttribute(QLatin1String("id"), m_prefixString + i.keyword);
        }
    }
    writeEndElement();
}

void QhpWriter::writeFiles()
{
    if (m_files.isEmpty())
        return;
    
    writeStartElement(QLatin1String("files"));
    foreach (QString f, m_files)
        writeTextElement(QLatin1String("file"), f);
    writeEndElement();
}

QT_END_NAMESPACE
