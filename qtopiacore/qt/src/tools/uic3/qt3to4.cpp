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

#include "qt3to4.h"

#include <QApplication>
#include <QDir>
#include <QFile>
#include <QHash>
#include <QLibraryInfo>
#include <QXmlDefaultHandler>
#include <QXmlSimpleReader>

class ContentHandler: public QXmlDefaultHandler
{
public:
    typedef QPair<QString, QString> Rule;
    typedef QList<Rule> RuleList;

public:
    ContentHandler() {}

    inline RuleList renamedHeaders() const
    { return rules(QString::fromLatin1("RenamedHeader")); }

    inline RuleList renamedClasses() const
    { return rules(QString::fromLatin1("RenamedClass")); }

    inline RuleList renamedEnums() const
    { return rules(QString::fromLatin1("RenamedEnumvalue")); }

    inline RuleList rules(const QString &kind) const
    { return m_rules.value(kind); }

    virtual bool startDocument()
    {
        m_rules.clear();
        m_state.current.clear();
        m_state.kind.clear();
        m_state.q3.clear();
        m_state.q4.clear();

        return true;
    }

    virtual bool startElement(const QString &, const QString &, const QString &qName, const QXmlAttributes &attrs)
    {
        if (qName == QLatin1String("item")) {
            m_state.kind = attrs.value("Type");

            m_state.current.clear();
            m_state.q3.clear();
            m_state.q4.clear();

            if (!m_rules.contains(m_state.kind))
                m_rules[m_state.kind].clear();

            return true;
        }

        return true; // ### only if it is a valid tag
    }


    virtual bool endElement(const QString &, const QString &, const QString &qName)
    {
        if (qName == QLatin1String("Qt3")) {
            m_state.q3 = m_state.current.trimmed();
        } else if (qName == QLatin1String("Qt4")) {
            m_state.q4 = m_state.current.trimmed();
        } else if (qName == QLatin1String("item")
                   && (m_state.kind == QLatin1String("RenamedHeader")
                       || m_state.kind == QLatin1String("RenamedEnumvalue")
                       || m_state.kind == QLatin1String("RenamedClass"))) {
            Rule r(m_state.q3, m_state.q4);
            m_rules[m_state.kind].append(r);
        }

        m_state.current.clear();
        return true;
    }

    virtual bool characters(const QString &ch)
    { m_state.current += ch; return true; }

    virtual bool error(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

    virtual bool fatalError(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

    virtual bool warning(const QXmlParseException &e)
    { Q_UNUSED(e); Q_ASSERT(0); return true; }

private:
    QHash<QString, RuleList> m_rules;

    struct state
    {
        QString current;
        QString kind;
        QString q3;
        QString q4;
    } m_state;
};

void Porting::readXML(RuleList *renamedHeaders, RuleList *renamedClasses, RuleList *renamedEnums)
{
    QString fileName = "q3porting.xml";
    QString filePath;
    //check QLibraryInfo::DataPath/filename
    filePath = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::DataPath) + "/" + fileName)  ;

    //check QLibraryInfo::PrefixPath/tools/porting/src/filename
    if (!QFile::exists(filePath))
        filePath = QDir::cleanPath(QLibraryInfo::location(QLibraryInfo::PrefixPath) + "/tools/porting/src/" + fileName);

    // for bootstrapping, look in the rules file in a location
    // relative to where it would be in the source tree
    if (!QFile::exists(filePath))
        filePath = QDir::cleanPath(QApplication::instance()->applicationDirPath() + "/../tools/porting/src/" + fileName);

    if (!QFile::exists(filePath)) {
        fprintf(stderr, "Error: Could not find rules file: %s\n", fileName.toLatin1().constData());
        Q_ASSERT(0);
    }

    ContentHandler handler;

    QXmlSimpleReader reader;
    reader.setContentHandler(&handler);
    reader.setErrorHandler(&handler);

    QFile file(filePath);
    file.open(QFile::ReadOnly);

    QXmlInputSource source(file);
    bool ok = reader.parse(&source);
    Q_UNUSED(ok);
    Q_ASSERT(ok);

    if (renamedHeaders)
        *renamedHeaders = handler.renamedHeaders();
    if (renamedClasses)
        *renamedClasses = handler.renamedClasses();
    if (renamedEnums)
        *renamedEnums = handler.renamedEnums();

}

Porting::Porting()
{
    readXML(&m_renamedHeaders, &m_renamedClasses, &m_renamedEnums);
}

int Porting::findRule(const RuleList &rules, const QString &q3)
{
    for (int i=0; i<rules.count(); ++i)
        if (rules.at(i).first == q3)
            return i;
    return -1;
}

QString Porting::renameHeader(const QString &headerName) const
{
    int index = findRule(m_renamedHeaders, headerName);
    return index == -1 ? headerName : m_renamedHeaders.at(index).second;
}

QString Porting::renameClass(const QString &className) const
{
    if (className == QLatin1String("QSplitter"))  // We don't want a Q3Splitter, ever!
        return className;

    int index = findRule(m_renamedClasses, className);
    return index == -1 ? className : m_renamedClasses.at(index).second;
}

QString Porting::renameEnumerator(const QString &enumName) const
{
    int index = findRule(m_renamedEnums, enumName);
    return index == -1 ? QString() : m_renamedEnums.at(index).second;
}
