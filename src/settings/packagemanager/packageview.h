/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA All rights reserved.
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

#ifndef PACKAGEVIEW_H
#define PACKAGEVIEW_H

#include <QMainWindow>
#include <QModelIndex>
#include <QtopiaAbstractService>
#include <QDSActionRequest>
#include <QTabWidget>
#include <QtopiaItemDelegate>

class PackageModel;
class QTreeView;
class QMenu;
class QActionGroup;
class QTextEdit;
class QAction;
class QShowEvent;
class QWaitWidget;
class QTabWidget;

#include "ui_packagedetails.h"
class PackageDetails : public QDialog, public Ui::PackageDetails
{
    Q_OBJECT
public:
    enum Type { Info, Confirm };
    enum Result { Proceed = QDialog::Accepted + 1};
    enum Option {
                    //used for Info type
                    None        = 0x0,
                    Install     = 0x1,
                    Uninstall   = 0x2,

                    //used for Confirm type
                    Allow       = 0x4,
                    Disallow    = 0x8
                };
    Q_DECLARE_FLAGS( Options, Option );

    PackageDetails(QWidget *parent,
                   const QString &title,
                   const QString &text,
                   Type type,
                   Options options);
    void init();

private:
    Type m_type;
    Options m_options;

    QAction *m_acceptAction;
    QAction *m_rejectAction;
    QMenu *m_contextMenu;
};
Q_DECLARE_OPERATORS_FOR_FLAGS(PackageDetails::Options);

class ViewDelegate: public QtopiaItemDelegate
{
    public:
        ViewDelegate( QObject *parent = 0 );
        virtual void paint( QPainter *painter,
                        const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const;
};

class PackageView : public QMainWindow
{
    Q_OBJECT
    friend class PackageManagerService;
    friend class PackageServiceInstaller;
public:

    PackageView(QWidget* parent = 0, Qt::WFlags flags = 0);
    ~PackageView();

signals:
    void targetChoiceChanged( const QString & );
    void serverListUpdate( const QStringList &, const QStringList & );

private slots:
    void init();
    void editServers();
    void serversChanged( const QStringList & );
    void serverChoice( QAction* a );
    void targetsChanged( const QStringList & );
    void targetChoice( QAction* );
    void showDetails( const QModelIndex &, PackageDetails::Type type );
    void displayDetails();
    void startInstall();
    void startUninstall();
    void confirmReenable();
    void activateItem( const QModelIndex & );
    void contextMenuShow();
    void postServerStatus( const QString & );
    void selectNewlyInstalled( const QModelIndex & );
private:
    QTreeView *installedView;
    QTreeView *downloadView;
    QTabWidget *tabWidget;
    PackageModel *model;
    QString prevTarget;
    QLabel *statusLabel;
    QWaitWidget *waitWidget;

    QMenu *menuServers;
    QMenu *menuTarget;
    QActionGroup *targetActionGroup;
    QActionGroup *serversActionGroup;
    QAction *reenableAction;
    QAction *detailsAction;
    QAction *installAction;
    QAction *uninstallAction;

    static const int InstalledIndex;
    static const int DownloadIndex;
};

class KeyFilter:public QObject
{
    Q_OBJECT

    public:
        KeyFilter(QTabWidget * tab, QObject *parent=0);

    protected:
        bool eventFilter(QObject *obj, QEvent *event);
        QTabWidget *m_tab;
};

#endif
