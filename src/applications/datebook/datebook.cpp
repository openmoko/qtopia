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

#include "datebook.h"

#include "dayview.h"
#include "monthview.h"
#include "appointmentdetails.h"
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
#   include "weekview.h"
#endif

#include "appointmentpicker.h"
#include "datebooksettings.h"
#include "exceptiondialog.h"
#include "entrydialog.h"
#include "alarmdialog.h"
#include "finddialog.h"
#ifdef GOOGLE_CALENDAR_CONTEXT
#include "accounteditor.h"
#endif

#include <qtopiaapplication.h>
#include <qtopianamespace.h>
#include <qtopianamespace.h>
#include <qtimestring.h>
#include <qtimezonewidget.h>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qappointmentmodel.h>

#include <QDL>
#include <QDLLink>
#include <QDSActionRequest>
#include <QDSData>
#include <QDebug>

#include <QSettings>
#include <QAction>
#include <QTimer>
#include <QDateTime>
#include <QDialog>
#include <QDir>
#include <QFile>
#include <QLabel>
#include <QLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QTextCodec>
#include <QTextStream>
#include <QRegExp>
#include <QStackedWidget>
#include <QToolBar>
#include <QCloseEvent>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>

#ifdef QTOPIA_DESKTOP
#   include <common/action.h>
#   include <qtopia/categoryselect.h>
#endif

#include <qtopiasendvia.h>
#include <qtopiaipcenvelope.h>

#include <qcontent.h>

#ifdef QTOPIA_PHONE
#   include <qtopia/qsoftmenubar.h>
#endif

#ifndef Q_OS_WIN32
#   include <unistd.h>
#endif

static const unsigned int   DATEBOOK_UPDATE_ICON_TIMEOUT    =   500;

DateBook::DateBook(QWidget *parent, Qt::WFlags f)
#ifndef QTOPIA_DESKTOP
: DateBookGui(parent, f),
#else
: DateBookGui(),
#endif
  editorView(0),
  exceptionDialog(0),
  aPreset(false),
  presetTime(-1),
  startTime(8), // an acceptable default
  compressDay(true),
  syncing(false),
  inSearch(false),
  closeAfterView(false),
  updateIconsTimer(0),
  prevOccurrences()
{
    init();

    if ( updateIconsTimer )
        updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );
}

DateBook::~DateBook()
{
}

void DateBook::flush()
{
    //  TODO
    syncing = true;
}

void DateBook::reload()
{
    //  TODO
    syncing = false;
}

void DateBook::updateAlarms()
{
    /*
    time may have gone backwards... but giving the same alarms
    again won't hurt (server will ignore) so send them all again,
    */
    // no real way to tell if forward or backward, so need to do the lot.
    // TODO model->updateAlarms();
}

void DateBook::selectToday()
{
    lastToday = QDate::currentDate();
    if (viewStack->currentWidget() == dayView) {
        viewDay(lastToday);
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
    } else if (viewStack->currentWidget() == weekView) {
        weekView->selectDate( lastToday );
#endif
    } else if (viewStack->currentWidget() == monthView) {
        monthView->setSelectedDate( lastToday );
    }
}

void DateBook::viewToday()
{
    lastToday = QDate::currentDate();
    viewDay(lastToday);
}

void DateBook::viewDay()
{
    viewDay(currentDate());
}

void DateBook::viewDay(const QDate& d)
{
    initDayView();
    actionDay->setChecked(true);
    dayView->selectDate(d);
    raiseView(dayView);

    updateIcons();
}

void DateBook::viewWeek()
{
    viewWeek(currentDate());
}

void DateBook::viewWeek(const QDate& d)
{
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
    initWeek();
    actionWeek->setChecked( true );
    weekView->selectDate( dt );
    raiseView( weekView );
    weekView->redraw();

    updateIcons();
#else
    Q_UNUSED(d);
#endif
}

void DateBook::viewMonth()
{
    viewMonth(currentDate());
}

void DateBook::viewMonth(const QDate& d)
{
    initMonthView();
    actionMonth->setChecked(true);
    raiseView(monthView);
    monthView->setSelectedDate(d);

    updateIcons();
}

void DateBook::nextView()
{
    QWidget* cur = viewStack->currentWidget();
    if (cur) {
        if (cur == dayView)
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
            viewWeek();
        else if (cur == weekView)
#endif
            viewMonth();
        else if (cur == monthView)
            viewDay();
    }
}

void DateBook::unfoldAllDay()
{
    if (dayView)
        dayView->setAllDayFolded(false);

    updateIcons();
}

void DateBook::foldAllDay()
{
    if (dayView)
        dayView->setAllDayFolded(true);

    updateIcons();
}

void DateBook::checkToday()
{
    if (lastToday != QDate::currentDate()) {
        if (lastToday == currentDate())
            selectToday();
        else
            lastToday = QDate::currentDate();
    }

    midnightTimer->start((QTime::currentTime().secsTo(QTime(23, 59, 59)) + 3) * 1000);
}

void DateBook::showSettings()
{
#ifndef QTOPIA_DESKTOP
    QSettings config("Trolltech", "qpe");
    config.beginGroup("Time");
    bool whichclock = config.value("AMPM").toBool();

    DateBookSettings frmSettings(whichclock, parentWidget);
    frmSettings.setStartTime(startTime);
    frmSettings.setAlarmPreset(aPreset, presetTime);
    frmSettings.setCompressDay(compressDay);

    if (QtopiaApplication::execDialog(&frmSettings)) {
        aPreset = frmSettings.alarmPreset();
        presetTime = frmSettings.presetTime();
        startTime = frmSettings.startTime();
        compressDay = frmSettings.compressDay();

        if (dayView)
            dayView->setDaySpan( startTime, 17 );

#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
        if ( weekView )
            weekView->setDayStarts( startTime );
# endif

        saveSettings();
    }
#endif  // !QTOPIA_DESKTOP
}

