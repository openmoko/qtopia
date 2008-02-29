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
#ifndef QWS_DATABASETABLE
#define QWS_DATABASETABLE

#include <qtopia/qpeglobal.h>
#include <qglobal.h>
#include <qarray.h>

class QTOPIA_EXPORT QWSDatabaseTable : public QScrollView
{
    Q_OBJECT

public:
    QWSDatabaseTable(const QString& dbname, QObject* parent=0, const char* name=0, int fl);
    ~QWSDatabaseTable();

    QString databaseName() const { return dbname; }

    /* not sure if these should be virtual */
    virtual uint addRecord(const Record &);
    virtual void removeRecord(uint pos);
    virtual void setRecord(uint pos, const Record &);
    virtual Record &at(uint pos) const; // reference, it will be a sub-class of record that is returned.
    virtual uint find(const Record &) const;
    virtual bool contains(const Record &) const;

    QHeader *header() const; // do we even want this?

    // I am pretty sure we will want this.
    // It means show this part of the record (the rest can still be used
    // for internal processing).
    virtual int addColumn(const QString &field, 
	    const QString &label = QString::null, int width = -1);
    virtual int addColumn(const QString &field, const QIconSet &iconset,
	    const QString &label = QString::null, int width = -1);

    virtual int removeColumn(const QString &field);

    int columns() const;
    virtual QString columnAt(int) const;

    virtual void setColumnText(const QString &field, const QString &label);
    virtual void setColumnText(const QString &field, const QIconsSet &is,
	    const QString &label);
    QString &columnText(QString &field) const;

    virtual void setColumnPosition(const QString &field, int pos);
    int columnPosition(const QString &field) const;

    virtual void setColumnWidth(const QString &field, int width);
    int columnWidth(const QString &field) const;

    // do we want multi-selection? column alignment? column width mode?
    // sorting ?

    void ensureRecordVisible(uint);
    void ensureCellVisible(uint pos, const QString &field);

    // iterate stuff.  No iterate stuff, its an Array of items, refernce
    // it by ints.

public slots:
    void setCategories( const QArray<int> & ); // set allowed categories
    void setCategoriesAll();
    void setCategoriesUnfiled();

signals:
    // various item stuff happened.

protected:

    virtual void paintCell(const QRect &r, uint pos, const QString &field);

    // lets pretend for now constant row height, makes scrolling much
    // much easier considering we don't want to read every Record to find
    // out what the total height will be.
    virtual int rowHeight() const; // row height (regardless of row);

    virtual bool filter(const Record &) const; // Normally just checks
    					       // current categories.

    virtual void cellClicked(uint pos, const QString &field);
    // change the record immidiately.
    // do nothing,
    // open widget to edit cell, (and 'know' that this is happening).
    
    // This is how to know widget is being edited, what is being done.
    // also sets current and currentField
    void setCurrentEditor(QWidget *, uint pos = 0, 
	    const QString &field = QString::null);
    QWidget *currentEditor() const; 

    // factory classes for when overiding record.
    virtual Record &constructRecord(const QByteArray &) const;
    // back to ByteArray, should be handled by normal inheritence.
    
protected slots:
    // no set unknown.  We don't show by default, we don't edit by default.
    // User can override by linking to different slot.
    void setCurrentCell(bool);
    void setCurrentCell(int);
    void setCurrentCell(const QDate &);
    void setCurrentCell(const QDateTime &);
    void setCurrentCell(const QString &); 

private:
};
#endif
