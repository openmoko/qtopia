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

#include "launcherview.h"
#include <QtopiaApplication>
#include <QVBoxLayout>
#include <QResizeEvent>
#include <QSoftMenuBar>
#include <QtopiaServiceRequest>
#include <QMenu>
#include <QDesktopWidget>
#include <QSpeedDial>
#include <QPainter>
#include <QSet>
#include <QSortFilterProxyModel>
#include <QPixmap>
#include <QItemDelegate>


LauncherViewListView::LauncherViewListView( QWidget *parent )
    : QListView( parent )
{
}

void LauncherViewListView::currentChanged( const QModelIndex &current, const QModelIndex &previous )
{
    QListView::currentChanged( current, previous );

    emit currentIndexChanged( current, previous );
}

void LauncherViewListView::focusOutEvent(QFocusEvent *)
{
    // Do nothing.  Don't need an update.
}

bool LauncherViewListView::viewportEvent(QEvent *e)
{
    switch (e->type()) {
    case QEvent::WindowActivate:
    case QEvent::WindowDeactivate:
        // suppress unneeded viewport update
        return true;
    default:
        break;
    }

    return QListView::viewportEvent(e);
}

class QLauncherProxyModel : public QSortFilterProxyModel
{

public:
    QLauncherProxyModel(QObject *parent=0) : QSortFilterProxyModel(parent), bouncePos(0), sortingStyle(LauncherView::NoSorting) { }
    virtual QVariant data ( const QModelIndex & index, int role = Qt::DisplayRole ) const;

    void setItem(const QModelIndex&);
    void clearItems();
    bool hasBouncer() const {return items.count() != 0;}
    void doBounceTick();
    void setSorting(LauncherView::SortingStyle);

    enum Roles { BusyRole = Qt::UserRole+1 };

private:
    QMap<QPair<int,int>, QModelIndex> items;
    int bouncePos;
    LauncherView::SortingStyle sortingStyle;
};

QVariant QLauncherProxyModel::data ( const QModelIndex & index, int role ) const
{
    static int offSets[] = { 0, 1, 2, 1, 0, -1, -2, -1};
    if (role == QLauncherProxyModel::BusyRole)
        return QVariant(hasBouncer() && items.contains(qMakePair<int, int>(index.row(), index.column())));
    else
        return QSortFilterProxyModel::data(index, role);
}

void QLauncherProxyModel::setItem(const QModelIndex& index)
{
    clearItems();
    items.insert(QPair<int, int>(index.row(), index.column()), index);
}

void QLauncherProxyModel::clearItems()
{
    bouncePos=0;
    QMap<QPair<int,int>, QModelIndex> olditems=items;
    items.clear();
    foreach(QModelIndex item, olditems)
    {
        emit dataChanged(item, item);
    }
}

void QLauncherProxyModel::doBounceTick()
{
    bouncePos++;
    bouncePos%=8;
    foreach(QModelIndex item, items)
    {
        emit dataChanged(item, item);
    }
}

void QLauncherProxyModel::setSorting(LauncherView::SortingStyle style)
{
    sortingStyle=style;
    if(style==LauncherView::LanguageAwareSorting)
    {
        sort(0);
        setDynamicSortFilter(true);
    }
    else
        setDynamicSortFilter(false);
}

class LauncherViewDelegate : public QItemDelegate
{
public:
    LauncherViewDelegate(QObject *parent=0) : QItemDelegate(parent) {}
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &opt, const QModelIndex &index) const
    {
        QItemDelegate::paint(painter, opt, index); 
        QVariant value = index.data(QLauncherProxyModel::BusyRole);
        if (value.toBool()) {
            int size = QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
            QPixmap pm = QIcon(":icon/wait").pixmap(size,size);
            painter->drawPixmap(opt.rect.right()-size, opt.rect.top(), pm);
        }
    }
};


