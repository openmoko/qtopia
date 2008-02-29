/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the tools applications of the Qt Toolkit.
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

/*
    jambiapiparser.h
*/

#ifndef JAMBIAPIPARSER_H
#define JAMBIAPIPARSER_H

#include <QStack>
#include <QXmlDefaultHandler>

#include "codeparser.h"

struct ClassOrEnumInfo
{
    QString tag;
    QString javaName;
    QString cppName;
    Node *javaNode;
    Node *cppNode;

    ClassOrEnumInfo() : javaNode(0), cppNode(0) {}
};

class JambiApiParser : public CodeParser, private QXmlDefaultHandler
{
public:
    JambiApiParser(Tree *cppTree);
    ~JambiApiParser();

    void initializeParser(const Config &config);
    void terminateParser();
    QString language();
    QString sourceFileNameFilter();
    void parseSourceFile(const Location &location, const QString &filePath, Tree *tree);
    virtual void doneParsingSourceFiles(Tree *tree);

private:
    bool startElement(const QString &namespaceURI, const QString &localName,
                      const QString &qName, const QXmlAttributes &attributes);
    bool endElement(const QString &namespaceURI, const QString &localName,
                    const QString &qName);
    bool fatalError(const QXmlParseException &exception);
    void jambifyDocsPass2(Node *node);
    bool makeFunctionNode(InnerNode *parent, const QString &synopsis, FunctionNode **funcPtr);

    Tree *cppTre;
    Tree *javaTre;

    bool metJapiTag;
    Location japiLocation;
    QStack<ClassOrEnumInfo> classAndEnumStack;
};

#endif
