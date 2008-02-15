/****************************************************************************
**
** Copyright (C) 2007-2008 TROLLTECH ASA. All rights reserved.
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

#ifndef LISTSCREEN_H
#define LISTSCREEN_H

#include <QListWidget>
#include <QContent>
#include <QKeyEvent>

class IViewer;
class QContentSet;
class QContentSetModel;
class QAction;
class QKeyEvent;
class ImageScreen;
class InfoScreen;

class ListScreen : public QListWidget
{
    Q_OBJECT
public:
    ListScreen(IViewer *viewer);
private:
    void createActions();
    void createMenu();
    void setupContent();
    void populate();
    void setupUi();
    void openImage(int row); 
    void keyPressEvent(QKeyEvent *event);
private slots:
    void onOpenImage();
    void onImageActivated(const QModelIndex &index);
    void onRenameImage();
    void onDeleteImage();
    void onShowInfo();
    void updateList(QContentIdList a,QContent::ChangeType b);
private:
    IViewer          *_viewer;
    QContentSet      *_cs;
    QContentSetModel *_csm;
    QAction          *_openAction;
    QAction          *_renameAction;
    QAction          *_deleteAction;
    QAction          *_showInfoAction;
};

#endif
