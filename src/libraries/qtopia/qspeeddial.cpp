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

#include "qspeeddial.h"

#include <qtopiaapplication.h>
#include <qtranslatablesettings.h>
#include <qexpressionevaluator.h>

#include <QPainter>
#include <QLineEdit>
#include <QLayout>
#include <QPushButton>
#include <QEvent>
#include <QKeyEvent>
#include <QLabel>
#include <QDialog>
#include <QMultiHash>
#include <QDebug>
#include <QListWidgetItem>
#include <QAction>
#include <QMenu>
#include <QSettings>

#ifdef QTOPIA_PHONE
# include <qtopia/qsoftmenubar.h>
#endif

#include <time.h>



static QMultiHash<QString, QtopiaServiceDescription*>* recs=0;
static QString recs_filename;
static QDateTime recs_ts;

static void clearReqs()
{
    if(recs)
    {
        delete recs;
        recs = 0;
    }
}

static void updateReqs()
{
    // recs_filename may not exist (never written), in which case, must
    // at least read once, even though recs_ts remains null.
    bool forceread=false;

    if ( recs_filename.isEmpty() ) {
        recs_filename = QTranslatableSettings(QLatin1String("Trolltech"),QLatin1String("SpeedDial")).fileName();
        forceread = true; // once
    }
    QFileInfo fi(recs_filename);
    QDateTime ts = fi.lastModified();

    if( forceread || ts != recs_ts )
    {
        clearReqs();

        recs = new QMultiHash<QString, QtopiaServiceDescription*>;
        recs_ts = ts;

        QTranslatableSettings cfg(QLatin1String("Trolltech"),QLatin1String("SpeedDial"));
        cfg.beginGroup(QLatin1String("Dial"));

        QStringList d = cfg.value(QLatin1String("Defined")).toString().split( ',');
        for(QStringList::ConstIterator it = d.begin(); it != d.end(); ++it)
        {
            cfg.endGroup();
            cfg.beginGroup(QLatin1String("Dial") + *it);

            QString s = cfg.value(QLatin1String("Service")).toString();

            if (s.isEmpty()
                || cfg.value(QLatin1String("RemoveIfUnavailable")).toBool()
                  && QtopiaService::app(s).isEmpty()
            )
                continue;

            QByteArray r = cfg.value(QLatin1String("Requires")).toByteArray();
            QExpressionEvaluator expr(r);
            if ( !r.isEmpty() && !(expr.isValid() && expr.evaluate() && expr.result().toBool()) )
                continue;

            QString m = cfg.value(QLatin1String("Message")).toString();
            QByteArray a = cfg.value(QLatin1String("Args")).toByteArray();
            QString l = cfg.value(QLatin1String("Label")).toString();
            QString ic = cfg.value(QLatin1String("Icon")).toString();

            QtopiaServiceRequest req(s, m.toLatin1());

            if(!a.isEmpty())
                QtopiaServiceRequest::deserializeArguments(req, a);

            QtopiaServiceDescription* t = new QtopiaServiceDescription(req, l, ic);
            recs->insert((*it), t);
        }
    }
}

static void writeReqs(const QString& changed)
{
    QSettings cfg(QLatin1String("Trolltech"),QLatin1String("SpeedDial"));
    QStringList strList;
    QHashIterator<QString, QtopiaServiceDescription*> it(*recs);
    QtopiaServiceDescription* rec;
    bool found = false;

    while( it.hasNext() )
    {
        it.next();

        rec = it.value();
        strList.append(it.key());
        if( changed.isNull() || changed == it.key())
        {
            cfg.beginGroup(QLatin1String("Dial")+it.key());
            cfg.setValue(QLatin1String("Service"),rec->request().service());
            cfg.setValue(QLatin1String("Message"),QString(rec->request().message()));
            cfg.setValue(QLatin1String("Args"),
                         QtopiaServiceRequest::serializeArguments(rec->request()));
            cfg.setValue(QLatin1String("Label"),rec->label());
            cfg.setValue(QLatin1String("Icon"),rec->iconName());
            found = true;
        }
    }

    if( !found )
    {
        // removed it
        cfg.remove(QLatin1String("Dial") + changed);
    }

    cfg.endGroup();

    cfg.beginGroup(QLatin1String("Dial"));
    cfg.setValue(QLatin1String("Defined"), strList.join(QString(',')));

    if ( !recs_filename.isEmpty() )
        recs_ts = QFileInfo(recs_filename).lastModified();
}

