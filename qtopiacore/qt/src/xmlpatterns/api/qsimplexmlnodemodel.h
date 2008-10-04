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

#ifndef QSIMPLEXMLNODEMODEL_H
#define QSIMPLEXMLNODEMODEL_H

#include <QtXmlPatterns/QAbstractXmlNodeModel>
#include <QtXmlPatterns/QXmlQuery>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(XmlPatterns)

template<typename T> class QExplicitlySharedDataPointer;
class QSimpleXmlNodeModelPrivate;
class Q_XMLPATTERNS_EXPORT QSimpleXmlNodeModel : public QAbstractXmlNodeModel
{
public:
    QSimpleXmlNodeModel(const QXmlNamePool &namePool);
    virtual ~QSimpleXmlNodeModel();

    virtual QUrl baseUri(const QXmlNodeModelIndex &node) const;
    QXmlNamePool &namePool() const;
    virtual QVector<QXmlName> namespaceBindings(const QXmlNodeModelIndex&) const;
    virtual QString stringValue(const QXmlNodeModelIndex &node) const;
    virtual QXmlNodeModelIndex elementById(const QXmlName &id) const;
    virtual QVector<QXmlNodeModelIndex> nodesByIdref(const QXmlName &idref) const;

private:
    Q_DECLARE_PRIVATE(QSimpleXmlNodeModel)
};

QT_END_NAMESPACE

QT_END_HEADER

#endif
