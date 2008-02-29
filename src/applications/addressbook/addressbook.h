/**********************************************************************
** Copyright (C) 2000-2002 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
**
** This file may be distributed and/or modified under the terms of the
** GNU General Public License version 2 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/
#ifndef Addressbook_H
#define Addressbook_H

#include <qmainwindow.h>
#include <qmap.h>

class AbEditor;
class AbLabel;
class AbTable;
class QPEToolBar;
class QPopupMenu;
class QToolButton;
class QLineEdit;
class QDialog;
class Ir;
class QAction;
class CategorySelect;
class QVBox;

class AddressbookWindow: public QMainWindow
{
    Q_OBJECT
public:
    AddressbookWindow( QWidget *parent = 0, const char *name = 0, WFlags f = 0 );
    ~AddressbookWindow();

protected:
    void resizeEvent( QResizeEvent * e );
    void showList();
    void showView();
    enum EntryMode { NewEntry=0, EditEntry };
    void editPersonal();
    void editEntry( EntryMode );
    void duplicateEntry();
    void closeEvent( QCloseEvent *e );

public slots:
    void appMessage(const QCString &, const QByteArray &);
    void setDocument( const QString & );
    void reload();
    void flush();

private slots:
    void slotListNew();
    void slotListView();
    void slotListDelete();
    void slotViewBack();
    void slotViewEdit();
    void slotViewDuplicate();
    void slotPersonalView();
    void listIsEmpty( bool );
    void slotSettings();
    void writeMail();
    void slotBeam();
    void beamDone( Ir * );
    void slotFind(bool);
    void search();
    void findWrapped();
    void findNotFound();
    void findFound();
    void slotSetCategory( int );
    void slotUpdateToolbar(int row, int col);
    void contactFilterSelected( int idx );

private:
    void receiveFile( const QString & );
    void initFields();	// inititialize our fields...
    AbLabel *abView();

    QPEToolBar *listTools;
    QToolButton *deleteButton;
    CategorySelect *catSelect;
    QValueList<int> allFields,
	            orderedFields;
    QStringList slOrderedFields;
    enum Panes { paneList=0, paneView, paneEdit };
    QVBox *listView;
    AbEditor *abEditor;
    AbLabel *mView;
    AbTable *abList;

    QPEToolBar *searchBar;
    QLineEdit *searchEdit;

    QAction *actionNew, *actionEdit, *actionTrash, *actionFind, *actionBeam, 
	*actionPersonal, *actionMail, *actionDuplicate;

    bool bAbEditFirstTime;
    int viewMargin;

    bool syncing;
    QMap<int,int> contactMap;
};

#endif