void DateBook::showAccountSettings()
{
    /* TODO possible not a dialog in that cancel might not make sense. */
#ifdef GOOGLE_CALENDAR_CONTEXT
    QDialog diag;
    QVBoxLayout *vl = new QVBoxLayout;
    AccountEditor *editor = new AccountEditor;
    editor->setModel(model);
    vl->addWidget(editor);
    diag.setLayout(vl);
    QtopiaApplication::execDialog(&diag);
#endif
}

void DateBook::changeClock()
{
}

void DateBook::changeWeek(bool m)
{
    /* no need to redraw, each widget catches.  Do need to
    store though for widgets we haven't made yet */
    onMonday = m;
}

void DateBook::appMessage(const QString& msg, const QByteArray& data)
{
    if (msg == "alarm(QDateTime,int)") {
        /*@ \message
            \arguments when warn
          Register or respond to a preset alarm.
          The appointment is at \i when and the alarm goes off \i warn minutes before.
        */

        QDataStream stream(data);
        QDateTime when;
        int warn;
        stream >> when >> warn;

        //  May be more than one item.
        QDateTime alarmTime = when.addSecs(60 * warn);
        QOccurrenceModel *om = new QOccurrenceModel(alarmTime, alarmTime, this);
        om->completeFetch();

        for (int i = 0; i < om->rowCount(); i++) {
            QOccurrence o = om->occurrence(i);
            QAppointment a = o.appointment();

            if (a.hasAlarm() && o.alarmDelay() == warn) {
                int stopTimer = 0;
                bool sound = false;
                QAppointment::AlarmFlags alarm = a.alarm();

                if (alarm | QAppointment::Audible) {
                    Qtopia::soundAlarm();
                    stopTimer = startTimer(5000);
                    sound = true;
                }

                AlarmDialog dlg(parentWidget->isVisible() ? parentWidget : 0);
                dlg.setModal(true);
                bool needShow = (dlg.exec(o) == AlarmDialog::Details);

                if (sound)
                    killTimer(stopTimer);

                if (needShow) {
                    initDayView();
                    showAppointmentDetails(o);
                }

                //  Clear the alarm now that it has been sounded
                a.clearAlarm();
                model->updateAppointment(a);

                if (dlg.getSkipDialogs())
                    break;
            }
        }

    /*@ \service Receive
    This service is invoked when typed data is received from outside the device,
    such as via IR beaming. The actual service is Receive/mimetype, such as
    Receive/image or Receive/image/jpeg.
    */
    }
    #if 0
    else if ( msg == "receiveData(QString,QString)" ) {
        /*@ \message
            \arguments file mimetype
          Handle incoming IR data.
        */
        QString f,t;
        stream >> f >> t;
        if (t.toLower() == "text/x-vcalendar")
            receiveFile(f);
        QFile::remove(f);
    }
    #endif
}

void DateBook::qdlActivateLink( const QDSActionRequest& request )
{
    // If we are currently viewing an appointment, chuck it on the
    // previous appointments stack
    if ( appointmentDetails &&
         viewStack->currentWidget() == appointmentDetails ) {
        prevOccurrences.push( appointmentDetails->occurrence() );
    }

    // Grab the link from the request and check that is one of ours
    QDLLink link( request.requestData() );
    if ( link.service() != "Calendar" ) {
        QDSActionRequest( request ).respond( "Link doesn't belong to Calendar" );
        return;
    }

    const QByteArray data = link.data();
    QDataStream refStream( data );
    QUniqueId u;
    QDate date;
    refStream >> u >> date;

    QAppointment a = model->appointment( u );
    if ( a.isValid() ) {
        initDayView();
        showAppointmentDetails( QOccurrence( date, a ) );
        showMaximized();
        QDSActionRequest( request ).respond();
    } else {
        QMessageBox::warning(
            this,
            tr("Calendar"),
            "<qt>" + tr("The selected event no longer exists.") + "</qt");
        QDSActionRequest( request ).respond( "Event doesn't exist" );
    }

}

void DateBook::qdlRequestLinks( const QDSActionRequest& request )
{
    QDSActionRequest processingRequest( request );
    AppointmentPicker evtPick( this, parentWidget );
    evtPick.setModal( true );
    evtPick.showMaximized();
    if ( evtPick.exec() ) {
        if ( !evtPick.appointmentSelected() ) {
            processingRequest.respond( "No Event Selected" );
        } else {
            QList<QDSData> links;
            QAppointment appointment = evtPick.currentAppointment();
            links.push_back( appointmentQDLLink( appointment ) );

            QByteArray array;
            {
                QDataStream ds( &array, QIODevice::WriteOnly );
                ds << links;
            }

            processingRequest.respond(
                QDSData( array, QDLLink::listMimeType() ) );
        }
    } else {
        processingRequest.respond( "Event selection cancelled" );
    }
}

QDSData DateBook::appointmentQDLLink( QAppointment& appointment )
{
    if ( appointment == QAppointment() )
        return QDSData();

    // Check if we need to create the QDLLink
    QString keyString = appointment.customField( QDL::SOURCE_DATA_KEY );
    if ( keyString.isEmpty() ||
         !QDSData( QLocalUniqueId( keyString ) ).isValid() ) {
        QByteArray dataRef;
        QDataStream refStream( &dataRef, QIODevice::WriteOnly );
        refStream << appointment.uid();

        QDLLink link( "Calendar",
                      dataRef,
                      appointment.description(),
                      QString( "pics/datebook/DateBook" ) );

        QDSData linkData = link.toQDSData();
        QLocalUniqueId key = linkData.store();
        appointment.setCustomField( QDL::SOURCE_DATA_KEY, key.toString() );
        model->updateAppointment( appointment );

        return linkData;
    }

    // Get the link from the QDSDataStore
    return QDSData( QLocalUniqueId( keyString ) );
}

