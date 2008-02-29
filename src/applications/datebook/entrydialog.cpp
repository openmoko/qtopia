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

#include "entrydialog.h"
#include "repeatentry.h"
#include "appointmentdetails.h"

#include <qtopiaapplication.h>
#include <qcategoryselector.h>

#include <qtimestring.h>
#include <qtimezonewidget.h>
#if !defined(QTOPIA_PHONE)
#include <pixmapdisplay.h>
#endif

#ifdef QTOPIA_PHONE
#include <qiconselector.h>
#endif

#include <QDL>
#include <QDLEditClient>

#ifdef QTOPIA_DESKTOP
#include <worldtimedialog.h>
#endif

#include <QDebug>
#include <QCheckBox>
#include <QStyle>
#include <QRegExp>
#include <QComboBox>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QToolButton>
#include <QLabel>
#include <QLayout>
#include <QTimer>
#include <QTextEdit>
#include <QTabWidget>
#include <QGroupBox>
#include <QDateTimeEdit>

#include <stdlib.h>


static bool onceAWeek(const QAppointment &e)
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
        if (i != orig && e.repeatOnWeekDay(i))
            return false;
    }
    return true;
}

static void setOnceAWeek( QAppointment &e )
{
    int orig = e.start().date().dayOfWeek();

    for (int i = 1; i <= 7; i++) {
        if (i != orig)
            e.setRepeatOnWeekDay(i, false);
    }
}

static void addOrPick( QComboBox* combo, const QString& t )
{
    for (int i=0; i<combo->count(); i++) {
        if ( combo->itemText(i) == t ) {
            combo->setCurrentIndex(i);
            return;
        }
    }
    combo->setEditText(t);
}

//------------------------------------------------------------------------------

#ifdef QTOPIA_DESKTOP
#   define WNDFLAGS f | WStyle_Customize | WStyle_DialogBorder | WStyle_Title
#else
#   define WNDFLAGS f
#endif

EntryDialog::EntryDialog( bool startOnMonday, const QDateTime &start, const QDateTime &end,
                          QWidget *parent, Qt::WFlags f )
    : QDialog( parent, WNDFLAGS ), startWeekOnMonday( startOnMonday )
{
    init();
    setDates(start,end);
}

EntryDialog::EntryDialog( bool startOnMonday, const QAppointment &appointment,
                          QWidget *parent, Qt::WFlags f )
    : QDialog( parent, WNDFLAGS ), mAppointment(appointment), mOrigAppointment( appointment ), startWeekOnMonday( startOnMonday )
{
    init();
    if (mAppointment.timeZone().isValid()) {
        entry->timezone->setCurrentZone(mAppointment.timeZone().id());
    }

    setDates(mAppointment.start(),mAppointment.end());
    entry->comboCategory->selectCategories( mAppointment.categories() );
    addOrPick( entry->comboDescription, mAppointment.description() );
    addOrPick( entry->comboLocation, mAppointment.location() );
    entry->spinAlarm->setValue(mAppointment.alarmDelay());
    entry->spinAlarm->setEnabled(mAppointment.hasAlarm());
    if ( mAppointment.hasAlarm() ) {
        if (mAppointment.alarm() == QAppointment::Audible)
            entry->comboSound->setCurrentIndex(2);
        else
            entry->comboSound->setCurrentIndex(1);
    } else {
        entry->comboSound->setCurrentIndex(0);
    }
    entry->checkAllDay->setChecked( mAppointment.isAllDay() );
    if(!mAppointment.notes().isEmpty())
        editNote->setHtml(mAppointment.notes()); //PlainText(mAppointment.notes());

    if ( mAppointment.hasRepeat() ) {
        entry->repeatCheck->setEnabled(true);
        if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Daily)
            entry->repeatSelect->setCurrentIndex(1);
        else if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Weekly
                && onceAWeek(mAppointment) )
            entry->repeatSelect->setCurrentIndex(2);
        else if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Yearly)
            entry->repeatSelect->setCurrentIndex(3);
        else
            entry->repeatSelect->setCurrentIndex(4);
        if (mAppointment.repeatForever()) {
            // so that when opens, will have view at or close to start date of appointment,
            entry->repeatCheck->setChecked(false);
            entry->repeatDate->setDate(mAppointment.start().date());
        } else {
            entry->repeatCheck->setChecked(true);
            entry->repeatDate->setDate(mAppointment.repeatUntil());
        }
    } else {
        entry->repeatSelect->setCurrentIndex(0);
        entry->repeatCheck->setChecked(false);
        entry->repeatCheck->setEnabled(false);
        // so that when opens, will have view at or close to start date of appointment,
        entry->repeatDate->setDate(mAppointment.start().date());
    }
    setRepeatLabel();

    QDL::loadLinks( appointment.customField( QDL::CLIENT_DATA_KEY ),
                    QDL::clients( this ) );
    editnoteQC->verifyLinks();
}

