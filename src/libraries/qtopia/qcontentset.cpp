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
#include <qcontentset.h>
#include <private/contentserverinterface_p.h>
#include <private/contentlnksql_p.h>
#include <private/drmcontent_p.h>

#include <qtopiasql.h>
#include <qmimetype.h>
#include <qstorage.h>
#include <qtopiaipcenvelope.h>
#ifndef QTOPIA_CONTENT_INSTALLER
#include <qtopiaapplication.h>
#endif
#include <qtopialog.h>
#include <qtopiaipcadaptor.h>
#include <private/qcontent_p.h>

/*
  QContentSetPrivate is a helper class for QContentSet

  \c explicitLinks contains a list of pointers to \c QContent objects
  which have been explicitly added to the set, for example, using \c add(...)

  If no objects have been explicitly added this list will be empty.

  \c implicitLinks contains a list of identifiers which point to records of objects
  which are matches for the current filter set.

  The implicitLinks list is updated whenever:
  \list
      \o the filter expression changes
      \o the database changes (see below).
  \endlist

  The contents of the set is the union of the above two lists.

  If an explicitly added object matches the filter, it is added to the
  \c implicitList, and the pointer is stored in the cache.  In this case the
  explicit list is not updated.

  If the nth object in the set is requested, the count is started from
  zero at the beginning of the explicit list and then into the implicit list.
  Note: It is possible for the explicit list to be of length zero.

  The following conditions hold:
  \list
  \o If the object is found in the explicit list a pointer to a valid object
  is obtained.
  \o If the object is found in the implicit list, the cache is
  checked by record number, failing a cache hit:
     \list
     \o a new object is constructed based on the record number
     \o the new object is stored in the cache and returned.
     \endlist
  \o If this list is in process with the server, the server maintains a
  pointer to it and requests it to update itself by calling \c updateIfMatch(...)
  \o If this list is in another process then it listens for \c QCop messages
  connected to the serverMessage slot.
  \endlist
*/
class QContentSetPrivate
{
public:
    QContentSetPrivate();
    QContentSetPrivate( const QContentSetPrivate & copy );
    ~QContentSetPrivate();
    bool contains( const QContent & ) const;
    void addCriteria( const QContentFilter &QContentFilter, QContentFilter::Operand operand );
    void addCriteria( const QContentFilter::FilterType&, const QString& filter, QContentFilter::Operand operand  );
    void clearFilter();
    void add( QContent * );
    void remove( QContent * );
    void installContent();
    void uninstallContent();
    void clear();
    QContentSetPrivate &operator=( const QContentSetPrivate &rhs );

    static QMutex mutex;
private:
    QContentFilter currentFilter;
    QStringList currentSortOrder;
    QContentList explicitLinks;
    QContentIdList implicitLinks;
    QContentSet *q;
    bool implicitLinksNeedsFlush;
    QBasicTimer syncTimer;

    friend class QContentSet;
    friend class QContentSetModel;
    friend class ContentServer;
};

QMutex QContentSetPrivate::mutex(QMutex::Recursive);

/*
    FilteredLessThanFunctor is a private functor class to allow qSort to be used to sort QContent objects into
    order based upon the filter sort criteria supplied in the constructor.
*/
class FilteredLessThanFunctor
{
public:
    FilteredLessThanFunctor(const QStringList &sorts);
    bool operator()(const QContent&, const QContent&);
private:
    typedef int (*Comparison)(const QContent& left, const QContent& right);
    QList<Comparison> filterSet;
    static int nameCompare(const QContent& left, const QContent& right);
    static int nameCompareDescending(const QContent& left, const QContent& right);
    static int typeCompare(const QContent& left, const QContent& right);
    static int typeCompareDescending(const QContent& left, const QContent& right);
    static int sizeCompare(const QContent& left, const QContent& right);
    static int sizeCompareDescending(const QContent& left, const QContent& right);
    static int fileCompare(const QContent& left, const QContent& right);
    static int fileCompareDescending(const QContent& left, const QContent& right);
    static int timeCompare(const QContent& left, const QContent& right);
    static int timeCompareDescending(const QContent& left, const QContent& right);

    void init(const QStringList &sorts);
};

FilteredLessThanFunctor::FilteredLessThanFunctor(const QStringList &sorts)
{
    init(sorts);
}

void FilteredLessThanFunctor::init(const QStringList &sorts)
{
    QListIterator<QString> sortList(sorts);
    sortList.toBack();
    while(sortList.hasPrevious())
    {
        QString field=sortList.previous().toLower();
        bool descending=field.endsWith(" desc");
        if (descending)
            field.chop(5);
        if(field == "name")
        {
            if(descending)
                filterSet.append(nameCompareDescending);
            else
                filterSet.append(nameCompare);
        }
        else if(field == "type")
        {
            if(descending)
                filterSet.append(typeCompareDescending);
            else
                filterSet.append(typeCompare);
        }
        else if(field == "size")
        {
            if(descending)
                filterSet.append(sizeCompareDescending);
            else
                filterSet.append(sizeCompare);
        }
        else if(field == "file")
        {
            if(descending)
                filterSet.append(fileCompareDescending);
            else
                filterSet.append(fileCompare);
        }
        else if(field == "time")
        {
            if(descending)
                filterSet.append(timeCompareDescending);
            else
                filterSet.append(timeCompare);
        }
    }
}

