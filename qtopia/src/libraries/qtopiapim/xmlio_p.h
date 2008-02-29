/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef PIMXMLIO_PRIVATE_H
#define PIMXMLIO_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/qtopiapim.h>
#include <qtopia/pim/pimrecord.h>
#ifdef Q_OS_WIN32
#include <qxml.h> // needed for import of QMap<QString,QString>
#endif

class QFile;

class QTOPIAPIM_EXPORT PrRecord : public PimRecord
{
public:
    void setUid(QUuid u) { p_setUid(u); }

    QArray<int> &categoriesRef() { return mCategories; }
    QMap<QString, QString> &customRef() { return customMap; }
};

class QTOPIAPIM_EXPORT PimVector : public QVector<PimRecord> {
public:
    PimVector();

    PimVector(uint size);

    ~PimVector();

    PimRecord *at(uint i) const { return QVector<PimRecord>::at(i); }

    void append( PimRecord *i );

    void remove( PimRecord *i );

    uint count() const { return nextindex; }

    void clear();
    void sort();

protected:
    void setDirty() { dirty = TRUE; }
    virtual int compareItems(Item d1, Item d2);

private:
    uint nextindex;
    bool dirty;
};

template <class T>
class  SortedRecords : public PimVector
{
public:
    SortedRecords() : PimVector() { mKey = -1; mAscending = FALSE; }

    SortedRecords(uint size) : PimVector(size) {mKey = -1; mAscending = FALSE;} 

    ~SortedRecords() {}

    //const T* operator[](int i) const { return (T*)PimVector::at(i); }
    const T* at(int i) const { return (T*)PimVector::at(i); }

    // fast but utimately slow if you want to keep it sorted.
    // temporary solution.
    void append( T *i )
    { PimVector::append((PimRecord *)i); }

    void remove( T *i )
    { PimVector::remove((PimRecord *)i); }
    
    void setSorting(int key, bool ascending = FALSE)
    { mKey = key; mAscending = ascending; setDirty(); }
    
    int sortKey() const { return mKey; }
    bool ascending() const { return mAscending; }

protected:
    virtual int compareItems(Item d1, Item d2)
    { return PimVector::compareItems(d1, d2); }

    int mKey;
    bool mAscending;
};

class QTOPIAPIM_EXPORT PimXmlIO {
public:
    // subclass contstructors should call setDataFilename and setJournalFilename
    PimXmlIO( const QMap<int,QCString> &, const QMap<QCString,int> & );
  virtual ~PimXmlIO();

  bool loadData();
  bool saveData(const QList<PimRecord> &);

  static QArray<int>idsFromString( const QString &str );
  QString customToXml(const PimRecord * ) const;
  static QString idsToString( const QArray<int> &ids );

  static QString dateToXml( const QDate &d );
  static QDate xmlToDate( const QString &s );

  void setDataFilename(const QString &str) { mFilename = str; };
  void setJournalFilename(const QString &str) { mJournalFilename = str; };

  QString dataFilename() const { return mFilename; };
  QString journalFilename() const { return mJournalFilename; };

  // should perhaps be moved back to protected
  static QUuid uuidFromInt( int );
  static int uuidToInt( const QUuid &);
  static int generateUid(); 

protected:
  virtual const char *recordStart() const = 0;
  virtual const char *listStart() const = 0;
  virtual const char *listEnd() const = 0;

  virtual PimRecord *createRecord() const = 0;

  // PimRecord * must be either stored or deleted by these functions.
  virtual bool internalAddRecord(PimRecord *) = 0;
  virtual bool internalRemoveRecord(PimRecord *) = 0;
  virtual bool internalUpdateRecord(PimRecord *) = 0;

  virtual QString recordToXml(const PimRecord *);

  // helper functions
  void assignNewUid(PimRecord *) const;

  void setUid(PimRecord &r, const QUuid &u) const;
  QUuid uid(const PimRecord &r) const;

  enum journal_action { ACTION_ADD, ACTION_REMOVE, ACTION_REPLACE };
  void updateJournal(const PimRecord &, journal_action);

  bool isDataCurrent() const;
private:

  bool lockDataFile(QFile &);
  bool unlockDataFile(QFile &);


  bool loadFile(const QString &);

  void setLastDataReadTime(QDateTime dateTime) { mLastRead = dateTime; }
  QDateTime lastDataReadTime() const { return mLastRead; }


  QDateTime mLastRead;
  QString mFilename, mJournalFilename;

  const QMap<int,QCString> &keyToIdentifier;
  const QMap<QCString,int> &identifierToKey;
};

#endif
