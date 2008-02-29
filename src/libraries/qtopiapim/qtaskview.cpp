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

#include <qtopia/pim/qtaskview.h>

#include <QLabel>
#include <QLayout>
#include <QListWidget>
#include <QAction>
#include <QDebug>
#include <QPainter>
#include <QMenu>
#include <QTextDocument>
#include <QTextFrame>
#include <QAbstractTextDocumentLayout>

#include <qtopiaapplication.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#endif

/*!
  \class QTaskDelegate
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QTaskDelegate class provides drawing of QTaskModel items (\l{QTask}{QTasks}).

  By using QTaskDelegate, applications using QTasks can achieve a consistent
  look and feel.

  The tasks are presented by rendering their \l {QTask::description()}{description()} in bold
  text.  Generally, this class will be used with a QTaskListView or other similar list view.

  The following image shows three QTask objects, rendered using a QTaskDelegate and a QTaskListView:
  \image qtaskview.png "QTaskListView and QTaskDelegate"

  \sa QTask, QTaskListView, QTaskModel
*/

/*!
  Constructs a QTaskDelegate with parent \a parent.
*/
QTaskDelegate::QTaskDelegate( QObject * parent )
    : QAbstractItemDelegate(parent)
{
}

/*!
  Destroys a QTaskDelegate.
*/
QTaskDelegate::~QTaskDelegate() {}

/*!
  \internal
  Provides an alternate font based off the \a start font.  Reduces the size of the returned font
  by at least step point sizes.  Will attempt a total of six point size steps beyond the requested
  point size until a valid font size that differs from the starting font size is found.
*/
QFont QTaskDelegate::differentFont(const QFont& start, int step) const
{
    int osize = QFontMetrics(start).lineSpacing();
    QFont f = start;
    for (int t=1; t<6; t++) {
        int newSize = f.pointSize() + step;
        if ( newSize > 0 )
            f.setPointSize(f.pointSize()+step);
        else
            return start; // we cannot find a font -> return old one
        step += step < 0 ? -1 : +1;
        QFontMetrics fm(f);
        if ( fm.lineSpacing() != osize )
            break;
    }
    return f;
}

/*!
  \internal
  Returns the font to use for painting the main label text of the item.
  Due to the nature of rich text painting in Qt 4.0 attributes such as bold and italic will be
  ignored.

  By default returns the font of the style option \a o.
*/
QFont QTaskDelegate::mainFont(const QStyleOptionViewItem &o) const
{
    return o.font;
}

/*!
  \internal
  Paints the element at \a index using \a painter with style options \a option.

  The \c description() of the QTask corresponding to \a index is rendered
  as bold rich text, using the font from \l mainFont() as the base font.

  If the task is selected, it will be drawn with a highlighted background and
  a different foreground color.
*/
void QTaskDelegate::paint(QPainter *painter, const QStyleOptionViewItem & option,
        const QModelIndex & index ) const
{
    QString text = "<b>";
    text += qobject_cast<const QTaskModel*>(index.model())->task(index).description();
    text += "<b>";

    painter->save();
    painter->setClipRect(option.rect);

    // fill rect based on row background
    // or assume can be left to list class
    bool selected = (option.state & QStyle::State_Selected) == QStyle::State_Selected;
    QBrush baseBrush = selected ? option.palette.highlight() : option.palette.base();
    QBrush textBrush = selected ? option.palette.highlightedText() : option.palette.text();
    QPalette modpalette(option.palette);

    modpalette.setBrush(QPalette::Text, textBrush);
    modpalette.setBrush(QPalette::Base, baseBrush);

    painter->setBrush(baseBrush);
    painter->setPen(textBrush.color());

    painter->fillRect(option.rect, baseBrush);

    // set up fonts, fbold, fsmall, fsmallbold
    QFont fbold = mainFont(option);

    int x = option.rect.x();
    int y = option.rect.y();
    int width = option.rect.width();
    int height = option.rect.height()-1;

    // draw label bold
    painter->setFont(fbold);

    // fit inside available width;
    QFontMetrics fboldM(fbold);

    // somehow underline appropriate characters.
    // clm->markSearchedText(text);, if richtext, would be <u>...</u> but only first
    // QTaskModel maybe could do a better job of this.

    QRect space;
    bool rtl = QtopiaApplication::layoutDirection() == Qt::RightToLeft ;
    if ( rtl )
        space = QRect(option.rect.x(), y, width, height);
    else
        space = QRect(x, y, width, height);

    // draw label/fileas
    QTextOption to;
    to.setWrapMode(QTextOption::WordWrap);
    to.setAlignment(QStyle::visualAlignment(qApp->layoutDirection(),
                Qt::AlignLeft));

    QString drawText;
    /* elidedText drops formatting from rich text */
    if(fboldM.width(text) > space.width())
        drawText = elidedText(fboldM, space.width(), Qt::ElideRight, text);
    else
        drawText = text;

    // Painting simple rich text, although all I need to do is deal with a bit of
    // an underline.
    QTextDocument document;
    document.setDefaultFont(fbold);
    document.setHtml(drawText); // or setPlainText or create using QTextCursor, etc.

    /* seemingly the only way to stop wrapping in a QTextDocument... */
    document.setPageSize(space.size().expandedTo(QSize(1000, 0)));

    QTextCursor cursor = document.rootFrame()->firstCursorPosition();
    QTextBlockFormat frmt = cursor.blockFormat();
    frmt.setAlignment(QStyle::visualAlignment( qApp->layoutDirection(), Qt::AlignLeft ));
    cursor.setBlockFormat( frmt );
    QTextCharFormat charFmt = cursor.charFormat();
    QAbstractTextDocumentLayout::PaintContext ctx;
    ctx.palette = modpalette;
    ctx.clip = QRect(0, 0, space.width(), space.height());
    painter->restore();
    painter->save();
    painter->translate(space.x(), space.y());
    painter->setClipRect(0, 0, space.width(), space.height());
    document.documentLayout()->draw(painter, ctx);
    painter->restore();
}