bool FilteredLessThanFunctor::operator()(const QContent &s1, const QContent &s2)
{
    //return s1.toLower() < s2.toLower();
    foreach(Comparison function, filterSet)
    {
        int result=(*function)(s1, s2);
        if(result > 0)
            return true;
        else if(result < 0)
            return false;
        // else it's equal, so keep checking
    }
    return false;   // if we get here, assume false, all else being equal
}

int FilteredLessThanFunctor::nameCompare(const QContent& left, const QContent& right)
{
    return QString::compare(left.name(), right.name());
}

int FilteredLessThanFunctor::nameCompareDescending(const QContent& left, const QContent& right)
{
    return nameCompare(right, left);
}

int FilteredLessThanFunctor::typeCompare(const QContent& left, const QContent& right)
{
    return QString::compare(left.type(), right.type());
}

int FilteredLessThanFunctor::typeCompareDescending(const QContent& left, const QContent& right)
{
    return typeCompare(right, left);
}

int FilteredLessThanFunctor::sizeCompare(const QContent& left, const QContent& right)
{
    return (left.size() > right.size()) ? 1 : (left.size() < right.size() ? -1 : 0);
}

int FilteredLessThanFunctor::sizeCompareDescending(const QContent& left, const QContent& right)
{
    return sizeCompare(right, left);
}

int FilteredLessThanFunctor::fileCompare(const QContent& left, const QContent& right)
{
    if(left.linkFileKnown() && right.linkFileKnown())
        return QString::compare(left.linkFile(), right.linkFile());
    else if(left.fileKnown() && right.fileKnown())
        return QString::compare(left.file(), right.file());
    else
        return 0;
}

int FilteredLessThanFunctor::fileCompareDescending(const QContent& left, const QContent& right)
{
    return fileCompare(right, left);
}

int FilteredLessThanFunctor::timeCompare(const QContent& left, const QContent& right)
{
    return (left.lastUpdated() > right.lastUpdated()) ? 1 : (left.lastUpdated() < right.lastUpdated() ? -1 : 0);
}

int FilteredLessThanFunctor::timeCompareDescending(const QContent& left, const QContent& right)
{
    return timeCompare(right, left);
}

////////////////////////////////////////////////////////////////
//
// QContentSet implementation

/*!
  \class QContentSet
  \brief The QContentSet class represents a set of QContent objects.

  When building GUI elements representing a collection of \c QContent
  objects perform the following:
  \list
  \o create a new \c QContent set with a suitable filter
  \o create a \c QContentSetModel() model to display in the GUI.
  \endlist
  As the model is automatically kept up-to-date, this is preferable to accessing a
  list of \c QContent.

  The set is synchronized so that the data is as up-to-date as
  possible when new storage media (such as CF or SD cards) are inserted
  or removed, or when other applications change content files.

  The mechanism for this synchronization is as follows:
  \list
  \o Accesses and changes to the metadata backing store must be via
  the QContent API.
  \o If customizing Qtopia, directly altering the backing store ,for examply via SQL statemenst, is not supported.
  \o When a \c QContentSet object is created it opens
  a  \c QCop listener on the \c QContent channel internally.
  \o If a call to the API changes one or more metadata records, a message is sent on
  the \c QContent channel containing a datastream of all the record numbers
  affected.
  \o If a \c QContentSet receives a message on the \c QCop channel
  reflecting a change in a record held in the set, or it makes a change to
  such a record itself, then the following signal is emitted:
     \list
     \o the changed QContentIdList signal - for a change affecting one or more specific records.
     \o the changed() signal - for a change affecting many records.
     \endlist
  \endlist

  \ingroup qtopiaemb
*/

/*!
    \enum QContentSet::Priority

    This enum specifies the priority to use when scanning a directory.

    \value LowPriority use low priority
    \value NormalPriority use normal priority
    \value HighPriority use low priority - directory will be scanned before
        lower priority jobs.
*/

/*!
  Create a new empty \c QContentSet with parent \a parent.
*/
QContentSet::QContentSet( QObject *parent )
    : QObject( parent )
{
    init();
    sync();
}

/*!
  Create a new \c QContentSet with parent \a parent, containing all records
  from the backing store which pass all the filters listed in \a catFilters.
 */
QContentSet::QContentSet( const QContentFilter &catFilters, QObject *parent )
    : QObject( parent )
{
    init();
    d->currentFilter = catFilters;
    sync();
}


/*!
  Create a new \c QContentSet with parent \a parent, containing all records
  from the backing store which pass all the filters listed in \a catFilters
  sorted by \a sortOrder.
 */
QContentSet::QContentSet( const QContentFilter &catFilters, const QStringList &sortOrder, QObject *parent )
    : QObject( parent )
{
    init();
    d->currentFilter = catFilters;
    d->currentSortOrder = sortOrder;
    sync();
}


/*!
  Create a new \c QContentSet with parent \a parent, containing all records
  from \a original.
*/
QContentSet::QContentSet( const QContentSet &original, QObject *parent)
    : QObject( parent )
{
    init();
    *d = *original.d;
    d->q = this;
    sync();
}

/*!
    Create a new \c QContentSet with parent \a parent, containing all records
    from the backing store which pass the filtering criteria specified by \a tag and \a filter.
 */

