/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/
#include "mergeitem.h"

#include <QXmlStreamWriter>
#include <QDebug>

#include "merge.h"

//#define DEBUG_MERGE

class MergeElement
{
public:
    MergeElement() : parent(0), matchingItemCount(0) {}
    virtual ~MergeElement() {
        foreach(MergeElement *child, items)
            delete child;
        items.clear();
    }

    enum Type {
        Simple,
        Identifier,
    };

    virtual Type type() const { return Simple; }

    void addChild(MergeElement *child);
    void finalize();

    MergeElement *child(const QString &nameSpace, const QString &name) {
        foreach(MergeElement *item, items) {
            if (item->namespaceUri == nameSpace && item->name == name)
                return item;
        }
        return 0;
    }


    QString namespaceUri;
    QString name;
    QString text;
    QXmlStreamAttributes attributes;

    // and the sub elements
    QList<MergeElement *> items;
    MergeElement *parent;

    int matchingItemCount;
};

#ifdef DEBUG_MERGE
QString dumpItems(const QList<MergeElement*> &items)
{
    QString ret = "\n";
    QTextStream stream(&ret);
    foreach (const MergeElement *item, items) {
        stream << "<" << item->name << ">";
        if (item->items.count())
            stream << dumpItems(item->items).toLocal8Bit().constData();
        else
            stream << item->text;
        stream << "</" << item->name << ">" << endl;
    }
    return ret;
}
#define RETURN_REASON(ret,reason) {qDebug() << "returning" << ret << "because:" << #reason << "(" << left->reason << "vs" << right->reason << ")"; return ret;}
#else
#define RETURN_REASON(ret,reason) return ret;
#endif

bool mergeElementLessThan(const MergeElement *left, const MergeElement *right)
{
    if (!left || !right) {
        if (right)
            return true;
        return false;
    }

    if (left->namespaceUri < right->namespaceUri)
        RETURN_REASON(true,namespaceUri);
    if (left->name < right->name)
        RETURN_REASON(true,name);
    if (left->text != right->text) {
        if (left->text.isEmpty())
            RETURN_REASON(false,text);
        if (right->text.isEmpty())
            RETURN_REASON(true,text);
    }
    if (left->text < right->text)
        RETURN_REASON(true,text);
    if (left->attributes.count() < right->attributes.count())
        RETURN_REASON(true,attributes.count());
    if (left->attributes.count() > right->attributes.count())
        RETURN_REASON(false,attributes.count());
    if (left->items.count() < right->items.count())
        RETURN_REASON(true,items.count());
    if (left->items.count() > right->items.count())
        RETURN_REASON(false,items.count());
    for (int i = 0; i < left->items.count(); ++i) {
        if (mergeElementLessThan(left->items[i], right->items[i]))
            RETURN_REASON(true,items[i]);
    }
    for (int i = 0; i < left->attributes.count(); ++i) {
        QXmlStreamAttribute attr = left->attributes[i];
        QXmlStreamAttribute attrOther = right->attributes[i];
        if (attr.namespaceUri() < attrOther.namespaceUri())
            RETURN_REASON(true,attributes[i].namespaceUri().toString());
        if (attr.name() < attrOther.name())
            RETURN_REASON(true,attributes[i].name().toString());
        if (attr.value() < attrOther.value())
            RETURN_REASON(true,attributes[i].value().toString());
    }
#ifdef DEBUG_MERGE
    qDebug() << "returning false because: default case";
#endif
    return false;
}

class MergeIdentifierElement : public MergeElement
{
public:
    Type type() const { return Identifier; }
    QString clientid;
    QString serverid;
};

void MergeElement::addChild(MergeElement *child)
{
    if (items.count()) {
        MergeElement *item = items.last();
        if (item->name == child->name && item->namespaceUri == child->namespaceUri) {
            matchingItemCount++;
        } else {
#ifdef DEBUG_MERGE
            QString before = dumpItems(items);
            qDebug() << "Sorting...";
#endif
            qSort(items.end()-matchingItemCount, items.end(), mergeElementLessThan);
#ifdef DEBUG_MERGE
            QString after = dumpItems(items);
            if ( before != after )
                qDebug() << "addChild: Sorting changed items from" << before << "to" << after;
#endif
            matchingItemCount = 1;
        }
    } else {
        matchingItemCount = 1;
    }
    child->parent = this;
    items.append(child);
}

