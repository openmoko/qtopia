/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#include "singleview_p.h"

#include <qthumbnail.h>

#include <qtopiaapplication.h>
#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#endif

#include <qpainter.h>
#include <qevent.h>
#include <qcontent.h>

SingleView::SingleView( QWidget* parent, Qt::WFlags f )
    : QWidget( parent, f ), model_( 0 ), selection_( 0 )
{
    right_pressed = false;

#ifndef QTOPIA_KEYPAD_NAVIGATION
    // Enable stylus press events
    QtopiaApplication::setStylusOperation( this, QtopiaApplication::RightOnHold );
#endif

    // Accept focus
    setFocusPolicy( Qt::StrongFocus );

    // Respond to file changes
    connect( qApp,
        SIGNAL(contentChanged(const QContentIdList&,const QContent::ChangeType)),
        this,
        SLOT( contentChanged(const QContentIdList&,const QContent::ChangeType)));
}

void SingleView::setModel( QAbstractListModel* model )
{
    model_ = model;

    setSelectionModel( new QItemSelectionModel( model_ ) );
}

QAbstractListModel* SingleView::model() const
{
    return model_;
}

void SingleView::setSelectionModel( QItemSelectionModel* selection )
{
    // Disconnect current selection model
    if( selection_ ) {
        disconnect( selection_, SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&,const QModelIndex&)) );
    }
    selection_ = selection;
    // Connect new selection model
    if( selection_ ) {
        connect( selection_, SIGNAL(currentChanged(const QModelIndex&,const QModelIndex&)),
            this, SLOT(currentChanged(const QModelIndex&,const QModelIndex&)) );
    }
}

QItemSelectionModel* SingleView::selectionModel() const
{
    return selection_;
}

void SingleView::currentChanged( const QModelIndex& index, const QModelIndex& /*previous*/ )
{
    // Retrieve currently selected
    current_file = qvariant_cast<QString>( model_->data( index, Qt::UserRole ) );

    // Clear buffer and update widget
    buffer = QPixmap();
    update();
}

void SingleView::paintEvent( QPaintEvent* )
{
    // If image not loaded, load into buffer
    if( buffer.isNull() ) {
        // Load scaled image to fit widget keeping original wight height ratio
        buffer = loadThumbnail( current_file, size() );
    }

    QPainter painter( this );

    // Cacluculate offset to center image
    QPoint offset( ( width() - buffer.width() ) / 2, ( height() - buffer.height() ) / 2 );

    // Draw image
    painter.drawPixmap( offset, buffer );
}

void SingleView::keyPressEvent( QKeyEvent* e )
{
    switch( e->key() ) {
    // If select key, emit select signal
#ifdef QTOPIA_KEYPAD_NAVIGATION
    case Qt::Key_Select:
#endif
    case Qt::Key_Enter:
        emit selected();
        break;
    // Move current selection down one row
    case Qt::Key_Right:
    case Qt::Key_Up:
        moveForward();
        break;
    // Move current selection up one row
    case Qt::Key_Left:
    case Qt::Key_Down:
        moveBack();
        break;
    default:
        e->ignore();
    }
}

#ifdef QTOPIA_PDA
void SingleView::mouseReleaseEvent( QMouseEvent* e )
{
    // If left button clicked, emit selected
    if( !right_pressed && e->button() == Qt::LeftButton ) emit selected();
}
#endif

void SingleView::mousePressEvent( QMouseEvent* e )
{
#ifndef QTOPIA_PDA
    Q_UNUSED(e);
    emit selected();
#else
    right_pressed = false;
    // If right button pressed, emit held
    if( e->button() == Qt::RightButton ) {
        right_pressed = true;
        emit held( e->globalPos() );
    }
#endif
}

void SingleView::moveForward()
{
    // Move current selection down one row
    QModelIndex index = selection_->currentIndex();
    index = model_->index( index.row() + 1 );
    if( !index.isValid() ) {
        index = model_->index( 0 );
    }
    selection_->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

void SingleView::moveBack()
{
    // Move current selection up one row
    QModelIndex index = selection_->currentIndex();
    index = model_->index( index.row() - 1 );
    if( !index.isValid() ) {
        index = model_->index( model_->rowCount() - 1 );
    }
    selection_->setCurrentIndex( index, QItemSelectionModel::ClearAndSelect );
}

QPixmap SingleView::loadThumbnail( const QString &filename, const QSize &size )
{
    QThumbnail thumbnail( filename );

    return thumbnail.pixmap( size );
}

void SingleView::contentChanged(const QContentIdList &idList,const QContent::ChangeType changeType)
{
    QModelIndex idx = selectionModel()->currentIndex();
    if ( idx.isValid() ) {
        currentChanged(idx,idx);
    }
}