QContentSet::QContentSet( QContentFilter::FilterType tag, const QString& filter, QObject *parent )
    : QObject( parent )
{
    init();
    d->currentFilter = QContentFilter( tag, filter );
    sync();
}


/*!
    Create a new \c QContentSet with parent \a parent, containing all records
    from the backing store which pass the filtering criteria specified by \a tag and \a filter
    and sorted by \a sortOrder.
 */

QContentSet::QContentSet( QContentFilter::FilterType tag, const QString& filter, const QStringList &sortOrder, QObject *parent )
    : QObject( parent )
{
    init();
    d->currentFilter = QContentFilter( tag, filter );
    d->currentSortOrder = sortOrder;
    sync();
}


void QContentSet::init()
{
    d = new QContentSetPrivate;
    d->q = this;
#ifndef QTOPIA_CONTENT_INSTALLER
    connect(qApp, SIGNAL(contentChanged(const QContentIdList&,QContent::ChangeType)),
            this, SLOT(contentChanged(const QContentIdList&,QContent::ChangeType)));
    connect(QContentUpdateManager::instance(), SIGNAL(refreshRequested()),
            this, SLOT(refreshRequested()));
    connect(qApp, SIGNAL(resetContent()),
            this, SLOT(refreshRequested()));
#endif
}

/*!
  Destroy the QContentSet object.
*/
QContentSet::~QContentSet()
{
}

/*!
    Scan for added/removed content in \a path with priority \a priority.
    Directories will be scanned recusively in the background.
*/
void QContentSet::scan( const QString &path, Priority priority )
{
    QtopiaIpcAdaptor qo(QLatin1String("QPE/DocAPI"));
    qo.send(SIGNAL(scanPath(const QString&,int)), path, priority);
}

/*!
  Finds all documents in the system's document directories which
  match the filter \a mimefilter, and appends the resulting \l QContent objects to \a folder.
*/
void QContentSet::findDocuments(QContentSet* folder, const QString &mimefilter)
{
    QContentSet d;
    if (!mimefilter.isEmpty())
        d.addCriteria(QContentFilter::MimeType, mimefilter, QContentFilter::Or);
    d.addCriteria(QContentFilter::Role, QLatin1String("Document"), QContentFilter::And);
    folder->appendFrom(d);
    QStorageMetaInfo storage;
    QFileSystemFilter fsf;
    fsf.documents = QFileSystemFilter::Set;
    fsf.removable = QFileSystemFilter::Set;
    foreach ( QFileSystem *fs, storage.fileSystems(&fsf) ) {
        QContentSet ide;
        ide.setCriteria(QContentFilter::Location, fs->path());
        ide.addCriteria(QContentFilter::MimeType, mimefilter, QContentFilter::And );
        folder->appendFrom(ide);
    }
}

/*!
  Return true if the set contains the object \a content, that is:
  \list
  \o if it has been explicitly added
  \o if it matches the filter expression.
  \endlist

  \internal
  If the object is not in the internal lists, but is a valid object
  and matches the current filter expression it is added.
*/
bool QContentSet::contains( const QContent &content ) const
{
    const_cast<QContentSet *>(this)->sync();

    if ( d->explicitLinks.contains( content ))
        return true;
    if ( d->implicitLinks.contains( content.id() ))
        return true;
    return false;
}

/*!
  Explicitly add the \c QContent object \a content to this set but not to the
  backing store.  \c QContentSet::contains() is called first
  and does not add if the object is already present.

  If using this method and \c QContentSet::remove(), a filter
  expression is not used.   This is useful for building up a set
  of \c Content objects to display or to perform bulk operations
  such as moving to a new location.

  If a filter expression is used and the object added matches it,  this
  method is effectively a null operation - although it will update the
  internal cache.
*/

void QContentSet::add( const QContent &content )
{
    if ( ! contains( content ))
    {
        emit aboutToSort();

            d->explicitLinks.append( content );

        if( !d->currentSortOrder.isEmpty() )
            qSort(d->explicitLinks.begin(), d->explicitLinks.end(), FilteredLessThanFunctor(d->currentSortOrder));

        emit sorted();
    }
    emit changed( QContentIdList() << content.id(), QContent::Added );
}

/*!
  Remove the \c QContent object \a cl from this set.  Note:  This does not remove the
  object from the backing store.  This is useful for trimming down a set
  of \c Content objects to display to the user, or perform bulk operations
  on such as moving to a new location.

  The method cannot remove an object that matches the filter expression, if one is
  set.

  \sa  QContentSet::add()
*/
void QContentSet::remove( const QContent &cl )
{
    if ( d->explicitLinks.contains( cl ))
    {
        emit aboutToSort();

        d->explicitLinks.removeAll( cl );

        emit sorted();
     }
     emit changed(QContentIdList() << cl.id(), QContent::Removed );
}

/*!
    Remove all content from this set.
*/
void QContentSet::clear()
{
    d->clear();
    d->syncTimer.start(0, this);
    emit changed();
}

/*!
  Store the meta information objects into the backing store, and create a file on the
  file system if the content type is appropriate.
*/
void QContentSet::installContent()
{
    d->installContent();
}

