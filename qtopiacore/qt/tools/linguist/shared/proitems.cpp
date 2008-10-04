/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#include <QtCore/QFileInfo>

#include "proitems.h"
#include "abstractproitemvisitor.h"

QT_BEGIN_NAMESPACE
// --------------- ProItem ------------
void ProItem::setComment(const QByteArray &comment)
{
    m_comment = comment;
}

QByteArray ProItem::comment() const
{
    return m_comment;
}

// --------------- ProBlock ----------------
ProBlock::ProBlock(ProBlock *parent)
{
    m_blockKind = 0;
    m_parent = parent;
}

ProBlock::~ProBlock()
{
    qDeleteAll(m_proitems);
}

void ProBlock::appendItem(ProItem *proitem)
{
    m_proitems << proitem;
}

void ProBlock::setItems(const QList<ProItem *> &proitems)
{
    m_proitems = proitems;
}

QList<ProItem *> ProBlock::items() const
{
    return m_proitems;
}

void ProBlock::setBlockKind(int blockKind)
{
    m_blockKind = blockKind;
}

int ProBlock::blockKind() const
{
    return m_blockKind;
}

void ProBlock::setParent(ProBlock *parent)
{
    m_parent = parent;
}

ProBlock *ProBlock::parent() const
{
    return m_parent;
}

ProItem::ProItemKind ProBlock::kind() const
{
    return ProItem::BlockKind;
}

bool ProBlock::Accept(AbstractProItemVisitor *visitor)
{
    visitor->visitBeginProBlock(this);
    for (QList<ProItem *>::iterator it = m_proitems.begin(); it != m_proitems.end(); ++it) {
        if (!(*it)->Accept(visitor))
            return false;
    }
    return visitor->visitEndProBlock(this);
}

// --------------- ProVariable ----------------
ProVariable::ProVariable(const QByteArray &name, ProBlock *parent)
    : ProBlock(parent)
{
    setBlockKind(ProBlock::VariableKind);
    m_variable = name;
    m_variableKind = SetOperator;
}

void ProVariable::setVariableOperator(VariableOperator &variableKind)
{
    m_variableKind = variableKind;
}

ProVariable::VariableOperator ProVariable::variableOperator() const
{
    return m_variableKind;
}

void ProVariable::setVariable(const QByteArray &name)
{
    m_variable = name;
}

QByteArray ProVariable::variable() const
{
    return m_variable;
}

bool ProVariable::Accept(AbstractProItemVisitor *visitor)
{
    visitor->visitBeginProVariable(this);
    for (QList<ProItem *>::iterator it = m_proitems.begin(); it != m_proitems.end(); ++it) {
        if (!(*it)->Accept(visitor))
            return false;
    }
    return visitor->visitEndProVariable(this);
}

// --------------- ProValue ----------------
ProValue::ProValue(const QByteArray &value, ProVariable *variable)
{
    m_variable = variable;
    m_value = value;
}

void ProValue::setValue(const QByteArray &value)
{
    m_value = value;
}

QByteArray ProValue::value() const
{
    return m_value;
}

void ProValue::setVariable(ProVariable *variable)
{
    m_variable = variable;
}

ProVariable *ProValue::variable() const
{
    return m_variable;
}

ProItem::ProItemKind ProValue::kind() const
{
    return ProItem::ValueKind;
}

bool ProValue::Accept(AbstractProItemVisitor *visitor)
{
    return visitor->visitProValue(this);
}

// --------------- ProFunction ----------------
ProFunction::ProFunction(const QByteArray &text)
{
    m_text = text;
}

void ProFunction::setText(const QByteArray &text)
{
    m_text = text;
}

QByteArray ProFunction::text() const
{
    return m_text;
}

ProItem::ProItemKind ProFunction::kind() const
{
    return ProItem::FunctionKind;
}

bool ProFunction::Accept(AbstractProItemVisitor *visitor)
{
    return visitor->visitProFunction(this);
}

// --------------- ProCondition ----------------
ProCondition::ProCondition(const QByteArray &text)
{
    m_text = text;
}

void ProCondition::setText(const QByteArray &text)
{
    m_text = text;
}

QByteArray ProCondition::text() const
{
    return m_text;
}

ProItem::ProItemKind ProCondition::kind() const
{
    return ProItem::ConditionKind;
}

bool ProCondition::Accept(AbstractProItemVisitor *visitor)
{
    return visitor->visitProCondition(this);
}

// --------------- ProOperator ----------------
ProOperator::ProOperator(OperatorKind operatorKind)
{
    m_operatorKind = operatorKind;
}

void ProOperator::setOperatorKind(OperatorKind operatorKind)
{
    m_operatorKind = operatorKind;
}

ProOperator::OperatorKind ProOperator::operatorKind() const
{
    return m_operatorKind;
}

ProItem::ProItemKind ProOperator::kind() const
{
    return ProItem::OperatorKind;
}

bool ProOperator::Accept(AbstractProItemVisitor *visitor)
{
    return visitor->visitProOperator(this);
}

// --------------- ProFile ----------------
ProFile::ProFile(const QString &fileName)
    : QObject(), ProBlock(0)
{
    m_modified = false;
    setBlockKind(ProBlock::ProFileKind);
    m_fileName = fileName;

    QFileInfo fi(fileName);
    m_displayFileName = fi.fileName();
}

ProFile::~ProFile()
{

}

QString ProFile::displayFileName() const
{
    return m_displayFileName;
}

QString ProFile::fileName() const
{
    return m_fileName;
}

void ProFile::setModified(bool modified)
{
    m_modified = modified;
}

bool ProFile::isModified() const
{
    return m_modified;
}


bool ProFile::Accept(AbstractProItemVisitor *visitor)
{
    visitor->visitBeginProFile(this);
    for (QList<ProItem *>::iterator it = m_proitems.begin(); it != m_proitems.end(); ++it) {
        if (!(*it)->Accept(visitor))
            return false;
    }
    return visitor->visitEndProFile(this);
}

QT_END_NAMESPACE
