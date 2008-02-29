/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include "bookmarkmodel.h"
#include "bookmark.h"

#include <QModelIndex>
#include <QVariant>
#include <QFile>
#include <QDomDocument>
#include <QTextStream>
#include <Qtopia>


/*!
  \internal
  \class BookmarkModel
  \brief Data model for a list of Bookmark objects.

  \mainclass
  Holds a list of Bookmark objects, allowing multiple modifications to be made to the list's
  underlying data within a single callback. The data is not sorted, so ordering may be
  imposed artificially.

  \sa BookmarkList
  \sa Bookmark
*/


/*! \fn BookmarkModel::BookmarkModel(QObject *parent)
   Object initialisation. Calls load().
*/
BookmarkModel::BookmarkModel(QObject *parent)
    : QAbstractListModel(parent)
{
    load();
}

/*! \fn int BookmarkModel::rowCount(const QModelIndex &parent = QModelIndex()) const
   Returns the number of items in the data.
*/
int BookmarkModel::rowCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent);

    return dat.count();
}

/*! \fn QModelIndex BookmarkModel::createIdx(int row)
  Creates and returns an index for the given row.
*/
QModelIndex BookmarkModel::createIdx(int row)
{
    return createIndex(row,0);
}

/*! \fn QVariant BookmarkModel::data(const QModelIndex & index, int role = Qt::DisplayRole) const
  Returns a QVariant object for the data at the given index, for the given role. Roles supported
  are: Qt::TextAlignmentRole and Qt::DisplayRole. Typically, this method is used by the view(s)
  to retrieve information prior to display.
*/
QVariant BookmarkModel::data(const QModelIndex & index,int role) const
{
    if ( !index.isValid() ) {
        return QVariant();
    }

    if ( role == Qt::TextAlignmentRole ) {
        return int(Qt::AlignLeft | Qt::AlignVCenter);
    }
    if ( role == Qt::DisplayRole ) {
        if ( (index.row() < 0) || index.row() >= dat.count() ) {
            // Bad row.
            return "";
        }
        return dat.at(index.row());
    }

    return QVariant();
}

/*! \fn bool BookmarkModel::contains(const Bookmark &bookmark)
 */
bool BookmarkModel::contains(const Bookmark &bookmark)
{
    // Unfortunately, cannot use dat.contains(item) here, since QVariant's operator==
    // doesn't support custom types registered with qRegisterMetaType(). Therefore, search
    // the list.
    // !!!!!!! TODO this being the case, there is no reason why we can't just maintain the
    // Bookmarks in the list, and get data(...) to create new QVariants each time - as it
    // returns by value anyway, there would be no less efficiency. In fact, this is probably
    // how you're meant to do it...
    for( QVector<QVariant>::const_iterator i = dat.begin(); i != dat.end(); i++ ) {
        Bookmark b = (*i).value<Bookmark>();
        if ( b == bookmark ) {
            // ALready have this one.
            return true;
        }
    }

    return false;
}

/*!
    Adds a new Bookmark to the list's data. If the Bookmark already exists
    within the list, it will not be added. If refresh is true, the view will
    also be updated, consequently it is possible to add multiple items to the
    list before updating it.

    Returns true if the Bookmark was added, false if it was not, e.g. because
    that Bookmark already exists in the list.
*/
bool BookmarkModel::addItem(const Bookmark &bookmark,bool refresh)
{
    // TODO!!! Change this to be void return, and force the callingn code to call contains(.)
    // if required.

    // Wrap the Bookmark within a QVariant object.
    QVariant item = QVariant::fromValue(bookmark);

    // We don't want to add bookmarks multiple times, so we need to check that the data set
    // does not already contain bookmark.
    if ( contains(bookmark) ) {
        return false;
    }

    dat.append(item);

    // Notify the view(s) that we've got a change, ie that we've added one at the end of the list.
    if ( refresh ) {
        reset();
    }

    return true;
}

/*! \fn void BookmarkModel::insert(int row,const Bookmark &bookmark,bool refresh=false)
  Inserts the given Bookmark at the row index. Does not check for duplicate items.
*/
void BookmarkModel::insert(int row,const Bookmark &bookmark,bool refresh)
{
    // Wrap the Bookmark within a QVariant object.
    QVariant item = QVariant::fromValue(bookmark);

    dat.insert(row,1,item);
    if ( refresh ) {
        reset();
    }
}

/*!
  Removes the item at the given row, and notifies the view(s) that the data has changed.
  Returns false if row is invalid.
*/
bool BookmarkModel::remove(int row,bool refresh)
{
    if ( (row < 0) || (row >= rowCount()) ) {
        return false;
    }

    dat.remove(row);

    if ( refresh) {
        reset();
    }
    return true;
}

/*! \fn Bookmark BookmarkModel::getItem(int row) const
  Retrieves the item at the given row. Assumes that row is valid.
*/
Bookmark BookmarkModel::getItem(int row) const
{
    const QVariant &variant = dat.at(row);
    return variant.value<Bookmark>();
}

// Loads in the bookmarks data from secondary storage.
// Called indirectly by ctor.
void BookmarkModel::load()
{
    // Create the XML document.
    QDomDocument dataDocument;

    // Create the QFile, if necessary.
    QFile dataFile(Qtopia::applicationFileName("HelpBrowser","bookmarks.xml"));

    if ( dataFile.exists() ) {
        // Load it.
        if ( !(dataFile.open(QIODevice::ReadOnly | QIODevice::Text)) ) {
            // Could not open the file...
            qWarning("BookmarkModel::load() - Could not load bookmarks data; could not open file.");
            return;
        }

        // Now load into dataDocument.
        if ( !dataDocument.setContent(&dataFile) ) {
            qWarning("BookmarkModel::load() - Could not load bookmarks data; could not read.");
            dataFile.close();
            return;
        }

        dataFile.close();

        // Pass the contents of dataDocument to the list.
        QDomElement docElem = dataDocument.documentElement();
        QDomNode node = docElem.firstChild();
        while (!node.isNull()) {
            // Try to convert the next node to an element.
            QDomElement elem = node.toElement();
            if (!elem.isNull()) {
                // The node is truly an element, so we can query it for its attributes.
                Bookmark bookmark(elem);
                addItem(bookmark,false);
            }
            node = node.nextSibling();
        }
    }
}

// Writes the current set of bookmarks to secondary storage.
void BookmarkModel::write()
{
    // Build a DOM document, using the data in the list.
    QDomDocument dataDocument;
    // Give dataDocument a root node.
    QDomElement root = dataDocument.createElement("Bookmarks");
    dataDocument.appendChild(root);

    // Iterate through the list's contents, adding each item to the document.
    for ( int row = 0; row < rowCount(); row++ ) {
        Bookmark bookmark = getItem(row);
        // Store the bookmark as XML data.
        bookmark.appendAsXML(dataDocument,root);
    }

    // Create the file to write out the data.
    QFile dataFile(Qtopia::applicationFileName("HelpBrowser","bookmarks.xml"));
    if ( dataFile.open(QFile::WriteOnly | QFile::Text) ) {
        QTextStream outStream(&dataFile);
        dataDocument.save(outStream,DATAFILE_INDENT);
        dataFile.close();
    } else {
        qWarning("BookmarkModel::write() - Could not save Bookmarks data.");
    }
}