/*!
  Remove the meta information objects in this set from the backing store,
  and remove any files from the file system if the content type is appropriate.
  In the case of DRM controlled files this will also remove any rights objects.
*/
void QContentSet::uninstallContent()
{
    d->uninstallContent();
}

/*!
  Return a \c QList<QContent> of items in this set.  Note:  This is a
  relatively expensive operation, and generally should not be used unless
  it is known that only a few items will be returned.

  It is also a snapshot of the currently known items in the list,
  which could go out of date immediately after it is obtained.

  When considering using this method first examine the process of
  retrieving a pointer to the internal model and using an MVC solution.
*/
QContentList QContentSet::items() const
{
    QContentList l;
    QContentSetModel csModel(this);
    for( int i = 0; i < csModel.rowCount(); ++i )
        l.append( csModel.content( i ) );
    return l;
}

/*!
  Return a \c QContentIdList of content Ids in this set.

  It is a snapshot of the currently known content in the list,
  which could go out of date immediately after it is obtained.

  When considering using this method first examine the process of
  retrieving a pointer to the internal model and using an MVC solution.

  \sa QContentSet::items()
*/
QContentIdList QContentSet::itemIds() const
{
    QContentIdList l;
    QContentSetModel csModel(this);
    for( int i = 0; i < csModel.rowCount(); ++i )
        l.append( csModel.contentId( i ) );
    return l;
}

/*!
  \fn void QContentSet::changed(const QContentIdList &idList, QContent::ChangeType type)

  This signal is emitted when \c QContent with Ids \a idList, included in the
  filter expression for this \c QContentSet are changed, for example,
  by another application.

  \a type specifies the type of change \l{QContent::ChangeType}.
*/

/*!
  \fn void QContentSet::changed()

  Emitted when a number of \c QContent objects included in the filter
  expression for this \c QContentSet are changed
  by another application or by removable media being inserted or removed.
*/

/*!
    \fn QContentSet::aboutToSort()

    This signal is emitted when this \c QContentSet is about to be sorted.
*/

/*!
    \fn QContentSet::sorted()

    This signal is emitted when this \c QContentSet has been sorted.
 */

/*!
    \fn QContentSet::contentChanged(int start, int end)

    This signal is emitted when content items between the \a start and \a end indexes have changed.
*/

/*!
    \fn QContentSet::contentAboutToBeRemoved(int start, int end)

    This signal is emitted when content items between the \a start and \a end indexes are about to be removed.
*/

/*!
    \fn QContentSet::contentRemoved()

    This signal is emitted when the content removal indicated by \c{QContentSet::contentAboutToBeRemoved(int start, int end)}
    has been completed.
*/

/*!
    \fn QContentSet::contentAboutToBeInserted(int start, int end)

    This signal is emitted when content items are about to be inserted between the \a start and \a end indexes.
*/

/*!
    \fn QContentSet::contentInserted()

    This signal is emitted when the content insertion indicated by \c{QContentSet::contentAboutToBeInserted(int start, int end)}
    has been completed.
*/

bool QContentSet::removeId(QContentId id)
{
    QMutexLocker lock(&d->mutex);
    int idx = d->implicitLinks.indexOf(id);
    if (idx >= 0) {
        emit contentAboutToBeRemoved( idx, idx );
        d->implicitLinks.removeAt(idx);
        emit contentRemoved();
        qLog(DocAPI) << id << "Removed from set";
        emit changed(QContentIdList() << id, QContent::Removed);
        return true;
    }

    return false;
}

/*!
    Returns true if this set is empty; otherwise false.
*/
bool QContentSet::isEmpty() const
{
    const_cast<QContentSet *>(this)->sync();
    return d ? d->explicitLinks.isEmpty() && d->implicitLinks.isEmpty() : true;
}

/*!
    Appends the contents of \a other to this \c QContentSet.
    Currently it appends them as explicit items to the current QContenSet, in the future, it will concatenate the two filter sets to create a new aggregate filter set.
*/
void QContentSet::appendFrom( QContentSet& other )
{
    // QTOPIA_DOCAPI_TODO when moving to combinatorial filtering, add "other"s filter set or'd with the overall original filter set
    QContentSetModel othermodel(&other);
    for (int i=0; i< othermodel.rowCount(); i++)
    {
        add(othermodel.content(i));
    }
}

/*!
    Return the number of \c QContent objects in this set.
*/
int QContentSet::count() const
{
    const_cast<QContentSet *>(this)->sync();
    return d->explicitLinks.count() + d->implicitLinks.count();
}

/*!
    Find a \c QContent object for the executable \a exec in the current QContentSet.
    Returns an empty/invalid QContent if unsuccessful.
*/
QContent QContentSet::findExecutable( const QString& exec ) const
{
    QContentSetModel csModel(this);
    for (int i=0; i< csModel.rowCount(); i++)
        if (csModel.content(i).executableName() == exec)
            return csModel.content(i);
    return QContent();
}

