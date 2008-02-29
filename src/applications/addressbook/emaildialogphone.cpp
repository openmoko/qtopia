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

#include "emaildialogphone.h"

#include <QLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QMenu>
#include <QGroupBox>

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>


EmailDialog::EmailDialog( QWidget *parent, bool readonly )
    : QDialog( parent )
{
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new EmailDialogList( this, readonly );
    mList->setFrameStyle( QFrame::NoFrame );

    l->addWidget( mList );
    if (!readonly) {
        mEditBox = new QGroupBox( tr("Email Address:"), this );
        mEdit = new EmailLineEdit( mEditBox );
        QVBoxLayout *vbox = new QVBoxLayout;
        vbox->addWidget(mEdit);
        mEditBox->setLayout(vbox);
        mEditBox->hide();

        connect( mList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), mEdit, SLOT(currentChanged(QListWidgetItem*,QListWidgetItem*)) );
        connect( mEdit, SIGNAL(textChanged(QString)), mList, SLOT(setCurrentText(QString)) );
        connect( mList, SIGNAL(editItem()), SLOT(edit()));
        mEdit->installEventFilter(this);

        connect( mEdit, SIGNAL(moveUp()), mList, SLOT(moveUp()) );
        connect( mEdit, SIGNAL(moveDown()), mList, SLOT(moveDown()) );
        l->addWidget( mEditBox );
    } else {
        connect( mList, SIGNAL(itemActivated(QListWidgetItem*)), this, SLOT(accept()) );
    }
    mList->setFocus();
    connect ( mList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(currentChanged(QListWidgetItem*)));

    setWindowState(windowState() | Qt::WindowMaximized);

    setWindowTitle( tr("Email List") );
}

EmailDialog::~EmailDialog()
{
}

void EmailDialog::edit()
{
    mEdit->selectAll();
    mEditBox->show();
    mEdit->setEditFocus(true);
}

bool EmailDialog::eventFilter( QObject *o, QEvent *e )
{
    if ( o == mEdit && e->type() == QEvent::LeaveEditFocus )
        mEditBox->hide();
    return false;
}

void EmailDialog::showEvent( QShowEvent *e )
{
    QDialog::showEvent( e );
    mList->update();
}

void EmailDialog::setEmails( const QString &def, const QStringList &em )
{
    mList->setEmails( def, em );
}

QString EmailDialog::defaultEmail() const
{
    return mList->defaultEmail();
}

QStringList EmailDialog::emails() const
{
    return mList->emails();
}

QString EmailDialog::selectedEmail() const
{
    return mSelected;
}

void EmailDialog::currentChanged(QListWidgetItem *current)
{
    if (current)
        mSelected = current->text();
    else
        mSelected.clear();
}

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent, const QString& txt, int after )
    : QListWidgetItem( txt, 0 )
{
    parent->insertItem(after, this);
}

EmailDialogList::EmailDialogList( QWidget *parent, bool ro )
    : QListWidget( parent ), readonly(ro)
{
    mDefaultPix = QIcon( ":image/email" );
    mNormalPix = QIcon();

    mDefaultIndex = -1;
    if( !Qtopia::mousePreferred() )
        setEditFocus( false );
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect( this, SIGNAL(itemActivated(QListWidgetItem*)), SLOT(editItem(QListWidgetItem*)) );
    connect( this, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), this, SLOT(updateMenus()));

    QMenu *menu = QSoftMenuBar::menuFor( this );

    mNewAction = menu->addAction(
            QIcon(":icon/new"), tr("New"), this, SLOT(newEmail()) );
    mSetDefaultAction = menu->addAction(
            QIcon(":icon/email"), tr("Set as default"),  this, SLOT(setAsDefault()) );
    mDeleteAction = menu->addAction(
            QIcon(":icon/trash"), tr("Delete"), this, SLOT(deleteEmail()) );
}

void EmailDialogList::updateMenus()
{
    mSetDefaultAction->setVisible(currentItem() != newItemItem);
    mDeleteAction->setVisible(currentItem() != newItemItem);
}

void EmailDialogList::editItem(QListWidgetItem* i)
{
    if ( i == newItemItem )
        newEmail();
    else
        emit editItem();
}

void EmailDialogList::setCurrentText( const QString &t )
{
    if( currentItem() == NULL && t.isEmpty() )
        return;             // Nothing selected, and no text entered. Nothing to do.

    if( currentItem() == NULL )
        newEmail( t );      // Nothing selected, but text entered - create new email.
    else if( currentItem()->text() != t )
        ((EmailDialogListItem *)currentItem())->setText( t );  // Set text of current item.
}

