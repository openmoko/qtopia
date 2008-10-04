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

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.


#ifndef Patternist_FunctionFactory_H
#define Patternist_FunctionFactory_H

#include <QHash>
#include <QSharedData>

#include "qexpression_p.h"
#include "qfunctionsignature_p.h"
#include "qprimitives_p.h"
#include "qxmlname.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short An entry point for looking up and creating FunctionCall instances.
     *
     * @ingroup Patternist_functions
     * @see <a href ="http://www.w3.org/TR/xpath-functions/">XQuery 1.0
     * and XPath 2.0 Functions and Operators</a>
     * @see <a href="http://www.w3.org/TR/xpath20/#dt-function-signature">XML Path
     * Language (XPath) 2.0, Definition: Function signatures</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class FunctionFactory : public QSharedData
    {
    public:

        typedef QExplicitlySharedDataPointer<FunctionFactory> Ptr;
        typedef QList<FunctionFactory::Ptr> List;

        virtual ~FunctionFactory();

        /**
         * Creates a function call implementation.
         *
         * A FunctionFactory represents a set of functions, which it
         * is able to instantiate and to serve FunctionSignatures for. Conventionally,
         * a FunctionFactory per namespace exists.
         *
         * @note This function should not issue any error unless it is absolutely
         * confident that the error cannot be fixed in another way. For example, in
         * some cases it might be that a function is available in another FunctionFactory
         * and it would therefore be wrong to issue an error signalling that no function
         * by that @p name exists, but leave that to the callee.
         * @param name the name of the function to create. In Clark syntax, this could
         * for example be {http://www.w3.org/2005/04/xpath-functions}lower-case
         * @param arguments the function's operands
         * @param context the usual StaticContext which supplies compile time data
         * and reporting functionality.
         * @param r the SourceLocationReflection that identifies the callsite.
         * @returns an instance of Expression which is the function implementation
         * for @p name. Or, a static error was raised.
         */
        virtual Expression::Ptr createFunctionCall(const QXmlName name,
                                                   const Expression::List &arguments,
                                                   const StaticContext::Ptr &context,
                                                   const SourceLocationReflection *const r) = 0;

        /**
         * Determines whether a function with the name @p name and arity @p arity
         * is available. The implementation operates on the result of
         * retrieveFunctionSignature() to determine the result.
         *
         * @param np the NamePool.
         * @param name the name of the function. For example fn:string-join.
         * @param arity the number of arguments the function must have.
         */
        virtual bool isAvailable(const NamePool::Ptr &np,
                                 const QXmlName name,
                                 const xsInteger arity);

        virtual FunctionSignature::Hash functionSignatures() const = 0;

        /**
         * Determines whether this FunctionFactory contains the function signature
         * @p signature.
         *
         * The implementation uses functionSignatures().
         */
        bool hasSignature(const FunctionSignature::Ptr &signature) const;

    protected:
        /**
         * @short This constructor cannot be removed, because it can't be synthesized, for
         * some reason.
         */
        inline FunctionFactory()
        {
        }

        /**
         * This is a convenience function for sub-classes. It retrieves the
         * function signature for function with name @p name.
         *
         * According to the specifications are function signatures identified by their
         * name and arity, but currently is the arity not part of the signature.
         *
         * If no function could be found for the given name, @c null is returned.
         */
        virtual FunctionSignature::Ptr retrieveFunctionSignature(const NamePool::Ptr &np, const QXmlName name) = 0;

    private:
        Q_DISABLE_COPY(FunctionFactory)
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
