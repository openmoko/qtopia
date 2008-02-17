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



#ifndef MAILLISTVIEW_H
#define MAILLISTVIEW_H

#include <qtablewidget.h>
#include <qtimer.h>
#include <qsettings.h>
#include <qsoftmenubar.h>
#include <QMailMessage>
#include "emaillistitem.h"

class MailListView : public QTableWidget
{
        Q_OBJECT

public:
    MailListView(QWidget *parent, const char *name);
    int sortedColumn();
    bool isAscending();
    void setSorting(int, bool);
    void setShowEmailsOnly(bool);
    bool showEmailsOnly();

    int labelPos(int at);
    void moveSection(int section, int toIndex);
    void defineSort(int column, bool ascend);
    EmailListItem *getRef(QMailId id);

    void setByArrival(bool on);
    bool arrivalDate();

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    QString currentMailbox();
    void setCurrentMailbox(const QString &mailbox);

    void treeInsert(const QMailId& id, const QSoftMenuBar::StandardLabel );
    void treeInsert(const QMailIdList& idList, const QSoftMenuBar::StandardLabel );
    void clear();

    void ensureWidthSufficient(const QString &text);
    bool singleColumnMode();
    void setSingleColumnMode(bool singleColumnMode);

    void readConfig( QSettings *conf );
    void writeConfig( QSettings *conf );

    EmailListItem* emailItemFromIndex( const QModelIndex & i ) const;

    void setSelectedItem(QTableWidgetItem* item);
    void setSelectedId(const QMailId& id);
    void setSelectedRow(int row);
    void resetNameCaches();

signals:
    void itemPressed(EmailListItem *);
    void backPressed();
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
    bool ascending, hVisible, arrival, emailsOnly;
    int maxColumnWidth;
    bool mSingleColumnMode;
    QStringList columns;

    QString _mailbox;
};

#endif
