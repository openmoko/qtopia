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
#ifndef CONTACTMESSAGEHISTORYLIST_H
#define CONTACTMESSAGEHISTORYLIST_H

#include <QWidget>
#include <qtopia/pim/qcontact.h>

class ContactMessageHistoryModel;
class QListView;
class QContactModel;
class QModelIndex;

class ContactMessageHistoryList : public QWidget
{
    Q_OBJECT

public:
    ContactMessageHistoryList( QWidget *parent );
    virtual ~ContactMessageHistoryList();

    QContact entry() const {return ent;}

    void setModel(QContactModel *model);

public slots:
    void init( const QContact &entry );

signals:
    void externalLinkActivated();
    void backClicked();

protected:
    void keyPressEvent( QKeyEvent *e );
    bool eventFilter(QObject*, QEvent*);

protected slots:
    void updateItemUI(const QModelIndex& idx);
    void showMessage(const QModelIndex &idx);

private:
    QContact ent;
    bool mInitedGui;
    ContactMessageHistoryModel *mModel;
    QListView *mListView;
    QContactModel *mContactModel;
};

#endif // CONTACTMESSAGEHISTORYLIST_H

