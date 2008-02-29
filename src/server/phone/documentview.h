/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
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

#ifndef _DOCUMENTVIEW_H_
#define _DOCUMENTVIEW_H_

#include "launcherview.h"

class QContentFilterDialog;
class QLabeledProgressBar;
class QValueSpaceItem;

class DocumentLauncherView : public LauncherView
{
    Q_OBJECT
    public:
        DocumentLauncherView(QWidget* parent = 0, Qt::WFlags fl = 0);
        virtual ~DocumentLauncherView();

        virtual void setFilter( const QContentFilter &filter );

    private slots:
        void launcherRightPressed(QContent);
        void beamDoc();
        void deleteDocWorker();
        void deleteDoc(int r);
        void deleteDoc();
        void propertiesDoc();
        void openRightsIssuerURL();
        void selectDocsType();
        void selectDocsCategory();
        void contentSetChanged();
        void updateScanningStatus();
        void currentChanged( const QModelIndex &current, const QModelIndex &previous );

    private:
        QLabel *typeLbl;
        QAction *actionDelete;
        QAction *actionProps;
        QContent deleteLnk;
        QAbstractMessageBox *deleteMsg;
        QDialog *propDlg;
        QContent propLnk;
        QMenu *rightMenu;
        QAction *actionBeam;
        QAction *actionRightsIssuer;
        QContentFilterDialog *typeDlg;
        QLabel *categoryLbl;
        QContentFilterDialog *categoryDlg;
        QLabeledProgressBar *scanningBar;
        QTimer *scanningBarUpdateTimer;
        QValueSpaceItem *scanningVSItem;
        QValueSpaceItem *updatingVSItem;
        QValueSpaceItem *installingVSItem;

    public:
        QAction *separatorAction;
};

#endif // _DOCUMENTVIEW_H_
