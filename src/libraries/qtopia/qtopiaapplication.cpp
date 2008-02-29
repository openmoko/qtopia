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

#include <qtopialog.h>
#include <QValueSpaceObject>
#include <custom.h>
#include <qtopianamespace.h>

#ifdef Q_WS_QWS
#ifndef QT_NO_SXE
#include <private/qtransportauth_qws_p.h>
#include <qsxepolicy.h>
#endif
#endif

#include <stdlib.h>

#ifdef Q_WS_QWS
#include <time.h>
#endif

#include <qfile.h>
#include <qlist.h>
#include <qqueue.h>
#include <qpainter.h>
#include <qvalidator.h>
#include <qlineedit.h>
#include <qtextedit.h>
#include <QTextBrowser>
#include <qdesktopwidget.h>
#include <qtranslator.h>
#include <qsoundqss_qws.h>
#include <QTextCursor>

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>

#ifdef Q_WS_QWS
#  include <qwindowsystem_qws.h>
#endif

#include <qtextstream.h>
#include <qpalette.h>
#include <qbuffer.h>
#include <qregexp.h>
#include <qdir.h>
#include <qlabel.h>
#include <qdialog.h>
#include <qtextcodec.h>
#include <qevent.h>
#include <qmessagebox.h>
#include <qclipboard.h>
#include <qtimer.h>
#include <qpixmapcache.h>
#include <qmessagebox.h>
#include <qpushbutton.h>
#include <qdebug.h>
#include <QSettings>
#include <qspinbox.h>
#include <qcombobox.h>
#include <QMenu>
#include <QLocale>
#include <QDateEdit>
#include <QCalendarWidget>
#include <QDialogButtonBox>
#include <QLayout>
#include <QResizeEvent>
#include <private/qlocale_p.h>
#include <QDateTimeEdit>

#ifdef Q_WS_QWS
#include <qwsdisplay_qws.h>
#endif

#include <qtopiaapplication.h>
#include <qtopia/private/qtopiaresource_p.h>
#include <qtopia/private/qdateparser_p.h>
#include <qstylefactory.h>
#include <qstorage.h>
#ifdef Q_WS_QWS
#  ifdef QTOPIA_PHONE
#    include <qtopia/qphonestyle.h>
#  else
#    include <qtopia/qpdastyle.h>
#  endif
#  include <qtopia/private/qpedecoration_p.h>
#endif
#include <qtopianamespace.h>
#include <qmimetype.h>
#include "qpluginmanager.h"

#ifdef Q_OS_UNIX
#include <unistd.h>
#include <sys/file.h>
#ifdef Q_WS_QWS
#include <errno.h>
#include <sys/stat.h>
#endif
#endif

#ifndef Q_OS_WIN32
#include <sys/ioctl.h>
#include <sys/soundcard.h>
#else
#include <mmsystem.h>
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif // Q_OS_WIN32

#include <math.h>

#ifdef QTOPIA_KEYPAD_NAVIGATION
#include <qtopia/private/contextkeymanager_p.h>
#include <qsoftmenubar.h>
#endif

#ifdef QTOPIA_KEYPAD_NAVIGATION
bool mousePreferred = false;
#endif

#ifdef QTOPIA_DBUS_IPC
#include <qtdbus/qdbusconnection.h>
#include <qtdbus/qdbuserror.h>
#include <qtdbus/qdbusmessage.h>
#include <qtdbus/qdbusconnectioninterface.h>
#include "dbusipccommon_p.h"
#include "dbusapplicationchannel_p.h"
#endif

#include "ezxphonestyle.h"

#include "qcontent_p.h"

#ifdef QTOPIA_KEYPAD_NAVIGATION
enum QPEWidgetFlagsEnum {
    MenuLikeDialog = 0x01,
} QPEWidgetFlags;

QMap<const QWidget *, int> qpeWidgetFlags;

#endif

// declare QtopiaApplicationLifecycle
/*
  \internal
  \class QtopiaApplicationLifeCycle
  \brief The QtopiaApplicationLifeCycle class controls the lifecycle of a QtopiaApplication based application.

  Applications in Qtopia are launched either directly through executable
  invocation, or indirectly through reception of a message on their QCop
  application channel.

  Application termination is a two stage process.  When an application becomes
  idle and could otherwise terminate, it enters a "lazy shutdown" state.

 */
class QtopiaApplicationLifeCycle : public QObject
{
Q_OBJECT
public:
    QtopiaApplicationLifeCycle(QtopiaApplication * app);

    void registerRunningTask(const QString &, QObject *);
    void unregisterRunningTask(const QString &);

    void reinit();
    bool willKeepRunning() const;

public slots:
    void unregisterRunningTask(QObject *);

signals:
    void quit();

protected:
    virtual bool eventFilter(QObject *obj, QEvent *e);

private slots:
    void doQuit();

private:
    QValueSpaceObject *vso();
    void updateLazyShutdown();
    void recalculateQuit();
    void recalculateInfo();

    bool m_queuedQuit;
    bool m_lazyShutdown;
    QMap<QString, QObject *> m_runningTasks;
    bool m_canQuit;
    bool m_uiActive;
    QtopiaApplication *m_app;
    QValueSpaceObject *m_vso;
    QString m_name;
};

// define QtopiaApplicationLifeCycle
QtopiaApplicationLifeCycle::QtopiaApplicationLifeCycle(QtopiaApplication *app)
: QObject(app), m_queuedQuit(false), m_lazyShutdown(false), m_canQuit(true),
  m_uiActive(false), m_app(app), m_vso(0)
{
    Q_ASSERT(m_app);
    m_app->installEventFilter(this);
    m_name = QCoreApplication::applicationName();

    updateLazyShutdown();
    recalculateInfo();
    recalculateQuit();
}

bool QtopiaApplicationLifeCycle::willKeepRunning() const
{
    return !m_canQuit;
}

void QtopiaApplicationLifeCycle::reinit()
{
    QString newName = QCoreApplication::applicationName();
    if(newName != m_name) {
        m_name = newName;

        // Ahhh.  Name change
        delete m_vso;
        m_vso = 0;

        // Reinitialise the ValueSpace layers
        QValueSpace::reinitValuespace();

        QValueSpaceObject *obj = vso();
        for(QMap<QString, QObject *>::ConstIterator iter = m_runningTasks.begin();
                iter != m_runningTasks.end();
                ++iter)
            obj->setAttribute("Tasks/" + iter.key(), true);

        updateLazyShutdown();
        recalculateQuit();
        recalculateInfo();
    }
}

void QtopiaApplicationLifeCycle::doQuit()
{
    if(!m_canQuit) {
        m_queuedQuit = false;
        return;
    }
    recalculateQuit();
    m_queuedQuit = false;

    if(!m_canQuit)
        return;

    if(!m_lazyShutdown)
        emit quit();
}

bool QtopiaApplicationLifeCycle::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::Show || e->type() == QEvent::Hide) {
        if(obj->isWidgetType() && static_cast<QWidget *>(obj)->isTopLevel()) {
            recalculateQuit();
        }
    }

    return QObject::eventFilter(obj, e);
}

void QtopiaApplicationLifeCycle::updateLazyShutdown()
{
    QSettings cfg(QLatin1String("Trolltech"), QLatin1String("Launcher"));
    cfg.beginGroup(QLatin1String("AppLoading"));
    m_lazyShutdown = cfg.value(QLatin1String("LazyShutdown"), false).toBool();
}

void QtopiaApplicationLifeCycle::recalculateInfo()
{
    vso()->setAttribute("Info/Pid", ::getpid());
    vso()->setAttribute("Info/Name", QtopiaApplication::applicationName());
}

void QtopiaApplicationLifeCycle::recalculateQuit()
{
    bool runningTasks = !m_runningTasks.isEmpty();
    bool uiActive = false;

    QWidgetList widgets = m_app->topLevelWidgets();
    for(int ii = 0; ii < widgets.count() && !uiActive; ++ii) {
        if(!widgets.at(ii)->isHidden())
            uiActive = true;
    }

    if(uiActive != m_uiActive) {
        vso()->setAttribute("Tasks/UI", uiActive);
        m_uiActive = uiActive;
    }
    m_canQuit = !runningTasks && !uiActive;

    if(m_canQuit && !m_queuedQuit) {
        m_queuedQuit = true;
        QTimer::singleShot(0, this, SLOT(doQuit()));
    }
}

void QtopiaApplicationLifeCycle::registerRunningTask(const QString &name,
                                                     QObject *obj)
{
    Q_ASSERT(name != "UI");

    if (m_runningTasks.contains(name)) {
        QObject *obj = m_runningTasks.value(name);
        if (obj)
            QObject::disconnect(obj, SIGNAL(destroyed(QObject *)),
                    this, SLOT(unregisterRunningTask(QObject *)));
        m_runningTasks[name] = obj;
    } else {
        m_runningTasks.insert(name, obj);
        vso()->setAttribute("Tasks/" + name, true);
    }

    if(obj)
        QObject::connect(obj, SIGNAL(destroyed(QObject *)),
                         this, SLOT(unregisterRunningTask(QObject *)));

    recalculateQuit();
}

void QtopiaApplicationLifeCycle::unregisterRunningTask(const QString &name)
{
    QMap<QString, QObject *>::Iterator iter = m_runningTasks.find(name);
    if(iter != m_runningTasks.end()) {
        vso()->removeAttribute("Tasks/" + name);
        m_runningTasks.erase(iter);
    }

    recalculateQuit();
}

void QtopiaApplicationLifeCycle::unregisterRunningTask(QObject *obj)
{
    Q_ASSERT(obj);

    for(QMap<QString, QObject *>::ConstIterator iter = m_runningTasks.begin();
        iter != m_runningTasks.end();
        ++iter) {
        if(obj == (*iter)) {
            unregisterRunningTask(iter.key());
            return;
        }
    }
}

QValueSpaceObject *QtopiaApplicationLifeCycle::vso()
{
    if(!m_vso) {
        m_vso = new QValueSpaceObject("/System/Applications/" + m_name.toLatin1());
        m_vso->setAttribute("Tasks/UI", m_uiActive);
    }

    return m_vso;
}

/*
   Currently only modifies the short date format string and time format string
*/
class QtopiaSystemLocale : public QSystemLocale
{
public:
    QtopiaSystemLocale()
        : QSystemLocale()
    {
        readSettings();
    }
    virtual ~QtopiaSystemLocale() {}

    virtual QVariant query( QueryType type, QVariant in ) const {
        switch ( type ) {
        case DateFormatShort:
            if ( !mDateFormat.isEmpty() )
                return QVariant( mDateFormat );
            break;
        case TimeFormatShort:
            if ( !mTimeFormat.isEmpty() )
                return QVariant( mTimeFormat );
            break;
        default: break;
        }
        return QSystemLocale::query( type, in );
    }

    void readSettings() {
        QSettings config("Trolltech", "qpe");
        config.beginGroup( "Date" );
        QVariant v = config.value("DateFormat");
        if (v.isValid()) {
            mDateFormat = v.toString();
            // need to turn it into the QT format.
            mDateFormat.replace("%Y", "yyyy");
            mDateFormat.replace("%M", "MM");
            mDateFormat.replace("%D", "dd");
        } else {
            mDateFormat.clear();
        }
        config.endGroup();
        config.beginGroup( "Time" );
        v = config.value("AMPM");
        // time format is either 12 or 24 hour time.  We don't show seconds (ss) in Qtopia
        if (v.toBool()) {
            mTimeFormat = "h:mm AP";
        } else {
            mTimeFormat = "H:mm";
        }
    }

private:
    QString mDateFormat;
    QString mTimeFormat;
};

class QSpinBoxLineEditAccessor : public QSpinBox
{
public:
    QLineEdit *getLineEdit() { return lineEdit(); }
};

class CalendarMenu : public QMenu
{
    Q_OBJECT
public:
    CalendarMenu(QWidget *parent=0)
        : QMenu(parent), w(0)
    {
        addAction(QIcon(":icon/month"), tr("Calendar"),
                this, SLOT(showCalendar()));
    }

    void setTargetWidget(QDateEdit *widget) {
        if ( targetWidget )
            targetWidget->removeEventFilter(this);
        targetWidget = widget;
        if ( targetWidget )
            targetWidget->installEventFilter(this);
    }

private slots:
    void showCalendar()
    {
        /* will need a cancel as well */
        if (!w) {
            w = new QCalendarWidget();
            w->setWindowFlags(Qt::Popup);
            w->setHorizontalHeaderFormat(QCalendarWidget::SingleLetterDayNames);
            w->setVerticalHeaderFormat(QCalendarWidget::NoVerticalHeader);
            connect(w, SIGNAL(activated(const QDate &)),
                    this, SLOT(selectDate(const QDate &)));
            QWidget *table = w->findChild<QWidget*>("qt_calendar_calendarview");
            table->installEventFilter(this);
#ifdef QTOPIA_KEYPAD_NAVIGATION
            QSoftMenuBar::setLabel(w, Qt::Key_Context1, QSoftMenuBar::NoLabel);
#endif
        }
        if (targetWidget) {
            w->blockSignals(true);
#if QT_VERSION < 0x040200
            w->setDate(targetWidget->date());
#else
            w->setSelectedDate(targetWidget->date());
#endif
            w->blockSignals(false);
        }
        w->setEditFocus(true);
        w->showMaximized();
    }

    void hideCalendar()
    {
        w->hide();
    }


    void selectDate(const QDate &date)
    {
        if (targetWidget)
            targetWidget->setDate(date);
        hideCalendar();
    }

    bool eventFilter(QObject *o, QEvent *e)
    {
        if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if (o == targetWidget) {
                if (ke->key() == Qt::Key_Select) {
                    if ( e->type() == QEvent::KeyPress )
                        showCalendar();
                    return true;
                }
            } else {
                if (e->type() == QEvent::KeyRelease && ke->key() == Qt::Key_Back) {
                    hideCalendar();
                    ke->accept();
                    return true;
                }
            }
        }
        return QMenu::eventFilter(o,e);
    }

private:
    QCalendarWidget *w;
    QPointer<QDateEdit> targetWidget;
};

class CalendarTextNavigation : public QObject
{
    Q_OBJECT
public:
    CalendarTextNavigation(QObject *parent = 0)
        : QObject(parent), dateText(0), dateFrame(0) { }

    void setTargetWidget(QCalendarWidget *widget) {
        if ( targetWidget ) {
            QWidget *table = targetWidget->findChild<QWidget*>("qt_calendar_calendarview");
            table->removeEventFilter(this);
            if (dateFrame)
                dateFrame->deleteLater();
            dateFrame = 0;
            dateText = 0;
        }
        targetWidget = widget;
        if ( targetWidget ) {
            QWidget *table = targetWidget->findChild<QWidget*>("qt_calendar_calendarview");
            table->installEventFilter(this);
        }
    }

    void showDateLabel()
    {
        if (!targetWidget)
            return;
        dateParser.setInputText(inputText);

        acceptTimer.start(1500, this);

        QString text = dateParser.text();
        // insert formatting info.
        int start = dateParser.highlightStart();
        int end = dateParser.highlightEnd();

        QString fText = "<qt>" + text.left(start)
            + "<b>" + text.mid(start, end - start) + "</b>"
            + text.mid(end);
        dateText->setText(fText);

        QSize s = dateFrame->sizeHint();
        QRect r = targetWidget->geometry(); // later, just the table section
        dateFrame->setGeometry(r.x() + (r.width() - s.width())>>1,
                r.y() + (r.height() - s.height())>>1, s.width(), s.height());
        // need to set palette after geometry update as phonestyle sets transparency
        // effect in move event.
        QPalette p = dateFrame->palette();
        p.setBrush(QPalette::Window, dateFrame->window()->palette().brush(QPalette::Window));
        dateFrame->setPalette(p);
        dateFrame->setAutoFillBackground(true);
        dateFrame->setBackgroundRole(QPalette::Window);
        dateFrame->raise();
        dateFrame->show();
    }

    void hideDateLabel()
    {
        dateFrame->hide();
        if (targetWidget && dateParser.state() != QValidator::Invalid)
            targetWidget->setSelectedDate(dateParser.date());
        inputText.clear();
        acceptTimer.stop();
    }

