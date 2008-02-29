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

#include "templatetext.h"
#include <qtopiaapplication.h>
#include <QSettings>
#include <qsoftmenubar.h>
#include <qaction.h>
#include <qlistwidget.h>
#include <qcontent.h>
#include <qlayout.h>
#include <qmessagebox.h>
#include <qlineedit.h>
#include <qmenu.h>

#define QTOPIA_INTERNAL_LANGLIST

TemplateTextDialog::TemplateTextDialog(QWidget *parent)
    : QDialog( parent ), userTemplates( 0 )
{
    QtopiaApplication::setMenuLike( this, true );
    setWindowTitle( tr( "Templates" ));
    QGridLayout *top = new QGridLayout( this );

    mTemplateList = new QListWidget( this );
    top->addWidget( mTemplateList, 0, 0 );
    loadTexts();
    connect(mTemplateList, SIGNAL(itemActivated(QListWidgetItem*)),
            this, SLOT(selected()) );
    connect(mTemplateList, SIGNAL(currentRowChanged(int)),
            this, SLOT(slotUpdateActions()) );

    QMenu *templateContext = QSoftMenuBar::menuFor( this );
    removeAction = new QAction(tr("Remove template item"), this );
    removeAction->setVisible(false);
    connect(removeAction, SIGNAL(triggered()), this, SLOT(slotRemove()) );
    resetAction = new QAction(tr("Reset to default"), this );
    connect(resetAction, SIGNAL(triggered()), this, SLOT(slotReset()) );
    templateContext->addAction( removeAction );
    templateContext->addAction( resetAction );
}

TemplateTextDialog::~TemplateTextDialog()
{
    saveTexts();
}

QString TemplateTextDialog::text()
{
    if (mTemplateList->currentRow() > 0)
        return mTemplateList->currentItem()->text();
    return "";
}

void TemplateTextDialog::loadTexts()
{
    QSettings conf("Trolltech","qtmail");
    conf.beginGroup("qtmailtemplates");
    mTemplateList->addItem( tr( "New template text..." ));

    int stdcount = conf.value( "std-template-count", 0 ).toInt();
    int usercount = conf.value( "user-template-count", -1 ).toInt();
    bool defaultSettings = (usercount == -1);
    if (usercount < 0)
	usercount = 0;

    //do not load templates if language has changed
    if ( Qtopia::languageList()[0] != conf.value( "language", "").toString() )
        stdcount = 0;

    if ( !defaultSettings ) {
	if ( usercount ) {
	    for (int x = 0; x < usercount; x++) {
		QString str;
		QString key = "user-" + QString::number(x + 1);
		str = conf.value( key, "" ).toString();
		if (!str.isEmpty())
		    mTemplateList->addItem( str );
	    }
	    userTemplates = usercount;
	}

	if ( stdcount) {
	    for (int x = 0; x < stdcount; x++) {
		QString str;
		QString key = "std-" + QString::number(x + 1);
		str = conf.value( key, "" ).toString();
		if (!str.isEmpty())
		    mTemplateList->addItem( str );
	    }
	}
    } else {
	// default settings
        mTemplateList->addItem( tr( ":-)", "message template" ));
        mTemplateList->addItem( tr( ":-(", "message template" ));
        mTemplateList->addItem( tr( "I love you", "message template" ));
        mTemplateList->addItem( tr( "I love you too", "message template" ));
        mTemplateList->addItem( tr( "Do you miss me?", "message template" ));
        mTemplateList->addItem( tr( "I miss you", "message template" ));
        mTemplateList->addItem( tr( "When are you coming back?", "message template" ));
        mTemplateList->addItem( tr( "I'll be there soon", "message template" ));
        mTemplateList->addItem( tr( "Are you ready yet?", "message template" ));
        mTemplateList->addItem( tr( "You're late", "message template" ));
        mTemplateList->addItem( tr( "You're very late!", "message template" ));
        mTemplateList->addItem( tr( "Please call me", "message template" ));
        mTemplateList->addItem( tr( "Please call me ASAP", "message template" ));
        mTemplateList->addItem( tr( "Please call me Now!", "message template" ));
    }

    if ( mTemplateList->count() )
        mTemplateList->setCurrentRow( 0 );
    conf.endGroup();
}

void TemplateTextDialog::saveTexts()
{
    QSettings conf("Trolltech","qtmail" );
    conf.beginGroup( "qtmailtemplates" );
    conf.clear();
    int count = mTemplateList->count();
    if (count > 0) {
        conf.setValue( "language", Qtopia::languageList()[0] );
        conf.setValue( "std-template-count", count - userTemplates - 1 );
        conf.setValue( "user-template-count", userTemplates );
        int x;
        for (x = 1; x <= userTemplates ; ++x) {
            QString str;
            if (mTemplateList->item( x ))
                str = mTemplateList->item( x )->text();
            conf.setValue( "user-" + QString::number( x ), str );
        }
        for (; x < count; ++x) {
            QString str;
            if (mTemplateList->item( x ))
                str = mTemplateList->item( x )->text();
            conf.setValue( "std-" + QString::number( x-userTemplates ), str );
        }

   }
   conf.endGroup();
}

void TemplateTextDialog::selected()
{
    if (mTemplateList->currentRow() == 0) { // New template text
        NewTemplateTextDialog *newTextDialog = new NewTemplateTextDialog();
        QtopiaApplication::execDialog( newTextDialog );
        if (newTextDialog->result() && !newTextDialog->text().isEmpty()) {
            mTemplateList->insertItem( 1, newTextDialog->text() );
            mTemplateList->setCurrentRow(1);
            userTemplates++;
        }
        delete newTextDialog;
    } else {
        done(1);
    }
}

void TemplateTextDialog::slotRemove()
{
    int cur = mTemplateList->currentRow();
    int newCur = cur;
    if (cur > 0) {
        if ((cur - 1) < userTemplates)
            --userTemplates;
        delete mTemplateList->item ( cur );
        if (newCur >= mTemplateList->count())
            newCur = mTemplateList->count() - 1;
        mTemplateList->setCurrentRow( newCur );
        slotUpdateActions();
    }
}

void TemplateTextDialog::slotReset()
{
    QString msg( tr("<p>Reset to defaults?") );
    if (QMessageBox::information(this, tr("Reset all templates"), msg,
        QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
        mTemplateList->clear();
        userTemplates = 0;
        saveTexts();
        loadTexts();
    }
}

void TemplateTextDialog::slotUpdateActions()
{
    bool normalItem = (mTemplateList->currentRow() > 0);
    removeAction->setVisible( normalItem );
}


NewTemplateTextDialog::NewTemplateTextDialog(QWidget *parent)
    : QDialog( parent )
{
    setWindowTitle( tr( "Enter new text" ) );
    QGridLayout *top = new QGridLayout( this );

    mEdit = new QLineEdit( this );
    top->addWidget( mEdit, 0, 0 );
}

QString NewTemplateTextDialog::text()
{
    return mEdit->text();
}