/*!
    Find a \c QContent object for the \a filename in the current QContentSet.
    Returns an empty/invalid QContent if unsuccessful.

    Paths are not acceptable in \a filename, ie the \c QString \a filename must
    not contain any "/" characters.

    Note that if more than one item with the \a filename exists in the QContentSet
    no guarantee is provided of which is returned.

    This method is typically used with filters such that only one \a filename item
    exists in the filtered set.
*/
QContent QContentSet::findFileName( const QString& filename ) const
{
    Q_ASSERT( !filename.contains( QDir::separator() ));

    QContentSetModel csModel(this);
    QString fn = filename;
    fn.prepend( QDir::separator() );
    for (int i=0; i< csModel.rowCount(); i++)
        if (csModel.content(i).file().endsWith( fn ))
            return csModel.content(i);
    return QContent();
}

/*!
  \fn QStringList QContentSet::types() const

  Returns the list of types in the set.

    For applications, games and settings the type is application/x-executable
    for documents the type is the document's MIME type, or application/octet-stream if unknown.

*/

QStringList QContentSet::types() const
{
    QStringList result;
    QContentSetModel csModel(this);
    for (int i=0; i<csModel.rowCount(); i++) {
        const QContent& item=csModel.content(i);
        if (!result.contains(item.type()))
            result.append(item.type());
    }
    return result;
}


/*!
    Assigns the given \a content to this QContent and returns a reference
    to this QContent.
*/
QContentSet &QContentSet::operator=(const QContentSet& content)
{
    *d = *content.d;
    d->q = this;
    sync();
    emit changed();
    return *this;
}

void QContentSet::contentChanged(const QContentIdList &ids, QContent::ChangeType ct)
{
    if( ct == QContent::Removed )
    {
        QContentIdList removedList;
        foreach( QContentId id, ids )
        {
            int index = d->implicitLinks.indexOf( id );

            if( index != -1 )
            {
                emit contentAboutToBeRemoved( index, index );
                d->implicitLinks.removeAt( index );
                emit contentRemoved();
                removedList.append(id);
            }

            for( index = 0; index < d->explicitLinks.count(); index++ )
            {
                if( d->explicitLinks[ index ].id() == id )
                {
                    int row = d->implicitLinks.count() + index;

                    emit contentAboutToBeRemoved( row, row );
                    d->explicitLinks.removeAt( index );
                    index--;
                    emit contentRemoved();
                    removedList.append(id);
                }
            }
        }
        if(removedList.count() != 0)
        {
            emit changed(removedList, ct);
        }
    }
    else
    {
        d->implicitLinksNeedsFlush = true;
        sync();

        if( ct == QContent::Updated )
        {
            int start;
            for( start = 0; start < d->implicitLinks.count(); start++ )
            {
                if( ids.contains( d->implicitLinks[ start ] ) )
                {
                    int end;
                    for( end = start + 1; end < d->implicitLinks.count() && ids.contains( d->implicitLinks[ end ] ); end++ );

                    emit contentChanged( start, end - 1 );
                }
            }
            for( start = 0; start < d->explicitLinks.count(); start++ )
            {
                if( ids.contains( d->explicitLinks[ start ].id() ) )
                {
                    int end;
                    for( end = start + 1; end < d->explicitLinks.count() && ids.contains( d->explicitLinks[ end ].id() ); end++ );

                    emit contentChanged( d->implicitLinks.count() + start, d->implicitLinks.count() + end - 1 );
                }
            }
        }
        QContentIdList changedList;
        foreach( QContentId id, ids )
        {
            if(d->implicitLinks.indexOf( id ) != -1)
                changedList.append(id);
            else
                for( int index = 0; index < d->explicitLinks.count(); index++ )
                    if( d->explicitLinks[ index ].id() == id )
                        changedList.append(id);
        }
        if(changedList.count() != 0)
        {
            emit changed(changedList, ct);
        }

    }
}

/*!
  Clear the current filter expression on the set
 */
void QContentSet::clearFilter()
{
    d->clearFilter();
    d->syncTimer.start(0, this);
    emit changed();
}

/*!
    Joins a filtering criteria of \c FilterType \a kind and value \a filter to the current filter set using the given \a operand.
*/
void QContentSet::addCriteria( QContentFilter::FilterType kind, const QString &filter, QContentFilter::Operand operand )
{
    d->addCriteria(kind, filter, operand);
    d->syncTimer.start(0, this);
    emit changed();
}

/*!
    \overload
    Joins a group of filtering criteria \a filters to the current filter set using the given \a operand.
 */
void QContentSet::addCriteria(const QContentFilter& filters, QContentFilter::Operand operand )
{
    d->addCriteria(filters, operand);
    d->syncTimer.start(0, this);
    emit changed();
}

/*!
    Sets a filtering criteria of \c FilterType \a kind and value \a filter to the current filter set.
 */
void QContentSet::setCriteria(QContentFilter::FilterType kind, const QString &filter)
{
    d->clearFilter();
    d->addCriteria(kind, filter, QContentFilter::NoOperand );
    sync();
    emit changed();
}

/*!
    \overload
    Sets a group of filtering criteria \a filters to the current filter set.
 */
void QContentSet::setCriteria(const QContentFilter& filters)
{
    d->clearFilter();
    d->addCriteria(filters, QContentFilter::NoOperand);
    sync();
    emit changed();
}

/*!
    Returns a copy of the current filter for this \c QContentSet.
*/
QContentFilter QContentSet::filter() const
{
    return d->currentFilter;
}

/*!
    Sets the attributes content is ordered by to \a sortOrder.
 */
