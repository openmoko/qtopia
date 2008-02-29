/****************************************************************************
**
** Copyright (C) 2006-2008 Trolltech ASA. All rights reserved.
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
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROREADER_H
#define PROREADER_H

#include <QtCore/QStack>
#include <QtCore/QList>
#include <QtCore/QTextStream>

class ProBlock;
class ProItem;
class ProFile;

class ProReader
{
public:
    ProReader();
    ProFile *read(const QString &fileName);
    void setEnableBackSlashFixing(bool enable);
    inline int currentLine() const { return m_currentLineNumber; }

protected:
    ProFile *read(QIODevice *device, const QString &name = QLatin1String("device"));

    void writeItem(const QList<ProItem *> &items, int index, QTextStream &out, QString indent);
    ProBlock *currentBlock();
    void updateItem();
    bool parseline(QByteArray line);
    void insertVariable(const QByteArray &line, int *i);
    void insertOperator(const char op);
    void insertComment(const QByteArray &comment);
    void enterScope(bool multiLine);
    void leaveScope();
    void finalizeBlock();
    void cleanup();

private:
    QStack<ProBlock *> m_blockstack;
    ProBlock *m_block;

    ProItem *m_commentItem;
    QByteArray m_proitem;
    QByteArray m_pendingComment;
    bool m_syntaxerror;
    bool m_fixBackSlashes;
    int m_currentLineNumber;
    bool m_contNextLine;
};

#endif //PROREADER_H