int firstAvailableSlot()
{
    // possible slots - 1 ~ 99
    QSettings cfg(QLatin1String("Trolltech"),QLatin1String("SpeedDial"));
    cfg.beginGroup(QLatin1String("Dial"));
    QStringList d = cfg.value(QLatin1String("Defined")).toString().split( ',');
    QList<int> defined;
    bool ok;
    int num;
    // convert to int
    foreach (QString str, d) {
        num = str.toInt(&ok);
        if (ok)
            defined << num;
    }
    qSort(defined);
    num = 1;
    // search for the first empty slot
    foreach (int i, defined) {
        if (num != i)
            break;
        else
            num++;
    }
    // all slots are used.
    if (num == 100)
        num = 0;
    return num;
}


class QSpeedDialDialog : public QDialog
{
    Q_OBJECT
public:
    QSpeedDialDialog(const QString& l, const QString& ic, const QtopiaServiceRequest& a,
        QWidget* parent);
    QString choice();

private slots:
    void store(const int item);

private:
    QtopiaServiceRequest action;
    QString label, icon;
    QLineEdit *inputle;
    QWidget *currinfo;
    QLabel *curricon;
    QLabel *currlabel;
    QPushButton *ok;
    QSpeedDialList *list;

    QString userChoice;
};

class QSpeedDialItem
{
public:
    QSpeedDialItem(const QString& i, const QtopiaServiceDescription& rec, QListView* parent);

    void changeRecord(QtopiaServiceDescription* src);

    QString input() const { return _input; }
    QtopiaServiceDescription description() const { return _record; }
    void clear()
    {
        _record = QtopiaServiceDescription();
    }

private:
    QString _input;
    QtopiaServiceDescription _record;
};


class QSpeedDialModel : public QAbstractListModel
{
    friend class QSpeedDialItem;

public:
    QSpeedDialModel(QObject* parent = 0);
    ~QSpeedDialModel();

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    QSpeedDialItem* item(const QModelIndex & index);
    QSpeedDialItem* item(int index);
    QSpeedDialItem* takeItem(int index);

private:
    QList<QSpeedDialItem*> mItems;

    void addItem(QSpeedDialItem* item);
};

class QSpeedDialItemDelegate : public QAbstractItemDelegate
{
public:
    QSpeedDialItemDelegate(QListView* parent);
    ~QSpeedDialItemDelegate();

    void paint(QPainter * painter, const QStyleOptionViewItem & option, const QModelIndex & index) const;
    QSize sizeHint(const QStyleOptionViewItem & option, const QModelIndex & index) const;

    void setSelectionDetails(QString label, QString icon);

private:
    QListView* parentList;
    int iconSize;
    int textHeight;
    int itemHeight;

    QIcon selIcon;
    QString selText;
};




class QSpeedDialListPrivate : QObject
{
    Q_OBJECT
public:
    QSpeedDialListPrivate(QSpeedDialList* parent) :
        delegate(parent),
        model(parent),
        parentList(parent),
        a_del(0)
    {
        parent->setItemDelegate(&delegate);
        parent->setModel(&model);
        actionChooser = 0;
        connect(parentList,SIGNAL(currentRowChanged(int)),
                this,SLOT(updateActions()));
    }



    void setSelectionDetails(QString label, QString icon)
    {
        delegate.setSelectionDetails(label, icon);
    }

    QSpeedDialItem* item(int row)
    {
        return model.item(row);
    }

    QSpeedDialItemDelegate delegate;
    QSpeedDialModel model;
    QSpeedDialList* parentList;
    QtopiaServiceSelector* actionChooser;

    QAction *a_del;

    QString seltext, selicon;
    int sel;
    int sel_tid;

private slots:
    void updateActions()
    {
        QSpeedDialItem* i = item(parentList->currentRow());
        if (a_del)
            a_del->setEnabled(i && !i->description().request().isNull());
    }
};





QSpeedDialItem::QSpeedDialItem(const QString& i, const QtopiaServiceDescription& rec, QListView* parent) :
    _input(i),
    _record(rec)
{
    ((QSpeedDialModel*)parent->model())->addItem(this);
}

void QSpeedDialItem::changeRecord(QtopiaServiceDescription* src)
{
    _record = *src;
}



