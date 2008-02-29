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

#ifndef PIMXMLIO_PRIVATE_H
#define PIMXMLIO_PRIVATE_H

#include <qvector.h>
#include <qlist.h>
#include <qdatetime.h>
#include <qtopia/pim/qpimrecord.h>
#ifdef Q_OS_WIN32
#include <qxml.h> // needed for import of QMap<QString,QString>
#endif

class QFile;

struct XmlIOField {
    const char *text;
    int fieldId;
};

class QPimXmlIO {
public:
    // subclass contstructors should call setDataFilename and setJournalFilename
    QPimXmlIO( );
  virtual ~QPimXmlIO();

  bool loadData();
  bool saveData(const QList<QPimRecord*> &);

  static QList<QString> idsFromString( const QString &str );
  QString customToXml(const QPimRecord * ) const;
  static QString idsToString( const QList<QString> &ids );

  static QString dateToXml( const QDate &d );
  static QDate xmlToDate( const QString &s );

  void setDataFilename(const QString &str) { mFilename = str; };
  void setJournalFilename(const QString &str) { mJournalFilename = str; };
  void setScope(const QString &str) { mScope = str; }

  QString dataFilename() const { return mFilename; };
  QString journalFilename() const { return mJournalFilename; };

  // should perhaps be moved back to protected
  QUniqueId generateUid() const;

protected:
  virtual const char *recordStart() const = 0;
  virtual const char *listStart() const = 0;
  virtual const char *listEnd() const = 0;

  virtual QPimRecord *createRecord() const = 0;

  // QPimRecord * must be either stored or deleted by these functions.
  virtual bool internalAddRecord(QPimRecord *) = 0;
  virtual bool internalRemoveRecord(QPimRecord *) = 0;
  virtual bool internalUpdateRecord(QPimRecord *) = 0;


  // helper functions
  void assignNewUid(QPimRecord *) const;

  void setUid(QPimRecord &r, const QUniqueId & u) const;

  enum journal_action { ACTION_ADD, ACTION_REMOVE, ACTION_REPLACE };
  void updateJournal(const QPimRecord &, journal_action);

  bool isDataCurrent() const;

  virtual QString recordToXml(const QPimRecord *) const;
  virtual void setFields(QPimRecord *, const QMap<QString, QString> &) const;
  virtual QMap<QString, QString> fields(const QPimRecord *) const;

private:

  bool lockDataFile(QFile &);
  bool unlockDataFile(QFile &);


  bool loadFile(const QString &);

  void setLastDataReadTime(QDateTime dateTime) { mLastRead = dateTime; }
  QDateTime lastDataReadTime() const { return mLastRead; }


  QDateTime mLastRead;
  QString mFilename, mJournalFilename;
  QString mScope;
};

#endif
