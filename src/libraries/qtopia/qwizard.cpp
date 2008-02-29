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

#include "qwizard.h"

#include <QLabel>
#include <QEvent>
#include <QAction>
#include <QPushButton>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QSignalMapper>
#include <QKeyEvent>
#include <QMenu>
#include <QDebug>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qsoftmenubar.h>
#endif

/*!
  \class QWizard
  \brief The QWizard class provides a simple wizard implementation

  To use the Wizard follow these steps:
  \list
  \o add it as a custom widget in designer and make it a container
  \o either wrap it in a dialog in the code, or use it directly as a widget. If a dialog is used,
  trap the accept generated by the "OK" in the title-bar to manage
  user-generated closes
  \o add the page widgets inside
  \o call loadPages() to load the widgets as pages
  \o connect to the aboutToShowPage signal to prepare pages
  \o connect to the accept/reject signals to close the wizard on completion.
  \endlist

  Note: if one page of the wizard has controls which enable and disable other pages
  use connectPageEnabler instead of calling setAppropriate manually.

  \ingroup dialogs
*/

class QWizardPrivate : public QWidget
{
    Q_OBJECT

public:
    QWizardPrivate( QWidget *parent = 0, Qt::WFlags f = 0 );
    virtual ~QWizardPrivate();
    void loadPages( QObject *p);
    void addPage( QWidget * );
    QSize sizeHint() const;
    void connectPageEnabler( QObject *, const char *, QWidget * );

signals:
    void aboutToShowPage( int );
    void accept();
    void reject();

protected:
    void keyPressEvent( QKeyEvent * );
    void showEvent( QShowEvent * );

private slots:
    void setAppropriate( int );
    void showPage( int );
    void nextPage();
    void prevPage();
    void finish();

private:
    void setupUi();
    void retitle();
    uint lastAppropriate( uint );
    uint firstAppropriate( uint );
    bool buttonIsCancel;
    bool buttonIsFinish;
    bool dirty;

    QSignalMapper *sigmap;
    QAction *cancelAct;
    QAction *nextAct;
    QAction *prevAct;
    QAction *finishAct;
    QMap<QWidget *, bool> appropriate;
    QMap<QWidget *, bool> finishButtonOn;
    QMap<QWidget *, bool> nextButtonOn;
    QMap<QWidget *, bool> prevButtonOn;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // escape/cancel by holding down next button not implemented
    // QTimer *holdNextTimer;
    QMenu *contextMenu;
#endif
    int pageCount;

    QLabel* labelTitle;
    QStackedWidget* widgetStack;
    QPushButton* buttonPrev;
    QPushButton* buttonNext;
    QFrame *bottomLine;
    QFrame *topLine;
    QVBoxLayout* baseLayout;
    QHBoxLayout* buttonLayout;

    friend class QWizard;
};

/*! \fn void QWizard::aboutToShowPage( int page )
  This signal is emitted before the page \a page is shown. By connecting
  to this signal some last minute preparations on \a page can be done.
*/

/*! \fn void QWizard::reject()
  This signal is emitted if the user has clicked on the Cancel button.
*/

/*! \fn void QWizard::accept()
  This signal is emitted if the user has clicked on the Finish button.
*/

/*! \property QWizard::nextText
    \brief the text shown on the "next" button

    This property that is shown on the "next" button. By default this
    property is initialized to "&Next >"
*/

/*! \property QWizard::prevText
    \brief the text shown on the "back" button

    This property that is shown on the "back" button. By default this
    property is initialized to "< &Back"
*/
/*!
 Create a new Qtopia QWizard with parent \a parent and flags \a f.  This is a
 small compact wizard with a simple label at the top for each page and
 only two buttons at the bottom to control pages.  On the phone edition, the
 page switching is performed via context bar buttons and the page GUI buttons
 are not displayed.

 On the first appropriate page of the QWizard the two buttons are labelled \i cancel
 and \i next.  On the last appropriate page they are labelled \i previous and
 \i finish.  On all other pages the buttons are labelled \i previous and \i next.
 The context menu contains \i cancel and \i finish, as well as \i previous
 and \i next options.
 */