////////////////////////////////////////////////////////////////
//
// LauncherView implementation
#include <QStringListModel>
LauncherView::LauncherView( QWidget* parent, Qt::WFlags fl )
    : QWidget(parent, fl)
        , icons(NULL)
        , model(NULL)
        , nColumns(1)
        , busyTimer(0)
        , bpModel(NULL)
{
    QVBoxLayout *vbl = new QVBoxLayout(this);
    vbl->setMargin(0);
    vbl->setSpacing(0);
    icons = new LauncherViewListView(this);
    icons->setItemDelegate(new LauncherViewDelegate(icons));
    vbl->addWidget(icons);
    setFocusProxy(icons);

    QtopiaApplication::setStylusOperation( icons->viewport(), QtopiaApplication::RightOnHold );

    icons->setFrameStyle( QFrame::NoFrame );
    icons->setResizeMode( QListView::Fixed );
    icons->setSelectionMode( QAbstractItemView::SingleSelection );
    icons->setSelectionBehavior( QAbstractItemView::SelectItems );
    //icons->setUniformItemSizes( true );
    icons->setLayoutMode(QListView::Batched);
//    icons->setWordWrap( true );

    contentSet = new QContentSet(this);
    contentSet->setSortOrder(QStringList() << "name");
    bpModel = new QLauncherProxyModel(this);
    model = new QContentSetModel(contentSet, bpModel);
    bpModel->setSourceModel(model);

//    setViewMode(QListView::IconMode);
    setViewMode(QListView::ListMode);

    connect( icons, SIGNAL(clicked(const QModelIndex &)),
             SLOT(itemClicked(const QModelIndex &)));
    connect( icons, SIGNAL(activated(const QModelIndex &)),
             SLOT(returnPressed(const QModelIndex &)) );
    connect( icons, SIGNAL(pressed(const QModelIndex &)),
             SLOT(itemPressed(const QModelIndex &)));

#ifndef QTOPIA_PHONE
    setBackgroundType( Ruled, QString() );
#endif

    icons->setModel(bpModel);
}

LauncherView::~LauncherView()
{
}

void LauncherView::setBusy(bool on)
{
    setBusy(icons->currentIndex(), on);
}

void LauncherView::setBusy(const QModelIndex &index, bool on)
{
    if ( on )
    {
        bpModel->setItem(index);
        bpModel->doBounceTick();
    }
    else
    {
        bpModel->clearItems();
    }
}

void LauncherView::timerEvent ( QTimerEvent * event )
{
    if ( event->timerId() == busyTimer ) {
        bpModel->doBounceTick();
    } else {
        QWidget::timerEvent( event );
    }
}

void LauncherView::setViewMode( QListView::ViewMode m )
{
    Q_ASSERT(icons);
    if(m==QListView::ListMode)
        icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    else
        icons->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    icons->setViewMode(m);
    calculateGridSize();
}

QListView::ViewMode LauncherView::viewMode() const
{
    Q_ASSERT(icons);
    return icons->viewMode();
}

void LauncherView::removeAllItems()
{
    contentSet->clear();
}

void LauncherView::addItem(QContent* app, bool resort)
{
    Q_UNUSED(resort);
    if(app == NULL)
        return;
    contentSet->add( *app );
    if(resort)
        bpModel->sort(0);
}

void LauncherView::removeItem(const QContent &app)
{
    contentSet->remove(app);
}


void LauncherView::returnPressed(const QModelIndex &item)
{
    emit clicked(model->content(bpModel->mapToSource(item)));
}

void LauncherView::itemClicked(const QModelIndex & index)
{
    if(QApplication::mouseButtons () == Qt::LeftButton)
    {
        icons->setCurrentIndex( index );
        emit clicked(model->content(bpModel->mapToSource(index)));
    }
}

void LauncherView::itemPressed(const QModelIndex & index)
{
    if(QApplication::mouseButtons () == Qt::RightButton)
    {
        icons->setCurrentIndex( index );
        emit rightPressed(model->content(bpModel->mapToSource(index)));
    }
}

void LauncherView::resizeEvent(QResizeEvent *e)
{
    QWidget::resizeEvent( e );
    calculateGridSize();
}

void LauncherView::changeEvent(QEvent *e)
{
    if (e->type() == QEvent::PaletteChange) {
#ifndef QTOPIA_PHONE
        setBackgroundType( Ruled, QString() );
#endif
        QPalette pal(palette());
        //pal.setColor(QPalette::Text, textCol);    // todo later, include text color setting. Not necessary for now.
        icons->setPalette(pal);
    }
    if (e->type() == QEvent::StyleChange)
        calculateGridSize();
    QWidget::changeEvent(e);
}

void LauncherView::showEvent(QShowEvent *e)
{
    QWidget::showEvent(e);

    resize(size().width(), size().height()+1); // HACK: to get around rtl bug for QListView in Qt 4.2.2
}

void LauncherView::setColumns(int columns)
{
    nColumns = columns;
    calculateGridSize();
}

void LauncherView::setFilter(const QContentFilter &filter)
{
    if( filter != mainFilter )
    {
        mainFilter = filter;

        contentSet->setCriteria( mainFilter & typeFilter & categoryFilter );
        resetSelection();
    }
}