    bool eventFilter(QObject *o, QEvent *e)
    {
        if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
            QKeyEvent* ke = (QKeyEvent*)e;
            if (ke->text().length() > 0 && ke->text()[0].isPrint()) {
                if (e->type() == QEvent::KeyPress) {
                    if (!dateFrame) {
                        dateFrame = new QFrame(targetWidget);
                        QVBoxLayout *vl = new QVBoxLayout;
                        dateText = new QLabel;
                        vl->addWidget(dateText);
                        dateFrame->setLayout(vl);
                        dateFrame->setFrameShadow(QFrame::Plain);
                        dateFrame->setFrameShape(QFrame::Box);
                    }
                    if (inputText.isEmpty()) {
                        dateParser.setFormat(QLocale::system().dateFormat(QLocale::ShortFormat));
                        dateParser.setDefaultDate(targetWidget->selectedDate());
                    }

                    inputText += ke->text();
                    showDateLabel();
                }
                ke->accept();
                return true;
            } else {
                if (inputText.length() > 0) {
                    if (e->type() == QEvent::KeyRelease) {
                        switch(ke->key()) {
                            case Qt::Key_Back:
                            case Qt::Key_Backspace:
                                inputText = inputText.left(inputText.length()-1);
                                if (inputText.isEmpty()) {
                                    dateFrame->hide();
                                    inputText.clear();
                                    acceptTimer.stop();
                                } else {
                                    showDateLabel();
                                }
                                break;
                            case Qt::Key_Select:
                                hideDateLabel();
                                break;
                        }
                    }
                    ke->accept();
                    return true;
                }
            }
        }
        return QObject::eventFilter(o,e);
    }

    void timerEvent(QTimerEvent *e) {
        if (e->timerId() == acceptTimer.timerId())
            hideDateLabel();
    }

private:
    QLabel *dateText;
    QFrame *dateFrame;
    QBasicTimer acceptTimer;
    QtopiaDateParser dateParser;
    QString inputText;
    QPointer<QCalendarWidget> targetWidget;
};

class ShadowWidget : public QWidget
{
public:
    ShadowWidget(int size);

    int shadowSize() const { return shSize; }
    void setTarget(QWidget *w);

protected:
    void paintEvent(QPaintEvent *e);
    void resizeEvent(QResizeEvent *r);
    bool eventFilter(QObject *o, QEvent *e);

private:
    inline int alphaAt(int dist, int maxDist) const;
    void generateShadow();

private:
    static QMap<int,QBrush*> brushes;
    int shSize;
    QPointer<QWidget> widget;
};

QMap<int,QBrush*> ShadowWidget::brushes;

ShadowWidget::ShadowWidget(int size)
    : QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
        shSize(size)
{
    generateShadow();
    QPalette pal;
    pal.setBrush(QPalette::Background, QColor(0,0,0,0));
    setPalette(pal);
}

void ShadowWidget::setTarget(QWidget *w)
{
    if (widget != w) {
        if (w) {
            setGeometry(w->x()+shadowSize(), w->y()+shadowSize(),
                    w->width(), w->height());
            w->installEventFilter(this);
        } else if (widget) {
            widget->removeEventFilter(this);
        }
        widget = w;
    }
}

bool ShadowWidget::eventFilter(QObject *o, QEvent *e)
{
    if (e->type() == QEvent::Resize && widget && (QWidget*)o == widget) {
        setGeometry(widget->x()+shadowSize(),
                widget->y()+shadowSize(),
                widget->width(), widget->height());
    }

    return false;
}

int ShadowWidget::alphaAt(int dist, int maxDist) const
{
    return qMax(0,95-dist*dist*95/(maxDist*maxDist));
}

void ShadowWidget::generateShadow()
{
    if (brushes.contains(shSize))
        return;

    QBrush *br = new QBrush [5];
    // Right
    {
        QImage img(shSize, 8, QImage::Format_ARGB32_Premultiplied);
        for (int x = 0; x < shSize; x++) {
            int alpha = alphaAt(x, shSize);
            int g = 63*alpha/255;
            img.setPixel(x, 0, qRgba(g,g,g,alpha));
        }
        for (int y = 1; y < img.height(); y++)
            memcpy(img.scanLine(y), img.scanLine(y-1), img.bytesPerLine());
        QPixmap pm = QPixmap::fromImage(img);
        br[1] = QBrush(Qt::black, pm);
    }

    // Bottom
    {
        QImage img(8, shSize, QImage::Format_ARGB32_Premultiplied);
        for (int y = 0; y < shSize; y++) {
            int alpha = alphaAt(y, shSize);
            int g = 63*alpha/255;
            QRgb pix = qRgba(g,g,g,alpha);
            for (int x = 0; x < img.width(); x++) {
                img.setPixel(x, y, pix);
            }
        }
        QPixmap pm = QPixmap::fromImage(img);
        br[3] = QBrush(Qt::black, pm);
    }

    // Radial pixmap for corners
    QImage radImg(shSize*2+1, shSize*2+1, QImage::Format_ARGB32_Premultiplied);
    QPoint c(shSize, shSize);
    for (int y = 0; y < radImg.height(); y++) {
        for (int x = 0; x < radImg.width(); x++) {
            QPoint dp = c - QPoint(x, y);
            int dist = (int)(::sqrt(dp.x()*dp.x() + dp.y()*dp.y()) + 0.5);
//            int dist = dp.manhattanLength();
            int alpha = alphaAt(dist, shSize);
            int g = 63*alpha/255;
            radImg.setPixel(x, y, qRgba(g,g,g,alpha));
        }
    }
    QPixmap radPm = QPixmap::fromImage(radImg);

    // Top-Right
    br[0] = QBrush(Qt::black, radPm.copy(shSize, 1, shSize, shSize));
    // Bottom-Right
    br[2] = QBrush(Qt::black, radPm.copy(shSize, shSize, shSize, shSize));
    // Bottom-Left
    br[4] = QBrush(Qt::black, radPm.copy(1, shSize, shSize, shSize));

    brushes[shSize] = br;
}

void ShadowWidget::paintEvent(QPaintEvent *)
{
    QBrush *br = brushes.value(shSize);

    QPainter p(this);
    // Top-right
    p.setBrushOrigin(width()-shSize, 0);
    p.fillRect(width()-shSize, 0, shSize, shSize, br[0]);
    // Right
    p.setBrushOrigin(width()-shSize, 0);
    p.fillRect(width()-shSize, shSize, shSize, height()-shSize*2, br[1]);
    // Bottom-Right
    p.setBrushOrigin(width()-shSize, height()-shSize);
    p.fillRect(width()-shSize, height()-shSize, shSize, shSize, br[2]);
    // Bottom
    p.setBrushOrigin(0, height()-shSize);
    p.fillRect(shSize, height()-shSize, width()-shSize*2, shSize, br[3]);
    // Bottom-Left
    p.setBrushOrigin(0, height()-shSize);
    p.fillRect(0, height()-shSize, shSize, shSize, br[4]);
}

void ShadowWidget::resizeEvent(QResizeEvent *r)
{
    QRegion rgn(r->size().width()-shSize, 0, shSize, r->size().height());
    rgn |= QRect(0, r->size().height()-shSize, r->size().width(), shSize);
    setMask(rgn);
}

static const int npressticks=10;

class PressTickWidget : public QWidget
{
public:
    PressTickWidget()
        : QWidget(0, Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint),
        pressWidget(0), pressTick(0), rightPressed(false)
    {
        setAttribute(Qt::WA_NoSystemBackground);
        setAutoFillBackground(false);
        QSettings cfg(QLatin1String("Trolltech"),QLatin1String("presstick"));
        cfg.beginGroup(QLatin1String("PressTick"));
        pm = QPixmap(QLatin1String(":image/")+cfg.value(QLatin1String("Image")).toString());
        cfg.endGroup();
        offsets.resize(npressticks);
        tickCount = 0;
        for (int i = 0; i < npressticks; i++) {
            cfg.beginGroup(QLatin1String("Tick")+QString::number(i));
            if (cfg.contains("Dx") || cfg.contains("Dy")) {
                int dx = cfg.value(QLatin1String("Dx")).toInt();
                int dy = cfg.value(QLatin1String("Dy")).toInt();
                offsets[i] = QPoint(dx, dy);
                bound |= QRect(offsets[i], pm.size());
                tickCount++;
            }
            cfg.endGroup();
        }
    }

    void startPress(QWidget *w, const QPoint &pos)
    {
        pressTick = npressticks;
        rightPressed = false;
        pressWidget = w;
        pressPos = pos;
        pressTimer.start(500/pressTick, this); // #### pref.
        QPoint gpos = pressWidget->mapToGlobal(pos);
        setGeometry(gpos.x()+bound.x(), gpos.y()+bound.y(), bound.width(), bound.height());
    }

    void cancelPress()
    {
        pressTimer.stop();
        pressWidget = 0;
        hide();
    }

    bool endPress(const QPoint &pos)
    {
        pressTimer.stop();
        hide();
        if (rightPressed && pressWidget ) {
            // Right released
            QApplication::postEvent(pressWidget,
                new QMouseEvent(QEvent::MouseButtonRelease, pos,
                        pressWidget->mapToGlobal(pos),
                        Qt::RightButton, Qt::LeftButton|Qt::RightButton, 0));
            // Left released, off-widget
            QApplication::postEvent(pressWidget,
                new QMouseEvent(QEvent::MouseMove, QPoint(-1,-1),
                        Qt::LeftButton, Qt::LeftButton, 0 ) );
            QApplication::postEvent(pressWidget,
                new QMouseEvent(QEvent::MouseButtonRelease, QPoint(-1,-1),
                        Qt::LeftButton, Qt::LeftButton, 0 ) );
            rightPressed = false;
            pressWidget = 0;
            return true; // don't send the real Left release
        }
        pressWidget = 0;

        return false;
    }

    bool active() const { return pressWidget; }
    const QPoint &pos() const { return pressPos; }
    QWidget *widget() const { return pressWidget; }

protected:
    void paintEvent(QPaintEvent *e)
    {
        Q_UNUSED(e);

        if (pressTick < tickCount) {
            int dx = offsets[pressTick].x();
            int dy = offsets[pressTick].y();
            QPainter p(this);
            if (pressTick == tickCount-1) {
                p.setCompositionMode(QPainter::CompositionMode_Clear);
                p.eraseRect(rect());
                p.setCompositionMode(QPainter::CompositionMode_Source);
            }
            p.drawPixmap(bound.width()/2+dx, bound.height()/2+dy,pm);
        }
    }

    void timerEvent(QTimerEvent *e)
    {
        if (e->timerId() == pressTimer.timerId() && pressWidget) {
            if (pressTick) {
                pressTick--;
                if (pressTick < tickCount) {
                    show();
                    update();
                }
            } else {
                // Right pressed
                pressTimer.stop();
                hide();
                QApplication::postEvent(pressWidget,
                        new QMouseEvent(QEvent::MouseButtonPress, pressPos,
                            pressWidget->mapToGlobal(pressPos),
                            Qt::RightButton, Qt::LeftButton, 0 ) );
                rightPressed = true;
            }
        }
    }

private:
    QWidget *pressWidget;
    QPoint pressPos;
    QVector<QPoint> offsets;
    QRect bound;
    QPixmap pm;
    int tickCount;
    int pressTick;
    bool rightPressed;
    QBasicTimer pressTimer;
};

class QtopiaApplicationData {
public:
    QtopiaApplicationData() : pressHandler(0),
        kbgrabber(0), kbregrab(false), notbusysent(false),
        preloaded(false), nomaximize(false), noshow(false),
        qpe_main_widget(0), skiptimechanged(0), qpe_system_locale(0),
        lifeCycle(0)
#ifdef QTOPIA_DBUS_IPC
        , m_appChannel(0)
#endif
#ifndef QTOPIA_DBUS_IPC
                , qcopQok(false)
#endif
#ifdef QTOPIA_KEYPAD_NAVIGATION
        , editMenu(0)
        , calendarMenu(0)
#endif
        , sysChannel(0), pidChannel(0)
    {
    }

    PressTickWidget *pressHandler;
    int kbgrabber;
    bool kbregrab;
    bool notbusysent;
    QString appName;
    QString appFullPath;
    struct QCopRec {
        QCopRec(const QString &ch, const QString &msg,
                               const QByteArray &d) :
            channel(ch), message(msg), data(d)
            {
                channel.detach();
                message.detach();
                data.detach();
            }

        QString channel;
        QString message;
        QByteArray data;
    };

    bool preloaded;
    bool nomaximize;
    bool noshow;

    QWidget* qpe_main_widget;
    QPointer<QWidget> lastraised;
    int skiptimechanged;
    QtopiaSystemLocale *qpe_system_locale;
    QtopiaApplicationLifeCycle *lifeCycle;

#ifdef QTOPIA_DBUS_IPC
    DBUSQtopiaApplicationChannel *m_appChannel;
#endif

#ifndef QTOPIA_DBUS_IPC
    bool qcopQok;
    QQueue<QCopRec*> qcopq;

    void enqueueQCop(const QString &ch, const QString &msg,
                               const QByteArray &data)
    {
        qcopq.enqueue(new QCopRec(ch,msg,data));
    }

    void sendQCopQ()
    {
        if ( qcopQok ) {
            QCopRec* r;
            while( !qcopq.isEmpty() ) {
                r = qcopq.dequeue();
                // remove from queue before sending...
                // event loop can come around again before getting
                // back from the send.
                QtopiaApplication *app = qobject_cast<QtopiaApplication *>(qApp);
                if ( app )
                    app->pidMessage( r->message, r->data );
                delete r;
            }
        }
    }
#endif

    static void qpe_show_dialog( QDialog* d, bool nomax )
    {
        if( QString(d->objectName()) == "__nomove" ) // hack, don't do anything if the dialog
                                                        // specifically requests it through this mechanism
            return;

        QSize sh = d->sizeHint();

#ifdef QTOPIA_PDA
        if ( d->parentWidget() && !d->parentWidget()->topLevelWidget()->isMaximized() )
            nomax = true;
#endif

        QDesktopWidget *desktop = QApplication::desktop();
        int screen = desktop->screenNumber(d);
        if (d->parentWidget())
            screen = desktop->screenNumber(d->parentWidget());
        QRect desktopRect(desktop->availableGeometry(screen));
        QRect fg = d->frameGeometry();
        QRect cg = d->geometry();
        int frameWidth = fg.width() - cg.width();
        int maxY = desktopRect.height() - (fg.height() - cg.height());
        int h = sh.height();
        h = qMax(h,d->heightForWidth(desktopRect.width()-frameWidth));

#ifdef QTOPIA_KEYPAD_NAVIGATION
        if ( h >= maxY || ((!nomax) && (h > desktopRect.height()*3/5)) || (d->windowFlags()&Qt::WindowStaysOnTopHint)) {
            if (desktop->screenNumber(d) != screen)
                d->setGeometry(desktopRect); // appear on the correct screen
            d->showMaximized();
        } else {
            d->show();
            fg = d->frameGeometry();
            cg = d->geometry();
            int lb = cg.left()-fg.left()+desktopRect.left();
            int bb = fg.bottom() - cg.bottom();
            d->setFixedSize(desktopRect.width() - frameWidth, h);
            d->setGeometry(lb, desktopRect.bottom() - h - bb + 1,
                    desktopRect.width() - frameWidth, h);
        }
//        d->raise();
//        d->activateWindow();
#else
        int w = qMax(sh.width(),d->width());
        int maxX = desktopRect.width() - frameWidth;

        if ( (w >= maxX && h >= maxY) || ( (!nomax) && ( w > desktopRect.width()*3/4 || h > desktopRect.height()*3/4 ) ) ) {
            d->showMaximized();
        } else {
            // try centering the dialog around its parent
            QPoint p(0,0);
            if ( d->parentWidget() ) {
                QPoint pp = d->parentWidget()->mapToGlobal( QPoint(0,0) );
                p = QPoint( pp.x() + d->parentWidget()->width()/2,
                        pp.y() + d->parentWidget()->height()/ 2 );
            } else {
                p = QPoint( maxX/2, maxY/2 );
            }

            p = QPoint( p.x() - w/2, p.y() - h/2 );

            if ( w >= maxX ) {
                if ( p.y() < 0 )
                    p.setY(0);
                if ( p.y() + h > maxY )
                    p.setY( maxY - h);

                d->resize(maxX, h);
                d->move(0, p.y() );
            } else if ( h >= maxY ) {
                if ( p.x() < 0 )
                    p.setX(0);
                if ( p.x() + w > maxX )
                    p.setX( maxX - w);

                d->resize(w, maxY);
                d->move(p.x(),0);
            } else {
                d->resize(w, h);
            }

            d->show();
        }
#endif
    }