QSpeedDialDialog::QSpeedDialDialog(const QString& l, const QString& ic,
    const QtopiaServiceRequest& a, QWidget* parent) :
    QDialog(parent),
    action(a),
    label(l),
    icon(ic)
{
    setModal(true);

    QVBoxLayout *vb = new QVBoxLayout(this);

    list = new QSpeedDialList(l, ic, this);
    if (list->count() > 0) {
        int currentRow = firstAvailableSlot() - 1;
        list->setCurrentRow(currentRow);
        // scroll to the row above the current row to indicate this row is the next available slot.
        list->scrollTo(list->model()->index( currentRow > 0 ? currentRow - 1 : currentRow, 0, QModelIndex()));
    } else {
        list->setCurrentRow(-1);
    }
    vb->addWidget(list);

    setWindowTitle(list->windowTitle());

    connect(list, SIGNAL(rowClicked(const int)),
        this, SLOT(store(const int)));

#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike(this, true);
#endif
}

QString QSpeedDialDialog::choice()
{
    return userChoice;
}

void QSpeedDialDialog::store(const int item)
{
    userChoice = list->rowInput(item);
    QSpeedDial::set(userChoice, QtopiaServiceDescription(action, label, icon));
    accept();
}



/*!
  \class QSpeedDialList
  \brief The QSpeedDialList class provides a list widget for editing Speed Dial entries.

  If you need a dialog that allows the user to select a spot to insert an already selected
  action (eg. adding a QContact's phone number to Speed Dial list), use
  QSpeedDial::addWithDialog.

  \ingroup qtopiaemb
*/

/*!
  \fn QSpeedDialList::currentRowChanged(int row)

  This signal is emitted when the user selects a different row (either with the keypad or the mouse).
  \a row contains the newly selected row.
*/

/*!
  \fn QSpeedDialList::rowClicked(int row)

  This signals is emitted when the user either clicks on a row with the mouse, or presses the keypad
  Select key while a row is selected.
  \a row contains the newly selected row.
*/

/*!
  Constructs a QSpeedDialList with parent \a parent.
*/
QSpeedDialList::QSpeedDialList(QWidget* parent) :
    QListView(parent)
{
    updateReqs();
    init(QString());

#ifdef QTOPIA_PHONE
    QMenu *contextMenu = QSoftMenuBar::menuFor(this);

    QAction *a_edit = new QAction( QIcon( ":icon/edit" ), tr("Set...", "set action"), this);
    connect( a_edit, SIGNAL(triggered()), this, SLOT(editItem()) );
    contextMenu->addAction(a_edit);

    d->a_del = new QAction( QIcon( ":icon/trash" ), tr("Delete"), this);
    connect( d->a_del, SIGNAL(triggered()), this, SLOT(clearItem()) );
    contextMenu->addAction(d->a_del);
#endif

    d->actionChooser = new QtopiaServiceSelector(this);
    QtopiaApplication::setMenuLike( d->actionChooser, true );
    connect(this, SIGNAL(rowClicked(int)), this, SLOT(editItem(int)));
    connect(selectionModel(), SIGNAL(selectionChanged(const QItemSelection&,const QItemSelection&)),
            this, SLOT(selectionChanged()));
}

/*!
  Destructs the QSpeedDialList.
*/
QSpeedDialList::~QSpeedDialList()
{
    delete d;
}

/*!
  \internal

  Used by QSpeedDialDialog to create a special single-action insertion version of the list.
*/
QSpeedDialList::QSpeedDialList(const QString& label, const QString& icon, QWidget* parent) :
    QListView(parent)
{
    updateReqs();
    init(QString());

    d->setSelectionDetails(label, icon);
}

/*!
  Removes the Speed Dial in the list at row \a row.
*/
void QSpeedDialList::clearItem(int row)
{
    QSpeedDialItem* item = d->item(row);
    if( item  ) {
        item->clear();
        QSpeedDial::remove(item->input());
    }
}

/*!
  \overload

  Clears the item at the currently selected row.
*/
void QSpeedDialList::clearItem()
{
    QSpeedDialItem* item = d->item(currentRow());
    if( item  ) {
        item->clear();
        QSpeedDial::remove(item->input());
    }
}