void QContentSet::setSortOrder( const QStringList &sortOrder )
{
    d->currentSortOrder = sortOrder;
    d->implicitLinksNeedsFlush = true;
    sync( true );
    emit changed();
}

/*!
    Returns the attributes the content is ordered by.
 */
QStringList QContentSet::sortOrder() const
{
    return d->currentSortOrder;
}

/*!
  \reimp
*/
void QContentSet::timerEvent(QTimerEvent *e)
{
    if (e->timerId() == d->syncTimer.timerId()) {
        sync();
        d->syncTimer.stop();
    }
}

bool QContentSet::sync( bool resort )
{
    if (d->implicitLinksNeedsFlush) {
        d->implicitLinksNeedsFlush = false;

        QContentIdList implicits = QContent().database()->matches( QContentIdList(), d->currentFilter, d->currentSortOrder );

        if( resort )
        {
            d->implicitLinks = implicits;

            emit changed();
        }
        else
            updateImplicits( implicits );

        return true;
    }

    return false;
}

void QContentSet::updateImplicits( const QContentIdList &implicits )
{
    int existingIndex = 0;
    int updatedIndex = 0;
    bool updatesSeen = false;

    while( existingIndex < d->implicitLinks.count() || updatedIndex < implicits.count() )
    {
        if( existingIndex == d->implicitLinks.count() )
        {
            int start = existingIndex;
            int end   = existingIndex + implicits.count() - 1 - updatedIndex;

            emit contentAboutToBeInserted( start, end );

            for( ; updatedIndex < implicits.count(); updatedIndex++ )
                d->implicitLinks.append( implicits[ updatedIndex ] );

            emit contentInserted();
            if(start != end)
                updatesSeen = true;

            break;
        }

        if( updatedIndex == implicits.count() )
        {
            int start = existingIndex;
            int end   = d->implicitLinks.count() - 1;

            emit contentAboutToBeRemoved( start, end );

            while( existingIndex < d->implicitLinks.count() )
                d->implicitLinks.removeLast();

            emit contentRemoved();
            if(start != end)
                updatesSeen = true;

            break;
        }

        if( existingIndex < d->implicitLinks.count() &&
            updatedIndex < implicits.count() &&
            d->implicitLinks[ existingIndex ] == implicits[ updatedIndex ] )
        {
            existingIndex++;
            updatedIndex++;
            continue;
        }

        int lastMatch = implicits.indexOf( d->implicitLinks[ existingIndex ], updatedIndex + 1 );

        if( lastMatch != -1 )
        {
            int start = existingIndex;
            int end   = existingIndex + lastMatch - 1 - updatedIndex;

            emit contentAboutToBeInserted( start, end );

            for( ; updatedIndex < lastMatch; updatedIndex++ )
                d->implicitLinks.insert( existingIndex++, implicits[ updatedIndex ] );

            emit contentInserted();
            if(start != end)
                updatesSeen = true;

            continue;
        }

        lastMatch = existingIndex;

        while( ++lastMatch < d->implicitLinks.count() && d->implicitLinks[ lastMatch ] != implicits[ updatedIndex ] );


        int start = existingIndex;
        int end   = lastMatch - 1;

        emit contentAboutToBeRemoved( start, end );

        while( lastMatch-- != existingIndex )
            d->implicitLinks.removeAt( lastMatch );

        emit contentRemoved();
        if(start != end)
            updatesSeen = true;
    }
    if(updatesSeen)
        emit changed();
}

void QContentSet::refreshRequested()
{
    d->implicitLinksNeedsFlush = true;
    sync();
    emit changed();
}

/*!
    Returns the number of content items in the database that match the content filter \a filter.
*/
int QContentSet::count( const QContentFilter &filter )
{
    return QContent::database()->recordCount( filter );
}

////////////////////////////////////////////////////////////////
//
// ContentSetModel implementation

class QContentSetModelPrivate
{
public:
    QDrmRights::Permission selectPermission;
    QDrmRights::Permissions mandatoryPermissions;
};

/*!
  \class QContentSetModel
  QAbstractItemModel subclass

  The QContentSetModel provides a model to represent the data in a QContentSet.
*/

/*!
  Construct a new model based on the QContentSet \a cls with parent \a parent.
*/
QContentSetModel::QContentSetModel( const QContentSet *cls, QObject *parent )
    : QAbstractListModel(parent), contentSet(cls), d(0)
{
    d = new QContentSetModelPrivate;

    d->selectPermission = QDrmRights::InvalidPermission;
    d->mandatoryPermissions = QDrmRights::NoPermissions;

    connect( cls, SIGNAL(contentAboutToBeInserted(int,int)),
             this, SLOT(beginInsertContent(int,int)) );
    connect( cls, SIGNAL(contentInserted()),
             this, SLOT(endInsertContent()) );
    connect( cls, SIGNAL(contentAboutToBeRemoved(int,int)),
             this, SLOT(beginRemoveContent(int,int)) );
    connect( cls, SIGNAL(contentRemoved()),
             this, SLOT(endRemoveContent()) );
    connect( cls, SIGNAL(aboutToSort()), this, SLOT(emitLayoutAboutToBeChanged()) );
    connect( cls, SIGNAL(sorted()), this, SLOT(emitLayoutChanged()) );
    connect( cls, SIGNAL(changed() ), this, SLOT(doReset()) );
    connect( cls, SIGNAL(destroyed() ), this, SLOT(clearContentSet()) );
}

