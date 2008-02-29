/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QSVGSTRUCTURE_P_H
#define QSVGSTRUCTURE_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qsvgnode_p.h"

#include "QtCore/qlist.h"
#include "QtCore/qhash.h"

class QSvgTinyDocument;
class QSvgNode;
class QPainter;
class QSvgDefs;

class QSvgStructureNode : public QSvgNode
{
public:
    QSvgStructureNode(QSvgNode *parent);
    ~QSvgStructureNode();
    QSvgNode *scopeNode(const QString &id) const;
    QSvgStyleProperty *scopeStyle(const QString &id) const;
    void addChild(QSvgNode *child, const QString &id, bool def = false);
    virtual QRectF bounds() const;
    virtual QRectF transformedBounds(const QMatrix &mat) const;
    QSvgNode *previousSiblingNode(QSvgNode *n) const;
protected:
    QList<QSvgNode*>          m_renderers;
    QHash<QString, QSvgNode*> m_scope;
    QList<QSvgStructureNode*> m_linkedScopes;
    mutable QRectF m_bounds;
};

class QSvgG : public QSvgStructureNode
{
public:
    QSvgG(QSvgNode *parent);
    virtual void draw(QPainter *p);
    Type type() const;
};

class QSvgDefs : public QSvgStructureNode
{
public:
    QSvgDefs(QSvgNode *parent);
    virtual void draw(QPainter *p);
    Type type() const;
};

class QSvgSwitch : public QSvgStructureNode
{
public:
    QSvgSwitch(QSvgNode *parent);
    virtual void draw(QPainter *p);
    Type type() const;
private:
    void init();
private:
    QString m_systemLanguage;
    QString m_systemLanguagePrefix;
private:
    static QHash<QString, bool> m_features;
    static QHash<QString, bool> m_extensions;
};

#endif // QSVGSTRUCTURE_P_H