const QContent LauncherView::currentItem() const
{
    return model->content(bpModel->mapToSource(icons->currentIndex()));
}

void LauncherView::calculateGridSize()
{
    QSize grSize;
    Q_ASSERT(model);
    Q_ASSERT(icons);

    int dw = width();
    int viewerWidth = dw-style()->pixelMetric(QStyle::PM_ScrollBarExtent)-6;
    int lineHeight=0;
    if ( viewMode() == QListView::IconMode ) {
        icons->setSpacing( 2 );

        int width = viewerWidth/nColumns;
        int iconHeight = width - fontMetrics().height() * 2;

        grSize = QSize(width, width);
        QSize icoSize = QSize(iconHeight, iconHeight);
        icons->setIconSize(icoSize);
    } else {
        icons->setSpacing( 1 );
        lineHeight = style()->pixelMetric(QStyle::PM_ListViewIconSize) + 4;
        grSize = QSize((viewerWidth-(nColumns+1)*icons->spacing())/nColumns,
                        lineHeight);
        icons->setIconSize(QSize());
    }

    icons->setGridSize(grSize);
}

void LauncherView::showType( const QContentFilter &filter )
{
    if( filter != typeFilter )
    {
        typeFilter = filter;

        contentSet->setCriteria( mainFilter & typeFilter & categoryFilter );
        resetSelection();
    }
}

void LauncherView::showCategory( const QContentFilter &filter )
{
    if( filter != categoryFilter )
    {
        categoryFilter = filter;

        contentSet->setCriteria( mainFilter & typeFilter & categoryFilter );
        resetSelection();
    }
}

void LauncherView::resetSelection()
{
    if (icons && model && icons->model()->rowCount()) // && !icons->currentItem())
    {
        icons->setCurrentIndex(icons->model()->index(0,0));
    }
}

void LauncherView::setSorting(SortingStyle style)
{
    Q_ASSERT(bpModel);
    bpModel->setSorting(style);
}

////////////////////////////////////////////////////////////////
//
// ApplicationLauncherView implementation

ApplicationLauncherView::ApplicationLauncherView(QWidget *parent)
    : LauncherView(parent), rightMenu(0)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu * softMenu = QSoftMenuBar::menuFor(this);
#endif
    rightMenu = new QMenu(this);
#ifdef QTOPIA_PHONE
    QAction *a_speed = new QAction(QIcon(":icon/phone/speeddial"),
                                   tr("Add to Speed Dial..."), this );
    connect( a_speed, SIGNAL(triggered()), this, SLOT(addSpeedDial()));

    softMenu->addAction(a_speed);
    rightMenu->addAction(a_speed);
#endif

    QObject::connect(this, SIGNAL(rightPressed(QContent)),
                     this, SLOT(launcherRightPressed(QContent)));
    setViewMode(QListView::ListMode);
    bpModel->setSorting(LauncherView::LanguageAwareSorting);
}


ApplicationLauncherView::ApplicationLauncherView(const QString &category, QWidget *parent)
    : LauncherView(parent), rightMenu(0)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QMenu * softMenu = QSoftMenuBar::menuFor(this);
#endif
    rightMenu = new QMenu(this);
#ifdef QTOPIA_PHONE
    QAction *a_speed = new QAction(QIcon(":icon/phone/speeddial"),
                                   tr("Add to Speed Dial..."), this );
    connect( a_speed, SIGNAL(triggered()), this, SLOT(addSpeedDial()));

    softMenu->addAction(a_speed);
    rightMenu->addAction(a_speed);
#endif

    QObject::connect(this, SIGNAL(rightPressed(QContent)),
                     this, SLOT(launcherRightPressed(QContent)));

    QContentFilter filters = QContentFilter( QContent::Application )
            & QContentFilter( QContentFilter::Category, category );
contentSet->setCriteria( filters );

    bpModel->setSorting(LauncherView::LanguageAwareSorting);
}

void ApplicationLauncherView::launcherRightPressed(QContent lnk)
{
    if(!lnk.isValid())
        return;

    rightMenu->popup(QCursor::pos());
}

#ifdef QTOPIA_PHONE
void ApplicationLauncherView::addSpeedDial()
{
    const QContent lnk(currentItem());
    QtopiaServiceRequest sreq;
    sreq = QtopiaServiceRequest("Launcher","execute(QString)");
    sreq << lnk.executableName();
    QSpeedDial::addWithDialog(Qtopia::dehyphenate(lnk.name()), lnk.iconName(), sreq, this);
}
#endif