/*!
  Destroys a QContentSetModel object.
 */

QContentSetModel::~QContentSetModel()
{
    delete d;
}

/*!
  Return the number of rows in the model - since this is a flat list the
  \a parent argument will always be the default null index, indicating a
  top level item.  The result is the count of the items in the backing
  store which match the filter expression, and/or any explicitly added
  non-matching items.
*/
int QContentSetModel::rowCount( const QModelIndex & /* parent */ ) const
{
    if (!contentSet)
        return 0;
    return contentSet->count();
}

/*!
  Return the appropriate QVariant data from the model for the given \a index.
  Depending on the \a role the QVariant will contain the name of the QContent
  object at that index, or its Icon, or relevant ToolTip text.  The "ToolTip"
  text is shown when the user hovers the cursor over the item in the
  model view.  The tooltip text will the comment() field from the
  QContent object, or if error() is true, the error text.  Additionally
  the drm rights in summary form will be shown.
*/
QVariant QContentSetModel::data( const QModelIndex & index, int role ) const
{
    if (!contentSet)
        return QVariant();

    const_cast<QContentSet *>(contentSet)->sync();
    switch ( role )
    {
        case Qt::DecorationRole:
            return d->selectPermission == QDrmRights::InvalidPermission
                    ? content( index ).icon()
                    : content( index ).icon( d->selectPermission );
        case Qt::DisplayRole:
            return content( index ).name();
        case Qt::ToolTipRole:
            return content( index ).error()
                ? content( index ).errorString()
                : content( index ).comment();
        case Qt::UserRole:
            if( content( index ).fileKnown() )
                return content( index ).file();
            if( content( index ).linkFileKnown() )
                return content( index ).linkFile();
    }
    return QVariant();
}

/*!
    \reimp
*/
Qt::ItemFlags QContentSetModel::flags( const QModelIndex &index ) const
{
    Qt::ItemFlags flags = QAbstractListModel::flags( index );

    if( d->mandatoryPermissions != QDrmRights::NoPermissions &&
        (content( index ).permissions() & d->mandatoryPermissions) != d->mandatoryPermissions )
        flags &= ~(Qt::ItemIsEnabled | Qt::ItemIsSelectable);

    return flags;
}

/*!
  Returns the QContent for row \a row.
*/
QContent QContentSetModel::content( uint row ) const
{

    if (!contentSet)
        return QContent();

    const_cast<QContentSet *>(contentSet)->sync();
    uint expCount = contentSet->d->explicitLinks.count();
    uint impCount = contentSet->d->implicitLinks.count();
    if ( row >= expCount + impCount )
        return QContent();
    if ( row < expCount )
        return contentSet->d->explicitLinks[ row ];

    row -= expCount;
    QContentId iid = contentSet->d->implicitLinks[ row ];
    if (!QContent::isCached(iid)) {
        int count = 0;
        QContentIdList iidList;
        for (unsigned int i = row>=10 ? row-10 : 0; i < impCount && count < 20; i++) {
            if (!QContent::isCached(contentSet->d->implicitLinks[i])) {
                iidList.append(contentSet->d->implicitLinks[i]);
                count++;
            }
        }
        QContent::cache(iidList);
    }

    return QContent(iid);
}

/*!
  Returns the QContent for model index \a index.
 */

QContent QContentSetModel::content( const QModelIndex & index ) const
{
    return content(index.row());
}

/*!
  Returns the QContentId for model index \a index.
 */

QContentId QContentSetModel::contentId( const QModelIndex & index ) const
{
    if (!contentSet)
        return QContent::InvalidId;

    const_cast<QContentSet *>(contentSet)->sync();
    return content(index).id();
}

/*!
  Returns the QContentId for row \a row.

  Note that the value returned may be QContent::InvalidId if the
  QContent has not been committed to the backing store.
*/
QContentId QContentSetModel::contentId( uint row ) const
{
    if (!contentSet)
        return QContent::InvalidId;

    const_cast<QContentSet *>(contentSet)->sync();
    return content(row).id();
}

/*!
    Sets the \a permissions a QContent must have to in order to be selectable in the content model.

    If a row in the model doesn't have all the mandatory permissions the Qt::ItemIsEnabled and Qt::ItemIsSelectable
    flags will be cleared.
*/
void QContentSetModel::setMandatoryPermissions( QDrmRights::Permissions permissions )
{
    d->mandatoryPermissions = permissions;

    emit dataChanged( QModelIndex(), QModelIndex() );
}

/*!
    Returns the permissions a QContent must have in order to be selectable in the content model.
*/
QDrmRights::Permissions QContentSetModel::mandatoryPermissions() const
{
    return d->mandatoryPermissions;
}

/*!
    Sets the \a permission which indicates the intended usage of the content in the model

    Content in the model which doesn't have the select permission will be displayed with an invalid rights icon.

    If the permission is QDrmRights::InvalidPermission the default permission for the content is used.
*/
void QContentSetModel::setSelectPermission( QDrmRights::Permission permission )
{
    d->selectPermission = permission;

    emit dataChanged( QModelIndex(), QModelIndex() );
}