    static void show_mx(QWidget* mw, bool nomaximize, QString &strName)
    {
        Q_UNUSED( strName );

        if ( mw->isVisible() ) {
            mw->raise();
            mw->activateWindow();
        } else {
            QDialog *dialog = qobject_cast<QDialog*>(mw);
#ifdef QTOPIA_PHONE
            if (dialog) {
                qpe_show_dialog(dialog,nomaximize);
            } else {
                if ( !nomaximize )
                    mw->showMaximized();
                else
                    mw->show();
            }
#else
            if (dialog) {
                bool max;
                QPoint p;
                QSize s;
                if ( read_widget_rect(strName, max, p, s) && validate_widget_size(mw, p, s) ) {
                    mw->resize(s);
                    mw->move(p);

                    if ( max && !nomaximize )
                        mw->showMaximized();
                    else
                        mw->show();
                } else
                    qpe_show_dialog(dialog,nomaximize);
            } else {
                bool max;
                QPoint p;
                QSize s;
                if ( read_widget_rect(strName, max, p, s) && validate_widget_size(mw, p, s) ) {
                    mw->resize(s);
                    mw->move(p);
                } else {    //no stored rectangle, make an estimation
                    QDesktopWidget *desktop = QApplication::desktop();
                    QRect desktopRect(desktop->availableGeometry(desktop->primaryScreen()));
                    int x = (desktopRect.width()-mw->frameGeometry().width())/2;
                    int y = (desktopRect.height()-mw->frameGeometry().height())/2;
                    mw->move( qMax(x,0), qMax(y,0) );
                    if ( !nomaximize )
                        mw->showMaximized();
                }
                if ( max && !nomaximize )
                    mw->showMaximized();
                else
                    mw->show();
            }
#endif
        }
    }

    static bool read_widget_rect(const QString &app, bool &maximized, QPoint &p, QSize &s)
    {
        maximized = true;

        // 350 is the trigger in qwsdefaultdecoration for providing a resize button
        QDesktopWidget *desktop = QApplication::desktop();
        if (desktop->screenGeometry(desktop->primaryScreen()).width() <= 350)
            return false;

        QSettings cfg(QLatin1String("Trolltech"),QLatin1String("qpe"));
        cfg.beginGroup(QLatin1String("ApplicationPositions"));
        QString str = cfg.value( app, QString() ).toString();
        QStringList l = str.split(QLatin1String(","));

        if ( l.count() == 5) {
            p.setX( l[0].toInt() );
            p.setY( l[1].toInt() );

            s.setWidth( l[2].toInt() );
            s.setHeight( l[3].toInt() );

            maximized = l[4].toInt();

            return true;
        }

        return false;
    }

    static bool validate_widget_size(const QWidget *w, QPoint &p, QSize &s)
    {
        QDesktopWidget *desktop = QApplication::desktop();
        QRect desktopRect(desktop->availableGeometry(desktop->primaryScreen()));
        int maxX = desktopRect.width();
        int maxY = desktopRect.height();
        int wWidth = s.width() + ( w->frameGeometry().width() - w->geometry().width() );
        int wHeight = s.height() + ( w->frameGeometry().height() - w->geometry().height() );

        // total window size is not allowed to be larger than desktop window size
        if ( ( wWidth >= maxX ) && ( wHeight >= maxY ) )
            return false;

        if ( wWidth > maxX ) {
            s.setWidth( maxX - (w->frameGeometry().width() - w->geometry().width() ) );
            wWidth = maxX;
        }

        if ( wHeight > maxY ) {
            s.setHeight( maxY - (w->frameGeometry().height() - w->geometry().height() ) );
            wHeight = maxY;
        }

        // any smaller than this and the maximize/close/help buttons will be overlapping
        if ( wWidth < 80 || wHeight < 60 )
            return false;

        if ( p.x() < 0 )
            p.setX(0);
        if ( p.y() < 0 )
            p.setY(0);

        if ( p.x() + wWidth > maxX )
            p.setX( maxX - wWidth );
        if ( p.y() + wHeight > maxY )
            p.setY( maxY - wHeight );

        return true;
    }

    static void store_widget_rect(QWidget *w, QString &app)
    {
        // 350 is the trigger in qwsdefaultdecoration for providing a resize button
        QDesktopWidget *desktop = QApplication::desktop();
        if (desktop->screenGeometry(desktop->primaryScreen()).width() <= 350 )
            return;

        // we use these to map the offset of geometry and pos.  ( we can only use normalGeometry to
        // get the non-maximized version, so we have to do it the hard way )
        int offsetX = w->x() - w->geometry().left();
        int offsetY = w->y() - w->geometry().top();

        QRect r;
        if ( w->isMaximized() )
            r = w->normalGeometry();
        else
            r = w->geometry();

        // Stores the window placement as pos(), size()  (due to the offset mapping)
        QSettings cfg(QLatin1String("Trolltech"),QLatin1String("qpe"));
        cfg.beginGroup(QLatin1String("ApplicationPositions"));
        QString s;
        s.sprintf("%d,%d,%d,%d,%d", r.left() + offsetX, r.top() + offsetY, r.width(), r.height(), w->isMaximized() );
        cfg.setValue( app, s );
    }

    static bool setWidgetCaptionFromAppName( QWidget* /*mw*/, const QString& /*appName*/, const QString& /*appsPath*/ )
    {
    /*
        // This works but disable it for now until it is safe to apply
        // What is does is scan the .desktop files of all the apps for
        // the applnk that has the corresponding argv[0] as this program
        // then it uses the name stored in the .desktop file as the caption
        // for the main widget. This saves duplicating translations for
        // the app name in the program and in the .desktop files.

        AppLnkSet apps( appsPath );

        QList<AppLnk> appsList = apps.children();
        for ( QListIterator<AppLnk> it(appsList); it.current(); ++it ) {
            if ( (*it)->exec() == appName ) {
                mw->setIcon( (*it)->pixmap() );
                mw->setCaption( (*it)->name() );
                return true;
            }
        }
    */
        return false;
    }

    void show(QWidget* mw, bool nomax)
    {
        setWidgetCaptionFromAppName( mw, appName, Qtopia::qtopiaDir() + QLatin1String("apps") );
        nomaximize = nomax;
        qpe_main_widget = mw;

#ifndef QTOPIA_DBUS_IPC
        qcopQok = true;
        sendQCopQ();
#endif

        show_mx(mw, nomax, appName);
    }

#ifdef QTOPIA_KEYPAD_NAVIGATION
    static void updateContext(QWidget *w)
    {
        if (!w->hasEditFocus())
            return;
        QLineEdit *l = qobject_cast<QLineEdit*>(w);
        if (!l && w->inherits("QSpinBox"))
            l = ((QSpinBoxLineEditAccessor*)w)->getLineEdit();
        if (!l && w->inherits("QComboBox"))
            l = ((QComboBox*)w)->lineEdit();
        if (l) {
            if (l->text().length() == 0 || l->isReadOnly())
                ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::RevertEdit);
            else if (l->cursorPosition() == 0)
                ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::NoLabel);
            else
                ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::BackSpace);
        } else if (w->inherits("QTextEdit") && !w->inherits("QTextBrowser")) {
            QTextEdit *l = (QTextEdit*)w;
            if (l->document()->isEmpty() || l->isReadOnly()) {
                ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::RevertEdit);
            } else {
                if (l->textCursor().position() == 0)
                    ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::NoLabel);
                else
                    ContextKeyManager::instance()->setStandard(w, Qt::Key_Back, QSoftMenuBar::BackSpace);
            }
        }
    }
#endif

#ifdef QTOPIA_KEYPAD_NAVIGATION
    static void updateButtonSoftKeys(QWidget *w)
    {
        QAbstractButton *b = qobject_cast<QAbstractButton*>(w);
        if (b && b->isCheckable()) {
            if (w->inherits("QCheckBox")) {
                if (b->isChecked())
                    ContextKeyManager::instance()->setStandard(w, Qt::Key_Select, QSoftMenuBar::Deselect);
                else
                    ContextKeyManager::instance()->setStandard(w, Qt::Key_Select, QSoftMenuBar::Select);
            }
        }
    }

    static void updateBrowserSoftKeys(QWidget *w)
    {
        QTextBrowser *tb = qobject_cast<QTextBrowser*>(w);
        if (!tb || ContextKeyManager::instance()->haveCustomLabelForWidget(w, Qt::Key_Select, w->hasEditFocus()))
            return;
        if (tb->textCursor().hasSelection() &&
            !tb->textCursor().charFormat().anchorHref().isEmpty()) {
            ContextKeyManager::instance()->setStandard(w, Qt::Key_Select, QSoftMenuBar::Select);
        } else {
            ContextKeyManager::instance()->setStandard(w, Qt::Key_Select, QSoftMenuBar::NoLabel);
        }
    }
#endif

#ifdef Q_WS_QWS
    const char *appKey;

    QString styleName;
#endif
    QString decorationName;
    QString decorationTheme;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    QPointer<QMenu> editMenu;
    QPointer<CalendarMenu> calendarMenu;
    QPointer<CalendarTextNavigation> calendarNav;
    QBasicTimer singleFocusTimer;
# ifdef QTOPIA_ENABLE_FADE_IN_WINDOW
    QBasicTimer fadeInTimer;
    double fadeInOpacity;
    QPointer<QWidget> fadeInWidget;
# endif
#endif
    QFileResourceFileEngineHandler *fileengine;
    QMap<QWidget*,ShadowWidget*> shadowMap;

    QtopiaChannel *sysChannel;
    QtopiaChannel *pidChannel;
};

int qtopia_muted=0;

static void setVolume(int t=0, int percent=-1)
{
    Q_UNUSED(t);
    Q_UNUSED(percent);
#if 0
    switch (t) {
        case 0: {
            QSettings cfg(QLatin1String("Trolltech"),QLatin1String("Sound"));
            cfg.beginGroup(QLatin1String("System"));
            if ( percent < 0 ) {
                percent = cfg.value(QLatin1String("Volume"),50).toInt();
                qtopia_muted = cfg.value(QLatin1String("Muted"),false).toBool();
            }
            int fd = 0;
#ifndef Q_OS_WIN32
            if ((fd = open("/dev/mixer", O_RDWR))>=0) { // Some devices require this, O_RDONLY doesn't always work
                int vol = qtopia_muted ? 0 : percent;
                // set both channels to same volume
                vol |= vol << 8;
                ioctl(fd, MIXER_WRITE(0), &vol);
                ::close(fd);
            }
#else
            HWAVEOUT handle;
            WAVEFORMATEX formatData;
            formatData.cbSize = sizeof(WAVEFORMATEX);
            formatData.wFormatTag = WAVE_FORMAT_PCM;
            formatData.nAvgBytesPerSec = 4 * 44000;
            formatData.nBlockAlign = 4;
            formatData.nChannels = 2;
            formatData.nSamplesPerSec = 44000;
            formatData.wBitsPerSample = 16;
            waveOutOpen(&handle, WAVE_MAPPER, &formatData, 0L, 0L, CALLBACK_NULL);
            int vol = qtopia_muted ? 0 : percent;
            unsigned int volume = (vol << 24) | (vol << 8);
            waveOutSetVolume( handle, volume );
            waveOutClose( handle );
#endif
        } break;
    }
#endif
}


/*!
  \class QtopiaApplication
  \brief The QtopiaApplication class implements the system services
   available to all Qtopia applications.

  By using QtopiaApplication instead of QApplication, a standard Qt
  application becomes a Qtopia application. It automatically follows
  style changes, quits and raises, and in the
  case of \l {Qtopia - Main Document Widget}{document-oriented} applications,
  changes the currently displayed document in response to the environment.

  To create a \l {Qtopia - Main Document Widget}{document-oriented}
  application use showMainDocumentWidget(); to create a
  non-document-oriented application use showMainWidget().

  A variety of signals are emitted when certain events occur, for
  example:
  \list
  \o timeChanged()
  \o clockChanged()
  \o weekChanged(),
  \o dateFormatChanged()
  \o volumeChanged()
  \endlist
  The following signals are emitted:
  \list
  \o appMessage() - if the application receives a \l {Qtopia IPC Layer}{Qtopia IPC} message on the
  QPE/Application/\i{appname} channel.
  \o flush() - when synching begins
  \o reload() - when syching ends.
  \endlist

  When synching begins or ends the application should save and reload any data files involved in synching.
  Most of these signals will intially be received and unfiltered through the appMessage() signal.

  This class also provides a set of useful static functions as follows:
  \list
  \o grabKeyboard() and ungrabKeyboard() - determine if the application takes control of the physical device buttons.
     For example, application launch keys.
  \o setStylusOperation() - sets the mode of operation of the stylus
  \o stylusOperation() - retrieve the mode of operation of the stylus
  \o qpeDir() - returns the qpeDir path
  \o documentDir() - returns the documentDir path
  \o setStylusOperation() - sets the mode of operation of the stylus
  \o stylusOperation() - retrieves the mode of operation of the stylus
  \o setInputMethodHint() - sets the input method hint
  \o inputMethodHint() - retrieves the input method hint.
 \endlist

  \ingroup environment
*/

/*!
  \fn void QtopiaApplication::clientMoused()

  \internal
*/

/*!
  \fn void QtopiaApplication::timeChanged();

  This signal is emitted when the time changes outside the normal
  passage of time, that is, if the time is set backwards or forwards.

  If the application offers the TimeMonitor service, it will get
  the QCop message that causes this signal even if it is not running,
  thus allowing it to update any alarms or other time-related records.
*/

/*!
  \fn void QtopiaApplication::categoriesChanged();

  This signal is emitted whenever a category is added, removed or edited.
*/

/*!
  \fn void QtopiaApplication::contentChanged(const QContentIdList &ids, QContent::ChangeType type)

  This signal is emitted whenever one or more QContent is stored, removed or edited.
  \a ids contains the list of Id's of the content that is being modified.
  \a type contains the type of change.
*/

/*!
  \fn void QtopiaApplication::resetContent()

  This signal is emitted whenever a new media database is attached, or the system needs to generallt reset
  the QContentSets to refresh their list of items.
 */

/*!
  \fn void QtopiaApplication::clockChanged( bool ampm );

  This signal is emitted when the clock style is changed. If
  \a ampm is true, the clock style is a 12-hour AM/PM clock, otherwise,
  it is a 24-hour clock.

  \warning When using the QTimeString functions all strings obtained by QTimeString
  should be updated to reflect the changes.

  \sa dateFormatChanged()
*/

/*!
    \fn void QtopiaApplication::volumeChanged( bool muted )

    This signal is emitted whenever the mute state is changed. If \a
    muted is true, then sound output has been muted.
*/

/*!
    \fn void QtopiaApplication::weekChanged( bool startOnMonday )

    This signal is emitted if the week start day is changed. If \a
    startOnMonday is true then the first day of the week is Monday otherwise
    the first day of the week is Sunday.
*/

/*!
    \fn void QtopiaApplication::dateFormatChanged()

    This signal is emitted whenever the date format is changed.

    \warning When using QTimeString functions, all QTimeString strings should be updated
    to reflect the changes.

    \sa clockChanged()
*/

/*!
    \fn void QtopiaApplication::flush()

    \internal
*/

/*!
    \fn void QtopiaApplication::reload()

    \internal
*/

/*!
  \fn void QtopiaApplication::appMessage( const QString& msg, const QByteArray& data )

  This signal is emitted when a message is received on the
  application's QPE/Application/\i{appname}  \l {Qtopia IPC Layer}{Qtopia} channel.

  The slot to which you connect this signal uses \a msg and \a data
  in the following way:

\code
    void MyWidget::receive( const QString& msg, const QByteArray& data )
    {
        QDataStream stream( data, QIODevice::ReadOnly );
        if ( msg == "someMessage(int,int,int)" ) {
            int a,b,c;
            stream >> a >> b >> c;
            ...
        } else if ( msg == "otherMessage(QString)" ) {
            ...
        }
    }
\endcode

   Note: Messages received here may be processed by QtopiaApplication
   and emitted as signals, such as flush() and reload().
*/

/*!
    Determine if this application will keep running because there are
    widgets or tasks still being used.
*/
bool QtopiaApplication::willKeepRunning() const
{
    if(type() == GuiServer) return true;
    Q_ASSERT(d->lifeCycle);

#ifndef QTOPIA_DBUS_IPC
    d->qcopQok = true;
    d->sendQCopQ();
#endif

    return d->lifeCycle->willKeepRunning();
}

