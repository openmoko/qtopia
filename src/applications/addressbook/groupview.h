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
#ifndef GROUPVIEW_H
#define GROUPVIEW_H

#include <QUniqueId>
#include <QWidget>
#include <QDialog>
#if defined(QTOPIA_TELEPHONY)
#include <QContent>
#endif

/* a view that shows categories/groups, and when one is selected
   shows the items belonging to that group.
   Also has context menu for selecting items to insert into that group.
*/

class GroupViewData;
class QModelIndex;
class QLineEdit;
class QItemSelectionModel;
#if defined(QTOPIA_TELEPHONY)
class RingToneSelect;
#endif

class GroupView : public QWidget
{
    Q_OBJECT
public:
    GroupView(bool allowMultiple=false, QWidget *parent = 0);
    ~GroupView();

    void setCurrentGroup(const QString &);
    QString currentGroup()const ;
    bool isCurrentSystemGroup() const;

    QStringList selectedGroups() const;
    void setSelectedGroups(QStringList groups);

    // QAbstractItemView like API
    QModelIndex currentIndex() const;
    void setCurrentIndex(const QModelIndex&);
    QItemSelectionModel *selectionModel() const;

signals:
    void groupActivated(const QString &id);
    void groupHighlighted(const QString &id);
    void backClicked();

public slots:
    void addGroup();
    void removeCurrentGroup();
    void editCurrentGroup();
#if defined(QTOPIA_TELEPHONY)
    void setGroupRingTone();
#endif
    void renameCurrentGroup();

protected:
    void keyPressEvent(QKeyEvent *);
    bool eventFilter( QObject *, QEvent *);

private slots:
    void updateGroups();
    void setCurrentIndex(const QModelIndex &, const QModelIndex &);
    void activateIndex(const QModelIndex &);

private:
    GroupViewData *d;
};

/*
   list of checkable items to say whether a contact is in the group/category or not.
*/
class GroupMembersData;
class GroupMembers : public QDialog
{
    Q_OBJECT
public:
    GroupMembers(QWidget * = 0);
    ~GroupMembers();

    void setGroup(const QString &);
    QString group() const;

    void accept();
    void reject();

protected slots:
    void search(const QString &);

private:
    void toggleGroupCheckbox(const QModelIndex &);

private:
    GroupMembersData *d;
};

/*
   retrieves a name for the group
   */
class GroupEdit : public QDialog
{
    Q_OBJECT
public:
    GroupEdit(QWidget * = 0);
    ~GroupEdit();

    void setName(const QString &);
    QString name() const;
private:
    QLineEdit *mName;
};

#if defined(QTOPIA_TELEPHONY)
/*
   selects a ringtone for the group
   */
class RingToneSelector : public QDialog
{
    Q_OBJECT
public:
    RingToneSelector(QWidget * = 0);
    ~RingToneSelector();

    void setCurrentTone( const QContent &tone );
    QString selectedRingTone() const;
private:
    RingToneSelect *mList;
};
#endif

#endif
