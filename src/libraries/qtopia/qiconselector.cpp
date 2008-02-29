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

#include <QListWidget>
#include <QIcon>
#include <QString>
#include <QList>
#include <QIcon>
#include <QSize>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QToolButton>

#include <qtopiaapplication.h>

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

#include "qiconselector.h"

class QIconSelectorData
{
public:
    QIconSelectorData()
    {
    }

    QIconSelectorData( const QIcon &icn, const QString &t )
    {
        icon = icn;
        text = t;
    }

    QString text;
    QIcon icon;
};

class QIconSelectorPrivate
{
public:
    QIconSelectorPrivate()
    {
        count = 0;
        current = -1;
        mouseWasInsidePopup = false;
    }

    ~QIconSelectorPrivate()
    {
        foreach (QIconSelectorData* d, data)
            delete d;
    }

    QListWidget *list;
    QList<QIconSelectorData*> data;
    int count;
    int current;
    bool mouseWasInsidePopup;
    QIcon icon;
    QWidget* mPopup;
    QSize iconSize;
};



//widget like a combobox that allows, but doesn't use text when not popped up
QIconSelector::QIconSelector( QWidget *parent )
    : QToolButton( parent )
{
    init();
}



QIconSelector::QIconSelector( const QIcon &icn, QWidget *parent )
    : QToolButton( parent )
{
    setIcon( icn );

    init();
}

QIconSelector::~QIconSelector()
{
    delete d->list;
    delete d->mPopup;
    delete d;
}

void QIconSelector::init()
{
    d = new QIconSelectorPrivate();
    int iconSize = style()->pixelMetric(QStyle::PM_SmallIconSize);
    d->iconSize = QSize(iconSize, iconSize);
    setSizePolicy(QSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed));

    if ( parentWidget() ) {
        //transparent
        QPalette pal = QToolButton::palette();
        pal.setBrush( QPalette::Button, parentWidget()->palette().window() );
        setPalette( pal );
    }

    d->mPopup = new QWidget(0, Qt::Popup | Qt::WindowStaysOnTopHint);

    d->list = new QListWidget(d->mPopup);
    d->list->setMouseTracking(true);
    d->list->setAutoScroll(false);
    d->list->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    d->list->setFrameStyle(QFrame::Box | QFrame::Plain);
    d->list->setLineWidth(1);
    d->list->move(0, 0);

    connect(this, SIGNAL(clicked()), this, SLOT(popup()));
}

QSize QIconSelector::sizeHint() const
{
    const int border = style()->pixelMetric(QStyle::PM_ButtonMargin) * 2;
    int w = 0, h = 0;
    if( !d->iconSize.isEmpty() ) {
        w = d->iconSize.width();
        h = d->iconSize.height();
    } else if( d->icon.isNull() ) {
        w = h = style()->pixelMetric(QStyle::PM_SmallIconSize);
    }

    return QSize( w+border, h+border ).expandedTo( QApplication::globalStrut() );
}

uint QIconSelector::count()
{
    return d->data.count();
}

void QIconSelector::insertItem( const QIcon &icn, const QString &text )
{
    d->data.insert(d->count, new QIconSelectorData(icn, text));
    if(!d->count)
        setCurrentIndex(d->count);
    ++d->count;
}

void QIconSelector::removeIndex( int index )
{
    d->data.removeAt( index );
    if( d->count > 0 )
        --d->count;
    setCurrentIndex( d->count ? d->count : -1 );
}

void QIconSelector::clear()
{
    d->data.clear();
    setCurrentIndex( -1 );
}

int QIconSelector::currentIndex() const
{
    return d->current;
}

void QIconSelector::setCurrentIndex( int index )
{
    if( index >= (int)d->data.count() || index < 0 )
        return;

    d->current = index;
    if( index == -1 )
        setIcon( QIcon() );
    else
    {
        setIcon( d->data.at( index )->icon );
        emit activated( index );
    }
}

void QIconSelector::setIcon( const QIcon &icn )
{
    d->icon = icn;
    QToolButton::setIcon(icn);
    updateGeometry();
}

QIcon QIconSelector::icon() const
{
    return d->icon;
}