void EntryDialog::setDates( const QDateTime& s, const QDateTime& e )
{
    mAppointment.setStart(s);
    mAppointment.setEnd(e);
    entry->startTime->setTime(s.time());
    entry->startDate->setDate(s.date());
    entry->endTime->setTime(e.time());
    entry->endDate->setDate(e.date());
}

#ifndef QTOPIA_DESKTOP
//  Catch resize events sent to scroll area's viewport, and force child widget to conform to parent's width
//  TODO: There has to be a better way to do this.
bool EntryDialog::eventFilter( QObject *receiver, QEvent *event )
{
    if( scrollArea && scrollArea->widget() && receiver == scrollArea->viewport() && event->type() == QEvent::Resize )
        scrollArea->widget()->setFixedWidth( scrollArea->viewport()->width() );
    return false;
}
#endif

void EntryDialog::init()
{
    setObjectName( "edit-appointment" );

    QGridLayout *gl = new QGridLayout( this );

    tw = new QTabWidget( this );
    gl->addWidget( tw, 0, 0, 0, 2 );

#ifdef QTOPIA_DESKTOP
    gl->setSpacing( 6 );
    gl->setMargin( 6 );
    QPushButton *buttonCancel = new QPushButton( this );
    buttonCancel->setText( tr("Cancel") );
    gl->addWidget( buttonCancel, 1, 2 );

    QPushButton *buttonOk = new QPushButton( this );
    buttonOk->setText( tr( "OK" ) );
    gl->addWidget( buttonOk, 1, 1 );
    buttonOk->setDefault( true );

    QSpacerItem *spacer = new QSpacerItem( 20, 20, QSizePolicy::Expanding, QSizePolicy::Minimum );
    gl->addItem( spacer, 1, 0 );
#else
    gl->setSpacing( 3 );
    gl->setMargin( 0 );
#endif

    QWidget *noteTab = new QWidget( this );
    QGridLayout *noteLayout = new QGridLayout( noteTab );

    editNote = new QTextEdit( noteTab );
    editNote->setLineWrapMode( QTextEdit::WidgetWidth );

    int rowCount = 0;
    editnoteQC = new QDLEditClient( editNote, "editnote" );

#ifdef QTOPIA_PHONE
    editnoteQC->setupStandardContextMenu();
#else
    PixmapDisplay *linkButton = new PixmapDisplay( noteTab );
    linkButton->setPixmap( QIcon( ":icon/qdllink" )
                                            .pixmap( QStyle::PixelMetric(QStyle::PM_SmallIconSize),true ) );
    connect( linkButton, SIGNAL(clicked()), client, SLOT(requestLink()) );
    noteLayout->addWidget( linkButton, rowCount++, 0, Qt::AlignRight );
    linkButton->setFocusPolicy( Qt::NoFocus );
#endif

    noteLayout->addWidget( editNote, rowCount++, 0 );

#ifndef QTOPIA_DESKTOP
    scrollArea = new QScrollArea( this );
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setFocusPolicy( Qt::NoFocus );
    scrollArea->setFrameStyle( QFrame::NoFrame );
    entry = new EntryDetails( scrollArea->viewport() );
    scrollArea->setWidget(entry);
    tw->addTab( scrollArea, tr("Event") );

    //  Event filter to catch viewport resizes to force child widget to conform to horiz. size.
    //  TODO: There has to be a better way to do this.
    scrollArea->viewport()->installEventFilter( this );
#else
    entry = new EntryDetails( tw );
    tw->addTab( entry, tr("Event") );
#endif

    tw->addTab( noteTab, tr("Notes") );

#if defined(QTOPIA_DESKTOP)
    appointmentDetails = new AppointmentDetails( this );
    connect(tw, SIGNAL(currentChanged(QWidget*)),
            this, SLOT(tabChanged(QWidget*)) );
    tw->addTab( appointmentDetails, tr("Summary") );
#else
    appointmentDetails = 0;
#endif

    // XXX should load/save thisentry->startDate
    entry->comboLocation->addItem(tr("Office"));
    entry->comboLocation->addItem(tr("Home","ie. not work"));

    // XXX enable these two lines to be able to specify local time appointments.
    entry->timezone->setLocalIncluded(true);
    entry->timezone->setCurrentZone("None");

    entry->comboDescription->setInsertPolicy(QComboBox::InsertAtCurrent);
    entry->comboLocation->setInsertPolicy(QComboBox::InsertAtCurrent);

#ifdef QTOPIA_PHONE
    // Setup the comboSound widget
    entry->comboSound->insertItem( QPixmap( ":image/noalarm" ), tr( "No Alarm" ) );
    entry->comboSound->insertItem( QPixmap( ":image/silent" ), tr( "Silent" ) );
    entry->comboSound->insertItem( QPixmap( ":image/audible" ), tr( "Audible" ) );
#endif

#ifdef QTOPIA_DESKTOP
    connect((QObject *)entry->timezone->d, SIGNAL(configureTimeZones()), this, SLOT(configureTimeZones()));
#endif

    entry->comboDescription->setFocus();

    connect( entry->repeatSelect, SIGNAL(activated(int)),
            this, SLOT(setRepeatRule(int)));
    connect( entry->repeatDate, SIGNAL(dateChanged(const QDate&)),
            this, SLOT(setEndDate(const QDate&)));

    connect( entry->spinAlarm, SIGNAL(valueChanged(int)),
             this, SLOT(turnOnAlarm()) );
    connect( entry->comboSound, SIGNAL(activated(int)),
             this, SLOT(checkAlarmSpin(int)) );

    connect( entry->startDate, SIGNAL(dateChanged(const QDate &)),
             this, SLOT(updateStartDateTime()));
    connect( entry->startTime, SIGNAL(timeChanged(const QTime &)),
             this, SLOT(updateStartTime()));
    connect( entry->startTime, SIGNAL(editingFinished()),
             this, SLOT(updateStartTime()));
    connect( entry->endDate, SIGNAL(dateChanged(const QDate &)),
             this, SLOT(updateEndDateTime()));
    connect( entry->endTime, SIGNAL(timeChanged(const QTime &)),
             this, SLOT(updateEndTime()));
    connect( entry->endTime, SIGNAL(editingFinished()),
             this, SLOT(updateEndTime()));

    connect( entry->repeatCheck, SIGNAL(toggled(bool)),
            this, SLOT(checkRepeatDate(bool)));

    connect( qApp, SIGNAL(weekChanged(bool)),
             this, SLOT(setWeekStartsMonday(bool)) );

#ifdef QTOPIA_DESKTOP
    connect( buttonOk, SIGNAL(clicked()), this, SLOT(accept()) );
    connect( buttonCancel, SIGNAL(clicked()), this, SLOT(reject()) );

    setMaximumSize( sizeHint()*2 );

    connect( entryDetails()->comboCategory,
             SIGNAL(editCategoriesClicked(QWidget*)),
             this, SLOT(editCategories(QWidget*)) );

    connect( this, SIGNAL(categorymanagerChanged()),
             this, SLOT(updateCategories()) );

    resize( 500, 300 );
#endif
}

