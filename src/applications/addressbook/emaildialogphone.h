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

#ifndef EMAILDIALOGPHONE_H
#define EMAILDIALOGPHONE_H

#include <QListWidget>
#include <QDialog>
#include <QStringList>
#include <QPixmap>
#include <QLineEdit>

class QAction;
class QGroupBox;

class EmailDialogList;
class EmailDialogListItem : public QListWidgetItem
{
    friend class EmailDialogList;
public:
    EmailDialogListItem( EmailDialogList *parent, const QString&, int after );
};

class EmailDialogList : public QListWidget
{
    friend class EmailLineEdit;
    Q_OBJECT
public:
    EmailDialogList( QWidget *parent, bool readonly );

    void setEmails( const QString &def, const QStringList &em );

    QString defaultEmail() const;
    QStringList emails() const;

signals:
    void editItem();
    void newItem();

public slots:
    void addEmail(const QString& email);
    void updateEmail(const QString& email);
    void deleteEmail();

protected slots:
    void setAsDefault();
    void editItem(QListWidgetItem* i);
    void updateMenus();

    void moveUp();
    void moveDown();

private:
    QIcon mDefaultPix;
    QIcon mNormalPix;
    int mDefaultIndex;
    QAction *mSetDefaultAction, *mDeleteAction, *mNewAction;
    bool readonly;
    QListWidgetItem *newItemItem;
};

class EmailLineEdit : public QLineEdit
{
    friend class EmailDialogList;
    Q_OBJECT
public:
    EmailLineEdit( QWidget *parent, const char *name = 0 );

signals:
    void setAsDefault();
    void moveUp();
    void moveDown();

protected:
    void keyPressEvent( QKeyEvent *ke );
};

class EmailDialog : public QDialog
{
    Q_OBJECT
public:
    EmailDialog( QWidget *parent, bool readonly = false);
    ~EmailDialog();

    void setEmails(const QString &def, const QStringList &em);

    QString defaultEmail() const;
    QStringList emails() const;

    QString selectedEmail() const;
protected:
    void showEvent( QShowEvent *e );
    bool eventFilter( QObject *o, QEvent *e );
    void addEmail(const QString& email);

protected slots:
    void currentChanged( QListWidgetItem* current );
    void updateCurrentText( );
    void edit();
    void newEmail();

private:
    EmailDialogList *mList;
    QGroupBox *mEditBox;
    EmailLineEdit *mEdit;
    QString mSelected;
    bool mCreatingEmail;
};

#endif // EMAILDIALOGPHONE_H
