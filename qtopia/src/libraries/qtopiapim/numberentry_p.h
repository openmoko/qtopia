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

#ifndef QTOPIA_INTERNAL_NUMBERENTRY
#define QTOPIA_INTERNAL_NUMBERENTRY

#include "contactfieldlist_p.h"
#ifdef QTOPIA_PHONE
#include <qtopia/phone/calllist.h>
#endif
#include <qdialog.h>
#include <qlineedit.h>
#include <qvaluelist.h>
#include <qmap.h>
#include <qstringlist.h>
#include <qlayout.h>
#include <qaction.h>
#include <qtopia/qpeglobal.h>
#include <qtopia/pim/qtopiapimwinexport.h>
#include <qtopia/categories.h>
#include <qtopia/qpemenubar.h>
#include <qtopia/contextbar.h>
#include <qtopia/contextmenu.h>
#include <qlabel.h>
#include <qtopia/pim/addressbookaccess.h>

class QTOPIAPIM_EXPORT NumberSelector : public ContactFieldList
{
    Q_OBJECT
public:
    NumberSelector( QValueList<PimContact::ContactFields> homeFields,
        QValueList<PimContact::ContactFields> busFields, 
        QWidget* parent = 0, const char* name = 0 );
    
    QStringList selectedNumbers();
    QList<PimContact> selectedContacts();
    int selectedCount();
    void addSelectedToCategory();
    
    void addUserEntry( const QString &number );
    void setSelectedList( QStringList selectedList );
    QString fullText( int );
            
public slots:
    void refresh();    
    
protected:
    virtual QString extraInfoText(int idx) const;
    QPixmap categoryPixmap();
    QPixmap unknownPixmap();

private:
    QValueList<PimContact::ContactFields> mHomeFields;
    QValueList<PimContact::ContactFields> mBusFields;
    QValueList<QUuid> mSelectedContacts;
    QValueList<int> mCategories;
    Categories mCategoryManager;
    QList<PimContact> mUserContacts;
    QMap<int, QString> mIndexToEmail;
    AddressBookAccess mAddressBook;
};

class QTOPIAPIM_EXPORT NumberEntryDialog : public QDialog
{
    Q_OBJECT
public:
    NumberEntryDialog( QValueList<PimContact::ContactFields> homeFields,
        QValueList<PimContact::ContactFields> busFields, bool textEntry,
        const char* caption, QWidget* parent = 0, 
        const char* name = 0, bool modal = TRUE, WFlags fl = 0 );

    virtual ~NumberEntryDialog();
        
    bool NumberEntryDialog::eventFilter(QObject *o, QEvent *e);
    
    QStringList numberList();
    QString number();
    void setSelected(const QStringList& selectedList);
    void setTextEntry(const QString& text);
    
protected slots:
    void accept();
    void numberChanged(const QString& number);
    void selectionChanged();
    void addToCategory();

protected:
    void handleFocus(QObject *o);
    bool handleKeyPress(QObject *o, QKeyEvent *ke);

    QLineEdit* mNumberEdit;
    NumberSelector* mNumberList;
    QStringList mResults;
    QVBoxLayout *mLayout;
    int mSelectedCount;
    QString mSelectedString;

#ifdef QTOPIA_PHONE
    QAction *mActionGroup;
    ContextMenu *mContextMenu;
#endif
    
    QLabel *mStatusLabel;
};
#endif
