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



#ifndef MAILLISTVIEW_H
#define MAILLISTVIEW_H

#include <qtablewidget.h>
#include <qtimer.h>
#include <qsettings.h>

#include "emaillistitem.h"
#include <qtopia/pim/qcontactmodel.h>

class MailListView : public QTableWidget
{
        Q_OBJECT

public:
    MailListView(QWidget *parent, const char *name);
    int sortedColumn();
    bool isAscending();
    void setSorting(int, bool);

    int labelPos(int at);
    void moveSection(int section, int toIndex);
    void defineSort(int column, bool ascend);
    uint getMailCount(QString type);
    EmailListItem *getRef(QUuid id);

    void setByArrival(bool on);
    bool arrivalDate();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QString currentMailbox();
    void setCurrentMailbox(const QString &mailbox);

    void treeInsert(Email *mail);
    void clear();

    void ensureWidthSufficient(const QString &text);
    bool singleColumnMode();
    void setSingleColumnMode(bool singleColumnMode);

    void readConfig( QSettings *conf );
    void writeConfig( QSettings *conf );

    EmailListItem* emailItemFromIndex( const QModelIndex & i ) const;

    QContactModel *contactModel(); // for addressbook lookup

signals:
    void itemPressed(EmailListItem *);
    void viewFolderList();
    void enableMessageActions( bool );

protected slots:
    void sizeChange(int, int, int);
    virtual void keyPressEvent( QKeyEvent *e ); // sharp and phone
    void mousePressEvent( QMouseEvent * e );
    void mouseReleaseEvent( QMouseEvent * e );
    void cancelMenuTimer();
    void itemMenuRequested();

    void scrollToLeft(int);


private:
    QTimer menuTimer;
    int sortColumn;
    bool ascending, hVisible, arrival;
    int maxColumnWidth;
    bool mSingleColumnMode;
    QStringList columns;

    QString _mailbox;
    QContactModel *mContactModel;
};

#endif