QWizard::QWizard( QWidget *parent, Qt::WFlags f )
    : QWidget(parent)
{
    // QWizardPrivate might be better off as a purely private implementation of
    // QWizard instead of being an actual QWidget that needs to be added to
    // QWizard.

    QVBoxLayout* baseLayout = new QVBoxLayout( this );
    d = new QWizardPrivate( this, f );
    baseLayout->addWidget( d );
    connect( d, SIGNAL( accept() ),
            this, SIGNAL( accept() ));
    connect( d, SIGNAL( reject() ),
            this, SIGNAL( reject() ));
    connect( d, SIGNAL( aboutToShowPage( int )),
            this, SIGNAL( aboutToShowPage( int )));
}

/*!
  Deconstructs QWizard.
*/
QWizard::~QWizard()
{
    delete d;
}

/*!
 Returns a pointer to the page at position \a index in the sequence, or
 0 if there is no such index. The first page has index 0.
 */
QWidget *QWizard::page ( int index ) const
{
    return d->widgetStack->widget( index );
}

/*!
 Returns the index of widget \a w, or -1 if \a w is not a child.
 */
int QWizard::indexOf( QWidget *w ) const
{
    return d->widgetStack->indexOf( w );
}

/*!
 Returns a pointer to the page at the current position  or
 0 if there is no pages in the QWizard.
 */
QWidget *QWizard::currentPage() const
{
    return d->widgetStack->currentWidget();
}

/*!
 Returns a QString of the text displayed on the \i prev button of the QWizard
 */
QString QWizard::prevText() const
{
    return d->buttonPrev->text();
}

/*!
 Returns a QString of the text displayed on the \i next button of the QWizard
 */
QString QWizard::nextText() const
{
    return d->buttonNext->text();
}

/*!
  Sets the text on the previous button to \a text.
*/
void QWizard::setPrevText( QString text )
{
    d->buttonPrev->setText( text );
}

/*!
  Sets the text on the next button to \a text.
*/
void QWizard::setNextText( QString text )
{
    d->buttonNext->setText( text );
}

/*!
 Load widget pages parented onto object \a p into the pagelist.  If \a p
 is 0 (NULL) then it loads pages parented onto itself.  The QWizard will
 treat any QWidget which is a direct child of object  \a p (that is, resides in
 p's children() list) as a page.  This indicates that making widgets a
 child of the QWizard in a  UI file, and using p = 0 will make them pages.
 loadPages() should be called  before trying to do anything with pages
 set by parenting, or calls to setAppropriate() and page()  which assume
 pages will fail.  The widget pages requires the windowTitle
 or caption property to be set as this is used for the title of the page.

\code
    QWizard *wiz = new QWizard;
    QWidget page1 = new QWidget( wiz );
    page1->setWindowTitle( "First page" );  // specifically set title
    QWidget *page2 = new MyPage2( wiz );    // custom ctor sets windowTitle
    wiz->loadPages();                       // OK to set things on pages now
    wiz->setAppropriate( page1, false );
    page2->setWindowTitle( "Second page" ); // can change title later
\endcode
 */
void QWizard::loadPages( QObject *p )
{
    if (!p)
        p = this;
    d->loadPages( p );
}

/*!
 Add the widget page \a w to this QWizard.  It will be added at the end
 of the sequence of pages.  Note: the windowTitle property of the
 widget is used to set the QWizard page title and  finishButton is false for each page by
 default. If this is the last page finishButton( w, true ) must be called.

 This method re-parents \a w onto the internal stacked widget in the QWizard.
 Note: re-parenting warnings can be safely ignored.

 If no title is set a page title will be created using the index of the
 page and the object name of the widget.
 */
void QWizard::addPage( QWidget *w )
{
    d->addPage( w );
}