/*!
  Register the task \a name as running.  If \a taskObj is supplied, the task
  will be automatically unregistered when \a taskObj is destroyed.  Tasks may
  always be manually unregistered by calling unregisterRunningTask().  For a
  broader discussion of tasks, please refer to the QtopiaApplication overview
  documentation.

  It is illegal to attempt to register two tasks with the same \a name
  simultaneously.  Attempting to do so will cause the application to assert in
  debuging mode and misbehave in release mode.

  Certain task names are reserved for use by the system and should not be used
  directly by application programmers.  The following table describes the list
  of reserved task names.

  \table
  \header \o name \o Description
  \row \o \c {UI} \o An implied system task that is "running" whenever the application has visible UI.  Visible UI is defined as toplevel widgets that are not hidden (as determined by QWidget::isHidden()), although they may be obscured by other windows.
  \row \o \c {QtopiaPreload} \o A task used by the system to keep preloaded applications running indefinately.
  \row \o \c {Qtopia*} \o All task names begining with "Qtopia" are reserved for future use.
  \endtable
  */
void QtopiaApplication::registerRunningTask(const QString &name,
                                            QObject *taskObj)
{
    if(type() == GuiServer) return; // Server doesn't support shutdown like this
    Q_ASSERT(d->lifeCycle);

    d->lifeCycle->registerRunningTask(name, taskObj);
}

/*!
  Unregister the task \a name previously registered with registerRunningTask().
  Attempting to register a task that is not running has no effect.
 */
void QtopiaApplication::unregisterRunningTask(const QString &name)
{
    if(type() == GuiServer) return; // Server doesn't support shutdown like this
    Q_ASSERT(d->lifeCycle);

    d->lifeCycle->unregisterRunningTask(name);
}

/*!
  Unregister the task \a taskObj previously registered with
  registerRunningTask().  Attempting to register a task that is not running has
  no effect.
 */
void QtopiaApplication::unregisterRunningTask(QObject *taskObj)
{
    if(type() == GuiServer) return; // Server doesn't support shutdown like this
    Q_ASSERT(d->lifeCycle);

    d->lifeCycle->unregisterRunningTask(taskObj);
}

#ifndef QTOPIA_DBUS_IPC
void QtopiaApplication::processQCopFile()
{
    QString qcopfn = d->appName;
#ifndef Q_OS_UNIX
    if (QApplication::winVersion() == Qt::WV_98)
        qcopfn = qcopfn.lower(); // append command name
#endif
    qcopfn.prepend( Qtopia::tempDir() + "qcop-msg-" );
    QFile qcopfile(qcopfn);

    if ( qcopfile.open(QIODevice::ReadWrite) ) {
#ifndef Q_OS_WIN32
#ifdef QTOPIA_POSIX_LOCKS
        struct flock fl;
        fl.l_type = F_WRLCK;
        fl.l_whence = SEEK_SET;
        fl.l_start = 0;
        fl.l_len = 0;
        fl.l_pid = getpid();
        if (fcntl(qcopfile.handle(), F_SETLKW, &fl) == -1) {
            qWarning("Could not acquire lock: %d %s", errno, strerror(errno));
        }
#else
        if (flock(qcopfile.handle(), LOCK_EX) == -1) {
            qWarning("Could not acquire lock: %d %s", errno, strerror(errno));
        }
#endif
#endif
        QDataStream ds( &qcopfile );
        QString channel, message;
        QByteArray data;
        while(!ds.atEnd()) {
            ds >> channel >> message >> data;
            d->enqueueQCop(channel,message,data);
        }
        Qtopia::truncateFile(qcopfile, 0);
#ifndef Q_OS_WIN32
        qcopfile.flush();
#ifdef QTOPIA_POSIX_LOCKS
        fl.l_type = F_UNLCK;
        if (fcntl(qcopfile.handle(), F_SETLK, &fl) == -1) {
            qWarning("Could not release lock: %d %s", errno, strerror(errno));
        }
#else
        if (flock(qcopfile.handle(), LOCK_UN) == -1) {
            qWarning("Could not release lock: %d %s", errno, strerror(errno));
        }
#endif
#endif
    }
}
#endif

/*!
    Loads the translation file specified by \a qms. \a qms can be the name
    of a library or application.
*/
#ifndef QT_NO_TRANSLATION
void QtopiaApplication::loadTranslations( const QString& qms )
{
    loadTranslations(QStringList(qms));
}


/*!
  This is an overloaded member function, provided for convenience.
  It behaves essentially like the above function.

  It is the same as calling \c{loadTranslations(QString)} for each entry of \a qms.
*/
void QtopiaApplication::loadTranslations( const QStringList& qms )
{
    QStringList qmList( qms );
    QStringList langs = Qtopia::languageList();

    QStringList qpepaths = Qtopia::installPaths();
    for (QStringList::ConstIterator qit = qpepaths.begin(); qit!=qpepaths.end(); ++qit) {
        for (QStringList::ConstIterator it = langs.begin(); it!=langs.end(); ++it) {
            QString lang = *it;

            //Optimisation: Assumption the source is en_US => don't load translations
            if ( lang == QLatin1String("en_US")){
                qLog(I18n) << "Loading of en_US requested, skipping loading of translation files";
                return;
            }
            QTranslator * trans;
            QString tfn;
            QMutableStringListIterator qmit( qmList );
            while ( qmit.hasNext() ) {
                trans = new QTranslator(qApp);
                tfn = *qit + QLatin1String("i18n/") + lang + QLatin1String("/") + qmit.next() + QLatin1String(".qm");
                qLog(I18n) << "Loading" << tfn;
                if ( trans->load( tfn )) {
                    qApp->installTranslator( trans );
                    qmit.remove();
                }
                else {
                    qLog(I18n) << "Cannot load " << tfn;
                    delete trans;
                }
            }
        }
    }
}
#endif

#ifdef QTOPIA4_TODO
#if defined (Q_WS_QWS)
extern bool qt_lineedit_password_visible_on_focus;
#endif
#endif

/*!
  Returns a pointer to the application's QtopiaApplication instance.
 */
QtopiaApplication *QtopiaApplication::instance()
{
    static bool check = false;
    static QtopiaApplication *instance = 0;

    if(!check) {
        if(qApp) {
            instance = qobject_cast<QtopiaApplication *>(qApp);
            check = true;
        }
    }

    return instance;
}

/*!
  Constructs a QtopiaApplication just as you would construct
  a QApplication, passing \a argc, \a argv, and \a t.

  For applications, \a t should be the default, GuiClient. Only
  the Qtopia server passes GuiServer.
*/
QtopiaApplication::QtopiaApplication( int& argc, char **argv, Type t )
    : QApplication( argc, argv, t )
{
#ifdef QTOPIA4_TODO
#if defined (Q_WS_QWS)
    // pdas and phones have uncertain input, so as long as line edit
    // has focus, should be able to see password text.
    qt_lineedit_password_visible_on_focus = true;
#endif
#endif

#ifdef Q_WS_QWS
#ifndef QT_NO_SXE
    {
        QTransportAuth *a = QTransportAuth::getInstance();
        SXEPolicyManager *p = SXEPolicyManager::getInstance();
        a->registerPolicyReceiver( p );
        a->setKeyFilePath( Qtopia::qtopiaDir() + QLatin1String("etc/") + QSXE_KEYFILE );
        a->setLogFilePath( Qtopia::tempDir() + QLatin1String("sxe_discovery.log") );
    }
#endif
#endif

    d = new QtopiaApplicationData;

    setFont( QFont( QLatin1String("helvetica"), 9, QFont::Normal ) );

    d->fileengine = new QFileResourceFileEngineHandler();

    applyStyle();

    QApplication::setQuitOnLastWindowClosed(false);

#if defined(Q_WS_QWS) && defined(Q_OS_UNIX)
    QString dataDir(Qtopia::tempDir());
    if ( mkdir( dataDir.toLatin1(), 0700 ) ) {
        if ( errno != EEXIST ) {
            qFatal( QString("Cannot create Qtopia data directory: %1")
                    .arg( dataDir ).toLatin1().constData() );
        }
    }

    struct stat buf;
    if ( lstat( dataDir.toLatin1(), &buf ) )
        qFatal( QString( "stat failed for Qtopia data directory: %1" )
                .arg( dataDir ).toLatin1().constData() );

    if ( !S_ISDIR( buf.st_mode ) )
        qFatal( QString( "%1 is not a directory" ).arg( dataDir ).toLatin1().constData() );

    if ( buf.st_uid != getuid() )
        qFatal( QString( "Qtopia data directory is not owned by user %1: %2" )
                .arg( getuid() ).arg( dataDir ).toLatin1().constData() );

    if ( (buf.st_mode & 0677) != 0600 )
        qFatal( QString( "Qtopia data directory has incorrect permissions: %1" )
                .arg( dataDir ).toLatin1().constData() );
#endif

#ifdef QTOPIA_DBUS_IPC
    // Connect to the session bus as early as possible
    QDBusConnection dbc = QDBus::sessionBus();
    if (!dbc.isConnected()) {
        qFatal( QString( "Unable to connect do D-BUS: %1")
                .arg( dbc.lastError().message()).toLatin1().constData() );
    }
#endif

#ifdef QTOPIA_KEYPAD_NAVIGATION
    QSettings config(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    config.beginGroup( QLatin1String("Device") );
    QString pi = config.value( QLatin1String("PrimaryInput"), QLatin1String("Keypad") ).toString().toLower();
    // anything other than touchscreen means keypad modal editing gets enabled
    bool keypadNavigation = pi != QLatin1String("touchscreen");
    QApplication::setKeypadNavigationEnabled(keypadNavigation);
    mousePreferred = !keypadNavigation;
#endif

    QPixmapCache::setCacheLimit(256);  // sensible default for smaller devices.

#ifdef QTOPIA4_TODO
    QMimeSourceFactory::setDefaultFactory(new ResourceMimeFactory);
#endif

    d->sysChannel = new QtopiaChannel( QLatin1String("QPE/System"), this );
    connect( d->sysChannel, SIGNAL(received(const QString&,const QByteArray&)),
             this, SLOT(systemMessage(const QString&,const QByteArray&)) );

#if defined(Q_WS_QWS)
    initApp( argc, argv );
#endif

    d->qpe_system_locale = new QtopiaSystemLocale();
#if QT_VERSION < 0x040200
    QLocale::setSystemLocale(d->qpe_system_locale);
#else
    //nothing to do here
    //The QSystemLocale constructor installs itself
    //and removes any earlier installed system locale
#endif

#ifndef QT_NO_TRANSLATION
    QStringList qms;
    qms << QLatin1String("qt");
    qms << QLatin1String("libqtopiabase");
    qms << QLatin1String("libqtopia");
    qms << QLatin1String("libqtopiacomm");

    loadTranslations(qms);
#endif

    QContentUpdateManager::manager = new QContentUpdateManager(this);

    if ( type() == GuiServer ) {
        setVolume();
    }

    installEventFilter( this );

}

#if defined(Q_WS_QWS)
/*!
    \internal
*/
void QtopiaApplication::initApp( int argc, char **argv )
{
    QString channel = QString(argv[0]);
    d->appFullPath = channel;
    channel.replace(QRegExp(QLatin1String(".*/")),"");
    d->appName = channel;
    qApp->setApplicationName(d->appName);

#if defined(QTOPIA_DBUS_IPC)
    // Take care of quicklauncher re-init case
    if (d->m_appChannel)
        delete d->m_appChannel;

    d->m_appChannel = new DBUSQtopiaApplicationChannel(d->appName, this);
    connect(d->m_appChannel, SIGNAL(received(const QString &, const QByteArray &)),
            this, SLOT(pidMessage(const QString &, const QByteArray &)));
#endif

    if(type() != GuiServer) {
        if(d->lifeCycle) {
            d->lifeCycle->reinit();
        } else {
            d->lifeCycle = new QtopiaApplicationLifeCycle(this);
            QObject::connect(d->lifeCycle, SIGNAL(quit()), this, SLOT(quit()));
        }
    }

    delete d->pidChannel;
    d->preloaded = false;
    if(type() != GuiServer)
        d->lifeCycle->unregisterRunningTask("QtopiaPreload");

    //enforce update of image and sound dirs when started by quicklauncher
    d->fileengine->setIconPath( QStringList() );

    loadTranslations(QStringList()<<channel);

    qt_fbdpy->setIdentity( channel ); // In E 2.3.6

#ifndef QTOPIA_DBUS_IPC
    channel = QLatin1String("QPE/pid/") + QString::number(getpid());
    d->pidChannel = new QtopiaChannel( channel, this);
    connect( d->pidChannel, SIGNAL(received(const QString&,const QByteArray&)),
            this, SLOT(dotpidMessage(const QString&,const QByteArray&)));
#endif

    {
        QtopiaIpcEnvelope env("QPE/QtopiaApplication",
                              "available(QString,int)");
        env << QtopiaApplication::applicationName() << ::getpid();
    }

#ifndef QTOPIA_DBUS_IPC
    processQCopFile();
#endif

    int a = 0;
    while (a < argc) {
        if(qstrcmp(argv[a], "-noshow") == 0) {
            argc-=1;
            for (int j = a; j < argc; j++)
                argv[j] = argv[j+1];
            d->noshow = true;
        } else {
            a++;
        }
    }

    /* overide stored arguments */
    setArgs(argc, argv);
}
#endif

struct InputMethodHintRec {
    InputMethodHintRec(QtopiaApplication::InputMethodHint h, const QString& p) :
        hint(h), param(p) {}
    QtopiaApplication::InputMethodHint hint;
    QString param;
};
static QMap<QWidget*,InputMethodHintRec*>* inputMethodDict=0;
static void deleteInputMethodDict()
{
    if ( inputMethodDict )
        delete inputMethodDict;
    inputMethodDict = 0;
}

static void createInputMethodDict()
{
    if ( !inputMethodDict ) {
        inputMethodDict = new QMap<QWidget*,InputMethodHintRec*>;
        qAddPostRoutine(deleteInputMethodDict);
    }
}

/*!
  Returns the currently set hint to the system as to whether
  widget \a w has any use for text input methods.

  \sa setInputMethodHint(), InputMethodHint
*/
QtopiaApplication::InputMethodHint QtopiaApplication::inputMethodHint( QWidget* w )
{
    if (inputMethodDict && w && inputMethodDict->contains(w)) {
        InputMethodHintRec *r = inputMethodDict->value(w);
        return r->hint;
    }
    return Normal;
}

/*!
  Returns the currently set hint parameter for
  widget \a w.

  \sa setInputMethodHint(), InputMethodHint
*/
QString QtopiaApplication::inputMethodHintParam( QWidget* w )
{
    if ( inputMethodDict && w && inputMethodDict->contains(w)) {
        InputMethodHintRec* r = inputMethodDict->value(w);
        return r->param;
    }
    return QString();
}

/*!
    \enum QtopiaApplication::InputMethodHint

    \value Normal the widget sometimes needs text input.
    \value AlwaysOff the widget never needs text input.
    \value AlwaysOn the widget always needs text input.
    \value Number the widget needs numeric input.
    \value PhoneNumber the widget needs phone-style numeric input.
    \value Words the widget needs word input.
    \value Text the widget needs non-word input.
    \value Named the widget needs special input, defined by param.
        Each input method may support a different range of special
        input types, but will default to Text if they do not know the
        type.

    By default, QLineEdit and QTextEdit have the Words hint
    unless they have a QIntValidator, in which case they have the Number hint.
    This is appropriate for most cases, including the input of names (new
    names being added to the user's dictionary).
    All other widgets default to Normal mode.

    \sa inputMethodHint(), setInputMethodHint()
*/

/*!
    \enum QtopiaApplication::PowerConstraint

    \value Disable all power saving functions are disabled.
    \value DisableLightOff the screen's backlight will not be turned off
        (dimming remains active).
    \value DisableReturnToHomeScreen the device will not go back to the
        homescreen (phone edition only).
    \value DisableSuspend the device will not suspend
    \value Enable all power saving functions are enabled.

    Less severe PowerConstraints always imply more severe constraints
    (e.g DisableLightOff implies DisableSuspend). The exact implications are
    determined by the integer value that each constraint is assigned to (enumeration value).
    A low valued constraint will always imply a high valued constraint.

    \sa setPowerConstraint()
*/

/*!
  Hints to the system that widget \a w has use for the text input method
  specified by \a named. Such methods are input-method-specific and
  are defined by the files in [qt_prefix]/etc/im/ for each input method.

  For example, the phone key input method includes support for the
  names input methods:

\list
  \o email
  \o netmask
  \o url
\endlist

  The effect in the phone key input method is to modify the binding of
  phone keys to characters (such as making "@" easier to input), and to
  add additional \i words to the recognition word lists, such as: \i www.

  If the current input method doesn't understand the hint, it will be
  ignored.

  \sa inputMethodHint(), InputMethodHint
*/
void QtopiaApplication::setInputMethodHint( QWidget *w, const QString& named )
{
    setInputMethodHint(w,Named,named);
}

/*!
  Hints to the system that widget \a w has use for text input methods
  as specified by \a mode.  If \a mode is \c Named, then \a param
  specifies the name.

  \sa inputMethodHint(), InputMethodHint
*/
void QtopiaApplication::setInputMethodHint( QWidget* w, InputMethodHint mode, const QString& param )
{
    createInputMethodDict();
    if ( mode == Normal ) {
        if (inputMethodDict->contains(w))
            delete inputMethodDict->take(w);
    } else if (inputMethodDict->contains(w)) {
        InputMethodHintRec *r = (*inputMethodDict)[w];
        r->hint = mode;
        r->param = param;
    } else {
        inputMethodDict->insert(w, new InputMethodHintRec(mode,param));
        connect(w, SIGNAL(destroyed(QObject*)), qApp, SLOT(removeSenderFromIMDict()));
    }
    if ( w->hasFocus() )
        sendInputHintFor(w,QEvent::None);
}

/*!
  Explicitly show the current input method.

  Input methods are indicated in the taskbar by a small icon. If the
  input method is activated (shown) then it takes up some proportion
  of the bottom of the screen, to allow the user to interact (input
  characters) with it.

  \sa hideInputMethod()
*/
void QtopiaApplication::showInputMethod()
{
    QtopiaChannel::send( QLatin1String("QPE/InputMethod"), QLatin1String("showInputMethod()") );
}

/*!
  Explicitly hide the current input method.

  The current input method is still indicated in the taskbar, but no
  longer takes up screen space, and can no longer be interacted with.

  \sa showInputMethod()
*/
void QtopiaApplication::hideInputMethod()
{
    QtopiaChannel::send( QLatin1String("QPE/InputMethod"), QLatin1String("hideInputMethod()") );
}

#ifdef QTOPIA_KEYPAD_NAVIGATION
static bool isSingleFocusWidget(QWidget *focus)
{
    bool singleFocusWidget = false;
    if (focus) {
        QWidget *w = focus;
        singleFocusWidget = true;
        while ((w = w->nextInFocusChain()) != focus) {
            if (w->isVisible() && focus != w->focusProxy() && w->focusPolicy() & Qt::TabFocus) {
                if ( singleFocusWidget ) {
                    qLog(UI) << "Multi Focus:";
                    qLog(UI) << "  Current Focus:" << focus;
                }
                qLog(UI) << "  Next Focus: " << w;
                singleFocusWidget = false;
            }
        }
        if ( singleFocusWidget )
            qLog(UI) << "Single Focus: " << w;
    }

    return singleFocusWidget;
}

#endif

void QtopiaApplication::mapToDefaultAction( QWSKeyEvent *ke, int key )
{
    // specialised actions for certain widgets. May want to
    // add more stuff here.
    if ( activePopupWidget() && activePopupWidget()->inherits( "QListBox" )
         && activePopupWidget()->parentWidget()
         && activePopupWidget()->parentWidget()->inherits( "QComboBox" ) )
        key = Qt::Key_Return;

    if ( activePopupWidget() && activePopupWidget()->inherits( "QMenu" ) )
        key = Qt::Key_Return;

#ifdef Q_WS_QWS
    ke->simpleData.keycode = key;
#else
    Q_UNUSED( ke );
#endif
}

#ifdef Q_WS_QWS
/*!
  \internal
  Filters Qt event \a e to implement Qtopia-specific functionality.
*/
bool QtopiaApplication::qwsEventFilter( QWSEvent *e )
{
    if ( type() == GuiServer ) {
        switch ( e->type ) {
            case QWSEvent::Mouse:
                if ( e->asMouse()->simpleData.state && !QWidget::find(e->window()) )
                    emit clientMoused();
        }
    }
    if ( e->type == QWSEvent::Key ) {
        if ( d->kbgrabber == 1 )
            return true;
#ifdef QTOPIA_PDA
        QWSKeyEvent *ke = (QWSKeyEvent *)e;
        if ( ke->simpleData.keycode == Qt::Key_F33 ) {
            // Use special "OK" key to press "OK" on top level widgets
            QWidget *active = activeWindow();
            QWidget *popup = 0;
            Qt::WindowFlags wf = active ? active->windowFlags() : (Qt::WindowFlags)0;
            if ( active && (wf & Qt::Popup) == Qt::Popup) {
                popup = active;
                active = active->parentWidget();
            }
            if ( active && (int)active->winId() == ke->simpleData.window &&
                    (wf & (Qt::Popup|Qt::Desktop)) == 0 ) {
                if ( ke->simpleData.is_press ) {
                    if ( popup )
                        popup->close();
                    if (QDialog *dlg = qobject_cast<QDialog*>(active)) {
                        dlg->accept();
                        return true;
                    } else {
                        QTimer::singleShot(0, active, SLOT(accept()));
                    } else {
                        // do the same as with the select key: Map to the default action of the widget:
                        mapToDefaultAction( ke, Qt::Key_Return );
                    }
                }
            }
        } else if ( ke->simpleData.keycode == Qt::Key_F30 ) {
            // Use special "select" key to do whatever default action a widget has
            mapToDefaultAction( ke, Qt::Key_Space );
        } else if ( ke->simpleData.keycode == Qt::Key_Escape &&
                    ke->simpleData.is_press ) {
            // Escape key closes app if focus on toplevel
            QWidget *active = activeWindow();
            Qt::WindowFlags wf = active ? active->windowFlags() : (Qt::WindowFlags)0;
            if ( active && active->windowType() == Qt::Window &&
                    (int)active->winId() == ke->simpleData.window &&
                    (wf & (Qt::Popup|Qt::Desktop)) == 0 ) {
                if (QDialog *dlg = qobject_cast<QDialog*>(active)) {
                    dlg->reject();
                    return true;
                } else if ( strcmp( argv()[0], "embeddedkonsole") != 0 ) {
                    active->close();
                }
            }
        }
#else
# ifdef QT_QWS_SL5XXX
        QWSKeyEvent *ke = (QWSKeyEvent *)e;
        if ( ke->simpleData.keycode == Qt::Key_F33 )
            ke->simpleData.keycode = Qt::Key_Back;
        else if (ke->simpleData.keycode == Qt::Key_F30)
            ke->simpleData.keycode = Qt::Key_Select;
        else if (ke->simpleData.keycode == Qt::Key_Escape)
            ke->simpleData.keycode = Qt::Key_Back;
# endif

#endif
    } else if ( e->type == QWSEvent::Focus ) {
        if ( !d->notbusysent ) {
            if ( qApp->type() != QApplication::GuiServer ) {
                QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("notBusy(QString)") );
                e << d->appName;
            }
            d->notbusysent=true;
        }

        QWSFocusEvent *fe = (QWSFocusEvent*)e;
        QWidget* nfw = QWidget::find(e->window());
        if ( !fe->simpleData.get_focus ) {
            QWidget *active = activeWindow();
            while ( active && active->windowType() == Qt::Popup ) {
                active->close();
                active = activeWindow();
            }
            if ( !nfw && d->kbgrabber == 2 ) {
                ungrabKeyboard();
                d->kbregrab = true; // want kb back when we're active
            }
        } else {
            // make sure our modal widget is ALWAYS on top
            QWidget *topm = activeModalWidget();
            if ( topm && (int)topm->winId() != fe->simpleData.window) {
                topm->raise();
                topm->activateWindow();
            }
            if ( d->kbregrab ) {
                grabKeyboard();
                d->kbregrab = false;
            }
        }
        if ( fe->simpleData.get_focus && inputMethodDict ) {
            InputMethodHint m = inputMethodHint( QWidget::find(e->window()) );
            if ( m == AlwaysOff )
                QtopiaApplication::hideInputMethod();
            if ( m == AlwaysOn )
                QtopiaApplication::showInputMethod();
        }
    } else if (e->type == QWSEvent::MaxWindowRect) {
        QMetaObject::invokeMethod(this, "updateDialogGeometry", Qt::QueuedConnection);
    }

    return QApplication::qwsEventFilter( e );
}
#endif

