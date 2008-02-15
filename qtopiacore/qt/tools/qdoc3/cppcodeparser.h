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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
  cppcodeparser.h
*/

#ifndef CPPCODEPARSER_H
#define CPPCODEPARSER_H

#include <qregexp.h>

#include "codeparser.h"
#include "node.h"

class ClassNode;
class CodeChunk;
class CppCodeParserPrivate;
class FunctionNode;
class InnerNode;
class Tokenizer;

class CppCodeParser : public CodeParser
{
public:
    CppCodeParser();
    ~CppCodeParser();

    virtual void initializeParser( const Config& config );
    virtual void terminateParser();
    virtual QString language();
    virtual QString headerFileNameFilter();
    virtual QString sourceFileNameFilter();
    virtual void parseHeaderFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void parseSourceFile( const Location& location,
				  const QString& filePath, Tree *tree );
    virtual void doneParsingHeaderFiles( Tree *tree );
    virtual void doneParsingSourceFiles( Tree *tree );

    const FunctionNode *findFunctionNode(const QString& synopsis, Tree *tree, Node *relative = 0, bool fuzzy = false);

protected:
    virtual QSet<QString> topicCommands();
    virtual Node *processTopicCommand( const Doc& doc, const QString& command,
				       const QString& arg );
    virtual QSet<QString> otherMetaCommands();
    virtual void processOtherMetaCommand( const Doc& doc,
					  const QString& command,
					  const QString& arg, Node *node );
    void processOtherMetaCommands( const Doc& doc, Node *node );

private:
    void reset( Tree *tree );
    void readToken();
    const Location& location();
    QString previousLexeme();
    QString lexeme();
    bool match( int target );
    bool matchCompat();
    bool matchTemplateAngles( CodeChunk *type = 0 );
    bool matchTemplateHeader();
    bool matchDataType( CodeChunk *type, QString *var = 0 );
    bool matchParameter( FunctionNode *func );
    bool matchFunctionDecl( InnerNode *parent, QStringList *parentPathPtr = 0,
			    FunctionNode **funcPtr = 0, const QString &templateStuff = QString() );
    bool matchBaseSpecifier( ClassNode *classe, bool isClass );
    bool matchBaseList( ClassNode *classe, bool isClass );
    bool matchClassDecl( InnerNode *parent, const QString &templateStuff = QString() );
    bool matchNamespaceDecl(InnerNode *parent);
    bool matchEnumItem( InnerNode *parent, EnumNode *enume );
    bool matchEnumDecl( InnerNode *parent );
    bool matchTypedefDecl( InnerNode *parent );
    bool matchProperty( InnerNode *parent );
    bool matchDeclList( InnerNode *parent );
    bool matchDocsAndStuff();
    bool makeFunctionNode(const QString &synopsis, QStringList *parentPathPtr,
			  FunctionNode **funcPtr, InnerNode *root = 0);
    void parseQiteratorDotH(const Location &location, const QString &filePath);
    void instantiateIteratorMacro(const QString &container, const QString &includeFile,
				  const QString &macroDef, Tree *tree);
    void createExampleFileNodes(FakeNode *fake);

    QMap<QString, Node::Type> nodeTypeMap;
    Tree *tre;
    Tokenizer *tokenizer;
    int tok;
    Node::Access access;
    FunctionNode::Metaness metaness;
    QString moduleName;
    QStringList lastPath;
    QRegExp varComment;
    QRegExp sep;

    QString sequentialIteratorDefinition;
    QString mutableSequentialIteratorDefinition;
    QString associativeIteratorDefinition;
    QString mutableAssociativeIteratorDefinition;
    QMap<QString, QString> sequentialIteratorClasses;
    QMap<QString, QString> mutableSequentialIteratorClasses;
    QMap<QString, QString> associativeIteratorClasses;
    QMap<QString, QString> mutableAssociativeIteratorClasses;

    static QStringList exampleFiles;
    static QStringList exampleDirs;
};

#endif
