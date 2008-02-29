/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
** 
** Trolltech ASA (c) 2007
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "metatranslator.h"
#include <QtCore/QString>
#include <QtCore/QStack>
#include <QtXml>

class XLIFFHandler : public QXmlDefaultHandler
{
public:
    XLIFFHandler( MetaTranslator *translator );

    virtual bool startElement( const QString& namespaceURI,
                               const QString& localName, const QString& qName,
                               const QXmlAttributes& atts );
    virtual bool endElement( const QString& namespaceURI,
                             const QString& localName, const QString& qName );
    virtual bool characters( const QString& ch );
    virtual bool fatalError( const QXmlParseException& exception );

    virtual bool endDocument();

private:
    enum XliffContext {
        XC_xliff,
        XC_group,
        XC_context_group,
        XC_context,
        XC_context_linenumber,
        XC_ph,
        XC_restype_plurals
    };
    void pushContext(XliffContext ctx);
    bool popContext(XliffContext ctx);
    XliffContext currentContext() const;
    bool hasContext(XliffContext ctx) const;

private:
    MetaTranslator *tor;
    MetaTranslatorMessage::Type m_type;
    QString m_language;
    QString m_context;
    QString m_source;
    QString m_comment;
    QStringList translations;
    QString m_fileName;
    int     m_lineNumber;

    QString accum;
    QString m_ctype;
    int ferrorCount;
    bool contextIsUtf8;
    bool messageIsUtf8;
    const QString m_URI;  // convenience and efficiency; urn:oasis:names:tc:xliff:document:1.1
    const QString m_URI12;  // convenience and efficiency; urn:oasis:names:tc:xliff:document:1.1
    QStack<int> m_contextStack;
};