void QIconSelector::popup()
{
    d->list->clear();
    d->mouseWasInsidePopup = false;

    QListIterator<QIconSelectorData *> it( d->data );
    foreach(QIconSelectorData *data, d->data)
    {
        QListWidgetItem *item = new QListWidgetItem(data->text, d->list);
        item->setIcon(data->icon);
    }

    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect(desktop->availableGeometry(desktop->screenNumber(this)));
    QPoint pos = mapToGlobal( QPoint(desktopRect.x(),desktopRect.y()+height()) );

    int x = pos.x();
    int y = pos.y();
    int totalItemsHeight = 0;

    for( int i = 0 ; i < d->list->count() ; ++i )
        totalItemsHeight += d->list->sizeHintForRow(i);

    int totalHeight = totalItemsHeight+(d->list->frameWidth()*2)+2;
    int w = d->list->sizeHintForColumn(0)+d->list->frameWidth()*2+2;
    int h = totalHeight;
    d->mPopup->resize(w, h);
    d->list->resize(w, h);

    //
    //  Ensure the entire widget will be visible
    //

    // fit for x, no resize. Assumes will w < desktopRect.width()
    // doesn't flip to left of x, rather squishes.
    if (x + w > desktopRect.right())
        x = desktopRect.right() - w;
    else if (x < desktopRect.left())
        x = desktopRect.left();

    // fit for y.  Try down, if not try up, else squish (e.g. snap to bottom)
    // first meet assumptions about y on screen and list height able to fit somehow.
    y = qMax(y, desktopRect.top());
    y = qMin(y, desktopRect.bottom());
    if (h > desktopRect.height()) d->list->resize(w, desktopRect.height());

    if (y + h > desktopRect.bottom()) {
        if (y - h >= desktopRect.top())
            y = y - h;
        else
            y = desktopRect.bottom() - h;
    }

    d->list->installEventFilter(this);
    d->list->viewport()->installEventFilter(this);
    d->mPopup->installEventFilter(this);

    bool block = d->list->signalsBlocked();
    d->list->blockSignals(true);
    d->list->setCurrentRow(currentIndex());
    d->list->blockSignals(block);

    d->mPopup->move(x, y);
    d->mPopup->show();
    d->mPopup->raise();

    d->list->update();
    d->list->setFocus();

#ifdef QTOPIA_PHONE
    QSoftMenuBar::setLabel( d->list, Qt::Key_Select, QSoftMenuBar::Select );
#endif
}

void QIconSelector::popdown()
{
    d->list->removeEventFilter(this);
    d->list->viewport()->removeEventFilter(this);
    d->mPopup->removeEventFilter(this);
    d->mPopup->hide();
}

void QIconSelector::itemSelected( int index )
{
    popdown();
    setCurrentIndex( index );
}

bool QIconSelector::eventFilter( QObject *obj, QEvent *e ) //mostly copied from qcombobox
{
    if( !e ) return true;

    if( obj == d->list || obj == d->list->viewport() || obj == d->mPopup )
    {
        QKeyEvent *ke = (QKeyEvent *)e;
        QMouseEvent *me = (QMouseEvent *)e;
        switch( e->type() )
        {
            case QEvent::KeyPress:
            {
                switch( ke->key() )
                {
                    case Qt::Key_Up:
                        if(!(ke->modifiers() & Qt::AltModifier))
                        {
                            if(d->list->currentRow() > 0)
                                d->list->setCurrentRow(d->list->currentRow() - 1);
                            else
                                d->list->setCurrentRow(d->list->count() - 1);
                        }
                        return true;

                    case Qt::Key_Down:
                        if(!(ke->modifiers() & Qt::AltModifier))
                        {
                            if(d->list->currentRow() > -1 &&
                                    d->list->currentRow() < d->list->count() - 1)
                                d->list->setCurrentRow(d->list->currentRow() + 1);
                            else
                                d->list->setCurrentRow(0);
                        }
                        return true;


                    case Qt::Key_F4:
                    case Qt::Key_Escape:
#ifdef QTOPIA_PHONE
                    case Qt::Key_Back:
                    case Qt::Key_No:
#endif
                        popdown();
                        return true;
                        break;


#ifdef QTOPIA_PHONE
                    case Qt::Key_Select:
#endif
                    case Qt::Key_Enter:
                    case Qt::Key_Return:
                        itemSelected(d->list->currentRow());
                        return false;

                    default:
                        break;
                }
            }
            case QEvent::MouseButtonPress:
            {
                if(!d->list->rect().contains(me->pos()))
                    popdown();
                else
                    itemSelected(d->list->row(d->list->itemAt(me->pos())));
                return true;
            }
            default:
                break;
        }
    }

    return QWidget::eventFilter( obj, e );
}

void QIconSelector::keyPressEvent( QKeyEvent *e )
{
    QToolButton::keyPressEvent( e );
}