/*!
  \reimp
  add the page \a w, giving it the title \a s
  */
void QWizard::addPage( QWidget *w, const QString &s )
{
    w->setWindowTitle( s );
    d->addPage( w );
}

/*!
  \reimp
*/
QSize QWizard::sizeHint() const
{
    return d->sizeHint();
}

/*!
  Connect the source object \a o signal \a s to the page \a w set
  appropriate.  The signal must supply a boolean.  The signals are mux'ed
  using a QSignalMapper so that when the object widget is toggled,
  the page's appropriate setting is toggled.  Useful for turning pages
  "on" and "Off" depending on a choices checkbox.  Since pages are
  initially enabled, toggle check boxes should be initially checked, so the
  toggling to checked/true toggles the page to enabled/true.
  */
void QWizard::connectPageEnabler( QObject *o, const char *s, QWidget *w )
{
    d->connectPageEnabler( o, s, w );
}

/*!
 If \a app is true then page \a w is considered relevant in the
 current context and should be displayed in the page sequence; otherwise
 the page should not be displayed in the page sequence.
 */
void QWizard::setAppropriate( QWidget *w, bool app )
{
    if ( d->appropriate[w] == app ) return;
    d->appropriate[w] = app;
    d->dirty = true;
}

/*!
 If \a enabled is true, the \i Next button on page \a w is enabled, otherwise it
 is disabled.  This can be used to disable moving forward in a QWizard until the page has been
 validly completed.
 */
void QWizard::setNextEnabled( QWidget *w, bool enabled )
{
    if ( d->nextButtonOn[w] == enabled ) return;
    d->nextButtonOn[ w ] = enabled;
    d->dirty = true;
}

/*!
 If \a enabled is true, the \i Prev button on page \a w is enabled, otherwise it
 is disabled.  On the first appropriate page in the sequence \i Prev is always disabled and a
 \i cancel button is displayed instead.
 */
void QWizard::setPrevEnabled( QWidget *w, bool enabled )
{
    if ( d->prevButtonOn[w] == enabled ) return;
    d->prevButtonOn[ w ] = enabled;
    d->dirty = true;
}

/*!
 If \a enabled is true, the \i Finish button on page \a w may be enabled, otherwise it
 is disabled.  Note:  \i Finish is displayed in the place of the \i Next
 button, and \i Next has priority.  To set \i finish, first use setNextEnabled( w,
 false ) which also  disables \i Finish in the context menu.

 This should not need to be called generally because the last appropriate
 page in the sequence will automatically have a finish button displayed.
 */
void QWizard::setFinishEnabled( QWidget *w, bool enabled )
{
    if ( d->finishButtonOn[w] == enabled ) return;
    d->finishButtonOn[ w ] = enabled;
    d->dirty = true;
}



/*!
 Display the page with index \a page.
 Emits aboutToShowPage( page ) signal.
 */
void QWizard::showPage( int page )
{
    d->showPage( page );
}

/*
  ==================================================================

  QWizard Private Implementation

  ==================================================================
  */

