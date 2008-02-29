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

#include "selectfolder.h"
#include <qtopiaapplication.h>
#include <qlistwidget.h>
#include <qlayout.h>
#include "emailfolderlist.h"

SelectFolderDialog::SelectFolderDialog(const QStringList list,
                                       QWidget *parent)
    : QDialog( parent )
{
    QtopiaApplication::setMenuLike( this, true );
    setWindowTitle( tr( "Select folder" ) );
    QGridLayout *top = new QGridLayout( this );
    for( int i = 0; i < list.count(); i++ ) {
        mMailboxList.append(MailboxList::mailboxTrName(list[i]));
    }

    mFolderList = new QListWidget( this );
    top->addWidget( mFolderList, 0, 0 );
    getFolders();

    // Required for current item to be shown as selected(?)
    if (mFolderList->count())
        mFolderList->setCurrentRow( 0 );

    connect(mFolderList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(selected()) );
}

SelectFolderDialog::~SelectFolderDialog()
{
}

int SelectFolderDialog::folder()
{
    return mFolderList->currentRow();
}

void SelectFolderDialog::getFolders()
{
    QStringList mboxList = mMailboxList;
    for (QStringList::Iterator it = mboxList.begin(); it != mboxList.end(); ++it)
        mFolderList->addItem( *it );
}

void SelectFolderDialog::selected()
{
    done(1);
}