/*!
  Presents the a dialog for the user to select an action performed for
  the Speed Dial entry at row \a row.
*/
void QSpeedDialList::editItem(int row)
{
    QSpeedDialItem* item = d->item(row);
    if( item && d->actionChooser ) {
        QtopiaServiceDescription desc = item->description();
        if ( d->actionChooser->edit(item->input(),desc) ) {
            if ( desc.request().isNull() ) {
                QSpeedDial::remove(item->input());
            } else {
                QSpeedDial::set(item->input(), desc);
            }
            reload(item->input());
        }
    }
}

/*!
  \overload

  Allows the user to edit the item at the currently selected row.
*/
void QSpeedDialList::editItem()
{
    editItem(currentRow());
}

/*!
  \internal

  Used to initialize the list and populate it with entries.
*/
void QSpeedDialList::init(const QString& f)
{
    d = new QSpeedDialListPrivate(this);
    d->sel = 0;
    d->sel_tid = 0;
    int fn = 0;

    //
    //  Create entries for each possible speed dial number
    //

    for( int i = 1; i < 100; i++ )
    {
        QString inp = QString::number(i);
        QHash<QString, QtopiaServiceDescription*>::iterator it = recs->find(inp);
        QtopiaServiceDescription* rec = 0;

        if(it != recs->end())
            rec = recs->find(inp).value();

        new QSpeedDialItem(QString::number(i), rec ? *rec : QtopiaServiceDescription(), this);
        if( f == inp || i == fn )
            setCurrentRow(i - 1);
    }

    if(f.isEmpty() && model()->rowCount() > 0)
        setCurrentRow(0);

    //
    //  Deal with speed dials above 100 ?
    //

    QHashIterator<QString, QtopiaServiceDescription*> it(*recs);
    QtopiaServiceDescription* rec;
    while( it.hasNext() )
    {
        it.next();
        rec = it.value();
        QString inp = it.key();
        bool ok;
        int num = inp.toInt(&ok);

        if( !ok || num > 99 ) // skip numbers 0..99, done above
        {
            new QSpeedDialItem(inp, *rec, this);
            if( f == inp )
                setCurrentRow(count() - 1);
        }
    }

    connect(this,SIGNAL(activated(const QModelIndex&)),
            this,SLOT(select(const QModelIndex&)));
    connect(this,SIGNAL(clicked(const QModelIndex&)),
            this,SLOT(click(const QModelIndex&)));
}

/*!
  Selects the Speed Dial entry on row \a row.
*/
void QSpeedDialList::setCurrentRow(int row)
{
    setCurrentIndex(model()->index(row, 0));
}

/*!
  Selects the row from the list that corresponds to the Speed Dial input, \a sd.
*/
void QSpeedDialList::setCurrentInput(const QString& sd)
{
    for(int i = 0; i < (int)count(); ++i)
    {
        if(d->item(i)->input() == sd)
        {
            setCurrentRow(i);
            return;
        }
    }
}


/*!
  Forces the entry for Speed Dial input \a sd to be refreshed from the source.
*/
void QSpeedDialList::reload(const QString& sd)
{
    QHash<QString, QtopiaServiceDescription*>::iterator it = recs->find(sd);
    QtopiaServiceDescription* r = 0;

    if(it != recs->end())
        r = it.value();

    for(int i = 0; i < (int)count(); ++i)
    {
        QSpeedDialItem *it = (QSpeedDialItem*)d->item(i);
        if( it && it->input() == sd )
        {
            if( r )
                it->changeRecord(r);
            else
                it->clear();

            update();
            return;
        }
    }

    if( r )
    {
        new QSpeedDialItem(sd, *r, this);
        setCurrentRow(count() - 1);
    }
}

/*!
  \internal

  Catches selection events and emits an event more meaningful to outside this class
  (emits a row isntead of a QModelIndex)
*/
void QSpeedDialList::selectionChanged()
{
    emit currentRowChanged(currentRow());
}

/*!
  \internal

  Catches selection events and emits an event more meaningful to outside this class
  (emits a row isntead of a QModelIndex)
*/
void QSpeedDialList::select(const QModelIndex& index)
{
    emit currentRowChanged(index.row());
}

/*!
  \internal

  Catches click events and emits an event more meaningful to outside this class
  (emits a row isntead of a QModelIndex)
*/
void QSpeedDialList::click(const QModelIndex& index)
{
    emit currentRowChanged(index.row());
    emit rowClicked(index.row());
}

