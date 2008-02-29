/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA All rights reserved.
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

#ifndef PACKAGEVIEW_H
#define PACKAGEVIEW_H

#include <QDialog>
#include <QModelIndex>
#include <QtopiaAbstractService>
#include <QDSActionRequest>

class PackageModel;
class QTreeView;
class QMenu;
class QActionGroup;
class QTextEdit;
class QAction;
class QShowEvent;

#include "ui_packagedetails.h"
class PackageDetails : public QDialog, public Ui::PackageDetails
{
public:
    enum{ Reenable = 3 };
    PackageDetails(QWidget *parent, bool modal)
                    : QDialog(parent)
    {
        setupUi(this);
        setModal(modal);
        connect( reenableButton, SIGNAL(clicked()),
             this, SLOT(reenable()) );
        reenableButton->setVisible( false );
    }

    Q_OBJECT
private slots:
    void reenable() { done(Reenable); }
};

class PackageView : public QDialog
{
    Q_OBJECT
    friend class PackageManagerService;
    friend class PackageServiceInstaller;
public:

    PackageView(QWidget* parent = 0, Qt::WFlags flags = 0);
    ~PackageView();

    static void displayMessage( const QString & );

signals:
    void targetChoiceChanged( const QString & );
    void serverListUpdate( const QStringList &, const QStringList & );

protected:
#ifdef QTOPIA_PHONE
    void keyPressEvent( QKeyEvent * );
#endif

    void showEvent( QShowEvent * );

private slots:
    void init();
    void editServers();
    void serversChanged( const QStringList & );
    void serverChoice( QAction* a );
    void targetsChanged( const QStringList & );
    void targetChoice( QAction* );
    void showDetails( const QModelIndex & );
    void installSelection();
    void updateText( const QModelIndex&, const QModelIndex& );
    void activateItem( const QModelIndex & );
    void postDomainUpdate( const QString & );

private:
    QTreeView *view;
    PackageModel *model;
    QTextEdit *info;
    QString prevTarget;

    bool listsPrepared;

    QMenu *menuServers;
    QMenu *menuTarget;
    QActionGroup *targetActionGroup;
    QActionGroup *serversActionGroup;

    static PackageView *latestInstance;
};

#endif