void DateBook::removeAppointmentQDLLink( QAppointment& appointment )
{
    if ( appointment == QAppointment() )
        return;

    // Release any client QDLLinks
    QString links = appointment.customField( QDL::CLIENT_DATA_KEY );
    if ( !links.isEmpty() ) {
        QDL::releaseLinks( links );
    }

    // Check if the Appointment is a QDLLink source, if so break it
    QString key = appointment.customField( QDL::SOURCE_DATA_KEY );
    if ( !key.isEmpty() ) {
        // Break the link in the QDSDataStore
        QDSData linkData = QDSData( QLocalUniqueId( key ) );
        QDLLink link( linkData );
        link.setBroken( true );
        linkData.modify( link.toQDSData().data() );

        // Now remove our reference to the link data
        linkData.remove();

        // Finally remove the stored key
        appointment.removeCustomField( QDL::SOURCE_DATA_KEY );
        model->updateAppointment( appointment );
    }
}

void DateBook::newAppointment()
{
    newAppointment("");
}

bool DateBook::newAppointment(const QString &description)
{
    QDateTime current = QDateTime::currentDateTime();
    current.setDate(currentDate());
    QDateTime start = current;
    QDateTime end = current;

    int mod = QTime(0, 0, 0).secsTo(current.time()) % 900;
    if (mod != 0) {
        mod = 900 - mod;
        current = current.addSecs(mod);
    }

    start.setTime(current.time());
    start.setDate(current.date());
    end = current.addSecs(3600);

    return newAppointment(start, end, description, QString());
}

void DateBook::editCurrentOccurrence()
{
    bool inViewMode = (appointmentDetails && viewStack->currentWidget() == appointmentDetails);

    if (inViewMode) {
        QOccurrence o = editOccurrence(appointmentDetails->occurrence());
        showAppointmentDetails(o);
    } else {
        editOccurrence(currentOccurrence());
    }
}

QOccurrence DateBook::editOccurrence(const QOccurrence &o)
{
    return editOccurrence( o, false );
}

static bool onOccurrence(int flag)
{
    return flag == ExceptionDialog::NotEarlier || flag == ExceptionDialog::Earlier;
}

static bool postSplit(int flag)
{
    return (flag & ExceptionDialog::Later);
}
QOccurrence DateBook::editOccurrence(const QOccurrence &o, bool preview)
{
#ifndef QTOPIA_DESKTOP
    if (checkSyncing())
        return QOccurrence();
#endif
    if ( editorView != 0 )
        return QOccurrence();

    QAppointment mainApp = o.appointment();
    QAppointment editedApp = o.appointment();

    int exceptionType = ExceptionDialog::All;
    bool hasEarlier = o.start() != mainApp.start();
    bool hasLater = o.nextOccurrence().isValid();

    if (mainApp.hasRepeat() && (hasEarlier || hasLater)) {
        exceptionType = askException(tr("Edit Event"));
        QAppointment a = editedApp;
        /* fix up exception types.
           for starting occurrence
                Earlier+Selected->Selected;
                Later->NotSelected;
                Later+Selected->All;
                Earlier->Cancel;
            for ending occrrence
                Later+Selected->Selected;
                Earlier->NotSelected;
                Earlier+Selected->All
                Later->Cancel;
        */
        if (!hasEarlier) {
            if (exceptionType & ExceptionDialog::Later) {
                exceptionType |= ExceptionDialog::Earlier;
            } else {
                exceptionType &= ExceptionDialog::NotEarlier;
            }
        } else if (!hasLater) {
            if (exceptionType & ExceptionDialog::Earlier) {
                exceptionType |= ExceptionDialog::Later;
            } else {
                exceptionType &= ExceptionDialog::NotLater;
            }
        }
        switch (exceptionType) {
            case ExceptionDialog::Selected:
            case ExceptionDialog::RetainSelected:
                // create exception.
                a.clearExceptions();
                a.setRepeatRule(QAppointment::NoRepeat);
                a.setStart(o.start());
                a.setUid(QUniqueId());
                if (exceptionType == ExceptionDialog::Selected)
                    editedApp = a;
                else
                    mainApp = a;
                break;
            case ExceptionDialog::Later:
            case ExceptionDialog::Earlier:
            case ExceptionDialog::NotLater:
            case ExceptionDialog::NotEarlier:
                // split appointment
                a.setUid(QUniqueId());
                a.clearExceptions();
                if (onOccurrence(exceptionType))
                    a.setStart(o.start());
                else
                    a.setStart(o.nextOccurrence().start());
                if (postSplit(exceptionType))
                    editedApp = a;
                else
                    mainApp = a;
                break;
            case ExceptionDialog::All:
                break;
            case ExceptionDialog::Cancel:
                return QOccurrence();
        }
    }

    editorView = new EntryDialog(onMonday, editedApp, this);
    editorView->setModal(true);
    if(preview)
        editorView->showSummary();
    editorView->setWindowTitle(tr("Edit Event", "window title" ));

    while (QtopiaApplication::execDialog(editorView)) {
        editedApp = editorView->appointment();

        QString error = validateAppointment(editedApp);
        if (!error.isNull()) {
            if ( QMessageBox::warning(
                    parentWidget->isVisible() ? parentWidget : 0,
                    "Error",
                    error,
                    QMessageBox::Ok, QMessageBox::Cancel ) == QMessageBox::Cancel )
            {
                delete editorView;
                editorView = 0;
                return QOccurrence();
            }
        } else {
            break;
        }
    }

    delete editorView;
    editorView = 0;

    QOccurrence show;
    switch (exceptionType) {
        // exceptions
        case ExceptionDialog::RetainSelected:
            // for some reason will create the exception properly,
            // except will also duplicate a non-repeating version
            // of the event set at the start.
            show = QOccurrence(o.start().date(), mainApp);
            model->replaceOccurrence(editedApp, show);
            model->updateAppointment(editedApp);
        case ExceptionDialog::Selected:
            show = QOccurrence(o.start().date(), editedApp);
            model->replaceOccurrence(mainApp, show);
            break;
        // splits
        case ExceptionDialog::Earlier:
        case ExceptionDialog::NotLater:
            model->updateAppointment(editedApp);
            model->replaceRemaining(editedApp, mainApp);
            show = QOccurrence(editedApp.start().date(), editedApp);
        case ExceptionDialog::Later:
        case ExceptionDialog::NotEarlier:
            model->replaceRemaining(mainApp, editedApp);
            show = QOccurrence(editedApp.start().date(), editedApp);
            break;
        case ExceptionDialog::All:
            model->updateAppointment(editedApp);
            show = QOccurrence(o.start().date(), editedApp);
            break;
        default:
            return QOccurrence();
    }

#ifndef QTOPIA_DESKTOP
    showAppointmentDetails(show);
#endif
    return show;
}