/*!
    Returns the permssion which indicated the intended usage of the content in the model.
*/
QDrmRights::Permission QContentSetModel::selectPermission() const
{
    return d->selectPermission;
}

/*
    Determine the row that a given Content ID belongs to
    Returns -1 if not found
*/
int QContentSetModel::rowForContentId(QContentId contentId)
{
    if (!contentSet)
        return -1;

    // Find row index for given Content ID
    int rowNum = 0;
    foreach(const QContent &content, contentSet->d->explicitLinks) {
        ++rowNum;
        if(content.id() == contentId)
            return rowNum;
    }
    foreach(QContentId dId, contentSet->d->implicitLinks) {
        ++rowNum;
        if(dId == contentId)
            return rowNum;
    }

    return contentSet->d->implicitLinks.count() + contentSet->d->explicitLinks.count();
}

void QContentSetModel::doReset()
{
    reset();
}

void QContentSetModel::beginInsertContent( int start, int end )
{
    beginInsertRows( QModelIndex(), start, end );
}

void QContentSetModel::endInsertContent()
{
    endInsertRows();
}

void QContentSetModel::beginRemoveContent( int start, int end )
{
    beginRemoveRows( QModelIndex(), start, end );
}

void QContentSetModel::endRemoveContent()
{
    endRemoveRows();
}

void QContentSetModel::contentChanged( int start, int end )
{
    emit dataChanged( index( start, 0 ), index( end, 0 ) );
}

void QContentSetModel::emitLayoutAboutToBeChanged()
{
    emit layoutAboutToBeChanged();
}

void QContentSetModel::emitLayoutChanged()
{
    emit layoutChanged();
}


void QContentSetModel::clearContentSet()
{
    contentSet = 0;
    reset();
}

////////////////////////////////////////////////////////////////
//
// QContentSetPrivate implementation

/*!
  \internal
  Private inner classes containing actual implementation, for binary
  compatibility purposes
*/
QContentSetPrivate::QContentSetPrivate()
    : q( 0 )
    , implicitLinksNeedsFlush( true )
{
}

QContentSetPrivate::QContentSetPrivate( const QContentSetPrivate & copy )
    : q( 0 )
{
    (*this) = copy;  // use operator =
}

QContentSetPrivate &QContentSetPrivate::operator=( const QContentSetPrivate &rhs )
{
    this->currentFilter = rhs.currentFilter;
    this->explicitLinks = rhs.explicitLinks;
    this->implicitLinks = rhs.implicitLinks;
    this->implicitLinksNeedsFlush = rhs.implicitLinksNeedsFlush;
    this->q = 0;
    return *this;
}

QContentSetPrivate::~QContentSetPrivate()
{
}

void QContentSetPrivate::installContent()
{
}

void QContentSetPrivate::uninstallContent()
{
}

void QContentSetPrivate::clear()
{
    currentFilter.clear();
    implicitLinks.clear();
    explicitLinks.clear();
    implicitLinksNeedsFlush = true;
}

void QContentSetPrivate::addCriteria( const QContentFilter &filter, QContentFilter::Operand operand )
{
    switch( operand )
    {
    case QContentFilter::And:
        currentFilter &= filter; break;
    case QContentFilter::Or:
        currentFilter |= filter; break;
    case QContentFilter::NoOperand:
        currentFilter = filter; break;
    }

    implicitLinksNeedsFlush = true;
}

void QContentSetPrivate::addCriteria( const QContentFilter::FilterType& tag, const QString& filter, QContentFilter::Operand operand )
{
    switch( operand )
    {
        case QContentFilter::And:
            currentFilter &= QContentFilter( tag, filter ); break;
        case QContentFilter::Or:
            currentFilter |= QContentFilter( tag, filter ); break;
        case QContentFilter::NoOperand:
            currentFilter = QContentFilter( tag, filter ); break;
    }

    implicitLinksNeedsFlush = true;
}

void QContentSetPrivate::clearFilter()
{
    currentFilter.clear();
    implicitLinksNeedsFlush = true;
}

bool QContentSetPrivate::contains( const QContent &m ) const
{
    QContentSetPrivate *that=const_cast<QContentSetPrivate*>(this);
    that->implicitLinks = QContent().database()->matches( implicitLinks, currentFilter, currentSortOrder );
    return explicitLinks.contains(m) || implicitLinks.contains( m.id() );
}

/*!
    \fn QContentSet::serialize(Stream &stream) const
    \internal
*/
template <typename Stream> void QContentSet::serialize(Stream &stream) const
{
    stream << d->currentFilter;

    QContentIdList explicitIds;

    foreach( const QContent &content, d->explicitLinks )
        explicitIds.append( content.id() );

    stream << explicitIds;
    stream << d->currentSortOrder;
}

/*!
    \fn QContentSet::deserialize(Stream &stream)
    \internal
*/
template <typename Stream> void QContentSet::deserialize(Stream &stream)
{
    QContentFilter filter;
    QContentIdList explicitIds;
    QStringList sortOrder;

    stream >> filter;
    stream >> explicitIds;
    stream >> sortOrder;

    clear();
    setSortOrder( sortOrder );
    setCriteria( filter );

    foreach( const QContentId &id, explicitIds )
        add( QContent( id ) );
}

Q_IMPLEMENT_USER_METATYPE(QContentSet)