QWizardPrivate::QWizardPrivate( QWidget *parent, Qt::WFlags f )
    : QWidget( parent, f )
{
    // set name such that we will not be loaded as a page
    setObjectName("_wiz_");

    setupUi();

    buttonIsCancel = true;  // the left button has "cancel" text
    buttonIsFinish = false; // the right button has "finish" text
    dirty = true;
    sigmap = 0;
    pageCount = 0;

    cancelAct = new QAction( this );
    cancelAct->setIcon( QIcon( ":icon/cancel" ));
    cancelAct->setText( "Cancel" );
    cancelAct->setShortcut( Qt::Key_Escape );
    nextAct = new QAction( this );
    nextAct->setIcon( QIcon( ":icon/18n/next" ));
    nextAct->setText( "Next" );
    prevAct = new QAction( this );
    prevAct->setIcon( QIcon( ":icon/i18n/previous" ));
    prevAct->setText( "Previous" );
    finishAct = new QAction( this );
    finishAct->setIcon( QIcon( ":icon/i18n/back" ));
    finishAct->setText( "Finish" );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // QSoftMenuBar::setLabel( this, Qt::Key_Context1, QSoftMenuBar::Options );
    QSoftMenuBar::setLabel( this, Qt::Key_Select, QSoftMenuBar::NoLabel );
    QSoftMenuBar::setLabel( this, Qt::Key_Back, QSoftMenuBar::Next );

    contextMenu = QSoftMenuBar::menuFor( this );
    // contextMenu->addAction( cancelAct );
    contextMenu->addAction( finishAct );
    contextMenu->addAction( nextAct );
    contextMenu->addAction( prevAct );

    buttonNext->hide();
    buttonPrev->hide();
#else
    connect( buttonNext, SIGNAL( clicked() ),
            this, SLOT( nextPage() ));
    connect( buttonPrev, SIGNAL( clicked() ),
            this, SLOT( prevPage() ));
#endif
    connect( cancelAct, SIGNAL(triggered()),
            this, SIGNAL(reject()));
    connect( nextAct, SIGNAL(triggered()),
            this, SLOT(nextPage() ));
    connect( prevAct, SIGNAL(triggered()),
            this, SLOT(prevPage()));
    connect( finishAct, SIGNAL(triggered()),
            this, SIGNAL(accept()));
}

QWizardPrivate::~QWizardPrivate()
{
    // nothing to do
}

void QWizardPrivate::setupUi()
{
    baseLayout = new QVBoxLayout( this );
    baseLayout->setSpacing( 8 );
    baseLayout->setMargin( 0 );

    labelTitle = new QLabel( this );
    labelTitle->setObjectName( "_wiz_labelTitle" ); // NO TR
    labelTitle->setText( tr( "Title of Step" ) );
    baseLayout->addWidget( labelTitle );

    topLine = new QFrame( this );
    topLine->setObjectName( "_wiz_topLine" );  // NO TR
    topLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    baseLayout->addWidget( topLine );

    widgetStack = new QStackedWidget( this );
    widgetStack->setObjectName( "_wiz_widgetStack" );  // NO TR
    baseLayout->addWidget( widgetStack );

    QFont f = labelTitle->font();
    f.setBold( true );
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // make things a bit more compact
    int ptSz = f.pointSize();
    f.setPointSize( ptSz - 1 );
    baseLayout->setSpacing( 2 );
#else
    // in phone mode the context buttons replace on screen buttons
    bottomLine = new QFrame( this );
    bottomLine->setObjectName( "_wiz_bottomLine" );
    bottomLine->setFrameStyle( QFrame::HLine | QFrame::Sunken );
    baseLayout->addWidget( bottomLine );

    buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing( 4 );
    buttonLayout->setMargin( 0 );
#endif
    labelTitle->setFont( f );

    buttonPrev = new QPushButton( this );
    buttonPrev->setObjectName( "_wiz_buttonPrev" );
    buttonNext = new QPushButton( this );
    buttonNext->setObjectName( "_wiz_buttonNext" );
#ifdef QTOPIA_PDA
    buttonPrev->setText( tr( "< &Back" ) );
    buttonLayout->addWidget( buttonPrev );

    buttonNext->setText( tr( "&Next >" ) );
    buttonLayout->addWidget( buttonNext );
    baseLayout->addLayout( buttonLayout );
#endif
}

void QWizardPrivate::loadPages( QObject *p )
{
    QObjectList ol = p->children();
    QWidget *w = 0;
    int beforeCount = pageCount;
    for ( int i = 0; i < ol.count(); ++i )
    {
        // dont add QWizards own internal widgets
        if ( !ol[i]->isWidgetType() ) continue;
        if (ol[i]->objectName().left(5) == "_wiz_") continue;
        w = static_cast<QWidget *>( ol[i] );
        addPage( w );
    }
    if ( beforeCount == pageCount )
    {
        qWarning( "loadPages added no children of wizard - check .ui file/code" ); // no tr
        return;
    }
    finishButtonOn[ w ] = true;
    dirty = true;
}