void DateBook::removeCurrentOccurrence()
{
    if (!occurrenceSelected())
        return;
    QOccurrence o = currentOccurrence();

    if (viewStack->currentWidget() == appointmentDetails)
        hideAppointmentDetails();

    removeOccurrence(o);
}

void DateBook::removeOccurrence(const QOccurrence &o)
{
#if !defined(QTOPIA_DESKTOP)
    if( checkSyncing() )
        return;
#endif

    QAppointment a = o.appointment();
    if (a.hasRepeat()) {
        //  Ask if just this one or the entire series
        int e = askException(tr("Delete Event"));
        switch (e) {
            case ExceptionDialog::Cancel:
                return;

            case ExceptionDialog::Selected:
                // Should check if the only one of its kind.. e.g. is
                // apointment next o, and o.n
                if (a.firstOccurrence() == o &&
                        !o.nextOccurrence().isValid()) {
                    removeAppointmentQDLLink(a);
                    model->removeAppointment(a);
                } else {
                    model->removeOccurrence(o);
                }
                break;

            case ExceptionDialog::All:
                removeAppointmentQDLLink(a);
                model->removeAppointment(a);
                break;

            case ExceptionDialog::RetainSelected:
                {
                    QAppointment a = o.appointment();
                    a.clearExceptions();
                    a.setStart(o.start());
                    a.setRepeatRule(QAppointment::NoRepeat);
                    model->updateAppointment(a);
                }
                break;

            case ExceptionDialog::Earlier:
            case ExceptionDialog::NotLater:
                // update start date
                {
                    QDateTime start = e == ExceptionDialog::Earlier ? o.start() : o.nextOccurrence().start();
                    if (start.isNull()) {
                        removeAppointmentQDLLink(a);
                        model->removeAppointment(a);
                    } else {
                        a.setStart(start);
                        if (!a.nextOccurrence(a.start().date().addDays(1)).isValid())
                            a.setRepeatRule(QAppointment::NoRepeat);
                        model->updateAppointment(a);
                    }
                }
                break;

            case ExceptionDialog::Later:
            case ExceptionDialog::NotEarlier:
                // update repeat till
                {
                    QDateTime start = e == ExceptionDialog::NotEarlier ? o.start() : o.nextOccurrence().start();
                    if (start.date() == a.start().date()) {
                        removeAppointmentQDLLink(a);
                        model->removeAppointment(a);
                    } else {
                        a.setRepeatUntil(start.date().addDays(-1));
                        if (!a.nextOccurrence(a.start().date().addDays(1)).isValid())
                            a.setRepeatRule(QAppointment::NoRepeat);
                        model->updateAppointment(a);
                    }
                    break;
                }
        }
    }
    else
    {
        if (!Qtopia::confirmDelete(parentWidget->isVisible() ? parentWidget : 0, tr("Calendar"), a.description()))
            return;
        removeAppointmentQDLLink(a);
        model->removeAppointment(a);
    }

    updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );
}

void DateBook::removeOccurrencesBefore()
{
#ifdef QTOPIA4_TODO
    QDialog dlg( parentWidget );
    dlg.setModal( true );
#ifdef QTOPIA_PHONE
    dlg.setWindowTitle( tr("Purge") );
#else
    dlg.setWindowTitle( tr("Purge Events") );
#endif

    QVBoxLayout *vb = new QVBoxLayout( &dlg );
    QLabel *lbl = new QLabel( tr("<qt>Please select a date. Everything on and before this date will be removed.</qt>"), &dlg );
    lbl->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    vb->addWidget( lbl );
    DateEdit *dp = new DateEdit( &dlg, false, true );
    vb->addWidget( dp );
#ifdef QTOPIA_DESKTOP
    QWidget *buttons = new QWidget( &dlg );
    vb->addWidget( buttons );
    QHBoxLayout *hb = new QHBoxLayout( buttons );
    hb->addStretch( 1 );
    QPushButton *ok = new QPushButton( tr("Ok"), buttons );
    hb->addWidget( ok );
    connect( ok, SIGNAL(clicked()), &dlg, SLOT(accept()) );
    QPushButton *cancel = new QPushButton( tr("Cancel"), buttons );
    hb->addWidget( cancel );
    connect( cancel, SIGNAL(clicked()), &dlg, SLOT(reject()) );
    dlg.resize( 100, 50 );
#endif

    if ( QtopiaApplication::execDialog( &dlg ) ) {
    purgeAppointments( dp->date() );
    }
#endif
}

void DateBook::removeOccurrencesBefore(const QDate &, bool)
{
#ifdef QTOPIA4_TODO
    if (date.isNull())
        return;
#ifndef QTOPIA_DESKTOP
    QDialog *wait = new QDialog( parentWidget, Qt::WStyle_Customize | Qt::WStyle_NoBorder );
    wait->setModal( true );
    QVBoxWidget *vb = new QVBoxWidget( wait );
    QLabel *l = new QLabel( tr("<b>Please Wait</b>"), vb );
    l->setAlignment( Qt::AlignCenter | Qt::AlignVCenter );
    wait->show();
    qApp->processAppointments();
#endif

    QDateTime from( date );
{
    const QList<PrAppointment*> &pappointments = model->appointmentsIO().appointments();
    foreach ( PrAppointment *e, pappointments ) {
        QDateTime s = e->startInCurrentTZ();
        if ( s < from )
            from = s;
    }
}

    QList<QAppointment> appointments = purge_getAppointments( from, date );

#ifndef QTOPIA_DESKTOP
    wait->close();
#endif

    // nothing to do
    if ( appointments.count() == 0 )
        return;

    // give the user a chance to back out
    if ( prompt && QMessageBox::warning( parentWidget->isVisible() ? parentWidget : 0, tr("WARNING"),
         tr( "<qt>You are about to delete %1 events. "
                 "Are you sure you want to do this?</qt>" ).arg( appointments.count() ),
         tr("Delete"), tr("Abort"), 0, 1, 1 ) )
        return;

#ifndef QTOPIA_DESKTOP
    wait->show();
    qApp->processAppointments();
#endif

    // delete everything (multiple passes because of interdependant appointments
    do {
    for ( QList<QAppointment>::iterator it = appointments.begin(); it != appointments.end(); ++it ) {
        model->removeAppointment( *it );
    }
    appointments = purge_getAppointments( from, date );
    } while ( appointments.count() > 0 );

#ifndef QTOPIA_DESKTOP
    delete wait;
#endif
#endif
}