/*!
   \reimp
*/
QSize QTaskDelegate::sizeHint(const QStyleOptionViewItem & option,
                              const QModelIndex &index) const
{
    Q_UNUSED(index);

    QFontMetrics fm(mainFont(option));

    return QSize(fm.width("M") * 10, fm.height() + 9);
}
/*!
  \class QTaskListView
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QTaskListView class provides a list view widget with some convenience functions
  for use with QTaskModel.

  The convenience functions provided by QTaskListView include functions for interpreting
  the view's model, delegate and current item as the corresponding QTaskModel, QTaskDelegate and
  QTask objects.  In addition, QTaskListView enforces using a QTaskModel (or a derivative)
  as the model.

  Upon construction, QTaskListView automatically sets itself to use a QTaskDelegate for drawing,
  sets \c Batched layout mode (\l setLayoutMode()), and sets the resize mode to \c Adjust
  (\l setResizeMode()).

  The following image shows three QTask objects, rendered using a QTaskDelegate and a QTaskListView:
  \image qtaskview.png "QTaskListView and QTaskDelegate"

  \sa QTask, QTaskDelegate, QTaskModel
*/

/*!
    \fn QTask QTaskListView::currentTask() const

    Return the QTask for the currently selected index, or
    an empty QTask if there is no selected index.
*/

/*!
  \fn QTaskModel *QTaskListView::taskModel() const

  Returns the QTaskModel set for the view.
*/

/*!
  \fn QTaskDelegate *QTaskListView::taskDelegate() const

  Returns the QTaskDelegate set for the view.  During
  construction, QTaskListView  will automatically create
  a QTaskDelegate to use as the delegate, but this can be
  overridden with a different delegate derived from
  QTaskDelegate if necessary.
*/

/*!
  Constructs a QTaskListView with parent \a parent.

  This also sets the layout mode to \c Batched for performance,
  the resize mode to \c Adjust, and creates a QTaskDelegate
  to use as the delegate.
*/
QTaskListView::QTaskListView(QWidget *parent)
    : QListView(parent)
{
    setItemDelegate(new QTaskDelegate(this));
    setResizeMode(Adjust);
    setLayoutMode(Batched);

}

/*!
  Destroys the QTaskListView.
*/
QTaskListView::~QTaskListView()
{
}

/*!
  \overload

  Sets the model for the view to \a model.

  Will only accept the model if it inherits or is a QTaskModel.
  If the \a model does not inherit a QTaskModel, the existing
  model will be retained.
*/
void QTaskListView::setModel( QAbstractItemModel *model )
{
    QTaskModel *tm = qobject_cast<QTaskModel *>(model);
    if (!tm)
        return;
    QListView::setModel(model);
}

