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


#include "viewatt.h"
#include <qcontent.h>

#include <qlayout.h>
#include <qlabel.h>
#include <qtablewidget.h>
#include <qheaderview.h>
#include <qevent.h>

//#define ENACT_INSTALLATION_IMMEDIATELY

ViewAtt::ViewAtt(Email *mailIn, bool _inbox, QWidget *parent, Qt::WFlags f)
    : QDialog(parent, f)
{
    mail = mailIn;
    inbox = _inbox;

    setWindowTitle( tr( "Attachments" ) );
    QVBoxLayout* vb = new QVBoxLayout( this );
    vb->setSpacing( 6 );

    label = new QLabel( this );
    label->setWordWrap( true );
    vb->addWidget( label );
    listView = new QTableWidget( this );
    listView->installEventFilter( this );
    listView->horizontalHeader()->setFocusPolicy( Qt::NoFocus );
    listView->verticalHeader()->setFocusPolicy( Qt::NoFocus );
    vb->addWidget( listView );
    listView->setFocus();
    QStringList columns;
    columns << tr( "Attachment" );
    columns << tr( "Type" );
    listView->setColumnCount( columns.count() );
    listView->setHorizontalHeaderLabels( columns );
    listView->verticalHeader()->hide();

    init();
}

void ViewAtt::accept()
{
#ifndef ENACT_INSTALLATION_IMMEDIATELY
    int i;
    for (i = 0; i < listView->rowCount(); ++i) {
        QTableWidgetItem *item = listView->item( i, 0 );
        setInstall( item );
    }
#endif
    QDialog::accept();
}

void ViewAtt::reject()
{
#ifdef ENACT_INSTALLATION_IMMEDIATELY
    // reset to initial state
    QListIterator<QTableWidgetItem*> it = on->entryIterator();
    while ( it.hasNext() ) {
        QTableWidgetItem *item = it.next();
        if ( item->checkState() != Qt::Checked ) {
            item->setChecked( Qt::Checked );
            setInstall( item );
        }
    }
    QListIterator<QTableWidgetItem*> it = off->entryIterator();
    while ( it.hasNext() ) {
        QTableWidgetItem *item = it.next();
        if ( item->checkState() == Qt::Checked ) {
            item->setChecked( Qt::Unchecked );
            setInstall( item );
        }
    }
#endif
    QDialog::reject();
}


void ViewAtt::init()
{
    if (inbox) {
        label->setText( tr("<p>Check attachments to add to Documents") );
#ifdef ENACT_INSTALLATION_IMMEDIATELY
        connect( listView, SIGNAL(clicked(QTableWidgetItem*)),
                 this, SLOT(setInstall(QTableWidgetItem*)) );
#endif
    } else {
        label->setText(tr("<p>These are the attachments in this mail"));
    }

    for ( int i = 0; i < (int)mail->messagePartCount(); i++ ) {
        MailMessagePart &part = mail->messagePartAt( i );
    QString name = part.prettyName();
#ifdef QTOPIA_PHONE
    if(part.name().isEmpty() && !part.contentLocation().isEmpty())
        name = part.contentLocation();
#endif
        if ( inbox ) {
            bool ins = !part.linkFile().isEmpty();
            if ( ins ) {
                ins = QContent( part.linkFile() ).isValid();
            }
            QTableWidgetItem *item = new QTableWidgetItem( name );
            item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
            item->setCheckState( ins ? Qt::Checked : Qt::Unchecked );
            listView->setRowCount( listView->rowCount() + 1 );
            listView->setItem( listView->rowCount() - 1, 0, item );
            item = new QTableWidgetItem( part.contentType() );
            item->setFlags( Qt::ItemIsEnabled );
            listView->setItem( listView->rowCount() - 1, 1, item );
#ifdef ENACT_INSTALLATION_IMMEDIATELY
            if ( ins )
                on.append(item);
            else
                off.append(item);
#endif
        } else {
            QTableWidgetItem *item = new QTableWidgetItem( name );
            listView->setRowCount( listView->rowCount() + 1 );
            listView->setItem( listView->rowCount() - 1, 0, item );
            item = new QTableWidgetItem( part.contentType() );
            listView->setItem( listView->rowCount() - 1, 1, item );
        }
    }
    if(!inbox)
        listView->setSelectionMode(QAbstractItemView::NoSelection);
    if(listView->rowCount() > 0)
        listView->selectRow(0);
}

void ViewAtt::setInstall(QTableWidgetItem* i)
{
    bool res = true;
    if ( !inbox )
        return;

    if ( i ) {
        int row = listView->row( i );
        QTableWidgetItem *item = listView->item( row, 0 );
        QString filename  = item->text(); // unique?
        bool checked = item->checkState() == Qt::Checked;

        if ( !mail->setAttachmentInstalled(filename, checked )) {
            // beep? status error message?
            item->setCheckState( checked ? Qt::Unchecked : Qt::Checked );
            res = false;
        }
    }

    if (!res) {
        QString title( tr("Attachment error") );
        QString msg( "<qt>" + tr("Storage for documents is full.<br><br>"
                                 "Some attachments could not be saved.") +
                     "</qt>" );
        QMessageBox::warning(qApp->activeWindow(), title, msg, tr("OK") );
    }

}

bool ViewAtt::eventFilter( QObject *o, QEvent *e )
{
    if ((o == listView) && (e->type() == QEvent::KeyPress)) {
        QKeyEvent *keyEvent = (QKeyEvent*)e;
        switch( keyEvent->key() ) {
#ifdef QTOPIA_PHONE
        case Qt::Key_Select:
#endif
        case Qt::Key_Space:
        case Qt::Key_Return:
        case Qt::Key_Enter:
        {
#ifdef QTOPIA_PHONE
            if(!inbox)
                return false;
            if(!listView->hasEditFocus())
                listView->setEditFocus(true);
#endif
            QTableWidgetItem* i = listView->currentItem();
            if (i) {
                int row = listView->row( i );
                QTableWidgetItem *item = listView->item( row, 0 );
                bool checked = item->checkState() == Qt::Checked;
                item->setCheckState( checked ? Qt::Unchecked : Qt::Checked );
                return true;
            }
            return false;
        }
        break;
        default:
            return false;
        }
    }
    return QDialog::eventFilter( o, e );
}