void DateBook::beamCurrentAppointment()
{
    if (viewStack->currentWidget() == appointmentDetails)
        hideAppointmentDetails();
    if (occurrenceSelected())
        beamAppointment(currentAppointment());
}

void DateBook::beamAppointment(const QAppointment &e)
{
    ::unlink(beamfile.toLocal8Bit()); // delete if exists

    QAppointment::writeVCalendar(beamfile, e);
    QFile file(beamfile);
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        QtopiaSendVia::sendData(this, data, "text/x-vcalendar");
    }
}

void DateBook::setDocument(const QString &filename)
{
    QContent doc(filename);
    if (doc.isValid())
        receiveFile(doc.file());
    else
        receiveFile(filename);
}

void DateBook::showAppointmentDetails()
{
#ifdef QTOPIA_DESKTOP
    if (occurrenceSelected())
        editOccurrence(currentOccurrence(), true);
#else
    if (occurrenceSelected())
        showAppointmentDetails(currentOccurrence());
#endif
}

void DateBook::showAppointmentDetails(const QOccurrence &o)
{
    if( isHidden() ) // only close after view if hidden on first activation
    {
        closeAfterView = true;
    }

    initAppointmentDetails();
    appointmentDetails->init(o);

    if (viewStack->currentWidget() != appointmentDetails) {
        appointmentDetails->previousDetails = viewStack->currentWidget();
        raiseView(appointmentDetails);
        updateIcons();
    }

    if ( dayView) {
        dayView->setCurrentOccurrence(o);
    }
}

void DateBook::hideAppointmentDetails()
{
    if ( prevOccurrences.count() > 0 ) {
        showAppointmentDetails( prevOccurrences.top() );
        prevOccurrences.pop();
    } else {
        raiseView(appointmentDetails->previousDetails);
        appointmentDetails->previousDetails->setFocus();
    }

    updateIcons();
}

/*void DateBook::find()
{
    // move it to the day view...
    viewDay();
    FindDialog frmFind("Calendar", parentWidget->isVisible() ? parentWidget : 0); // No tr
    frmFind.setUseDate(true);
    frmFind.setDate(currentDate());
    connect(&frmFind, SIGNAL(signalFindClicked(const QString&,const QDate&,bool,bool,int)),
              this, SLOT(slotDoFind(const QString&,const QDate&,bool,bool,const QCategoryFilter&)));
    connect(this, SIGNAL(signalNotFound()),
             &frmFind, SLOT(slotNotFound()));
    connect(this, SIGNAL(signalWrapAround()),
             &frmFind, SLOT(slotWrapAround()));
    QtopiaApplication::execDialog(&frmFind);
    inSearch = false;
}

void DateBook::doFind(const QString&, const QDate &,
            Qt::CaseSensitivity, bool, const QCategoryFilter &)
{
#ifdef QTOPIA4_TODO
    bool ok;
    QRegExp r(txt);
    r.setCaseSensitivity(caseSensitive);

    QOccurrence o = model->find(r, category, dt,
                             searchForward, &ok);

    if ( ok ) {
        dayView->selectDate( o.startInCurrentTZ().date() );
        dayView->setCurrentItem(o);
    }
#endif
}*/

void DateBook::updateIcons()
{
    bool showingDetails = (appointmentDetails && viewStack->currentWidget() == appointmentDetails);

    bool itemSelected = showingDetails ? occurrenceSelected() : false;
    bool itemEditable = itemSelected ? model->editable(currentOccurrence().uid()) : false;

#ifdef QTOPIA_PHONE
    actionEdit->setVisible(itemEditable);
    actionDelete->setVisible(itemEditable);

    actionNew->setVisible(!showingDetails);
    actionToday->setVisible(!showingDetails);
    actionDay->setVisible(!showingDetails && viewStack->currentWidget() != dayView);
    actionMonth->setVisible(!showingDetails && viewStack->currentWidget() != monthView);
    actionSettings->setVisible(!showingDetails);
    actionAccounts->setVisible(!showingDetails);

    if (QtopiaSendVia::isDataSupported("text/x-vcalendar"))
        actionBeam->setVisible(itemSelected);

    if (itemSelected)
        QSoftMenuBar::setLabel(dayView, Qt::Key_Select, QSoftMenuBar::View);
    else if (dayView)
        QSoftMenuBar::setLabel(dayView, Qt::Key_Select, QSoftMenuBar::NoLabel);

    if (dayView &&
        viewStack->currentWidget() == dayView &&
        dayView->allDayFoldingAvailable()) {
        actionShowAll->setVisible(dayView->allDayFolded());
        actionHideSome->setVisible(!dayView->allDayFolded());
    } else {
        actionShowAll->setVisible(false);
        actionHideSome->setVisible(false);
    }
#else
    //  TODO
    /*if (showingDetails) {
        if (sub_bar) sub_bar->hide();
        if (details_bar) details_bar->show();
    } else {
        if (sub_bar) sub_bar->show();
        if (details_bar) details_bar->hide();
    }

    actionBack->setEnabled(showingDetails);

    if (actionFind)
        actionFind->setEnabled(!view);*/
#endif
}

void DateBook::timerEvent(QTimerEvent *e)
{
    static int stop = 0;
    if (stop < 10) {
        Qtopia::soundAlarm();
        stop++;
    } else {
        stop = 0;
        killTimer(e->timerId());
    }
}