/*!
  Returns the input required to trigger the Speed Dial entry at Row \a row.
*/
QString QSpeedDialList::rowInput(int row) const
{
    if(row >= 0 && row < count())
        return d->item(row)->input();
    else
        return QString();
}

/*!
  Returns the input required for the currently selected Speed Dial entry.
*/
QString QSpeedDialList::currentInput() const
{
    QSpeedDialItem *it = d->item(currentRow());
    return it ? it->input() : QString();
}

/*!
  \internal

  Detects button presses and uses them to navigate the list and make selections
*/
void QSpeedDialList::keyPressEvent(QKeyEvent* e)
{
    int k = e->key();
    if( k >= Qt::Key_0 && k <= Qt::Key_9 )
    {
        d->sel = d->sel * 10 + k - Qt::Key_0;
        if ( d->sel_tid )
        {
            killTimer(d->sel_tid);
            d->sel_tid = 0;
        }

        if ( d->sel )
        {
            setCurrentInput(QString::number(d->sel));
            if ( d->sel < 10 )
                d->sel_tid = startTimer(800);
            else
                d->sel = 0;
        }
    }
#ifdef QTOPIA_PHONE
    else if( k == Qt::Key_Select )
    {
        if(currentRow() > -1)
            emit rowClicked(currentRow());
    }
#endif
    else
    {
        QListView::keyPressEvent(e);
    }
}

/*!
  \internal

  Used as a timer to help detect Speed Dial style user input (holding down number keys)
*/
void QSpeedDialList::timerEvent(QTimerEvent* e)
{
    if ( e->timerId() == d->sel_tid ) {
        killTimer(d->sel_tid);
        d->sel = 0;
        d->sel_tid = 0;
    } else {
        QListView::timerEvent( e );
    }
}

/*!
  \internal

  Make sure the list gets redisplayed on scroll
*/
void QSpeedDialList::scrollContentsBy(int dx, int dy)
{
    QListView::scrollContentsBy(dx, dy);
    update();
}

/*!
  Returns the number of rows in the list.
*/
int QSpeedDialList::count() const
{
    return ((QSpeedDialModel*)model())->rowCount();
}

/*!
  Returns the currently selected row number.
*/
int QSpeedDialList::currentRow() const
{
    return currentIndex().row();
}




/*!
  \class QSpeedDial
  \brief The QSpeedDial class provides access to the Speed Dial settings.

  The QSpeedDial class includes a set of static functions that give access to the
  Speed Dial settings. This class should not be instantiated.

  To allow the user to select an input for a given action, use addWithDialog().
  To directly modify the Speed Dial settings, use remove() and set().
  Use find() to retrieve the assigned action for a given input.

  \ingroup qtopiaemb
*/

/*!
  Provides a dialog that allows the user to select an input for action \a action,
  using \a label and \a icon as the display label and icon respectively. \a parent
  will be used as the dialog's parent.

  Returns the input that the user selected; it is up to you to use set() to
  assign the selection.

  If the user cancels the dialog, a null string is returned.

  \sa set()
*/
QString QSpeedDial::addWithDialog(const QString& label, const QString& icon,
    const QtopiaServiceRequest& action, QWidget* parent)
{
    QSpeedDialDialog dlg(label, icon, action, parent);
    if ( QtopiaApplication::execDialog(&dlg) )
        return dlg.choice();
    else
        return QString();
}

/*!
  Returns a QtopiaServiceDescription for the given Speed Dial input, \a input.
*/
QtopiaServiceDescription* QSpeedDial::find(const QString& input)
{
    QHash<QString, QtopiaServiceDescription*>::iterator it;
    QtopiaServiceDescription* rec;

    updateReqs();

    it = recs->find(input);
    if(it != recs->end())
    {
        rec = it.value();
        return rec;
    }

    return 0;
}

/*!
  Removes the action currently associated with the given Speed Dial input, \a input.
*/
void QSpeedDial::remove(const QString& input)
{
    updateReqs();
    recs->remove(input);
    writeReqs(input); // NB. must do this otherwise won't work
}

/*!
  Assigns the given QtopiaServiceDescription, \a r, as the action to perform when the given
  Speed Dial input, \a input, is detected.
*/
void QSpeedDial::set(const QString& input, const QtopiaServiceDescription& r)
{
    updateReqs();
    recs->replace(input,new QtopiaServiceDescription(r));
    writeReqs(input);
}