/*
 *  Destroys the object and frees any allocated resources
 */
EntryDialog::~EntryDialog()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 * public slot
 */
void EntryDialog::updateEndTime()
{
    // Filter time edits, only process the time when we are not in edit mode
    if ( !entry->endTime->hasEditFocus() ) {
        updateEndDateTime();
    }
}

/*
 * public slot
 */
void EntryDialog::updateEndDateTime()
{
    entry->startDate->blockSignals(true);
    entry->startTime->blockSignals(true);
    entry->endDate->blockSignals(true);
    entry->endTime->blockSignals(true);
    entry->repeatDate->blockSignals(true);

    QDateTime target = QDateTime(entry->endDate->date(), entry->endTime->time());

    // since setting the start can change the end, do this first.
    if (target.addSecs(-300) < mAppointment.start()) {
        mAppointment.setStart(target.addSecs(-300));
    }

    mAppointment.setEnd(target);
    entry->startDate->setDate(mAppointment.start().date());
    entry->startTime->setTime(mAppointment.start().time());
    entry->repeatDate->setDate(mAppointment.repeatUntil());

    entry->startDate->blockSignals(false);
    entry->startTime->blockSignals(false);
    entry->endDate->blockSignals(false);
    entry->endTime->blockSignals(false);
    entry->repeatDate->blockSignals(false);
}

