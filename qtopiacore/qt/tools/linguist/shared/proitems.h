/****************************************************************************
**
** Copyright (C) 2006-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Linguist of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef PROITEMS_H
#define PROITEMS_H

#include <QtCore/QObject>
#include <QtCore/QByteArray>
#include <QtCore/QList>

struct AbstractProItemVisitor;

class ProItem {
public:
    enum ProItemKind {
        ValueKind,
        FunctionKind,
        ConditionKind,
        OperatorKind,
        BlockKind
    };
    virtual ~ProItem() {}

    virtual ProItemKind kind() const = 0;

    void setComment(const QByteArray &comment);
    QByteArray comment() const;

    virtual bool Accept(AbstractProItemVisitor *visitor) = 0;
    int getLineNumber() { return m_lineNumber; }
    void setLineNumber(int lineNumber) { m_lineNumber = lineNumber; }

private:
    QByteArray m_comment;
    int m_lineNumber;
};

class ProBlock : public ProItem {
public:
    enum ProBlockKind {
        NormalKind          = 0x00,
        ScopeKind           = 0x01,
        ScopeContentsKind   = 0x02,
        VariableKind        = 0x04,
        ProFileKind         = 0x08,
        SingleLine          = 0x10
    };

    ProBlock(ProBlock *parent);
    ~ProBlock();

    void appendItem(ProItem *proitem);
    void setItems(const QList<ProItem *> &proitems);
    QList<ProItem *> items() const;

    void setBlockKind(int blockKind);
    int blockKind() const;

    void setParent(ProBlock *parent);
    ProBlock *parent() const;

    ProItem::ProItemKind kind() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
protected:
    QList<ProItem *> m_proitems;
private:
    ProBlock *m_parent;
    int m_blockKind;
};

class ProVariable : public ProBlock {
public:
    enum VariableOperator {
        AddOperator         = 0,
        RemoveOperator      = 1,
        ReplaceOperator     = 2,
        SetOperator         = 3,
        UniqueAddOperator   = 4
    };

    ProVariable(const QByteArray &name, ProBlock *parent);

    void setVariableOperator(VariableOperator &variableKind);
    VariableOperator variableOperator() const;

    void setVariable(const QByteArray &name);
    QByteArray variable() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
private:
    VariableOperator m_variableKind;
    QByteArray m_variable;    
};

class ProValue : public ProItem {
public:
    ProValue(const QByteArray &value, ProVariable *variable);

    void setValue(const QByteArray &value);
    QByteArray value() const;

    void setVariable(ProVariable *variable);
    ProVariable *variable() const;

    ProItem::ProItemKind kind() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
private:
    QByteArray m_value;
    ProVariable *m_variable;
};

class ProFunction : public ProItem {
public:
    ProFunction(const QByteArray &text);

    void setText(const QByteArray &text);
    QByteArray text() const;

    ProItem::ProItemKind kind() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
private:
    QByteArray m_text;
};

class ProCondition : public ProItem {
public:
    ProCondition(const QByteArray &text);

    void setText(const QByteArray &text);
    QByteArray text() const;

    ProItem::ProItemKind kind() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
private:
    QByteArray m_text;
};

class ProOperator : public ProItem {
public:
    enum OperatorKind {
        OrOperator      = 1,
        NotOperator     = 2
    };

    ProOperator(OperatorKind operatorKind);

    void setOperatorKind(OperatorKind operatorKind);
    OperatorKind operatorKind() const;

    ProItem::ProItemKind kind() const;

    virtual bool Accept(AbstractProItemVisitor *visitor);
private:
    OperatorKind m_operatorKind;
};

class ProFile : public QObject, public ProBlock {
    Q_OBJECT

public:
    ProFile(const QString &fileName);
    ~ProFile();

    QString displayFileName() const;
    QString fileName() const;

    void setModified(bool modified);
    bool isModified() const;
    
    virtual bool Accept(AbstractProItemVisitor *visitor);

private:
    QString m_fileName;
    QString m_displayFileName;
    bool m_modified;
};

#endif //PROITEMS_H
