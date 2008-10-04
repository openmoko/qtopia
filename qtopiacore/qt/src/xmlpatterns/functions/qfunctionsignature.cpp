/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtXMLPatterns module of the Qt Toolkit.
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

#include "qxmlname.h"

#include "qfunctionsignature_p.h"

QT_BEGIN_NAMESPACE

using namespace QPatternist;

FunctionSignature::FunctionSignature(const QXmlName nameP,
                                     const Arity minArgs,
                                     const Arity maxArgs,
                                     const SequenceType::Ptr &returnTypeP,
                                     const Expression::Properties props,
                                     const Expression::ID idP) : m_name(nameP),
                                                                 m_minArgs(minArgs),
                                                                 m_maxArgs(maxArgs),
                                                                 m_returnType(returnTypeP),
                                                                 m_arguments(),
                                                                 m_props(props),
                                                                 m_id(idP)
{
    Q_ASSERT(!nameP.isNull());
    Q_ASSERT(minArgs <= maxArgs || maxArgs == FunctionSignature::UnlimitedArity);
    Q_ASSERT(m_maxArgs >= -1);
    Q_ASSERT(returnTypeP);
}

void FunctionSignature::appendArgument(const QXmlName::LocalNameCode nameP,
                                       const SequenceType::Ptr &type)
{
    Q_ASSERT(type);

    m_arguments.append(FunctionArgument::Ptr(new FunctionArgument(QXmlName(StandardNamespaces::empty, nameP), type)));
}

QString FunctionSignature::displayName(const NamePool::Ptr &np) const
{
    QString result;
    result += np->displayName(m_name);
    result += QLatin1Char('(');

    FunctionArgument::List::const_iterator it(m_arguments.constBegin());
    const FunctionArgument::List::const_iterator end(m_arguments.constEnd());

    if(it != end)
    {
        while(true)
        {
            result += QLatin1Char('$');
            result += np->displayName((*it)->name());
            result += QLatin1String(" as ");
            result += (*it)->type()->displayName(np);

            ++it;
            if(it == end)
                break;

            result += QLatin1String(", ");
        }
    }

    if(m_maxArgs == FunctionSignature::UnlimitedArity)
        result += QLatin1String(", ...");

    result += QLatin1String(") as ");
    result += m_returnType->displayName(np);

    return result;
}

bool FunctionSignature::operator==(const FunctionSignature &other) const
{
    return name() == other.name() &&
           isArityValid(other.maximumArguments()) &&
           isArityValid(other.minimumArguments());
}

void FunctionSignature::setArguments(const FunctionArgument::List &args)
{
    m_arguments = args;
}

FunctionArgument::List FunctionSignature::arguments() const
{
    return m_arguments;
}

bool FunctionSignature::isArityValid(const xsInteger arity) const
{
    return arity >= m_minArgs && arity <= m_maxArgs;
}

QXmlName FunctionSignature::name() const
{
    return m_name;
}

FunctionSignature::Arity FunctionSignature::minimumArguments() const
{
    return m_minArgs;
}

FunctionSignature::Arity FunctionSignature::maximumArguments() const
{
    return m_maxArgs;
}

SequenceType::Ptr FunctionSignature::returnType() const
{
    return m_returnType;
}

Expression::Properties FunctionSignature::properties() const
{
    return m_props;
}

Expression::ID FunctionSignature::id() const
{
    return m_id;
}

QT_END_NAMESPACE