void MergeElement::finalize()
{
    if (matchingItemCount > 1) {
#ifdef DEBUG_MERGE
        QString before = dumpItems(items);
        qDebug() << "Sorting...";
#endif
        qSort(items.end()-matchingItemCount, items.end(), mergeElementLessThan);
#ifdef DEBUG_MERGE
        QString after = dumpItems(items);
        if ( before != after )
            qDebug() << "finalize: Sorting changed items from" << before << "to" << after;
#endif
    }
}

MergeItem::MergeItem(const QSyncMerge *parent)
    : rootElement(0), mMerge(parent)
{
}

MergeItem::~MergeItem()
{
    if (rootElement)
        delete rootElement;
}

QByteArray MergeItem::write(ChangeSource source) const
{
    if (!rootElement)
        return QByteArray();
    QByteArray result;
    QXmlStreamWriter writer(&result);
    writer.setAutoFormatting(true);

    writeElement(writer, rootElement, source);
    //qDebug() << "written" << QString::fromUtf8(result);
    return result;
}

void MergeItem::writeElement(QXmlStreamWriter &writer, MergeElement *item, ChangeSource source) const
{
    switch(item->type()) {
        case MergeElement::Simple:
            writer.writeStartElement(item->namespaceUri, item->name);
            foreach(const QXmlStreamAttribute &attr, item->attributes) {
                if (source != DataOnly || attr.name() != "maxItems")
                    writer.writeAttribute(attr);
            }
            if (item->items.count())
            {
                // recurse
                foreach(MergeElement *e, item->items) {
                    writeElement(writer, e, source);
                }
            } else {
                writer.writeCharacters(item->text);
            }
            writer.writeEndElement();
            break;
        case MergeElement::Identifier:
            {
                writer.writeStartElement(item->namespaceUri, item->name);
                MergeIdentifierElement *ident = (MergeIdentifierElement *)item;
                switch(source) {
                    case Client:
                        if (!ident->clientid.isEmpty()) {
                            writer.writeCharacters(ident->clientid);
                        } else if (!ident->serverid.isEmpty() && mMerge->canMap(ident->serverid, QSyncMerge::Server)) {
                            writer.writeCharacters(mMerge->map(ident->serverid, QSyncMerge::Server));
                        } else {
                            writer.writeAttribute("localIdentifier", "false");
                            writer.writeCharacters(ident->serverid);
                        }
                        break;
                    case Server:
                        if (!ident->serverid.isEmpty()) {
                            writer.writeCharacters(ident->serverid);
                        } else if (!ident->clientid.isEmpty() && mMerge->canMap(ident->clientid, QSyncMerge::Client)) {
                            writer.writeCharacters(mMerge->map(ident->clientid, QSyncMerge::Client));
                        } else {
                            writer.writeAttribute("localIdentifier", "false");
                            writer.writeCharacters(ident->clientid);
                        }
                        break;
                    case IdentifierOnly:
                        writer.writeAttribute("clientIdentifier", ident->clientid);
                        writer.writeAttribute("serverIdentifier", ident->serverid);
                        break;
                    case DataOnly:
                        break;
                }
                writer.writeEndElement();
            }
            break;
    }
}