void DateBook::closeEvent(QCloseEvent *e)
{
#ifdef QTOPIA_PHONE
    if ( ( viewStack->currentWidget() == appointmentDetails ) &&
         !closeAfterView ) {
        e->ignore();
        hideAppointmentDetails();
        return;
    }
    closeAfterView = false;
    if ( viewStack->currentWidget() == monthView ) {
        e->ignore();
        viewDay();
        return;
    }
#endif
    selectToday();
    if(syncing) {
        /* no need to save, did that at flush */
        e->accept();
        return;
    }
    // save settings will generate it's own error messages, no
    // need to do checking ourselves.
    saveSettings();
    //if ( model->save() )
    e->accept();
#if 0
    else {
        if ( QMessageBox::critical( parentWidget->isVisible() ? parentWidget : 0, tr( "Out of space" ),
                                    tr("<qt>Calendar was unable to save "
                                       "your changes. "
                                       "Free up some space and try again."
                                       "<br>Quit anyway?</qt>"),
                                    QMessageBox::Yes|QMessageBox::Escape,
                                    QMessageBox::No|QMessageBox::Default )
             != QMessageBox::No )
            e->accept();
        else
            e->ignore();
}
#endif
}

void DateBook::init()
{
    dayView = 0;
    monthView = 0;
    appointmentDetails = 0;
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
    weekView = 0;
#endif

    beamfile = Qtopia::tempDir() + "obex";

    QDir d;
    d.mkdir(beamfile);
    beamfile += "/appointment.vcs";

    model = new QAppointmentModel(this);
    loadSettings();
    DateBookGui::init();
#ifdef GOOGLE_CALENDAR_CONTEXT
    actionAccounts->setVisible(AccountEditor::editableAccounts(model));
#else
    actionAccounts->setVisible(false);
#endif

#ifndef QTOPIA_DESKTOP
    viewStack = new QStackedWidget(parentWidget);
    setCentralWidget(viewStack);

    viewToday();

    connect(qApp, SIGNAL(clockChanged(bool)), this, SLOT(changeClock()));
    connect(qApp, SIGNAL(dateFormatChanged()), this, SLOT(changeClock()));
    connect(qApp, SIGNAL(weekChanged(bool)), this, SLOT(changeWeek(bool)));
    connect(qApp, SIGNAL(timeChanged()), this, SLOT(checkToday()));
    connect(qApp, SIGNAL(timeChanged()), this, SLOT(updateAlarms()));

    connect(qApp, SIGNAL(flush()), this, SLOT(flush()));
    connect(qApp, SIGNAL(reload()), this, SLOT(reload()));

    connect(qApp, SIGNAL(appMessage(const QString&,const QByteArray&)),
            this, SLOT(appMessage(const QString&,const QByteArray&)));

    new CalendarService( this );

    // Set timer to go off 2 sections to midnight
    midnightTimer = new QTimer(this);
    connect(midnightTimer, SIGNAL(timeout()), this, SLOT(checkToday()));
    midnightTimer->start((QTime::currentTime().secsTo(QTime(23, 59, 59)) + 3) * 1000);
#endif // !QTOPIA_DESKTOP

    // Update icons timer, used to allow datebase to refresh before applying
    // the update
    updateIconsTimer = new QTimer( this );
    updateIconsTimer->setSingleShot( true );
    connect( updateIconsTimer,
             SIGNAL( timeout() ),
             this,
             SLOT( updateIcons() ) );
}

void DateBook::initDayView()
{
    if (!dayView) {
        dayView = new DayView;
        viewStack->addWidget(dayView);
        dayView->setDaySpan(startTime, 17);
        connect(dayView, SIGNAL(newAppointment()), this, SLOT(newAppointment()));
        connect(dayView, SIGNAL(removeOccurrence(const QOccurrence&)),
                this, SLOT(removeOccurrence(const QOccurrence&)));
        connect(dayView, SIGNAL(editOccurrence(const QOccurrence&)),
                this, SLOT(editOccurrence(const QOccurrence&)));
        connect(dayView, SIGNAL(beamAppointment(const QAppointment&)),
                this, SLOT(beamAppointment(const QAppointment&)));
        connect(dayView, SIGNAL(newAppointment(const QString&)),
                this, SLOT(newAppointment(const QString&)));
        connect(dayView, SIGNAL(dateChanged()), this, SLOT(updateIcons()));
        connect(dayView, SIGNAL(showDetails()), this, SLOT(showAppointmentDetails()));
        connect(dayView, SIGNAL(selectionChanged()), this, SLOT(updateIcons()));
    }
}

void DateBook::initMonthView()
{
    if (!monthView) {
        monthView = new MonthView;
        monthView->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
        viewStack->addWidget(monthView);
        connect(monthView, SIGNAL(activated(const QDate&)),
                this, SLOT(viewDay(const QDate&)));
    }
}

void DateBook::initAppointmentDetails()
{
    if (!appointmentDetails) {
        appointmentDetails = new AppointmentDetails(viewStack);
        viewStack->addWidget(appointmentDetails);
        connect(appointmentDetails, SIGNAL(done()), this, SLOT(hideAppointmentDetails()));
    }
}

#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
void DateBook::initWeekView()
{
    if (!weekView) {
        weekView = new WeekView( model, onMonday, viewStack );
        weekView->setDayStarts( startTime );
        viewStack->addWidget( weekView );
        connect( weekView, SIGNAL( dateActivated(const QDate&) ),
                this, SLOT( viewDay(const QDate&) ) );

            // qApp connections
        connect( qApp, SIGNAL(weekChanged(bool)),
                weekView, SLOT(setStartOnMonday(bool)) );
    }
}
#endif

void DateBook::initExceptionDialog()
{
    if (!exceptionDialog) {
        exceptionDialog = new ExceptionDialog(parentWidget);
        exceptionDialog->setModal(true);
    }
}

int DateBook::askException(const QString &action)
{
    initExceptionDialog();
    exceptionDialog->setWindowTitle(action);
    return exceptionDialog->exec();
}

