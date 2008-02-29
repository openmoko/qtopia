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

#ifndef HAVE_CONTACTFIELDSELECTOR_H
#define HAVE_CONTACTFIELDSELECTOR_H

#include <qtopia/pim/private/contactfieldlist_p.h>
#include <qtopia/pim/private/abtable_p.h>

#include <qdialog.h>
#include <qvaluelist.h>

class QTOPIAPIM_EXPORT ContactFieldSelectorList : public ContactFieldList
{
    Q_OBJECT
public:
    ContactFieldSelectorList( QWidget *parent, const char *name = 0, WFlags fl = 0 );
    void setFilterFields( const QValueList<PimContact::ContactFields> &ff );
    QValueList<PimContact::ContactFields> filterFields() const;

public slots:
    void refresh();
    void reset();
protected:
    QString extraInfoText( int idx ) const;

    QValueList<PimContact::ContactFields> mFilterFields;
    QMap<int,QString> mIndexToExtraInfo;
};

class QTOPIAPIM_EXPORT ContactFieldSelector : public QDialog
{
    Q_OBJECT
public:
    ContactFieldSelector( const QValueList<PimContact::ContactFields> &ff,
	    const QString &filterStr, bool allowMultiple, QWidget *parent = 0, const char *name = 0,
	    bool modal = TRUE, WFlags fl = 0 );
    ContactFieldSelector( QWidget *parent = 0, const char *name = 0,
	    bool modal = TRUE, WFlags fl = 0 );
    void setFilterFields( const QValueList<PimContact::ContactFields> &ff );
    QValueList<PimContact::ContactFields> filterFields() const;

    void setSelectedFields( const QStringList &fieldData );
    QStringList unknownFieldData() const;

    void setAllowMultiple( bool allowMultiple );
    bool allowMultiple() const;

    QString filter() const;

    bool hasSelected() const;
    QValueList<int> selected() const;
    PimContact contactAt( int idx ) const;
    int fieldTypeAt( int idx ) const;
    QString fieldDataAt( int idx ) const;

    bool eventFilter( QObject *o, QEvent *e );

public slots:
    void setFilter( const QString &filterStr );

    void refresh();

signals:
    void selected(ContactFieldSelector *, const QValueList<int>&);

protected slots:
    void accept();

protected:
    ContactFieldSelectorList *mFieldList;
private:
    void init();
    bool mFirstSetAllowMultiple;
    QLineEdit *mFindLE;
};

#endif