/*!
  Destroys the QtopiaApplication.
*/
QtopiaApplication::~QtopiaApplication()
{
    if ( !d->notbusysent ) {
        // maybe we didn't map a window - still tell the server we're not
        // busy anymore.
        if ( qApp->type() != QApplication::GuiServer ) {
            QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("notBusy(QString)") );
            e << d->appName;
        }
    }
    ungrabKeyboard();

    // Need to delete QtopiaChannels early, since the display will
    // be gone by the time we get to ~QObject().
    delete d->sysChannel;
    delete d->pidChannel;

#ifdef QTOPIA_KEYPAD_NAVIGATION
    delete d->editMenu;
    delete d->calendarMenu;
    delete d->calendarNav;
#endif
    delete d;
}

#ifdef Q_WS_QWS
#define setPaletteEntry(pal, cfg, role, defaultVal) \
    setPalEntry(pal, cfg, #role, QPalette::role, defaultVal)
static void setPalEntry( QPalette &pal, const QSettings &config, const QString &entry,
                                QPalette::ColorRole role, const QString &defaultVal )
{
    QString value = config.value( entry, defaultVal ).toString();
    if ( value[0] == '#' )
        pal.setColor( role, QColor(value) );
    else {
        QPixmap pix;
        pix = QPixmap(QLatin1String(":image/")+value);
        pal.setBrush( role, QBrush(QColor(defaultVal), pix) );
    }
}
#endif

/*!
  \internal
*/
void QtopiaApplication::applyStyle()
{
#ifdef Q_WS_QWS
    QString styleName;
    QSettings config(QLatin1String("Trolltech"),QLatin1String("qpe"));

    config.beginGroup( QLatin1String("Appearance") );

#ifdef QTOPIA_PHONE
    QString themeDir = Qtopia::qtopiaDir() + QLatin1String("etc/themes/");
    QString theme = config.value("Theme").toString(); // The server ensures this value is present and correct

    QSettings themeCfg(themeDir + theme, QSettings::IniFormat);
    themeCfg.beginGroup(QLatin1String("Theme"));

    // Update the icon path
    QStringList ip;
    QString iconPathText = themeCfg.value(QLatin1String("IconPath")).toString();
    if ( !iconPathText.isEmpty() )
        ip = iconPathText.split( ';', QString::SkipEmptyParts );
    d->fileengine->setIconPath( ip );

    styleName = config.value( QLatin1String("Style"), QLatin1String("phonestyle") ).toString();
#else
    styleName = config.value( QLatin1String("Style"), QLatin1String("Qtopia") ).toString();
#endif

    // Colors
    // This first bit is same as settings/appearence/appearence.cpp, readColorScheme()
    QPalette tempPal;
    setPaletteEntry( tempPal, config, Button, QLatin1String("#F0F0F0") );
    setPaletteEntry( tempPal, config, Background, QLatin1String("#EEEEEE") );
    QPalette pal( tempPal.button().color(), tempPal.background().color() );
    setPaletteEntry( pal, config, Button, QLatin1String("#F0F0F0") );
    setPaletteEntry( pal, config, Background, QLatin1String("#EEEEEE") );
    setPaletteEntry( pal, config, Base, QLatin1String("#FFFFFF") );
    setPaletteEntry( pal, config, Highlight, QLatin1String("#8BAF31") );
    setPaletteEntry( pal, config, Foreground, QLatin1String("#000000") );
    QString color = config.value( QLatin1String("HighlightedText"), QLatin1String("#FFFFFF") ).toString();
    pal.setColor( QPalette::HighlightedText, QColor(color) );
    color = config.value( QLatin1String("Text"), QLatin1String("#000000") ).toString();
    pal.setColor( QPalette::Text, QColor(color) );
    color = config.value( QLatin1String("ButtonText"), QLatin1String("#000000") ).toString();
    pal.setColor( QPalette::Active, QPalette::ButtonText, QColor(color) );

    QString val = config.value( QLatin1String("Shadow") ).toString();
    if (!val.isEmpty()) {
        pal.setColor( QPalette::Shadow, QColor(val));
    } else {
        pal.setColor( QPalette::Shadow,
            pal.color(QPalette::Normal, QPalette::Button).dark(400) );
    }

    val = config.value( QLatin1String("Text_disabled") ).toString();
    if (!val.isEmpty()) {
        pal.setColor( QPalette::Disabled, QPalette::Text, QColor(val));
    } else {
        pal.setColor( QPalette::Disabled, QPalette::Text,
            pal.color(QPalette::Active, QPalette::Base).dark() );
    }

    val = config.value( QLatin1String("Foreground_disabled") ).toString();
    if (!val.isEmpty()) {
        pal.setColor( QPalette::Disabled, QPalette::Foreground, QColor(val));
    } else {
        pal.setColor( QPalette::Disabled, QPalette::Foreground,
            pal.color(QPalette::Active, QPalette::Background).dark() );
    }

    color = config.value( QLatin1String("AlternateBase"), QLatin1String("#EEF5D6") ).toString();
    if ( color[0] == '#' )
        pal.setColor( QPalette::AlternateBase, QColor(color));
    else {
        QPixmap pix;
        pix = QPixmap(QLatin1String(":image/")+color);
        pal.setBrush( QPalette::AlternateBase, QBrush(pix));
    }

    setPalette( pal );

    // Widget style; 0 if not a QtopiaStyle subclass
    QtopiaStyle *style = internalSetStyle( styleName );

#ifdef QTOPIA4_TODO
    // Unpublished feature for now, since current windows will be broken.
    // Need to go through all windows and poke QScrollViews to updateScrollbars.
    extern bool qt_left_hand_scrollbars;
    int lh = config.value( "LeftHand", 0 ).toInt();
    qt_left_hand_scrollbars = lh;
#endif

    // Window Decoration
    QString dec = config.value( QLatin1String("Decoration"), QLatin1String("Qtopia") ).toString();
    QString decTheme = config.value( QLatin1String("DecorationTheme"), QLatin1String("") ).toString();
    if ( dec != d->decorationName || !decTheme.isEmpty()) {
        qwsSetDecoration( new QtopiaDecoration( dec ) );
        d->decorationName = dec;
        d->decorationTheme = decTheme;
    }

    // Font
    QString ff = config.value( QLatin1String("FontFamily"), font().family() ).toString();
    double fs = config.value( QLatin1String("FontSize"), font().pointSizeF() ).toDouble();
    QFont fn(ff);
    fn.setPointSizeF(fs);

    setFont( fn );

#ifdef QTOPIA_PHONE
    // text display of context menubar
    config.endGroup();
    config.beginGroup( QLatin1String("ContextMenu") );
    ContextKeyManager::instance()->setLabelType( (QSoftMenuBar::LabelType)config.value( QLatin1String("LabelType"), QSoftMenuBar::IconLabel ).toInt() );

    if ( style ) {
        QString path = themeCfg.value(QLatin1String("WidgetConfig")).toString();
        style->setTheme(path.isEmpty() ? path : themeDir+path);
    }
#endif
#endif
}

void QtopiaApplication::systemMessage( const QString &msg, const QByteArray &data)
{
#ifdef Q_WS_QWS
    QDataStream stream( data );
    if ( msg == QLatin1String("contentChanged(QContentIdList,QContent::ChangeType)") ) {
        QContentIdList ids;
        stream >> ids;
        QContent::ChangeType type;
        stream >> type;
        if (type != QContent::Added) {
            foreach (QContentId id, ids)
                QContent::invalidate(id);
        }
        emit contentChanged(ids,type);
    } else if ( msg == QLatin1String("applyStyle()") ) {
        applyStyle();
    } else if ( msg == QLatin1String("shutdown()") ) {
        if ( type() == GuiServer )
            shutdown();
    } else if ( msg == QLatin1String("quit()") ) {
        if ( type() != GuiServer && d->appName != QLatin1String("quicklauncher") )
            tryQuit();
    } else if ( msg == QLatin1String("close()") ) {
        if ( type() != GuiServer )
            hideOrQuit();
    } else if ( msg == QLatin1String("forceQuit()") ) {
        if ( type() != GuiServer )
            quit();
    } else if ( msg == QLatin1String("restart()") ) {
        if ( type() == GuiServer )
            restart();
    } else if ( msg == QLatin1String("grabKeyboard(QString)") ) {
        QString who;
        stream >> who;
        if ( who.isEmpty() )
            d->kbgrabber = 0;
        else if ( who != d->appName )
            d->kbgrabber = 1;
        else
            d->kbgrabber = 2;
    } else if ( msg == QLatin1String("language(QString)") ) {
        if ( type() == GuiServer ) {
            QString l;
            stream >> l;
            l += QLatin1String(".UTF-8");
            QString cl = getenv("LANG");
            if ( cl != l ) {
                if ( l.isNull() )
                    unsetenv( "LANG" );
                else
                    setenv( "LANG", l.toLatin1(), 1 );
                restart();
            }
        }
    } else if ( msg == QLatin1String("timeChange(QString)") ) {
        // doesn't make sense to do this if quicklauncher,
        // and may steal call to something that needs it.
        if (d->appName != QLatin1String("quicklauncher")) {
            d->skiptimechanged++;
            QString t;
            stream >> t;
            if ( t.isNull() )
                unsetenv( "TZ" );
            else {
                setenv( "TZ", t.toLatin1(), 1 );
                tzset(); // ensure TZ value is used by subsequent localtime() calls
            }
            // emit the signal so everyone else knows...
            emit timeChanged();
        }
    } else if ( msg == QLatin1String("categoriesChanged()") ) {
        emit categoriesChanged();
    } else if ( msg == QLatin1String("clockChange(bool)") ) {
        QTimeString::updateFormats();
        int tmp;
        stream >> tmp;
        emit clockChanged( tmp );
    } else if ( msg == QLatin1String("weekChange(bool)") ) {
        int tmp;
        stream >> tmp;
        emit weekChanged( tmp );
    } else if ( msg == QLatin1String("setDateFormat()") ) {
        QTimeString::updateFormats();
        d->qpe_system_locale->readSettings();
        emit dateFormatChanged();
    } else if ( msg == QLatin1String("setVolume(int,int)") ) {
        if ( type() == GuiServer ) {
            int t,v;
            stream >> t >> v;
            setVolume(t,v);
        }
        emit volumeChanged( qtopia_muted );
    } else if ( msg == QLatin1String("volumeChange(bool)") ) {
        stream >> qtopia_muted;
        if ( type() == GuiServer ) {
            {
            QSettings cfg(QLatin1String("Trolltech"),QLatin1String("Sound"));
            cfg.beginGroup(QLatin1String("System"));
            cfg.setValue(QLatin1String("Muted"),qtopia_muted);
            }
            setVolume();
        }
        emit volumeChanged( qtopia_muted );
    } else if ( msg == QLatin1String("flush()") ) {
        emit flush();
        // we need to tell the desktop
        QtopiaIpcEnvelope e( QLatin1String("QPE/Desktop"), QLatin1String("flushDone(QString)") );
        e << d->appName;
    } else if ( msg == QLatin1String("reload()") ) {
        // Reload anything stored in files...
        applyStyle();
        if ( type() == GuiServer ) {
            setVolume();
            QtopiaServiceRequest e( "QtopiaPowerManager", "setBacklight(int)" );
            e << -1;
            e.send();
        }
        // App-specifics...
        emit reload();
    } else if ( msg == QLatin1String("getMarkedText()") ) {
        if ( type() == GuiServer ) {
            const ushort unicode = 'C'-'@';
            const int scan = Qt::Key_C;
            qwsServer->processKeyEvent( unicode, scan, Qt::ControlModifier, true, false );
            qwsServer->processKeyEvent( unicode, scan, Qt::ControlModifier, false, false );
        }
    } else if ( msg == QLatin1String("wordsChanged(QString,int)") ) {
        QString dictname;
        int pid;
        stream >> dictname >> pid;
        if ( pid != getpid() ) {
            Qtopia::qtopiaReloadWords(dictname);
        }
    } else if ( msg == QLatin1String("RecoverMemory()") ) {
        if (qApp->type() != GuiServer) {
            QPixmapCache::clear();
        }
    }
#ifdef QTOPIA_KEYPAD_NAVIGATION
    else if ( msg == QLatin1String("updateContextLabels()") ) {
        QSettings config("Trolltech","qpe");
        config.beginGroup( "ContextMenu" );
        ContextKeyManager::instance()->setLabelType((QSoftMenuBar::LabelType)config.value( "LabelType", QSoftMenuBar::IconLabel).toInt());
        if (activeWindow() && focusWidget()) {
            ContextKeyManager::instance()->updateContextLabels();
        }
    }
#endif
    else if ( msg == QLatin1String("resetContent()") ) {
        emit resetContent();
    }
#else
    Q_UNUSED( msg );
    Q_UNUSED( data );
#endif
}

/*!
  \internal
*/
bool QtopiaApplication::raiseAppropriateWindow()
{
    bool r=false;

    // 1. Raise the main widget
    QWidget *top = d->qpe_main_widget;

    // XXX now can be multiple top level widgets.
    // currently only deal with one (first)
    if ( !top && topLevelWidgets().count() > 0)
        top = topLevelWidgets()[0];

    if ( top ) {
            if ( top->isVisible() )
            r = true;
        else if (d->preloaded) {
            // We are preloaded and not visible.. pretend we just started..
            QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("fastAppShowing(QString)"));
            e << d->appName;
        }

        d->show_mx(top,d->nomaximize, d->appName);
        top->raise();
        top->activateWindow();
    }

    QWidget *topm = activeModalWidget();

    // 2. Raise any parentless widgets (except top and topm, as they
    //     are raised before and after this loop).  Order from most
    //     recently raised as deepest to least recently as top, so
    //     that repeated calls cycle through widgets.
    QWidgetList list = topLevelWidgets();
    bool foundlast = false;
    QWidget* topsub = 0;
    if ( d->lastraised ) {
        foreach (QWidget* w, list) {
            if ( !w->parentWidget() && w != top && w != topm && w->isVisible() && w->windowType() != Qt::Desktop ) {
                if ( w == d->lastraised )
                    foundlast = true;
                if ( foundlast ) {
                    w->raise();
                    w->activateWindow();
                    topsub = w;
                }
            }
        }
    }
    foreach (QWidget* w, list) {
        if ( !w->parentWidget() && w != top && w != topm && w->isVisible() && w->windowType() != Qt::Desktop) {
            if ( w == d->lastraised )
                break;
            w->raise();
            w->activateWindow();
            topsub = w;
        }
    }
    d->lastraised = topsub;

    // 3. Raise the active modal widget.
    if ( topm && topm != top ) {
        topm->show();
        topm->raise();
        topm->activateWindow();
        // If we haven't already handled the fastAppShowing message
        if (!top && d->preloaded) {
            QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("fastAppShowing(QString)"));
            e << d->appName;
        }
        r = false;
    }

    return r;
}