void QWizardPrivate::addPage( QWidget *w )
{
    if ( appropriate.contains( w )) return;    // no-op if already added
    if ( w->windowTitle().isEmpty() )
        w->setWindowTitle( QString( "Pg %1 %2" ).arg( pageCount ).arg( w->objectName() ));

    w->setParent(widgetStack);
    widgetStack->addWidget( w );
    pageCount++;
    appropriate[ w ] = true;
    finishButtonOn[ w ] = false;
    nextButtonOn[ w ] = true;
    prevButtonOn[ w ] = true;
}

/*!
 Display the page with index \a page.
 Emits aboutToShowPage( page ) signal.
 */
void QWizardPrivate::showPage( int page )
{
    emit aboutToShowPage( page );
    widgetStack->setCurrentIndex( page );
    retitle();
}

/*!
  \internal
  \reimp
  */
QSize QWizardPrivate::sizeHint() const
{
    int width, height;
    width = widgetStack->sizeHint().width();
    height = 0;
    QList<QWidget *> wl;
    wl.append( labelTitle );
    wl.append( topLine );
    wl.append( widgetStack );
#ifdef QTOPIA_PDA
    wl.append( bottomLine );
    wl.append( buttonPrev );
#endif
    for ( int i = 0; i < wl.count(); ++i )
    {
        height += wl[i]->sizeHint().height();
    }
    height += ( wl.count() - 1 ) * layout()->spacing();
#ifdef QTOPIA_PDA
    int butWidth = buttonPrev->sizeHint().width()
        + buttonNext->sizeHint().width() + layout()->spacing();
    width = butWidth > width ? butWidth : width;
#endif
    return QSize( width, height );
}

/*!
  \internal
  \reimp
  */
void QWizardPrivate::showEvent( QShowEvent * )
{
    if ( widgetStack->currentWidget() == NULL )
    {
        qWarning( "attempt to show empty QWizard" );
    }
    else
    {
        retitle();
    }
}

/*!
  \internal
  */
void QWizardPrivate::connectPageEnabler( QObject *object, const char *signal, QWidget *page )
{
    if ( sigmap == NULL )
    {
        sigmap = new QSignalMapper( this );
        connect( sigmap, SIGNAL( mapped( int )),
                this, SLOT( setAppropriate( int )));
    }
    sigmap->setMapping( object, widgetStack->indexOf( page ));
    connect( object, signal, sigmap, SLOT( map() ));
}

/*!
  \reimp
  \internal
  Simply toggle page number \a wi
  Dont use this directly - use the connectPageEnabler method instead.
  */
void QWizardPrivate::setAppropriate( int wi )
{
    QWidget *w = widgetStack->widget( wi );
    appropriate[w] = !appropriate[w];
    dirty = true;
}

/*!
  \internal
 Go to next appropriate page in the QWizard.  If pages from current to
 one-before-last are not appropriate this will set the last page current,
 even if last is also not appropriate.

 If finish is set, will instead close the QWizard by calling accept().
 */
void QWizardPrivate::nextPage()
{
    QWidget *w = widgetStack->currentWidget();
    if ( buttonIsFinish )
    {
        if ( ! finishButtonOn[w] ) return;
        emit accept();
        return;
    }
    if ( ! prevButtonOn[w] ) return;
    uint lastWidgetIndex = pageCount - 1;
    uint currentIndex = widgetStack->indexOf( w );
    if ( currentIndex >= lastWidgetIndex ) return;
    while ( ++currentIndex < lastWidgetIndex &&
        ! appropriate[widgetStack->widget(currentIndex)] ) {}
    dirty = true;
    showPage( currentIndex );
}

