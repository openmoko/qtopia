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

#include "emaildialogphone.h"

#include <QAction>
#include <QLayout>
#include <QPainter>
#include <QKeyEvent>
#include <QMenu>

#include <qsoftmenubar.h>
#include <qtopiaapplication.h>


EmailDialog::EmailDialog( QWidget *parent, const char *name, bool modal, Qt::WFlags fl )
    : QDialog( parent, fl )
{
    setObjectName(name);
    setModal(modal);
    QVBoxLayout *l = new QVBoxLayout( this );
    mList = new EmailDialogList( this );
    mList->setFocusPolicy( Qt::NoFocus );
    mList->setFrameStyle( QFrame::NoFrame );

    mEdit = new EmailLineEdit( this );

    connect( mList, SIGNAL(currentItemChanged(QListWidgetItem*,QListWidgetItem*)), mEdit, SLOT(currentChanged(QListWidgetItem*,QListWidgetItem*)) );
    connect( mEdit, SIGNAL(textChanged(const QString&)), mList, SLOT(setCurrentText(const QString&)) );

    connect( mEdit, SIGNAL(newEmail()), mList, SLOT(newEmail()) );
    connect( mEdit, SIGNAL(deleteEmail()), mList, SLOT(deleteEmail()) );
    connect( mEdit, SIGNAL(setAsDefault()), mList, SLOT(setAsDefault()) );
    connect( mEdit, SIGNAL(moveUp()), mList, SLOT(moveUp()) );
    connect( mEdit, SIGNAL(moveDown()), mList, SLOT(moveDown()) );

    l->addWidget( mList );
    l->addWidget( mEdit );
    mEdit->setFocus();

    setWindowState(windowState() | Qt::WindowMaximized);

    setWindowTitle( tr("Email List") );
}

EmailDialog::~EmailDialog()
{
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

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent )
    : QListWidgetItem( parent )
{
}

EmailDialogListItem::EmailDialogListItem( EmailDialogList *parent, int after )
    : QListWidgetItem( 0 )
{
    parent->insertItem(after, this);
}

void EmailDialogListItem::setText( const QString &txt )
{
    QListWidgetItem::setText( txt );
    listWidget()->update();
}

void EmailDialogListItem::setPixmap( const QPixmap &pix )
{
    mDefaultPix = pix;
    listWidget()->update();
}

const QPixmap *EmailDialogListItem::pixmap() const
{
    return &mDefaultPix;
}

int EmailDialogListItem::width( const QListWidget *lb ) const
{
    return lb->viewport()->width();
}

const int mgn = 2;
int EmailDialogListItem::height( const QListWidget *lb ) const
{
    QFontMetrics fm = lb->fontMetrics();
    const int pixWidth = (mDefaultPix.width() > 0 ? mDefaultPix.width() : 24);
    int h = qMax( mDefaultPix.height()+mgn*2,
                                        fm.boundingRect( 0, 0, width(lb)-pixWidth-mgn,
                                        fm.lineSpacing(), 0,
                                        text().simplified()).height()+(mgn*2) );
    return h;
}

void EmailDialogListItem::paint( QPainter *p )
{

    //TODO : width of pixmap shouldn't be a hardcoded value
    const int itemWidth = width( listWidget() );
    const int itemHeight = height( listWidget() );
    const int w = itemWidth;
    const int h = itemHeight-(mgn*2);
    const int pixWidth = (mDefaultPix.width() > 0 ? mDefaultPix.width() : 24);
    int x = 0,
        y = 0+mgn;
    p->drawText( x, y, w-pixWidth-mgn, h, 0, text() );
    if( !mDefaultPix.isNull() )
    {
        x = w-pixWidth;
        y = itemHeight/2 - mDefaultPix.height()/2;
        p->drawPixmap( x, y, mDefaultPix );
    }
}

EmailDialogList::EmailDialogList( QWidget *parent )
    : QListWidget( parent )
{
    mDefaultPix = QPixmap( ":image/email" );
    mDefaultIndex = -1;
#ifdef QTOPIA_PHONE
    if( !Qtopia::mousePreferred() )
        setEditFocus( false );
#endif
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    QStringList::ConstIterator it;
    mDefaultIndex = -1;
    int idxCount = 0;
    EmailDialogListItem *prevItem = 0;
    for( it = em.begin() ; it != em.end() ; ++idxCount, ++it )
    {
        QString emTxt = (*it).simplified() ;
        if( emTxt.isEmpty() )
            continue;

        EmailDialogListItem *newItem = new EmailDialogListItem( this, idxCount );
        newItem->setText( emTxt );
        if( emTxt == def && mDefaultIndex == -1 )
        {
            newItem->setPixmap( mDefaultPix );
            mDefaultIndex = idxCount;
        }
        prevItem = newItem;
    }
    if( count() )
    {
        setCurrentRow( 0 );
        scrollToItem(currentItem());
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
        if( !item( i )->text().trimmed().isEmpty() )
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
    EmailDialogListItem *newItem = new EmailDialogListItem( this, lastIdx );
    newItem->setText( (!email.isNull() ? email : tr("user@domain")) );
    setCurrentItem(newItem);
    scrollToItem(newItem);
    if( lastIdx == -1 )
        setAsDefault();
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
            ((EmailDialogListItem *)item( mDefaultIndex ))->setPixmap( QPixmap() );
        mDefaultIndex = currentRow();
        ((EmailDialogListItem *)item( mDefaultIndex ))->setPixmap( mDefaultPix );
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

    QMenu *menu = QSoftMenuBar::menuFor( this );

    mNewAction = menu->addAction(
            QIcon(":icon/new"), tr("New"), this, SIGNAL(newEmail()) );
    mSetDefaultAction = menu->addAction(
            QIcon(":icon/email"), tr("Set as default"),  this, SIGNAL(setAsDefault()) );
    mDeleteAction = menu->addAction(
            QIcon(":icon/trash"), tr("Delete"), this, SIGNAL(deleteEmail()) );
}

void EmailLineEdit::currentChanged( QListWidgetItem *current, QListWidgetItem* previous )
{
    Q_UNUSED(previous);

    if( current )
    {
        setText( current->text() );
        selectAll();
        mSetDefaultAction->setEnabled( true );
        mDeleteAction->setEnabled( true );
    }
    else
    {
        setText( "" );
        mSetDefaultAction->setEnabled( false );
        mDeleteAction->setEnabled( false );
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