void QtopiaApplication::pidMessage( const QString &msg, const QByteArray & data)
{
#ifdef Q_WS_QWS
    if ( msg == QLatin1String("quit()") ) {
        tryQuit();
    } else if ( msg == QLatin1String("quitIfInvisible()") ) {
        if ( d->qpe_main_widget && !d->qpe_main_widget->isVisible() )
            quit();
    } else if ( msg == QLatin1String("close()") ) {
        hideOrQuit();
    } else if ( msg == QLatin1String("disablePreload()") ) {
        d->preloaded = false;
        if(type() != GuiServer)
            d->lifeCycle->unregisterRunningTask("QtopiaPreload");
        /* so that quit will quit */
    } else if ( msg == QLatin1String("enablePreload()") ) {
        if (d->qpe_main_widget) {
            if(type() != GuiServer)
                d->lifeCycle->registerRunningTask("QtopiaPreload", 0);
            d->preloaded = true;
        }
        /* so next quit won't quit */
    } else if ( msg == QLatin1String("raise()") ) {
        d->notbusysent = false;
        raiseAppropriateWindow();
        // Tell the system we're still chugging along...
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("appRaised(QString)"));
        e << d->appName;
    } else if ( msg == QLatin1String("flush()") ) {
        emit flush();
        // we need to tell the desktop
        QtopiaIpcEnvelope e( QLatin1String("QPE/Desktop"), QLatin1String("flushDone(QString)") );
        e << d->appName;
    } else if ( msg == QLatin1String("reload()") ) {
        emit reload();
    } else if ( msg == QLatin1String("setDocument(QString)") ) {
        QDataStream stream( data );
        QString doc;
        stream >> doc;
        QWidget *mw;

    mw = d->qpe_main_widget;
    // can be multiple top level widgets
    if( !mw && topLevelWidgets().count() > 0 ) {
           mw = topLevelWidgets()[0];
    }

        if ( mw ) {
            QMetaObject::invokeMethod(mw, "setDocument", Q_ARG(QString, doc));
        }
        raiseAppropriateWindow();
        // Tell the system we're still chugging along...
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("appRaised(QString)"));
        e << d->appName;
    } else if ( msg == QLatin1String("TimeMonitor::timeChange(QString)") ) {
        // Same as the QPE/System message
        if ( d->skiptimechanged ) {
            // Was handled in pidMessage()
            // (ie. we must offer TimeMonitor service)
            --d->skiptimechanged;
        } else {
            QDataStream stream( data );
            QString t;
            stream >> t;
            if ( t.isNull() )
                unsetenv( "TZ" );
            else {
                setenv( "TZ", t.toLatin1(), 1 );
                tzset(); // ensure TZ value is used by subsequent localtime() calls
            }
            // emit the signal so everyone else knows...
            emit timeChanged();
        }
    } else {
        emit appMessage( msg, data);
    }
#else
    Q_UNUSED( msg );
    Q_UNUSED( data );
#endif
}

#ifndef QTOPIA_DBUS_IPC
// Handle messages on "QPE/pid/<mypid>".  The only message on
// this channel should be "QPEProcessQCop()", which the server uses
// to prod the application to read its qcop-msg file.
void QtopiaApplication::dotpidMessage( const QString &msg, const QByteArray & )
{
    if ( msg == QLatin1String("QPEProcessQCop()")) {
        processQCopFile();
        d->sendQCopQ();
    }
}
#endif

/*!
    Get the main widget of this application.
*/
QWidget* QtopiaApplication::mainWidget() const
{
    return d->qpe_main_widget;
}

/*!
    Set the main widget for this application to \a wid.  If \a nomax is
    true, then do not maximize it.
*/
void QtopiaApplication::setMainWidget(QWidget *wid, bool nomax)
{
    Q_ASSERT(wid->isTopLevel());
    d->qpe_main_widget = wid;
    d->nomaximize = nomax;
}

/*!
    Show the main widget for this application.
*/
void QtopiaApplication::showMainWidget()
{
    if(!d->qpe_main_widget)
        qWarning("QtopiaApplication: Attempt to show null main widget.");

    if(d->noshow)
        d->noshow = false;
    else
        d->show(d->qpe_main_widget, d->nomaximize);
}

/*!
    Show the main widget for this application and ask it to show
    the document specified on the command-line.
*/
void QtopiaApplication::showMainDocumentWidget()
{
    QWidget *mw = mainWidget();
    if(mw && argc() == 2)
        QMetaObject::invokeMethod(mw, "setDocument",
                                  Q_ARG(QString, arguments().at(1)));

    showMainWidget();
}

/*!
  Sets widget \a mw as the mainWidget() and shows it. For small windows,
  consider passing true for \a nomaximize rather than the default false.

  \sa showMainDocumentWidget()
*/
void QtopiaApplication::showMainWidget( QWidget* mw, bool nomaximize )
{
    setMainWidget(mw, nomaximize);
    showMainWidget();
}

/*!
  Sets widget \a mw as the mainWidget() and shows it. For small windows,
  consider passing true for \a nomaximize rather than the default false.

  This calls designates the application as
  a \l {Qtopia - Main Document Widget}{document-oriented} application.

  The \a mw widget \i must have this slot: setDocument(const QString&).

  \sa showMainWidget()
*/
void QtopiaApplication::showMainDocumentWidget( QWidget* mw, bool nomaximize )
{
    setMainWidget(mw, nomaximize);
    showMainDocumentWidget();
}

/*!
  \internal
*/
QtopiaStyle *QtopiaApplication::internalSetStyle( const QString &styleName )
{
#ifdef Q_WS_QWS
    if (styleName != QLatin1String("themedstyle")){
        if ( styleName == d->styleName)
            return qobject_cast<QtopiaStyle*>(style());
    }else{
        QSettings config(QLatin1String("Trolltech"),QLatin1String("qpe"));
        config.beginGroup( QLatin1String("Appearance") );
        // For the Pixmap style we must remove the existing style here for theme changes to take effect
        setStyle( QLatin1String("windows") );
    }

    QtopiaStyle *newStyle = 0;

    if ( styleName == QLatin1String("QPE")  || styleName == QLatin1String("Qtopia") ) {
# ifdef QTOPIA_PHONE
#ifdef EZX_A780
        newStyle = new EzXPhoneStyle;
#else
        newStyle = new QPhoneStyle;
#endif
# else
        newStyle = new QPdaStyle;
# endif
    } else {
        newStyle = qobject_cast<QtopiaStyle*>(QStyleFactory::create(styleName));
    }

    if ( !newStyle ) {
#ifdef QTOPIA_PHONE
#ifdef EZX_A780
        newStyle = new EzXPhoneStyle;
#else
        //newStyle = new QPhoneStyle;
#endif

#else
        newStyle = new QPdaStyle;
#endif
        d->styleName = QLatin1String("QPE");
    } else {
        d->styleName = styleName;
    }

    if (qApp->style() != newStyle)
        setStyle( newStyle );

    return newStyle;
#else
    Q_UNUSED( styleName );
    return 0;
#endif
}

/*!
  \internal
*/
void QtopiaApplication::shutdown()
{
    // Implement in server's QtopiaApplication subclass
}

/*!
  \internal
*/
void QtopiaApplication::restart()
{
    // Implement in server's QtopiaApplication subclass
}

static QMap<QWidget*,QtopiaApplication::StylusMode>* stylusDict=0;
static void createDict()
{
    if ( !stylusDict )
        stylusDict = new QMap<QWidget*,QtopiaApplication::StylusMode>;
}

/*!
  Returns the current StylusMode for widget \a w.

  \sa setStylusOperation(), StylusMode
*/
QtopiaApplication::StylusMode QtopiaApplication::stylusOperation( QWidget* w )
{
    if (stylusDict && stylusDict->contains(w))
        return stylusDict->value(w);
    return LeftOnly;
}

/*!
    \enum QtopiaApplication::StylusMode

    \value LeftOnly the stylus only generates LeftButton
                        events (the default).
    \value RightOnHold the stylus generates RightButton events
                        if the user uses the press-and-hold gesture.

    \sa setStylusOperation(), stylusOperation()
*/

/*!
  Causes widget \a w to receive mouse events according to the stylus
  \a mode.

  \sa stylusOperation(), StylusMode
*/
void QtopiaApplication::setStylusOperation( QWidget* w, StylusMode mode )
{
    createDict();
    if ( mode == LeftOnly ) {
        stylusDict->remove(w);
    } else {
        stylusDict->insert(w,mode);
        connect(w,SIGNAL(destroyed()),qApp,SLOT(removeSenderFromStylusDict()));
    }
}

void QtopiaApplication::removeSenderFromIMDict()
{
    delete inputMethodDict->take(qobject_cast<QWidget*>(sender()));
}

