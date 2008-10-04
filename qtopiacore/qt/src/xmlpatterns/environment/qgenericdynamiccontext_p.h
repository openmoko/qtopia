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

#ifndef Patternist_GenericDynamicContext_H
#define Patternist_GenericDynamicContext_H

#include <QDateTime>
#include <QVector>

#include "qdaytimeduration_p.h"
#include "qstackcontextbase_p.h"
#include "qexpression_p.h"

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

namespace QPatternist
{
    /**
     * @short A DynamicContext supplying basic information that always is used.
     *
     * This DynamicContext is the first DynamicContext used during
     * a run and is always used. In addition, more contexts, such as
     * a Focus can be created.
     *
     * @author Frans Englich <fenglich@trolltech.com>
     */
    class GenericDynamicContext : public StackContextBase<DynamicContext>
    {
    public:
        typedef QExplicitlySharedDataPointer<GenericDynamicContext> Ptr;

        GenericDynamicContext(const NamePool::Ptr &np,
                              QAbstractMessageHandler *const messageHandler,
                              const LocationHash &locations);

        virtual xsInteger contextPosition() const;
        /**
         * @returns always @c null, the focus is always undefined when an GenericDynamicContext
         * is used.
         */
        virtual Item contextItem() const;
        virtual xsInteger contextSize();

        virtual void setFocusIterator(const Item::Iterator::Ptr &it);
        virtual Item::Iterator::Ptr focusIterator() const;

        virtual QAbstractMessageHandler * messageHandler() const;
        virtual QExplicitlySharedDataPointer<DayTimeDuration> implicitTimezone() const;
        virtual QDateTime currentDateTime() const;

        virtual QAbstractXmlReceiver *outputReceiver() const;
        void setOutputReceiver(QAbstractXmlReceiver *const receiver);

        virtual NodeBuilder::Ptr nodeBuilder(const QUrl &baseURI) const;
        void setNodeBuilder(NodeBuilder::Ptr &builder);

        virtual ResourceLoader::Ptr resourceLoader() const;
        void setResourceLoader(const ResourceLoader::Ptr &loader);

        virtual ExternalVariableLoader::Ptr externalVariableLoader() const;
        void setExternalVariableLoader(const ExternalVariableLoader::Ptr &loader);
        virtual NamePool::Ptr namePool() const;
        virtual QSourceLocation locationFor(const SourceLocationReflection *const reflection) const;
        virtual void addNodeModel(const QAbstractXmlNodeModel::Ptr &nm);
        virtual const QAbstractUriResolver *uriResolver() const;
        virtual ItemCacheCell &globalItemCacheCell(const VariableSlotID slot);
        virtual ItemSequenceCacheCell::Vector &globalItemSequenceCacheCells(const VariableSlotID slot);

        void setUriResolver(const QAbstractUriResolver *const resolver);

    private:
        QAbstractMessageHandler *       m_messageHandler;
        const QDateTime                 m_currentDateTime;
        const DayTimeDuration::Ptr      m_zoneOffset;
        QAbstractXmlReceiver *          m_outputReceiver;
        mutable NodeBuilder::Ptr        m_nodeBuilder;
        ExternalVariableLoader::Ptr     m_externalVariableLoader;
        ResourceLoader::Ptr             m_resourceLoader;
        NamePool::Ptr                   m_namePool;
        const LocationHash              m_locations;
        QAbstractXmlNodeModel::List     m_nodeModels;
        const QAbstractUriResolver *    m_uriResolver;
        ItemCacheCell::Vector           m_globalItemCacheCells;
        ItemSequenceCacheCell::Vector   m_globalItemSequenceCacheCells;
    };
}

QT_END_NAMESPACE

QT_END_HEADER

#endif