/*
 * public slot
 */
void EntryDialog::updateStartTime()
{
    // Filter time edits, only process the time when we are not in edit mode
    if ( !entry->startTime->hasEditFocus() ) {
        updateStartDateTime();
    }
}

/*
 * public slot
 */
void EntryDialog::updateStartDateTime()
{
    entry->startDate->blockSignals(true);
    entry->startTime->blockSignals(true);
    entry->endDate->blockSignals(true);
    entry->endTime->blockSignals(true);
    entry->repeatDate->blockSignals(true);

    // start always works.
    QDateTime s = QDateTime(entry->startDate->date(), entry->startTime->time());

    mAppointment.setStart(s);

    // modifying start modifies end, so no need check or modify anything.
    // just ensure we update the widget.
    entry->endDate->setDate(mAppointment.end().date());
    entry->endTime->setTime(mAppointment.end().time());
    entry->repeatDate->setDate(mAppointment.repeatUntil());

    entry->startDate->blockSignals(false);
    entry->startTime->blockSignals(false);
    entry->endDate->blockSignals(false);
    entry->endTime->blockSignals(false);
    entry->repeatDate->blockSignals(false);
}

void EntryDialog::checkRepeatDate(bool on)
{
    entry->repeatDate->blockSignals(true);
    if (on) {
        mAppointment.setRepeatUntil(entry->repeatDate->date());
        // and feed it back, to follow the mAppointment logic
        entry->repeatDate->setDate(mAppointment.repeatUntil());
    } else {
        mAppointment.setRepeatForever();
        entry->repeatDate->setDate(mAppointment.start().date());
    }
    entry->repeatDate->blockSignals(false);
}

void EntryDialog::editCustomRepeat()
{
    RepeatEntry e;
    e.setModal(true);
    appointment(); // update of the shown values;
    e.setAppointment(mAppointment);

    if ( QtopiaApplication::execDialog(&e) ) {
         mAppointment = e.appointment();
         entry->repeatCheck->setEnabled(true);
    }
    setRepeatLabel();
}

void EntryDialog::setWeekStartsMonday( bool onMonday )
{
    startWeekOnMonday = onMonday;
}

QAppointment EntryDialog::appointment( const bool includeQdlLinks )
{
    mAppointment.setDescription( entry->comboDescription->currentText() );
    mAppointment.setLocation( entry->comboLocation->currentText() );
    mAppointment.setCategories( entry->comboCategory->selectedCategories() );
    mAppointment.setAllDay( entry->checkAllDay->isChecked() );

    // don't set the time if theres no need too

    if (entry->timezone->currentZone() == "None")
        mAppointment.setTimeZone(QTimeZone());
    else
        mAppointment.setTimeZone(QTimeZone(entry->timezone->currentZone().toAscii().constData()));

    // all day appointments don't have an alarm
    if ( entry->checkAllDay->isChecked() )
        entry->comboSound->setCurrentIndex(0);
    // we only have one type of sound at the moment... LOUD!!!
    switch (entry->comboSound->currentIndex()) {
        case 0:
            mAppointment.clearAlarm();
            break;
        case 1:
            mAppointment.setAlarm( entry->spinAlarm->value(), QAppointment::Visible);
            break;
        case 2:
            mAppointment.setAlarm( entry->spinAlarm->value(), QAppointment::Audible);
            break;
    }
    // don't need to do repeat, repeat dialog handles that.

    if ( editNote->toPlainText().isEmpty() )
        mAppointment.setNotes( QString() );
    else
        mAppointment.setNotes( editNote->toHtml() );

    if ( includeQdlLinks ) {
        QString links;
        QDL::saveLinks( links, QDL::clients( this ) );
        mAppointment.setCustomField( QDL::CLIENT_DATA_KEY, links );
    }

    return mAppointment;
}