/*!
  \reimp
*/
bool QtopiaApplication::eventFilter( QObject *o, QEvent *e )
{
    if ( !o->isWidgetType() )
        return false;

#ifdef QT_NO_QWS_CURSOR
    if ( e->type() == QEvent::ToolTip )
        // if we have no cursor, probably don't want tooltips
        return true;
#endif

    if ( stylusDict && e->type() >= QEvent::MouseButtonPress && e->type() <= QEvent::MouseMove ) {
        QMouseEvent* me = (QMouseEvent*)e;
        StylusMode mode = stylusOperation(qobject_cast<QWidget*>(o));
        switch (mode) {
          case RightOnHold:
            switch ( me->type() ) {
              case QEvent::MouseButtonPress:
                if (me->button() == Qt::LeftButton) {
                    if (!d->pressHandler)
                        d->pressHandler = new PressTickWidget();
                    d->pressHandler->startPress((QWidget*)o, me->pos());
                }
                break;
              case QEvent::MouseMove:
                if (d->pressHandler && d->pressHandler->active()
                    && (me->pos()-d->pressHandler->pos()).manhattanLength() > 8) {
                    d->pressHandler->cancelPress();
                    delete d->pressHandler;
                    d->pressHandler = 0;
                }
                break;
              case QEvent::MouseButtonRelease:
                if (d->pressHandler && d->pressHandler->active()
                    && me->button() == Qt::LeftButton) {
                    int rv = d->pressHandler->endPress(me->pos());
                    delete d->pressHandler;
                    d->pressHandler = 0;
                    return rv;
                }
                break;
              default:
                break;
            }
            break;
          default:
            ;
        }
    } else if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = (QKeyEvent *)e;
        if ( ke->key() == Qt::Key_Enter )
        {
            if ( o->inherits( "QRadioButton" ) || o->inherits( "QCheckBox" ) )
            {
                postEvent( o, new QKeyEvent( e->type(), Qt::Key_Space,
                    ke->modifiers(), " ", ke->isAutoRepeat(), ke->count() ) );
                return true;
            }
        } else if ( ke->key() == Qt::Key_Hangup && e->type() == QEvent::KeyPress ) {
            /* XXX QComboBox does not ignore key events that it does not handle (qt 4.2.2) */
            if ( qobject_cast<QComboBox*>(o) != NULL ) {
                e->ignore();
                return true;
            }
        }
#ifdef QTOPIA_KEYPAD_NAVIGATION
        else if ( !mousePreferred && ((QWidget*)o)->hasEditFocus()
            && (ke->key() == Qt::Key_Left || ke->key() == Qt::Key_Right
                || ke->key() == Qt::Key_Up || ke->key() == Qt::Key_Down)) {
            QtopiaApplicationData::updateContext((QWidget*)o);
        } // end else if cursor dir.
        else if (ke->key() == QSoftMenuBar::menuKey()
                && ke->type() == QEvent::KeyPress) {
            QWidget *w = (QWidget *)o;
            qLog(UI) << "Menu key for:" << w;
            QWidget *cw = ContextKeyManager::instance()->findTargetWidget(w,
                        ke->key(), mousePreferred || w->hasEditFocus());
            if (cw && cw != w)
            {
                sendEvent(cw,e);
                return true;
            }
        } // end elsif menu key
#endif  // end QTOPIA_KEYPAD_NAVIGATION
    } // end else if key event.
    else if ( e->type() == QEvent::FocusIn
#ifdef QTOPIA_KEYPAD_NAVIGATION
              || (!mousePreferred && (e->type() == QEvent::EnterEditFocus ||
              e->type() == QEvent::LeaveEditFocus))
#endif
              ) {

        QWidget *w = (QWidget *)o;

        if (e->type() == QEvent::LeaveEditFocus) {
/*
            qDebug() << "QEvent::LeaveEditFocus" << w;
        } else if (e->type() == QEvent::EnterEditFocus) {
            qDebug() << "QEvent::EnterEditFocus" << w;
        } else {
            qDebug() << "QEvent::FocusIn" << w;
*/
        }

#ifdef QTOPIA_KEYPAD_NAVIGATION
        if (e->type() == QEvent::FocusIn && !mousePreferred
            && w->focusPolicy() != Qt::NoFocus) {
            if (isSingleFocusWidget(w))
                w->setEditFocus(true); // set edit focus now
            else
                d->singleFocusTimer.start(0, this); // may become single focus later
        }
#endif

        if (!activeWindow())
            return false;

        sendInputHintFor(w, e->type());
#ifdef QTOPIA_KEYPAD_NAVIGATION
        if (w->inherits("QDateEdit")) {
            if(!QSoftMenuBar::hasMenu(w, QSoftMenuBar::AnyFocus)) {
                if (!d->calendarMenu)
                    d->calendarMenu = new CalendarMenu();
                /* some how needs to hook up to original widget when select happens? */
                QSoftMenuBar::addMenuTo(w, d->calendarMenu, QSoftMenuBar::AnyFocus);
            }
            d->calendarMenu->setTargetWidget(qobject_cast<QDateEdit*>(w));
        }
        if ((mousePreferred || w->hasEditFocus()) &&
                (w->inherits("QLineEdit")
                 || (w->inherits("QTextEdit") && !w->inherits("QTextBrowser")))) {
            if(!QSoftMenuBar::hasMenu(w, QSoftMenuBar::EditFocus)) {
                if (!d->editMenu)
                    d->editMenu = QSoftMenuBar::createEditMenu();
                QSoftMenuBar::addMenuTo(w, d->editMenu, QSoftMenuBar::EditFocus);
            }
        }
 
        QAbstractButton *b = qobject_cast<QAbstractButton*>(w);
        if ( b && b->isCheckable() && !mousePreferred ) {
            connect(b, SIGNAL(toggled(bool)),
                qApp, SLOT(buttonChange(bool)));
        }
        
        QTextBrowser *tb = qobject_cast<QTextBrowser*>(w);
        if ( tb && !mousePreferred ) {
            connect(tb, SIGNAL(highlighted(const QString&)),
                    qApp, SLOT(textBrowserHighlightChange(const QString&)));
        }
#endif
    }
#ifdef QTOPIA_KEYPAD_NAVIGATION
    else if ( e->type() == QEvent::FocusOut ) {
        QWidget *w = (QWidget *)o;
        QFocusEvent *fe = (QFocusEvent*)e;
        if ((fe->reason() == Qt::PopupFocusReason)
                || (!focusWidget() && w->topLevelWidget()->isVisible() && activeWindow())) {
            ContextKeyManager::instance()->updateContextLabels();
        }
        if (d->editMenu && !d->editMenu->isActiveWindow() && (w->inherits("QLineEdit")
                || (w->inherits("QTextEdit") && !w->inherits("QTextBrowser")))) {
            QSoftMenuBar::removeMenuFrom(w, d->editMenu);
            if ( d->editMenu ) {
                d->editMenu->deleteLater();
                d->editMenu= 0;
            }
        }
        
        QAbstractButton *b = qobject_cast<QAbstractButton*>(w);
        if ( b && b->isCheckable() && !mousePreferred ) {
            disconnect(b, SIGNAL(toggled(bool)),
                qApp, SLOT(buttonChange(bool)));
        }
        QTextBrowser *tb = qobject_cast<QTextBrowser*>(w);
        if ( tb && !mousePreferred ) {
            disconnect(tb, SIGNAL(highlighted(const QString&)),
                    qApp, SLOT(textBrowserHighlightChange(const QString&)));
        }
    } else if (e->type() == QEvent::Show) {
        QMessageBox *mb = 0;
        if (((QWidget*)o)->testAttribute(Qt::WA_ShowModal))
            mb = qobject_cast<QMessageBox*>(o);
        QDialog *dlg = qobject_cast<QDialog*>(o);
        if (!mb && dlg && !Qtopia::hasKey(Qt::Key_No)) { // no context menu for QMessageBox
            if (!isMenuLike(dlg)) {
                if ( (dlg->windowFlags()&Qt::WindowSystemMenuHint) ) {
                    bool foundco = false;
                    QList<QObject*> childObjects = o->children();
                    if (childObjects.count()) {
                        foreach(QObject *co, childObjects) {
                            if (co->isWidgetType() && co->metaObject()->className() == QString("QMenu")) {
                                foundco = true;
                                break;
                            }
                        }
                    }
                    if (!foundco) {
                        // There is no context menu defined
                        if (!ContextKeyManager::instance()->haveLabelForWidget(dlg, QSoftMenuBar::menuKey(), QSoftMenuBar::AnyFocus))
                            (void)QSoftMenuBar::menuFor((QWidget*)o);
                    }
                }
                if (!ContextKeyManager::instance()->haveLabelForWidget(dlg, Qt::Key_Back, QSoftMenuBar::AnyFocus))
                    QSoftMenuBar::setLabel(dlg, Qt::Key_Back, QSoftMenuBar::Back);
            } else {
                QSoftMenuBar::setLabel(dlg, Qt::Key_Back, QSoftMenuBar::Cancel);
            }
        }
        QWidget *w = qobject_cast<QWidget *>(o);
        if (w && w->isWindow()) {  // HACK: to get around a bug with minimum size in Qt 4.2.2
            QDesktopWidget *desktop = QApplication::desktop();
            QRect availableRect(desktop->availableGeometry(desktop->primaryScreen()));
            w->setMaximumWidth(availableRect.width());
        }
        if (w && w->isWindow() && (w->windowFlags() & Qt::WindowType_Mask) == Qt::Popup) {
            ShadowWidget *shadow = 0;
            if (d->shadowMap.contains(w)) {
                shadow = d->shadowMap[w];
            } else {
                QDesktopWidget *desktop = QApplication::desktop();
                QRect desktopRect(desktop->screenGeometry(desktop->primaryScreen()));
                shadow = new ShadowWidget(desktopRect.width() < 240 ? 3 : 5);
                d->shadowMap.insert(w, shadow);
                shadow->setTarget(w);
            }
            shadow->show();
#ifdef QTOPIA_ENABLE_FADE_IN_WINDOW
        } else if (w && w->isWindow()) {
            if (d->fadeInWidget && d->fadeInOpacity < 1.0)
                d->fadeInWidget->setWindowOpacity(1.0);
            w->setWindowOpacity(0.25);
            d->fadeInOpacity = 0.25;
            d->fadeInWidget = w;
            d->fadeInTimer.start(100, this);
#endif
        }
    } else if (e->type() == QEvent::Hide) {
        QWidget *w = qobject_cast<QWidget *>(o);
        if (w && w->isWindow() && (w->windowFlags() & Qt::WindowType_Mask) == Qt::Popup) {
            if (d->shadowMap.contains(w)) {
                ShadowWidget *shadow = d->shadowMap[w];
                d->shadowMap.remove(w);
                shadow->hide();
                shadow->deleteLater();
            }
        }
/*
    } else if (e->type() == QEvent::Paint) {
        QPaintEvent *pe = (QPaintEvent*)e;
        qDebug() << "Paint event for" << qobject_cast<QWidget *>(o) << pe->rect();
*/
    }
#endif  // QTOPIA_KEYPAD_NAVIGATION

    return false;
}

#ifdef QTOPIA_PHONE
void QtopiaApplication::hideMessageBoxButtons( QMessageBox *mb )
{
    QList<QPushButton*> pbList = mb->findChildren<QPushButton*>();
    if (pbList.count() == 1 || pbList.count() == 2) {
        // We only handle MBs with 1 or 2 buttons.
        // Currently we assume that the affirmative is pb1 and
        // the negative is pb2.
        QPushButton *pb1 = pbList[0];
        QPushButton *pb2 = pbList.count() == 2 ? pbList[1] : 0;

        // First hide buttons
        QDialogButtonBox *bbox = mb->findChild<QDialogButtonBox*>();
        if (bbox)
            bbox->hide();

        // Use a simpler layout.
        QLabel *icon = mb->findChild<QLabel*>(QLatin1String("qt_msgboxex_icon_label"));
        QLabel *label = mb->findChild<QLabel*>(QLatin1String("qt_msgbox_label"));
        if (icon && label && mb->layout()) {
            delete mb->layout();
            QHBoxLayout *hb = new QHBoxLayout;
            hb->setSizeConstraint(QLayout::SetNoConstraint);
            hb->setMargin(style()->pixelMetric(QStyle::PM_DefaultChildMargin));
            hb->addWidget(icon);
            hb->addWidget(label);
            mb->setLayout(hb);
        }

        // let us decide the size of the dialog.
        mb->setMaximumSize(desktop()->availableGeometry().size());

        // Setup accels for buttons and remove focus from buttons.
        pb1->setFocusPolicy(Qt::NoFocus);
        pb1->setDefault(false);
        int accel = 0;
        if (pbList.count() == 1) {
            if (Qtopia::hasKey(Qt::Key_Back))
                accel = Qt::Key_Back;
            else if (Qtopia::hasKey(Qt::Key_Yes))
                accel = Qt::Key_Yes;
            pb1->setShortcut(accel);
            QString txt = pb1->text();
            txt = txt.replace("&", "");
            QSoftMenuBar::setLabel(mb, accel, QString(), txt);
        } else if (pbList.count() == 2 ) {
            if (Qtopia::hasKey(Qt::Key_Yes)) {
                accel = Qt::Key_Yes;
            } else {
                const QList<int> &cbtns = QSoftMenuBar::keys();
                if (cbtns.count()) {
                    if (cbtns[0] != Qt::Key_Back)
                        accel = cbtns[0];
                    else if (cbtns.count() > 1)
                        accel = cbtns[cbtns.count()-1];
                }
            }
            if (accel) {
                pb1->setShortcut(accel);
                QString txt = pb1->text();
                txt = txt.replace("&", "");
                QSoftMenuBar::setLabel(mb, accel, QString(), txt);
            }
            pb2->setFocusPolicy(Qt::NoFocus);
            pb2->setDefault(false);
            accel = Qtopia::hasKey(Qt::Key_No) ? Qt::Key_No : Qt::Key_Back;
            pb2->setShortcut(accel);
            QString txt = pb2->text();
            txt = txt.replace("&", "");
            QSoftMenuBar::setLabel(mb, accel, QString(), txt);
        }

        // show the dialog using our standard dialog geometry calcuation
        showDialog(mb);
        mb->setFocus();
    }
}
#endif  // QTOPIA_PHONE

void QtopiaApplication::sendInputHintFor(QWidget *w, QEvent::Type etype)
{
#ifdef Q_WS_QWS
#ifdef QTOPIA_PDA
    Q_UNUSED(etype);
#endif
    const QValidator* v = 0;
    InputMethodHintRec *hr=0;
    bool passwordFlag = false ;
    if ( inputMethodDict ) {
        if (!inputMethodDict->contains(w)) {
            QWidget* p = w->parentWidget();
            if ( p ) {
                if ( p->focusProxy() == w ) {
                    if (inputMethodDict->contains(p))
                        hr = inputMethodDict->value(p);
                } else {
                    p = p->parentWidget();
                    if (p && p->focusProxy() == w && inputMethodDict->contains(p))
                        hr = inputMethodDict->value(p);
                }
            }
        } else {
            hr = inputMethodDict->value(w);
        }
    }
    int n = hr ? (int)hr->hint : 0;
    QLineEdit *l = qobject_cast<QLineEdit*>(w);
    if (!l && w->inherits("QSpinBox")) {
        l = ((QSpinBoxLineEditAccessor*)w)->getLineEdit();
        if (!hr)
            n = (int)Number;
    }
    if (!l && w->inherits("QComboBox"))
        l = ((QComboBox*)w)->lineEdit();
    if (l) {
        if ( !n && !l->isReadOnly()) {
            if( l->echoMode()==QLineEdit::Normal){
                n = (int)Words;
            } else {
                n = (int)Text;
                passwordFlag = true;
            };
            v = l->validator();
        }
#ifdef QTOPIA_KEYPAD_NAVIGATION
        if( n && !mousePreferred ) {
            if (etype == QEvent::EnterEditFocus) {
                connect(l, SIGNAL(textChanged(const QString&)),
                        qApp, SLOT(lineEditTextChange(const QString&)));
            } else if (etype == QEvent::LeaveEditFocus) {
                disconnect(l, SIGNAL(textChanged(const QString&)),
                        qApp, SLOT(lineEditTextChange(const QString&)));
            }
        }
#endif
    } else if (w->inherits("QTextEdit") && !w->inherits("QTextBrowser")) {
        QTextEdit* l = (QTextEdit*)w;
        if ( !n && !l->isReadOnly()) {
            n = (int)Words;
        }
#ifdef QTOPIA_KEYPAD_NAVIGATION
        if( n && !mousePreferred ) {
            if (etype == QEvent::EnterEditFocus) {
                connect(l, SIGNAL(textChanged()),
                        qApp, SLOT(multiLineEditTextChange()));
            } else if (etype == QEvent::LeaveEditFocus) {
                disconnect(l, SIGNAL(textChanged()),
                        qApp, SLOT(multiLineEditTextChange()));
            }
        }
#endif
    }
    if ( !hr && v && v->inherits("QIntValidator") )
        n = (int)Number;

    qint32 windowId = 0;
    if (w) {
        // popups don't send window active events... so need
        // to find the window id for the actual active window,
        // rather than that of the popup.
        QWidget *p = w->topLevelWidget();;
        while(p->windowType() == Qt::Popup && p->parentWidget())
            p = p->parentWidget();
        windowId = p->topLevelWidget()->winId();
    }
    // find ancestor.. top ancestor, then get its' window id
    if (etype == QEvent::FocusIn || etype == QEvent::None) {
        if ( n == Named ) {
            QtopiaIpcEnvelope env(QLatin1String("QPE/InputMethod"), QLatin1String("inputMethodHint(QString,int)") );
            env << (hr ? hr->param : QString());
            env << windowId;
        } else {
            QtopiaIpcEnvelope env(QLatin1String("QPE/InputMethod"), QLatin1String("inputMethodHint(int,int)") );
            env << n;
            env << windowId;
        }
        QtopiaIpcEnvelope passwordenv(QLatin1String("QPE/InputMethod"), QLatin1String("inputMethodPasswordHint(bool,int)") );
        passwordenv << passwordFlag;
        passwordenv << windowId;


    }
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (qApp->focusWidget() == w && qApp->activeWindow()) {
        ContextKeyManager::instance()->updateContextLabels();
    }
#endif
#else
    Q_UNUSED(w);
    Q_UNUSED(etype);
#endif
}

