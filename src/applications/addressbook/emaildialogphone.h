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

#ifndef EMAILDIALOGPHONE_H
#define EMAILDIALOGPHONE_H

#include <qdialog.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qlistbox.h>
#include <qlineedit.h>

class QAction;

class EmailDialogList;
class EmailDialogListItem : public QListBoxItem
{
    friend class EmailDialogList;
public:
    EmailDialogListItem( EmailDialogList *parent = 0);
    EmailDialogListItem( EmailDialogList *parent, EmailDialogListItem *after );

    void setPixmap( const QPixmap &pix );
    virtual const QPixmap *pixmap() const;

    virtual int width( const QListBox *lb ) const;
    virtual int height( const QListBox *lb ) const;
protected:
    void setText( const QString &txt );

    virtual void paint( QPainter *p );
private:
    QPixmap mDefaultPix;
};

class EmailDialogList : public QListBox
{
    friend class EmailLineEdit;
    Q_OBJECT
public:
    EmailDialogList( QWidget *parent, const char *name = 0, WFlags fl = 0 );

    void setEmails( const QString &def, const QStringList &em );

    QString defaultEmail() const;
    QStringList emails() const;

protected slots:
    void setCurrentText( const QString &t );
    void newEmail();
    void newEmail( const QString &email );
    void deleteEmail();
    void setAsDefault();

    void moveUp();
    void moveDown();

private:
    QPixmap mDefaultPix;
    QPixmap mNormalPix;
    int mDefaultIndex;
};

class EmailLineEdit : public QLineEdit
{
    friend class EmailDialogList;
    Q_OBJECT
public:
    EmailLineEdit( QWidget *parent, const char *name = 0 );

signals:
    void newEmail();
    void deleteEmail();
    void setAsDefault();
    void moveUp();
    void moveDown();

protected slots:
    void currentChanged( QListBoxItem *current );

protected:
    void keyPressEvent( QKeyEvent *ke );

private:
    QAction *mSetDefaultAction, *mDeleteAction, *mNewAction;
};

class EmailDialog : public QDialog 
{
    Q_OBJECT
public:
    EmailDialog( QWidget *parent, const char *name = 0, bool modal = FALSE, WFlags fl = 0 );
    ~EmailDialog();

    void setEmails(const QString &def, const QStringList &em);

    QString defaultEmail() const;
    QStringList emails() const;

protected:
    void showEvent( QShowEvent *e );

private:
    EmailDialogList *mList;
    EmailLineEdit *mEdit;
};

#endif // EMAILDIALOGPHONE_H
