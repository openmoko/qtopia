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

#ifndef FEATURETREEMODEL_H
#define FEATURETREEMODEL_H

#include <QAbstractItemModel>
#include <QMap>
#include <QHash>
#include <QTextStream>

class Feature;
class Node;

uint qHash(const QModelIndex&);

class FeatureTreeModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    FeatureTreeModel(QObject *parent = 0);
    ~FeatureTreeModel();

    void clear();
    
    QVariant data(const QModelIndex &index, int role) const;
    bool setData(const QModelIndex &index, const QVariant &value, int role);
    Qt::ItemFlags flags(const QModelIndex &index) const;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const;
    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const;
    QModelIndex index(const Feature *feature) const;    
    QModelIndex parent(const QModelIndex &index) const;
    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;

    void addFeature(Feature *feature);    
    Feature* getFeature(const QModelIndex &index) const;

    void readConfig(QTextStream &stream);
    void writeConfig(QTextStream &stream) const;
    
public slots:
    void featureChanged();
    
private:
    QModelIndex createIndex(int row, int column,
                            const QModelIndex &parent,
                            const Node *feature) const;    
    QModelIndex index(const QModelIndex &parent, const Feature *feature) const;
    bool contains(const QString &section, const Feature *f) const;
    Node* find(const QString &section, const Feature *f) const;
    QStringList findDisabled(const QModelIndex &parent) const;    
    
    QMap<QString, QList<Node*> > sections;
    mutable QHash<QModelIndex, QModelIndex> parentMap;
    mutable QHash<const Feature*, QModelIndex> featureIndexMap;    
};

#endif // FEATURETREEMODEL_H