void MergeItem::read(const QByteArray &data, ChangeSource source)
{
    if (rootElement) {
        delete rootElement;
        rootElement = 0;
    }
    MergeElement *currentElement = 0;

    QXmlStreamReader reader(data);

    QXmlStreamAttributes identAttr;
    while (!reader.atEnd()) {
        switch(reader.readNext()) {
            // ignored token types.
            case QXmlStreamReader::NoToken:
            case QXmlStreamReader::Invalid:
            case QXmlStreamReader::EntityReference:
            case QXmlStreamReader::ProcessingInstruction:
            case QXmlStreamReader::StartDocument:
            case QXmlStreamReader::EndDocument:
            case QXmlStreamReader::Comment:
            case QXmlStreamReader::DTD:
                break;
            case QXmlStreamReader::StartElement:
                MergeElement *e;
                if (reader.qualifiedName() == "Identifier")
                    e = new MergeIdentifierElement;
                else
                    e = new MergeElement;

                e->attributes = reader.attributes();
                e->name = reader.name().toString();
                e->namespaceUri = reader.namespaceUri().toString();

                if (rootElement) {
                    currentElement->addChild(e);
                } else {
                    rootElement = e;
                }
                currentElement = e;

                break;
            case QXmlStreamReader::EndElement:
                // assumption, properly formed xml item, only one root
                // element.  Hence if parent is 0, its an error.
                // of course crashing isn't acceptable...

                if (currentElement->type() == MergeElement::Identifier) {
                    MergeIdentifierElement *ident = (MergeIdentifierElement *)currentElement;
                    switch(source) {
                        case Client:
                            ident->clientid = ident->text;
                            break;
                        case Server:
                            ident->serverid = ident->text;
                            break;
                        case IdentifierOnly:
                            ident->clientid = ident->attributes.value("clientIdentifier").toString();
                            ident->serverid = ident->attributes.value("serverIdentifier").toString();
                            break;
                        case DataOnly:
                            break;
                    }
                    ident->attributes.clear(); // no other recognized attr
                }

                currentElement->finalize();
                currentElement = currentElement->parent;
                if (!currentElement)
                    return;
                break;
            case QXmlStreamReader::Characters:
                if (!reader.isWhitespace())
                    currentElement->text = reader.text().toString();
                break;
        }
    }
}

QString MergeItem::serverIdentifier() const
{
    if (rootElement && rootElement->items.count()) {
        foreach(MergeElement *item, rootElement->items) {
            if (item->type() == MergeElement::Identifier) {
                MergeIdentifierElement *ident = (MergeIdentifierElement *)item;
                if (!ident->serverid.isEmpty())
                    return ident->serverid;
                else if (!ident->clientid.isEmpty() && mMerge->canMap(ident->clientid, QSyncMerge::Client))
                    return mMerge->map(ident->clientid, QSyncMerge::Client);
                break;
            }
        }
    }
    return QString();
}

QString MergeItem::clientIdentifier() const
{
    if (rootElement && rootElement->items.count()) {
        foreach(MergeElement *item, rootElement->items) {
            if (item->type() == MergeElement::Identifier) {
                MergeIdentifierElement *ident = (MergeIdentifierElement *)item;
                if (!ident->clientid.isEmpty())
                    return ident->clientid;
                else if (!ident->serverid.isEmpty() && mMerge->canMap(ident->serverid, QSyncMerge::Server))
                    return mMerge->map(ident->serverid, QSyncMerge::Server);
                break;
            }
        }
    }
    return QString();
}

/*
   Modifies the current merge item to only include top level fields also
   included in the given \a reference item.  This includes trimming list
   lengths to the reference items list lengths.
*/
void MergeItem::restrictTo(const MergeItem &reference)
{
    MergeElement *item, *referenceItem, *referenceRoot;
    QMutableListIterator<MergeElement *> it(rootElement->items);

    // assert rit.hasNext(); 
    // after all, shouldn't see items in reference that are not in target
    // top level
    referenceRoot = reference.rootElement;
    while(it.hasNext()) {
        item = it.next();
        referenceItem = referenceRoot->child(item->namespaceUri, item->name);
        if (referenceItem) {
            QXmlStreamAttributes replacement;
            // assuming attributes returns null for no key and empty for key but empty,
            // this should properly create a subset of attributes.
            for (int i = 0; i < referenceItem->attributes.count(); ++i) {
                QString aname = referenceItem->attributes[i].qualifiedName().toString();
                QString value = item->attributes.value(aname).toString();
                if (!value.isNull())
                    replacement.append(aname, value);
            }
            // make a quick check on lists length (already sorted in both
            if (!referenceItem->attributes.value("maxItems").isEmpty()) {
                bool ok;
                int max = referenceItem->attributes.value("maxItems").toString().toInt(&ok);
                if (ok && max > 0) {
                    QList<MergeElement *> excess = item->items.mid(max);
                    item->items = item->items.mid(0, max);
                    foreach(MergeElement *trash, excess)
                        delete trash;
                }
            }
        } else {
            it.remove();
        }
    }
}