/**
  \internal
 Go to prev appropriate page in the QWizard.  If pages from current to
 first are not appropriate this will set the first page current,
 even if first is also not appropriate.

 If cancel is set will instead close the QWizard by calling reject().
 */
void QWizardPrivate::prevPage()
{
    QWidget *w = widgetStack->currentWidget();
    if ( ! prevButtonOn[w] ) return;
    if ( buttonIsCancel )
    {
        emit reject();
        return;
    }
    uint currentIndex = widgetStack->indexOf( w );
    if ( currentIndex == 0 ) return;
    while ( --currentIndex > 0 &&
        ! appropriate[widgetStack->widget(currentIndex)] ) {}
    dirty = true;
    showPage( currentIndex );
}

/*!
  \internal
 Find the last page in the sequence after the current one which is
 appropriate.  If the current page is the last page, it is returned.
 */
uint QWizardPrivate::lastAppropriate( uint prevApprop )
{
    uint lastWidget = pageCount - 1;
    if ( prevApprop >= lastWidget ) return lastWidget;
    uint nextApprop = prevApprop;
    while ( ++prevApprop < lastWidget )
    {
        if ( appropriate[widgetStack->widget(prevApprop)] )
            nextApprop = prevApprop;
    }
    return ++nextApprop;
}

/*!
  \internal
 Find the prev page in the sequence, before the current one, which is
 appropriate.  If the current page is the first page it is returned.
 */
uint QWizardPrivate::firstAppropriate( uint prevApprop )
{
    if ( prevApprop == 0 ) return 0;
    uint nextApprop = prevApprop;
    while ( --prevApprop > 0 )
    {
        if ( appropriate[widgetStack->widget(prevApprop)] )
            nextApprop = prevApprop;
    }
    return --nextApprop;
}


/*!
  \internal
 Update the GUI elements on the QWizard, including the title label for the
 current page and the button labels.
 */
void QWizardPrivate::retitle()
{
    if ( !dirty ) return;

    QWidget *w = widgetStack->currentWidget();
    if ( w == NULL ) return;
    uint current = widgetStack->indexOf( w );
    labelTitle->setText( w->windowTitle() );
    uint first = firstAppropriate( current );
    uint last = lastAppropriate( current );
    nextAct->setEnabled( nextButtonOn[w] && current != last );
    prevAct->setEnabled( prevButtonOn[w] && current != first );
    finishAct->setEnabled( finishButtonOn[w] );
    buttonPrev->setText( current == first ? tr( "&Cancel" ) : tr( "< &Back" ));
    buttonPrev->setEnabled( current == first ? true : prevButtonOn[ w ]);
    buttonIsCancel = current == first && buttonPrev->isEnabled();
    buttonNext->setEnabled( current == last ? finishButtonOn[ w ] : nextButtonOn[ w ]);
    buttonIsFinish = current == last && buttonNext->isEnabled();
    dirty = false;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if ( current == last )
        QSoftMenuBar::setLabel( this, Qt::Key_Back,
                finishButtonOn[w] ? QSoftMenuBar::Back : QSoftMenuBar::NoLabel );
    else
        QSoftMenuBar::setLabel( this, Qt::Key_Back,
                nextButtonOn[w] ? QSoftMenuBar::Next : QSoftMenuBar::NoLabel );
    w->setFocus();  // update context menu
    setFocus();
#endif
}


/*!
  \internal
 Finish the QWizard. To exit the Qwizard and terminate the dialog,
 connect the accept signal to a done() slot on the parent.
 */
void QWizardPrivate::finish()
{
    emit accept();
}

/*!
  \internal
  \reimp
*/
void QWizardPrivate::keyPressEvent( QKeyEvent *e )
{
    switch ( e->key() )
    {
#ifdef QTOPIA_KEYPAD_NAVIGATION
        case Qt::Key_Back:
            setEditFocus( true );
            nextPage();
            e->accept();
            break;
#endif
        default:
            e->ignore();
    }
}

#include "qwizard.moc"