/*!
  \internal
*/
bool QtopiaApplication::notify(QObject* o, QEvent* e)
{
    bool r = QApplication::notify(o,e);
    if (e->type() == QEvent::Show && o->isWidgetType()
        && ((QWidget*)o)->testAttribute(Qt::WA_ShowModal)) {
        QMessageBox *mb = qobject_cast<QMessageBox*>(o);
        if (mb)
            hideMessageBoxButtons( mb );
    }
#if defined(QTOPIA_KEYPAD_NAVIGATION)
    if ( e->type() == QEvent::KeyPress || e->type() == QEvent::KeyRelease ) {
        QKeyEvent *ke = (QKeyEvent*)e;
        QWidget *w = qobject_cast<QWidget*>(o);

        if (!ke->isAccepted())
        {
            if ( ke->key() == Qt::Key_Hangup || ke->key() == Qt::Key_Call || ke->key() == Qt::Key_Flip )
            {
                // Send some unaccepted keys back to the server for processing.
                QtopiaIpcEnvelope e(QLatin1String("QPE/System"),QLatin1String("serverKey(int,int)"));
                e << ke->key() << int(ke->type() == QEvent::KeyPress);
            }
        }

        if (!ke->isAccepted()
                || (w && !w->hasEditFocus() && isSingleFocusWidget(w)
                    && !Qtopia::mousePreferred())) {
            if (w && ke->key() == Qt::Key_Back && e->type() == QEvent::KeyPress) {
                w = w->window();
                qLog(UI) << "Handling Back for" << w;
                if (QDialog *dlg = qobject_cast<QDialog*>(w)) {
                    if (isMenuLike(dlg)) {
                        qLog(UI) << "Reject dialog" << w;
                        dlg->reject();
                    } else {
                        qLog(UI) << "Accept dialog" << w;
                        dlg->accept();
                    }
                } else {
                    qLog(UI) << "Closing" << w;
                    w->close();
                }
                r = true;
            }
        }
    } else if (e->type() == QEvent::InputMethod) {
        // Try to keep the back button sensible when IM is composing.
        bool setBack = false;
        QSoftMenuBar::StandardLabel label = QSoftMenuBar::NoLabel;
        if (QLineEdit *le = qobject_cast<QLineEdit*>(o)) {
            setBack = true;
            QInputMethodEvent *ime = (QInputMethodEvent*)e;
            if (mousePreferred) {
                label = QSoftMenuBar::Back;
            } else {
                if (ime->commitString().length() + ime->preeditString().length() > 0
                    || le->cursorPosition() > 0)
                    label = QSoftMenuBar::BackSpace;
                else if (le->text().length() == 0)
                    label = QSoftMenuBar::RevertEdit;
            }
        } else if (QTextEdit *te = qobject_cast<QTextEdit*>(o)) {
            setBack = true;
            QInputMethodEvent *ime = (QInputMethodEvent*)e;
            if (mousePreferred) {
                label = QSoftMenuBar::Back;
            } else {
                if (ime->commitString().length() + ime->preeditString().length() > 0
                    || te->textCursor().position() > 0)
                    label = QSoftMenuBar::BackSpace;
                else if (te->document()->isEmpty())
                    label = QSoftMenuBar::Cancel;
            }
        }
        if (setBack)
            ContextKeyManager::instance()->setStandard(qobject_cast<QWidget*>(o), Qt::Key_Back, label);
    } else if (e->type() == QEvent::ChildAdded) {
#if QT_VERSION >= 0x040300
#error "Fix QCalendarWidget in keypad mode (see below)"
#else
        QCalendarWidget* w = qobject_cast<QCalendarWidget*>(o);
        if ( w ) {
            QWidget *prevMonth = w->findChild<QWidget*>("qt_calendar_prevmonth");
            QWidget *nextMonth = w->findChild<QWidget*>("qt_calendar_nextmonth");
            QWidget *monthButton = w->findChild<QWidget*>("qt_calendar_monthbutton");
            QWidget *yearButton = w->findChild<QWidget*>("qt_calendar_yearbutton");
            QWidget *yearEdit = w->findChild<QWidget*>("qt_calendar_yearedit");
            if ( prevMonth && nextMonth && monthButton && yearButton && yearEdit ) {

// This is the fix to go into QCalendarWidget
#ifdef QTOPIA_KEYPAD_NAVIGATION
                prevMonth->hide();
                nextMonth->hide();
                monthButton->setFocusPolicy(Qt::NoFocus);
                yearButton->setFocusPolicy(Qt::NoFocus);
                //yearEdit->setFocusPolicy(Qt::NoFocus);

                if (!d->calendarNav)
                    d->calendarNav = new CalendarTextNavigation();
                d->calendarNav->setTargetWidget(w);
#endif
// end-of-fix
            }
        }
#endif
#endif
    }

    /* Work around a QComboBox bug and Key_Hangup */
    if (e->type() == QEvent::ChildAdded) {
        QComboBox *w = qobject_cast<QComboBox*>(o);
        if ( w ) {
            w->installEventFilter(this);
        }
    }

    return r;
}


/*!
  \reimp
*/
void QtopiaApplication::timerEvent( QTimerEvent *e )
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (e->timerId() == d->singleFocusTimer.timerId()) {
        d->singleFocusTimer.stop();
        QWidget *w = focusWidget();
        if (w && isSingleFocusWidget(w))
            w->setEditFocus(true);
# ifdef QTOPIA_ENABLE_FADE_IN_WINDOW
    } else if (e->timerId() == d->fadeInTimer.timerId()) {
        if (d->fadeInWidget) {
            d->fadeInOpacity += 0.25;
            if (d->fadeInOpacity >= 1.0)
                d->fadeInTimer.stop();
            d->fadeInWidget->setWindowOpacity(d->fadeInOpacity);
        } else {
            d->fadeInTimer.stop();
        }
# endif
    }
#endif
}

void QtopiaApplication::removeSenderFromStylusDict()
{
    stylusDict->remove(qobject_cast<QWidget*>(sender()));
    if (d->pressHandler && d->pressHandler->widget() == sender())
        d->pressHandler->cancelPress();
}

/*!
  \internal
*/
bool QtopiaApplication::keyboardGrabbed() const
{
    return d->kbgrabber;
}


/*!
  Reverses the effect of grabKeyboard(). This is called automatically
  on program exit.
*/
void QtopiaApplication::ungrabKeyboard()
{
    QtopiaApplicationData* d = ((QtopiaApplication*)qApp)->d;
    if ( d->kbgrabber == 2 ) {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("grabKeyboard(QString)") );
        e << QString();
        d->kbregrab = false;
        d->kbgrabber = 0;
    }
}

/*!
  Grabs the physical keyboard keys, e.g. the application's launching
  keys. Instead of launching applications when these keys are pressed
  the signals emitted are sent to this application instead. Some games
  programs take over the launch keys in this way to make interaction
  easier.

  Under Qtopia Phone Edition this function does nothing.  It is not
  possible to grab the keyboard under Qtopia Phone Edition.

  \sa ungrabKeyboard()
*/
void QtopiaApplication::grabKeyboard()
{
#ifdef QTOPIA_PDA
    QtopiaApplicationData* d = ((QtopiaApplication*)qApp)->d;
    if ( qApp->type() == QApplication::GuiServer )
        d->kbgrabber = 0;
    else {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("grabKeyboard(QString)") );
        e << d->appName;
        d->kbgrabber = 2; // me
    }
#endif
}

/*!
    Enters the main event loop and waits until exit() is called.

    \sa QApplication::exec()
*/
int QtopiaApplication::exec()
{
#ifndef QTOPIA_DBUS_IPC
    d->qcopQok = true;
    d->sendQCopQ();
#endif

    return QApplication::exec();

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("closing(QString)") );
        e << d->appName;
    }

    processEvents();
    return 0;
}

/*!
  \internal
  External request for application to quit.  Quits if possible without
  loosing state.
*/
void QtopiaApplication::tryQuit()
{
    if ( activeModalWidget() || strcmp( argv()[0], "embeddedkonsole") == 0 )
        return; // Inside modal loop or konsole. Too hard to save state.

    {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("closing(QString)") );
        e << d->appName;
    }

    d->store_widget_rect(d->qpe_main_widget, d->appName);

    processEvents();

    quit();
}

/*!
  \internal
  User initiated quit.  Makes the window 'Go Away'.  If preloaded this means
  hiding the window.  If not it means closing all the toplevel widgets.
  As this is user initiated we don't need to check state.

  XXX - should probably be renamed hideApplication() or something (as it never
  actually forces a quit).
*/
void QtopiaApplication::hideOrQuit()
{
    d->store_widget_rect(d->qpe_main_widget, d->appName);

    processEvents();

    // If we are a preloaded application we don't actually quit, so emit
    // a System message indicating we're quasi-closing.
    if ( d->preloaded && d->qpe_main_widget ) {
        QtopiaIpcEnvelope e(QLatin1String("QPE/System"), QLatin1String("fastAppHiding(QString)") );
        e << d->appName;
        d->qpe_main_widget->hide();
    } else if(d->qpe_main_widget) {
        foreach(QWidget *wid, static_cast<QApplication *>(QCoreApplication::instance())->topLevelWidgets())
            wid->close();
    }
}

void QtopiaApplication::lineEditTextChange(const QString &)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (!mousePreferred)
        QtopiaApplicationData::updateContext((QWidget*)sender());
#endif
}

void QtopiaApplication::multiLineEditTextChange()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (!mousePreferred)
        QtopiaApplicationData::updateContext((QWidget*)sender());
#endif
}

void QtopiaApplication::buttonChange(bool)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (!mousePreferred)
        QtopiaApplicationData::updateButtonSoftKeys((QWidget*)sender());
#endif
}

void QtopiaApplication::textBrowserHighlightChange(const QString &)
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    if (!mousePreferred)
        QtopiaApplicationData::updateBrowserSoftKeys((QWidget*)sender());
#endif
}

void QtopiaApplication::removeFromWidgetFlags()
{
#ifdef QTOPIA_KEYPAD_NAVIGATION
    // removes calling object from flags.
    const QWidget *s = (const QWidget *)sender();
    if (qpeWidgetFlags.contains(s))
        qpeWidgetFlags.remove(s);
#endif
}

void QtopiaApplication::updateDialogGeometry()
{
    foreach (QWidget *widget, topLevelWidgets()) {
        QDialog *dlg = qobject_cast<QDialog*>(widget);
        if (dlg && dlg->isVisible())
            showDialog(dlg);
    }
}



/*!
    \fn void QtopiaApplication::showDialog( QDialog* dialog, bool nomax )

    Shows \a dialog. An heuristic approach is taken to
    determine the size and maximization of the dialog.

    \a nomax forces it to not be maximized.
*/
void QtopiaApplication::showDialog( QDialog* d, bool nomax )
{
    QtopiaApplicationData::qpe_show_dialog( d, nomax );
}


/*!
    \fn int QtopiaApplication::execDialog( QDialog* dialog, bool nomax )

    Shows and calls exec() on \a dialog. An heuristic approach is taken to
    determine the size and maximization of the dialog.

    \a nomax forces it to not be maximized.
*/
int QtopiaApplication::execDialog( QDialog* d, bool nomax )
{
    // Important to set WA_ShowModal before showing to maintain the
    // same behaviour as QDialog::exec() - i.e. flag is set when show()
    // happens.
    bool wasShowModal = d->testAttribute(Qt::WA_ShowModal);
    d->setAttribute(Qt::WA_ShowModal, true);
    showDialog(d,nomax);
    int rv = d->exec();
    d->setAttribute(Qt::WA_ShowModal, wasShowModal);
    return rv;
}

/*!
  This method temporarily overrides the current global power manager with
  the PowerConstraint \a constraint, allowing applications to control power saving
  functions during their execution.

  \sa PowerConstraint
*/
void QtopiaApplication::setPowerConstraint(PowerConstraint constraint)
{
    QtopiaServiceRequest e("QtopiaPowerManager", "setConstraint(int,QString)");
    e << (int)constraint << applicationName();
    e.send();
}

#ifdef Q_OS_WIN32
/*
  Provides minimal support for setting environment variables
 */
int setenv(const char* name, const char* value, int /*overwrite*/)
{
  QString msg(name);
  msg.append(QLatin1String("=")).append(value);
  // put env requires ownership of the value and will free it
  char * envValue = strdup(msg.toLocal8Bit().constData());
  return _putenv(envValue);
}

/*
  Provides minimal support for clearing environment variables
 */
void unsetenv(const char *name)
{
  setenv(name, "", 1);
}
#endif

#if (__GNUC__ > 2)
extern "C" void __cxa_pure_virtual();

void __cxa_pure_virtual()
{
    fprintf( stderr, "Pure virtual called\n"); // No tr
    abort();

}

#endif

#if defined(QPE_USE_MALLOC_FOR_NEW)

// The libraries with the skiff package (and possibly others) have
// completely useless implementations of builtin new and delete that
// use about 50% of your CPU. Here we revert to the simple libc
// functions.

void* operator new[](size_t size)
{
    return malloc(size);
}

void* operator new(size_t size)
{
    return malloc(size);
}

void operator delete[](void* p)
{
    if (p)
        free(p);
}

void operator delete[](void* p, size_t /*size*/)
{
    if (p)
        free(p);
}

void operator delete(void* p)
{
    if (p)
        free(p);
}

void operator delete(void* p, size_t /*size*/)
{
    if (p)
        free(p);
}

#endif

#ifdef QTOPIA_KEYPAD_NAVIGATION
/*!
  \a b equal to true sets the dialog \a diag to have MenuLike behaviour when executed,
  otherwise sets dialog to have default dialog behaviour.

  MenuLike dialogs typically have a single list of options, and
  should accept the dialog when the select key is pressed on the appropriate item,
  and when a mouse/stylus is used to click on an item. Key focus should be accepted by only one widget.

  By marking a dialog as MenuLike Qtopia will map the Back key to reject the dialog
  and will not map any key to accept the dialog - that must be user-defined.

  The default dialog behaviour is to include a cancel menu option in the context
  menu to reject the dialog and to map the Back key to accept the dialog.

  \sa isMenuLike()
*/
void QtopiaApplication::setMenuLike( QDialog *diag, bool b )
{
    if (b == isMenuLike(diag))
        return;

    // can't do here, if QDialog is const..... but it doesn't need to be.
    if (b)
        QSoftMenuBar::setLabel(diag, Qt::Key_Back, QSoftMenuBar::Cancel);
    else
        QSoftMenuBar::setLabel(diag, Qt::Key_Back, QSoftMenuBar::Back);

    if (qpeWidgetFlags.contains(diag)) {
        qpeWidgetFlags[diag] = qpeWidgetFlags[diag] ^ MenuLikeDialog;
    } else {
        // the ternery below is actually null.  if b is false, and flags
        // not present, then will never get here, hence b is always
        // true at this point.  Leave in though incase that logic
        // changes.
        qpeWidgetFlags.insert(diag, b ? MenuLikeDialog : 0);
        // connect to destructed signal.
        connect(diag, SIGNAL(destroyed()), qApp, SLOT(removeFromWidgetFlags()));
    }
}

/*!
  Returns true if the dialog \a diag is set to have MenuLike behaviour when executed,
  otherwise returns false.

  \sa setMenuLike()
*/
bool QtopiaApplication::isMenuLike( const QDialog *diag)
{
    if (qpeWidgetFlags.contains(diag))
        return (qpeWidgetFlags[diag] & MenuLikeDialog == MenuLikeDialog);
    return false; // default.
}
#endif

#include "qtopiaapplication.moc"
