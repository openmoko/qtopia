/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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
#ifndef CONTACTLISTPANE_H
#define CONTACTLISTPANE_H

#include <QWidget>
#include "qcontact.h"
#include "qpimsource.h"
#include "quniqueid.h"
#include "qcategorymanager.h"

#ifdef GREENPHONE_EFFECTS
#include <private/qsmoothlist_p.h>
#endif

class QContactListView;
class QTextEntryProxy;
class QLabel;
class QContactModel;
class QVBoxLayout;
class QModelIndex;
class QAbstractItemDelegate;

class ContactListPane : public QWidget
{
    Q_OBJECT
    public:
        ContactListPane(QWidget *w, QContactModel* model);

        QList<QUniqueId> selectedContactIds();

        QContact currentContact() const;
        void setCurrentContact(const QContact& contact);
        void showCategory(const QCategoryFilter &f);
        void resetSearchText();

        QContactModel* contactModel() const { return mModel; }

    signals:
        void contactActivated( QContact c );
        void backClicked();
        void currentChanged(const QModelIndex &, const QModelIndex &);

    public slots:
        void contactsChanged();

#ifdef QTOPIA_CELL
        void showLoadLabel(bool);
#endif

    protected:
        bool eventFilter( QObject *o, QEvent *e );
        void closeEvent( QCloseEvent *e );

    protected slots:
        void updateIcons();
        void search( const QString &k );
        void contactActivated(const QModelIndex &);

    protected:
        QContactListView *mListView;
#ifdef GREENPHONE_EFFECTS
        QSmoothList *mSmoothListView;
#endif
        QTextEntryProxy *mTextProxy;
#ifdef QTOPIA_CELL
        QLabel *mLoadingLabel;
#endif
        QLabel *mCategoryLabel;
        QContactModel *mModel;
        QVBoxLayout *mLayout;
        QAbstractItemDelegate *mDelegate;
        QLabel *mFindIcon;
};

#endif // CONTACTLISTPANE_H