QSpeedDialItemDelegate::QSpeedDialItemDelegate(QListView* parent)
    : QAbstractItemDelegate(parent)
{
    parentList = parent;

    iconSize = QApplication::style()->pixelMetric(QStyle::PM_ListViewIconSize);

    QFontMetrics fm(parent->font());
    textHeight = fm.height();

    if(iconSize > textHeight)
        itemHeight = iconSize;
    else
        itemHeight = textHeight;
}

QSpeedDialItemDelegate::~QSpeedDialItemDelegate()
{
}

void QSpeedDialItemDelegate::setSelectionDetails(QString label, QString icon)
{
    selText = label;
    selIcon = QIcon(QLatin1String(":image/")+icon);
}

void QSpeedDialItemDelegate::paint(QPainter * painter,
    const QStyleOptionViewItem & option, const QModelIndex & index) const
{
    QSpeedDialModel* model = (QSpeedDialModel*)index.model();

    if(model)
    {
        QFontMetrics fm(option.font);
        int x = option.rect.x();
        int y = option.rect.y();
        int width = option.rect.width();
        int height = option.rect.height()-1;
        bool selected = option.state & QStyle::State_Selected;

        if(selected)
        {
            painter->setPen(option.palette.highlightedText().color());
            painter->fillRect(option.rect, option.palette.highlight());
        }
        else
        {
            painter->setPen(option.palette.text().color());
            painter->fillRect(option.rect, option.palette.base());
        }

        QSpeedDialItem* item = model->item(index);
        QString input = item->input();
        QPixmap pixmap;
        QString label;

        if(selected && !selText.isEmpty())
        {
            pixmap = selIcon.pixmap(option.decorationSize);
            label = selText;
        }
        else
        {
            if (!item->description().iconName().isEmpty())
                pixmap = QIcon(QLatin1String(":icon/")+item->description().iconName()).pixmap(option.decorationSize);
            label = item->description().label();
        }

        QTextOption to;
        to.setAlignment( QStyle::visualAlignment(qApp->layoutDirection(),
                    Qt::AlignLeft) | Qt::AlignVCenter);
        bool rtl = qApp->layoutDirection() == Qt::RightToLeft;

        painter->drawText(QRect(x, y+1, width, height), input, to);
        if ( rtl )
            width -= fm.width(input);
        else
            x += fm.width(input);

        painter->drawText(QRect(x,y+1,width,height), QLatin1String(":"), to);
        if ( rtl )
            width -= fm.width(QLatin1String(": "));
        else
            x += fm.width(QLatin1String(": "));
        if(!pixmap.isNull())
        {
            if ( rtl ) {
                painter->drawPixmap(x+width-pixmap.width(), y+1, pixmap);
                width -= pixmap.width() + fm.width(QLatin1String(" "));
            } else {
                painter->drawPixmap(x, y+1, pixmap);
                x += pixmap.width() + fm.width(QLatin1String(" "));
            }
        }

        label = elidedText( fm, width, Qt::ElideRight, label );
        painter->drawText(QRect(x, y+1, width, height), label, to);
    }
}

QSize QSpeedDialItemDelegate::sizeHint(const QStyleOptionViewItem &, const QModelIndex &) const
{
    return QSize(parentList->viewport()->width(), itemHeight+2);
}



QSpeedDialModel::QSpeedDialModel(QObject* parent)
    : QAbstractListModel(parent)
{
}

QSpeedDialModel::~QSpeedDialModel()
{
    for(int i = 0; i < mItems.size(); i++)
        delete mItems[i];
}

int QSpeedDialModel::rowCount(const QModelIndex & parent) const
{
    if(parent.parent() == QModelIndex())
        return mItems.count();
    else
        return 0;
}

QVariant QSpeedDialModel::data(const QModelIndex &, int) const
{
    return QVariant();
}

QSpeedDialItem* QSpeedDialModel::item(const QModelIndex & index)
{
    int row = index.row();
    int rowcount = rowCount();

    if(row >= 0 && row < rowcount)
        return mItems.at(index.row());
    else if(rowcount)
        return mItems.at(rowcount - 1);
    else
        return 0;
}

QSpeedDialItem* QSpeedDialModel::item(int index)
{
    if(index >= 0 && index < mItems.count())
        return mItems.at(index);
    else
        return 0;
}

void QSpeedDialModel::addItem(QSpeedDialItem* item)
{
    mItems.append(item);
}



#include "qspeeddial.moc"