void DateBook::loadSettings()
{
    {
        QSettings config("Trolltech","qpe");
        config.beginGroup("Time");
        onMonday = config.value( "MONDAY" ).toBool();
    }

    {
        QSettings config("Trolltech","DateBook");
        config.beginGroup("Main");
        startTime = config.value("startviewtime", 8).toInt();
        aPreset = config.value("alarmpreset").toBool();
        presetTime = config.value("presettime").toInt();
#ifdef QTOPIA_PHONE
        compressDay = true;
#else
        compressDay = config.value("compressday", true).toBool();
#endif
    }
}

void DateBook::saveSettings()
{
    QSettings config("Trolltech","qpe");
    QSettings configDB("Trolltech","DateBook");
    configDB.beginGroup("Main");
    configDB.setValue("startviewtime",startTime);
    configDB.setValue("alarmpreset",aPreset);
    configDB.setValue("presettime",presetTime);
    configDB.setValue("compressday", compressDay);
}

bool DateBook::receiveFile(const QString &filename)
{
    QList<QAppointment> tl = QAppointment::readVCalendar(filename);

    QString msg = tr("<p>%1 new events.<p>Do you want to add them to your Calendar?",
                     "%1 number").
            arg(tl.count());

    if ( QMessageBox::information(parentWidget->isVisible() ? parentWidget : 0, tr("New Events"),
         msg, QMessageBox::Ok, QMessageBox::Cancel)==QMessageBox::Ok ) {
             QDateTime from,to;
             for( QList<QAppointment>::const_iterator it = tl.begin(); it != tl.end(); ++it ) {
                 if ( from.isNull() || (*it).start() < from )
                     from = (*it).start();
                 if ( to.isNull() || (*it).end() < to )
                     to = (*it).end();
                 model->addAppointment( *it );
             }

        // Change view to a sensible one...
#ifdef QTOPIA_PHONE
        if ( from.date() == to.date() )
            viewDay( from.date() );
        else
            viewMonth( from.date() );
#else
        if ( from.date() == to.date() ) {
    viewDay(from.date());
        } else {
#ifdef QTOPIA4_TODO
            initWeek();
            int fw,fy,tw,ty;
            weekView->calcWeek(from.date(), fw, fy);
            weekView->calcWeek(to.date(), tw, ty);
            if ( fw == tw && fy == ty ) {
                viewWeek(from.date());
            } else if ( from.date().month() == to.date().month()
                        && from.date().year() == to.date().year() ) {
                            viewMonth(from.date());
                        } else {
                            viewDay(from.date());
                        }
#endif
        }
#endif

        return true;
         }
         return false;
}

bool DateBook::newAppointment(const QDateTime& dstart, const QDateTime& dend, const QString& description, const QString& notes)
{
#ifndef QTOPIA_DESKTOP
    if (checkSyncing())
        return false;
#endif

    if ( editorView != 0 )
        return false;

    //
    //  Figure out a start and end times for the new appointment, if none given
    //

    QDateTime start = dstart;
    QDateTime end = dend;
    bool startNull = start.isNull();
    bool endNull = end.isNull();

    QDateTime current = QDateTime::currentDateTime();
    current.setDate(currentDate());

    if (viewStack->currentWidget()) {
        if(viewStack->currentWidget() == monthView)
            start.setDate(monthView->selectedDate());
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
        else if(viewStack->currentWidget() == weekView)
            start.setDate(weekView->selectedDate());
#endif
    }

    if (start.date().isNull())
        start.setDate(current.date());

    if (end.date().isNull())
        end.setDate(current.date());

    if ((startNull && end.time().isNull()) || !start.time().isValid()) {
        //  If no time is given, use the current time rounded up to the nearest 15 minutes
        int mod = QTime(0, 0, 0).secsTo(current.time()) % 900;
        if (mod != 0)
        {
            mod = 900 - mod;
            current = current.addSecs(mod);
        }
    }

    //  If no end time is given, default to one hour after start time
    if ((endNull && end.time().isNull()) || !end.time().isValid())
        end = start.addSecs(3600);

    //
    //  Prepare the appointment used by the dialog
    //

    QAppointment a;
    a.setDescription(description);
    a.setLocation("");
    a.setStart(start);
    a.setEnd(end);
    a.setNotes(notes);
    if (aPreset)
        a.setAlarm(presetTime, QAppointment::Audible);

    editorView = new EntryDialog(onMonday, a, parentWidget);
    editorView->setObjectName("new-event");
    editorView->setModal(true);
    editorView->setWindowTitle(tr("New Event"));

    while ( QtopiaApplication::execDialog( editorView ) ) {
        a = editorView->appointment();

        QString error = validateAppointment(a);
        if(!error.isNull()) {
            if (QMessageBox::warning(parentWidget->isVisible() ? parentWidget : 0, tr("Error!"),
                error, tr("Fix it"), tr("Continue"), 0, 0, 1 ) == 0)
                continue;
        }

        QUniqueId id = model->addAppointment(a);
        a.setUid(id);

        if (dayView) {
            dayView->setCurrentAppointment(a);
        }

        updateIconsTimer->start( DATEBOOK_UPDATE_ICON_TIMEOUT );

        delete editorView;
        editorView = 0;

        return true;
    }

    delete editorView;
    editorView = 0;

    return false;
}

void DateBook::addAppointment(const QAppointment &e)
{
    QDate d = e.start().date();
    initDayView();
    dayView->selectDate(d);
}

bool DateBook::occurrenceSelected() const
{
    if (viewStack->currentWidget() && viewStack->currentWidget() == dayView)
        return dayView->currentIndex().isValid();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence().isValid();
    return false;
}

QAppointment DateBook::currentAppointment() const
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentAppointment();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence().appointment();

    return QAppointment();
}

QOccurrence DateBook::currentOccurrence() const
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentOccurrence();
    if (appointmentDetails && viewStack->currentWidget() == appointmentDetails)
        return appointmentDetails->occurrence();

    return QOccurrence();
}

void DateBook::raiseView(QWidget *widget)
{
    if (!widget)
        return;

#ifndef QTOPIA_DESKTOP
    if (parentWidget)
        parentWidget->setObjectName(widget->objectName());
#endif

    viewStack->setCurrentIndex(viewStack->indexOf(widget));
}

