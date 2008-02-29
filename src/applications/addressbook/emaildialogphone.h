/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef EMAILDIALOGPHONE_H
#define EMAILDIALOGPHONE_H

#include <QListWidget>

#include <qdialog.h>
#include <qstringlist.h>
#include <qpixmap.h>
#include <qlineedit.h>

class QAction;

class EmailDialogList;
class EmailDialogListItem : public QListWidgetItem
{
    friend class EmailDialogList;
public:
    EmailDialogListItem( EmailDialogList *parent = 0);
    EmailDialogListItem( EmailDialogList *parent, int after );

    void setPixmap( const QPixmap &pix );
    virtual const QPixmap *pixmap() const;

    virtual int width( const QListWidget *lb ) const;
    virtual int height( const QListWidget *lb ) const;
protected:
    void setText( const QString &txt );

    virtual void paint( QPainter *p );
private:
    QPixmap mDefaultPix;
};

class EmailDialogList : public QListWidget
{
    friend class EmailLineEdit;
    Q_OBJECT
public:
    EmailDialogList( QWidget *parent );

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
    void currentChanged( QListWidgetItem* current, QListWidgetItem *previous );

protected:
    void keyPressEvent( QKeyEvent *ke );

private:
    QAction *mSetDefaultAction, *mDeleteAction, *mNewAction;
};

class EmailDialog : public QDialog
{
    Q_OBJECT
public:
    EmailDialog( QWidget *parent, const char *name = 0, bool modal = false, Qt::WFlags fl = 0 );
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
