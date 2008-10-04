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

#ifndef Patternist_BuiltinTypesFactory_H
#define Patternist_BuiltinTypesFactory_H

#include <QHash>
#include "qschematypefactory_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{

    /**
     * @short Factory for creating schema types for the types defined in XSL-T 2.0.
     *
     * Theses types are essentially the builtin primitive types, plus @c xs:integer,
     * and the types defined in the XPath Data Model.
     *
     * @ingroup Patternist_types
     * @see <a href="http://www.w3.org/TR/xpath-datamodel/#types-predefined">XQuery 1.0 and
     * XPath 2.0 Data Model, 2.6.2 Predefined Types</a>
     * @see <a href="http://www.w3.org/TR/xslt20/#built-in-types">XSL Transformations (XSLT)
     * Version 2.0, 3.13 Built-in Types</a>
     * @see <a href="http://www.w3.org/TR/xmlschema-2/#built-in-primitive-datatypes">XML Schema
     * Part 2: Datatypes Second Edition, 3.2 Primitive datatypes</a>
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class BasicTypesFactory : public SchemaTypeFactory
    {
    public:

        /**
         * Creates a primitive type for @p name. If @p name is not supported,
         * @c null is returned.
         * The intened supported types are the builtin primitive and derived types.
         * That is, the 19 W3C XML Schema types, and the additional 5 in the XPath Data MOdel.
         *
         * @note This does not handle user defined types, only builtin types.
         * @todo Update documentation, proportionally with progress.
         */
        virtual SchemaType::Ptr createSchemaType(const QXmlName ) const;

        virtual SchemaType::Hash types() const;

        /**
         * @returns the singleton instance of BasicTypesFactory.
         */
        static SchemaTypeFactory::Ptr self(const NamePool::Ptr &np);

    protected:
        /**
         * This constructor is protected. Use the static self() function
         * to retrieve a singleton instance.
         */
        BasicTypesFactory(const NamePool::Ptr &np);

    private:
        /**
         * A dictonary of builtin primitive and derived primitives.
         */
        SchemaType::Hash m_types;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