QDate DateBook::currentDate()
{
    if (dayView && viewStack->currentWidget() == dayView)
        return dayView->currentDate();
#if !defined(QTOPIA_PHONE) && defined(QTOPIA4_TODO)
    else if (weekView && viewStack->currentWidget() == weekView)
        return weekView->currentDate();
#endif
    else if (monthView && viewStack->currentWidget() == monthView)
        return monthView->selectedDate();
    else
        return QDate(); // invalid;
}

bool DateBook::checkSyncing()
{
    if (syncing) {
        if (QMessageBox::warning(parentWidget->isVisible() ? parentWidget : 0, tr("Calendar"),
            tr("<qt>Can not edit data, currently syncing</qt>"),
            QMessageBox::Ok, QMessageBox::Abort ) == QMessageBox::Abort)
        {
            // Okay, if you say so (eg. Qtopia Desktop may have crashed)....
            syncing = false;
        } else
            return true;
    }
    return false;
}

QString DateBook::validateAppointment(const QAppointment &e)
{
    // Check if overlaps with itself
    bool checkFailed = false;

    // check the next 12 repeats. should catch most problems
    QDate current_date = e.end().date();
    QOccurrence previous = e.nextOccurrence(current_date.addDays(1));
    int count = 12;
    while(count-- && previous.isValid()) {
        QOccurrence next = previous.nextOccurrence();
        if (!next.isValid())
            break;

        if(next.start() < previous.end()) {
            checkFailed = true;
            break;
        }

        previous = next;
    }

    if (checkFailed)
        return tr("<qt>Event duration is potentially longer "
                "than interval between repeats.</qt>");

    return QString();
}

/*!
    \service CalendarService Calendar
    \brief Provides the Qtopia Calendar service.

    The \i Calendar service enables applications to access features of
    the Calendar application.
*/

/*!
    \internal
*/
CalendarService::~CalendarService()
{
}

/*!
    Open a dialog so the user can create a new appointment. Default
    values are used.

    This slot corresponds to the QCop service message
    \c{Calendar::newAppointment()}.
*/
void CalendarService::newAppointment()
{
    datebook->newAppointment("");
}

/*!
    Open a dialog so the user can create a new appointment.  The new
    appointment will use the specified \a start and \a end times,
    \a description and \a notes.

    This slot corresponds to the QCop service message
    \c{Calendar::newAppointment(QDateTime,QDateTime,QString,QString)}.
*/
void CalendarService::newAppointment
            ( const QDateTime& start, const QDateTime& end,
              const QString& description, const QString& notes )
{
    datebook->newAppointment( start, end, description, notes );
}

/*!
    Add the specified \a appointment to the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::addAppointment(QAppointment)}.
*/
void CalendarService::addAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        datebook->model->addAppointment( appointment );
    }
}

/*!
    Update the specified \a appointment in the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::updateAppointment(QAppointment)}.
*/
void CalendarService::updateAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        datebook->model->updateAppointment( appointment );
    }
}

/*!
    Remove the specified \a appointment from the calendar.  The request will
    be ignored if the system is currently syncing.

    This slot corresponds to the QCop service message
    \c{Calendar::removeAppointment(QAppointment)}.
*/
void CalendarService::removeAppointment( const QAppointment& appointment )
{
    if ( !datebook->syncing ) {
        QAppointment a = appointment;
        datebook->removeAppointmentQDLLink( a );
        datebook->model->removeAppointment( appointment );
    }
}

/*!
    Open the calendar user interface and show the appointments for today.

    This slot corresponds to the QCop service message
    \c{Calendar::raiseToday()}.
*/
void CalendarService::raiseToday()
{
    datebook->viewToday();
    QtopiaApplication::instance()->showMainWidget();
}

/*!
    Switch the calendar to the next view.

    This slot corresponds to the QCop service message
    \c{Calendar::nextView()}.
*/
void CalendarService::nextView()
{
    datebook->nextView();
}

/*!
    Show the appointment indicated by \a uid in the calendar application.

    This slot corresponds to the QCop service message
    \c{Calendar::showAppointment(QUniqueId)}.
*/
void CalendarService::showAppointment( const QUniqueId& uid )
{
    QAppointment a = datebook->model->appointment(uid);
    QOccurrence o = a.nextOccurrence(QDate::currentDate());

    if (o.isValid()) {
        datebook->showAppointmentDetails(o);
    }
}

/*!
    Show the occurrence of the appointment indicated by \a uid on \a date.

    This slot corresponds to the QCop service message
    \c{Calendar::showAppointment(QUniqueId,QDate)}.
*/
void CalendarService::showAppointment( const QUniqueId& uid, const QDate& date )
{
    QAppointment a = datebook->model->appointment(uid);
    QOccurrence o = a.nextOccurrence(date);

    if (o.isValid()) {
        datebook->showAppointmentDetails(o);
    }
}

/*!
    Allow the system cleanup wizard to recover some space.
    The \a date to clean from is a hint only. The calendar program
    should only remove appointments and occurrences where doing so
    will save space.

    This slot corresponds to the QCop service message
    \c{Calendar::cleanByDate(QDate)}.
*/
void CalendarService::cleanByDate( const QDate& date )
{
    datebook->removeOccurrencesBefore( date, false );
}

/*!
    Activate the QDL link contained within \a request.

    The slot corresponds to a QDS service with a request data type of
    QDLLink::mimeType() and no response data.

    The slot corresponds to the QCop service message
    \c{Calendar::activateLink(QDSActionRequest)}.
*/
void CalendarService::activateLink( const QDSActionRequest& request )
{
    datebook->qdlActivateLink( request );
}

/*!
    Request for one or more QDL links using the hint contained within
    \a request.

    The slot corresponds to a QDS service with a request data type of
    "text/x-qstring" and response data type of QDLLink::listMimeType().

    The slot corresponds to the QCop service message
    \c{Calendar::requestLinks(QDSActionRequest)}.

*/
void CalendarService::requestLinks( const QDSActionRequest& request )
{
    datebook->qdlRequestLinks( request );
}