void EmailDialogList::setEmails( const QString &def, const QStringList &em )
{
    clear();
    if ( !readonly )
        newItemItem = new QListWidgetItem( QIcon(":image/email-new"), tr("Add Email Address"), this );
    QStringList::ConstIterator it;
    mDefaultIndex = -1;
    int idxCount = 0;
    EmailDialogListItem *prevItem = 0;
    for( it = em.begin() ; it != em.end() ; ++idxCount, ++it )
    {
        QString emTxt = (*it).simplified() ;
        if( emTxt.isEmpty() )
            continue;

        EmailDialogListItem *newItem = new EmailDialogListItem( this, emTxt, idxCount );
        if( emTxt == def && mDefaultIndex == -1 )
        {
            newItem->setIcon( mDefaultPix );
            mDefaultIndex = idxCount;
        } else {
            newItem->setIcon( mNormalPix );
        }
        prevItem = newItem;
    }
    if( count() )
    {
        setCurrentRow( 0 );
        scrollToItem(currentItem());
    } else if ( !readonly ) {
        newEmail();
    }
}

QString EmailDialogList::defaultEmail() const
{
    if( mDefaultIndex != -1 )
        return item( mDefaultIndex )->text();
    return QString();
}

QStringList EmailDialogList::emails() const
{
    QStringList em;
    for( int i = 0 ; i < count() ; ++i )
    {
        if( item(i) != newItemItem && !item(i)->text().trimmed().isEmpty() )
            em += item( i )->text();
    }
    return em;
}

void EmailDialogList::newEmail()
{
    newEmail( QString() );
}

void EmailDialogList::newEmail( const QString &email )
{
    int lastIdx = count()-1;
    EmailDialogListItem *newItem = new EmailDialogListItem( this, email, lastIdx );
    setCurrentItem(newItem);
    scrollToItem(newItem);
    if( lastIdx == 0 )
        setAsDefault();
    else
        newItem->setIcon(mNormalPix);
    emit itemActivated(newItem);
}

void EmailDialogList::deleteEmail()
{
    const int ci = currentRow();
    if( ci != -1 )
    {
        delete takeItem( ci );
        if( count() )
        {
            int ni = (ci > 0 ? ci-1 : 0);
            setCurrentRow(ni);
            if( ci == mDefaultIndex )
            {
                mDefaultIndex = -1;
                setAsDefault();
            }
        }
        else
        {
            mDefaultIndex = -1;
        }
    }
}

void EmailDialogList::setAsDefault()
{
    if( currentItem() != 0 )
    {
        if( mDefaultIndex != -1 )
            ((EmailDialogListItem *)item( mDefaultIndex ))->setIcon(mNormalPix);
        mDefaultIndex = currentRow();
        ((EmailDialogListItem *)item( mDefaultIndex ))->setIcon( mDefaultPix );
    }
}


void EmailDialogList::moveUp()
{
    if( !count() )
        return;

    if( ( currentRow() != -1) && item( currentRow() )->text().isEmpty() )
    {
        deleteEmail();
    }

    int curIdx = currentRow();
    --curIdx;
    if( curIdx < 0 )
        curIdx = count()-1;
    setCurrentRow( curIdx );
}

void EmailDialogList::moveDown()
{
    if( !count() )
        return;

    if( ( currentRow() != -1) && item( currentRow() )->text().isEmpty() )
    {
        deleteEmail();
    }

    int curIdx = currentRow();
    ++curIdx;
    if( curIdx >= (int) count() )
        curIdx = 0;
    setCurrentRow( curIdx );
}

EmailLineEdit::EmailLineEdit( QWidget *parent, const char *name )
    : QLineEdit( parent )
{
    setObjectName( name );
}

void EmailLineEdit::currentChanged( QListWidgetItem *current, QListWidgetItem* previous )
{
    Q_UNUSED(previous);

    if( current )
    {
        setText( current->text() );
        selectAll();
    }
    else
    {
        setText( "" );
    }
}

void EmailLineEdit::keyPressEvent( QKeyEvent *ke )
{
    if( ke->key() == Qt::Key_Up )
    {
        emit moveUp();
        ke->accept();
    }
    else if( ke->key() == Qt::Key_Down )
    {
        emit moveDown();
        ke->accept();
    }
    else
        QLineEdit::keyPressEvent( ke );
}