/*!
  Returns a list of QTasks selected in the view.  Fetching the complete QTask
  object can be expensive, so if a large number of tasks might be selected
  selectedTaskIds() could be used instead.

  \sa selectedTaskIds()
*/
QList<QTask> QTaskListView::selectedTasks() const
{
    QList<QTask> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->task(i));
    }
    return res;
}

/*!
  Returns the list of ids for QTasks selected from the view.

  \sa selectedTasks(), QTask::uid()
*/
QList<QUniqueId> QTaskListView::selectedTaskIds() const
{
    QList<QUniqueId> res;
    QModelIndexList list = selectionModel()->selectedIndexes();
    foreach(QModelIndex i, list) {
        res.append(taskModel()->id(i));
    }
    return res;
}

/***************************
  * QTaskSelector
  ***********************/
class QTaskSelectorPrivate
{
public:
    QTaskListView *view;
    bool mNewTaskSelected;
    bool mTaskSelected;
};

/*!
  \class QTaskSelector
  \mainclass
  \module qpepim
  \ingroup pim
  \brief The QTaskSelector class provides a way of selecting a single task from a QTaskModel.

  In addition, the user can optionally be allowed to indicate they want to create a new task,
  if none of the existing tasks are suitable.

  The following image shows a QTaskSelector with the option to create a new task highlighted.
  \image qtaskselector.png "QTaskSelector, with the new task option highlighted"
*/

/*!
  Constructs a QTaskSelector with parent \a parent.  If \a allowNew is true will also provide
  an option to indicate a new task should be created.
*/
QTaskSelector::QTaskSelector(bool allowNew, QWidget *parent)
    : QDialog(parent)
{
    d = new QTaskSelectorPrivate();
    d->mNewTaskSelected = false;
    d->mTaskSelected = false;
    setWindowTitle( tr("Select Tasks") );
    QVBoxLayout *l = new QVBoxLayout( this );

    d->view = new QTaskListView( this );
    d->view->setItemDelegate(new QTaskDelegate(d->view));
    d->view->setSelectionMode( QListView::SingleSelection );
    connect( d->view, SIGNAL(clicked(const QModelIndex&)), this, SLOT(setSelected(const QModelIndex&)) );
    connect( d->view, SIGNAL(activated(const QModelIndex&)), this, SLOT(setSelected(const QModelIndex&)) );

    l->addWidget( d->view );

    if( allowNew )
    {
#ifndef QTOPIA_PHONE
        //TODO pda
#else
        QMenu *menu = QSoftMenuBar::menuFor( this );
        menu->addAction( QIcon(":icon/new"), tr("New"), this, SLOT(setNewSelected()) );
#endif
    }

#ifndef QTOPIA_DESKTOP
#ifdef QTOPIA_PHONE
    QtopiaApplication::setMenuLike( this, true );
#endif
#endif
}

/*!
  \internal
  Accepts the dialog and indicates a that a new task should be created.
*/
void QTaskSelector::setNewSelected()
{
    d->mNewTaskSelected = true;
    accept();
}

/*!
  \internal
  Accepts the dialog and indicates a that a task at \a idx in the model was selected.
*/
void QTaskSelector::setSelected(const QModelIndex& idx)
{
    if (idx.isValid())
    {
        d->view->setCurrentIndex(idx);
        d->mTaskSelected = true;
    }
    accept();
}

/*!
  Sets the model providing the choice of tasks to \a model.
*/
void QTaskSelector::setModel(QTaskModel *model)
{
    d->view->setModel(model);
}

/*!
  Returns true if the dialog was accepted with the option to
  create a new task selected.
  Otherwise returns false.

  \sa taskSelected()
*/
bool QTaskSelector::newTaskSelected() const
{
    if (result() == Rejected)
        return false;
    return d->mNewTaskSelected;
}

/*!
  Returns true if the dialog was accepted with an existing task selected.
  Otherwise returns false.

  \sa newTaskSelected()
*/
bool QTaskSelector::taskSelected() const
{
    return d->mTaskSelected;
}

/*!
  Returns the task that was selected.  If no task was selected returns a null task.

  \sa taskSelected(), newTaskSelected()
*/
QTask QTaskSelector::selectedTask() const
{
    QTaskModel *m = qobject_cast<QTaskModel *>(d->view->model());

    if (result() == Rejected || d->mNewTaskSelected || !m || !d->view->currentIndex().isValid())
        return QTask();

    return m->task(d->view->currentIndex());
}