void EntryDialog::setRepeatLabel()
{
    if ( mAppointment.hasRepeat() ) {
        //cmdRepeat->setText( tr("Repeat...") );
        if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Daily)
            entry->repeatSelect->setCurrentIndex(1);
        else if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Weekly
                && onceAWeek(mAppointment))
            entry->repeatSelect->setCurrentIndex(2);
        else if (mAppointment.frequency() == 1 && mAppointment.repeatRule() == QAppointment::Yearly)
            entry->repeatSelect->setCurrentIndex(3);
        else
            entry->repeatSelect->setCurrentIndex(4);
    } else {
        entry->repeatSelect->setCurrentIndex(0);
    }
}

void EntryDialog::configureTimeZones()
{
#ifdef QTOPIA_DESKTOP
    // Show the WorldTime as a dialog
    WorldTimeDialog dlg(0, entry->timezone);
    dlg.exec();
#endif
}

void EntryDialog::turnOnAlarm()
{
    // if alarm spin spun, then user probably wants an alarm.  Make it loud :)
    if (entry->comboSound->currentIndex() == 0 && entry->spinAlarm->value() != 0)
        entry->comboSound->setCurrentIndex(2);
}

void EntryDialog::checkAlarmSpin(int aType)
{
    entry->spinAlarm->setEnabled( aType != 0 );
}

void EntryDialog::setRepeatRule(int i)
{
    switch (i) {
        case 0:
            mAppointment.setRepeatRule(QAppointment::NoRepeat);
            entry->repeatCheck->setChecked(false);
            entry->repeatCheck->setEnabled(false);
            break;
        case 1:
            mAppointment.setRepeatRule(QAppointment::Daily);
            mAppointment.setFrequency(1);
            entry->repeatCheck->setEnabled(true);
            break;
        case 2:
            mAppointment.setRepeatRule(QAppointment::Weekly);
            mAppointment.setFrequency(1);
            setOnceAWeek(mAppointment);
            entry->repeatCheck->setEnabled(true);
            break;
        case 3:
            mAppointment.setRepeatRule(QAppointment::Yearly);
            mAppointment.setFrequency(1);
            entry->repeatCheck->setEnabled(true);
            break;
        case 4:
        default:
            editCustomRepeat();
            break;
    }
}

void EntryDialog::setEndDate(const QDate &date)
{
    entry->repeatDate->blockSignals(true);

    mAppointment.setRepeatUntil(entry->repeatDate->date());
    entry->repeatDate->setDate(mAppointment.repeatUntil());

    entry->repeatDate->blockSignals(false);
}

void EntryDialog::updateCategories()
{
#ifdef QTOPIA_DESKTOP
    connect( this, SIGNAL( categorymanagerChanged() ),
             entry->comboCategory, SLOT( categorymanagerChanged() ) );
    emit categorymanagerChanged();
    disconnect( this, SIGNAL( categorymanagerChanged() ),
                entry->comboCategory, SLOT( categorymanagerChanged() ) );
#endif
}

void EntryDialog::accept()
{
    // see if anything changed - if not, just close
    if ( appointment( false ) == mOrigAppointment ) {
        hide();
        QDialog::reject();
        return;
    }

    // otherwise, see if we now have an empty description
    if ( entry->comboDescription->currentText().isEmpty() )
    {
        if (QMessageBox::warning(this, tr("New Event"),
                    tr("<qt>An event description is required. Cancel editing?</qt>"),
                    QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
            QDialog::reject();
            return;
        } else {
            entry->comboDescription->setFocus();
            return;
        }

    }

    // Otherwise, we're done
    hide();
    appointment();
    QDialog::accept();
}

void EntryDialog::tabChanged( QWidget *tab )
{
    if ( appointmentDetails && tab == appointmentDetails ) {
        appointmentDetails->init( appointment().firstOccurrence() );
    }
}

void EntryDialog::showSummary()
{
    tw->setCurrentIndex( tw->indexOf( appointmentDetails ) );
}

// ====================================================================

EntryDetails::EntryDetails( QWidget *parent )
    : QWidget( parent )
{
    setupUi( this );
}

EntryDetails::~EntryDetails()
{
}

