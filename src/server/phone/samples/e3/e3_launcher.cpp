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

#include "e3_launcher.h"
#include "phone/phoneheader.h"
#include "phone/contextlabel.h"
#include "windowmanagement.h"
#include "phone/themecontrol.h"
#include <QApplication>
#include <QDesktopWidget>
#include "qtopiaserverapplication.h"
#include "launcherview.h"
#include <QVBoxLayout>
#include <QContentFilter>
#include <QCategoryFilter>
#include <QListWidget>
#include <QEvent>
#include <QKeyEvent>
#include <QMouseEvent>
#include <QExportedBackground>
#include <QStringListModel>
#include <QVariant>
#include <QPainter>
#include <QSettings>
#include <QPen>
#include <QSoftMenuBar>
#include <QtopiaChannel>
#include <QKeyEvent>
#include "phone/qabstractbrowserscreen.h"
#include "e3_today.h"
#include <QLabel>
#include <QPalette>
#include <QTimeString>
#include <QOccurrence>
#include <QAppointment>
#include <QTask>
#include <ThemedView>

QTOPIA_REPLACE_WIDGET(QAbstractServerInterface, E3ServerInterface);

class NonModalLauncherView : public LauncherView
{
Q_OBJECT
public:
    NonModalLauncherView(QWidget *parent);

protected:
    virtual bool eventFilter(QObject *, QEvent *);
};

NonModalLauncherView::NonModalLauncherView(QWidget *parent)
: LauncherView(parent)
{
    icons->installEventFilter(this);
}

bool NonModalLauncherView::eventFilter(QObject *, QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        setEditFocus(true);
        resetSelection();
    } else if(e->type() == QEvent::FocusOut) {
        icons->clearSelection();
    } else if(e->type() == QEvent::KeyPress ||
              e->type() == QEvent::KeyRelease) {

        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch(ke->key()) {
            case Qt::Key_Up:
                if(QEvent::KeyPress == e->type())
                    focusPreviousChild();
                return true;
            case Qt::Key_Down:
                if(QEvent::KeyPress == e->type())
                    focusNextChild();
                return true;
            case Qt::Key_Back:
                e->ignore();
                return true;
        }

    }

    return false;
}

class NonModalListView : public QListView
{
Q_OBJECT
public:
    enum Navigation { Up = 0x0001, Down = 0x0002, Both = 0x0003 };
    NonModalListView(Navigation, QWidget *parent);

protected:
    virtual bool eventFilter(QObject *, QEvent *);

private:
    Navigation nav;
};

NonModalListView::NonModalListView(Navigation n, QWidget *parent)
: QListView(parent), nav(n)
{
    installEventFilter(this);
}

bool NonModalListView::eventFilter(QObject *, QEvent *e)
{
    if(e->type() == QEvent::FocusIn) {
        setEditFocus(true);
        if(model()->rowCount())
            setCurrentIndex(model()->index(0, 0));
    } else if(e->type() == QEvent::FocusOut) {
        clearSelection();
    } else if(e->type() == QEvent::KeyPress ||
              e->type() == QEvent::KeyRelease) {

        QKeyEvent *ke = static_cast<QKeyEvent *>(e);
        switch(ke->key()) {
            case Qt::Key_Up:
                if(QEvent::KeyPress == e->type() &&
                   (selectedIndexes().isEmpty() ||
                    selectedIndexes().first().row() == 0)) {
                    if(Up & nav)
                        focusPreviousChild();
                    return true;
                }
                break;
            case Qt::Key_Down:
                if(QEvent::KeyPress == e->type() &&
                   (selectedIndexes().isEmpty() ||
                    selectedIndexes().first().row() == (model()->rowCount() - 1))) {
                    if(Down & nav)
                        focusNextChild();
                    return true;
                }
                
                break;
            case Qt::Key_Back:
                e->ignore();
                return true;
        }

    }

    return false;
}

class E3ListDelegate : public QAbstractItemDelegate
{
Q_OBJECT
public:
    E3ListDelegate(QObject *parent);

    virtual QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const;
};


E3ListDelegate::E3ListDelegate(QObject *parent)
: QAbstractItemDelegate(parent)
{
}

QSize E3ListDelegate::sizeHint(const QStyleOptionViewItem &option, 
                               const QModelIndex &) const
{
    return QSize(64, 64);
}

void E3ListDelegate::paint(QPainter *painter, 
                           const QStyleOptionViewItem &option, 
                           const QModelIndex &index) const
{
    QSize s = option.rect.size();
    s.setHeight(s.height() - 4);
    s.setWidth(s.width() - 4);
    QPixmap pix = 
        qvariant_cast<QIcon>(index.data(Qt::DecorationRole)).pixmap(s);

    if(option.state & QStyle::State_Selected) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(option.palette.highlight());
        painter->drawRoundRect(option.rect);
    }

    int x = option.rect.x() + (option.rect.width() - pix.width()) / 2;
    int y = option.rect.y() + (option.rect.height() - pix.height()) / 2;
    painter->drawPixmap(x, y, pix);
}

class E3Separator : public QWidget
{
Q_OBJECT
public:
    E3Separator(QWidget *parent);

protected:
    virtual void paintEvent(QPaintEvent *);
};

E3Separator::E3Separator(QWidget *parent)
: QWidget(parent)
{
    setFixedHeight(1);
}

void E3Separator::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setPen(QPen(QPalette().brightText(), 1, Qt::DotLine));
    p.drawLine(0, 0, width(), 0);
}

class E3Label : public QWidget
{
Q_OBJECT
public:
    E3Label(QWidget *parent = 0);

    void setText(const QString &);
    void setPixmap(const QPixmap &);

signals:
    void activated();

protected:
    virtual void keyPressEvent(QKeyEvent *);
    virtual void mousePressEvent(QMouseEvent *);

private:
    QLabel *text;
    QLabel *pixmap;
};

class E3CalItem : public QWidget
{
Q_OBJECT
public:
    E3CalItem(E3Today *t, QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void dataChanged();
    void activated();

private:
    E3Today *today;
    bool isFocused;
    E3Label *textLabel;
    E3Label *appointmentLabel;
};

class E3TodoItem : public QWidget
{
Q_OBJECT
public:
    E3TodoItem(E3Today *t, QWidget *parent = 0);

protected:
    virtual void paintEvent(QPaintEvent *);
    virtual void focusInEvent(QFocusEvent *);
    virtual void focusOutEvent(QFocusEvent *);
    virtual void keyPressEvent(QKeyEvent *);

private slots:
    void activated();
    void dataChanged();

private:
    E3Today *today;
    bool isFocused;
    E3Label *textLabel;
};

E3Label::E3Label(QWidget *parent)
: QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setMargin(0);
    layout->setSpacing(0);
    setLayout(layout);

    pixmap = new QLabel(this);
    layout->addWidget(pixmap);

    text = new QLabel(this);
    layout->addWidget(text);

    layout->addStretch(10);
}

void E3Label::setText(const QString &txt)
{
    text->setText(txt);
}

void E3Label::setPixmap(const QPixmap &pix)
{
    pixmap->setPixmap(pix);
}

void E3Label::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Select) {
        emit activated();
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void E3Label::mousePressEvent(QMouseEvent *e)
{
    emit activated();
    e->accept();
}

E3CalItem::E3CalItem(E3Today *t, QWidget *parent)
: QWidget(parent), today(t), isFocused(false), textLabel(0), appointmentLabel(0)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    E3Separator *sep = new E3Separator(this);
    layout->addWidget(sep);

    textLabel = new E3Label(this);
    QPixmap pix(":image/datebook/DateBook_16");
    textLabel->setPixmap(pix);
    layout->addWidget(textLabel);
    appointmentLabel = new E3Label(this);
    appointmentLabel->setPixmap(pix);
    appointmentLabel->setText("World");
    layout->addWidget(appointmentLabel);

    QObject::connect(today, SIGNAL(todayChanged()), this, SLOT(dataChanged()));
    dataChanged();
    QObject::connect(textLabel, SIGNAL(activated()), this, SLOT(activated()));
    QObject::connect(appointmentLabel, SIGNAL(activated()), this, SLOT(activated()));
}

void E3CalItem::activated()
{
    QtopiaServiceRequest req("Calendar", "raiseToday()");
    req.send();
}

void E3CalItem::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Select) {
        activated();
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void E3CalItem::paintEvent(QPaintEvent *)
{
    if(isFocused) {
        QPainter p(this);
        QPalette pal;
        p.setBrush(pal.highlight());
        p.setPen(Qt::NoPen);
        p.drawRoundRect(rect());
    }
}

void E3CalItem::focusInEvent(QFocusEvent *)
{
    isFocused = true;
    update();
}

void E3CalItem::focusOutEvent(QFocusEvent *)
{
    isFocused = false;
    update();
}

void E3CalItem::dataChanged()
{
    if(today->dayStatus() == E3Today::MoreAppointments) {
        textLabel->hide();
        appointmentLabel->show();
    } else if(today->nextAppointment().isValid()) {
        textLabel->show();
        textLabel->setText(tr("No more entries today"));
        appointmentLabel->show();
    } else {
        textLabel->show();
        textLabel->setText(tr("No cal. entries for today"));
        appointmentLabel->hide();
    }

    if(today->nextAppointment().isValid()) {
        QOccurrence o = 
            today->nextAppointment().nextOccurrence(QDate::currentDate());

        QString str;
        str += o.start().toString(QTimeString::currentFormat());
        str += today->nextAppointment().description();

        appointmentLabel->setText(str);
    }
}

E3TodoItem::E3TodoItem(E3Today *t, QWidget *parent)
: QWidget(parent), today(t), isFocused(false), textLabel(0)
{
    setFocusPolicy(Qt::StrongFocus);

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    E3Separator *sep = new E3Separator(this);
    layout->addWidget(sep);

    textLabel = new E3Label(this);
    QPixmap pix(":image/todolist/TodoList_16");
    textLabel->setPixmap(pix);
    layout->addWidget(textLabel);

    QObject::connect(today, SIGNAL(todayChanged()), this, SLOT(dataChanged()));
    QObject::connect(textLabel, SIGNAL(activated()), this, SLOT(activated()));
    dataChanged();
}

void E3TodoItem::activated()
{
    QtopiaServiceRequest req("Application:todolist", "raise()");
    req.send();
}

void E3TodoItem::keyPressEvent(QKeyEvent *e)
{
    if(e->key() == Qt::Key_Select) {
        activated();
        e->accept();
    } else {
        QWidget::keyPressEvent(e);
    }
}

void E3TodoItem::paintEvent(QPaintEvent *)
{
    if(isFocused) {
        QPainter p(this);
        QPalette pal;
        p.setBrush(pal.highlight());
        p.setPen(Qt::NoPen);
        p.drawRoundRect(rect());
    }
}

void E3TodoItem::focusInEvent(QFocusEvent *)
{
    isFocused = true;
    update();
}

void E3TodoItem::focusOutEvent(QFocusEvent *)
{
    isFocused = false;
    update();
}

void E3TodoItem::dataChanged()
{
    int tasks = today->tasks();
    if(!tasks) {
        hide();
        return;
    }

    show();
    QString str;
    if(tasks == 1) {
        str = today->task(0).description();
    } else {
        str = QString(tr("%1 to-do notes not done")).arg(tasks);
    }

    textLabel->setText(str);
}

E3ServerInterface::E3ServerInterface(QWidget *parent, Qt::WFlags flags)
: QAbstractServerInterface(parent, flags), m_view(0), m_header(0), m_context(0),
  m_background(0), m_browser(0), m_today(0)
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
    QExportedBackground::initExportedBackground(desktopRect.width(),
                                                desktopRect.height());

    m_background = new QExportedBackground(this);
    QObject::connect(m_background, SIGNAL(wallpaperChanged()),
                     this, SLOT(wallpaperChanged()));
    wallpaperChanged();

    // Create header
    header();
    // Create context
    context();

    QVBoxLayout *layout = new QVBoxLayout(this);
    setLayout(layout);
    layout->setMargin(0);
    layout->setSpacing(0);

    m_theme = new ThemedView(this);
    layout->addWidget(m_theme);
    layout = new QVBoxLayout(m_theme);
    m_theme->setLayout(layout); 
    layout->setMargin(0);
    layout->setSpacing(0);
    ThemeControl::instance()->registerThemedView(m_theme, "Home");

    E3Separator *sep = new E3Separator(m_theme);
    layout->addWidget(sep);

    // Quick launcher bar
    m_view = new NonModalLauncherView(m_theme);
    m_view->setFixedHeight(40);
    m_view->setItemDelegate(new E3ListDelegate(m_view));
    layout->addWidget(m_view);
    m_view->setViewMode(QListView::IconMode);
    QObject::connect(m_view, SIGNAL(clicked(QContent)), this, SLOT(launch(QContent)));
    quickApps();

    sep = new E3Separator(m_theme);
    layout->addWidget(sep);

    // Info list
    m_today = new E3Today(this);
    E3CalItem *calItem = new E3CalItem(m_today, m_theme);
    layout->addWidget(calItem);
    E3TodoItem *todoItem = new E3TodoItem(m_today, m_theme);
    layout->addWidget(todoItem);

    layout->addStretch(10);
    
    // Context buttons
    idleContext();

    QtopiaChannel *e3 = new QtopiaChannel("QPE/E3", this);
    QObject::connect(e3, SIGNAL(received(QString, QByteArray)),
                     this, SLOT(received(QString, QByteArray)));

    QObject::connect(ThemeControl::instance(), SIGNAL(themeChanged()),
                     this, SLOT(loadTheme()));
    loadTheme();
}

bool E3ServerInterface::event(QEvent *e)
{
    if(e->type() == QEvent::WindowActivate ||
       e->type() == QEvent::Show)
        m_today->forceUpdate();

    return QAbstractServerInterface::event(e);
}

void E3ServerInterface::received(const QString &message, const QByteArray &)
{
    if(message == "showApplications()")
        showApps();
}

void E3ServerInterface::showApps()
{
    if(!m_browser)
        m_browser = qtopiaWidget<QAbstractBrowserScreen>();

    if(m_browser) {
        m_browser->showMaximized();
        m_browser->raise();
        m_browser->resetToView("Main");
    }
}

void E3ServerInterface::wallpaperChanged()
{
    QPixmap wallpaper = m_background->wallpaper();
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
    wallpaper = wallpaper.scaled(desktopRect.size());
    QExportedBackground::setExportedBackground(wallpaper);
}

void E3ServerInterface::header()
{
    m_header = new PhoneHeader();
    m_header->show();
    WindowManagement::protectWindow(m_header);
    ThemeControl::instance()->registerThemedView(m_header, "Title");
    m_header->resize(QApplication::desktop()->screenGeometry().width(),
                     m_header->height());
}

void E3ServerInterface::context()
{
    m_context = new ContextLabel(0, Qt::FramelessWindowHint |
                                    Qt::Tool |
                                    Qt::WindowStaysOnTopHint );
    m_context->show();
    m_context->move(QApplication::desktop()->screenGeometry().topLeft()); // move to the correct screen
    WindowManagement::protectWindow(m_context);
    m_context->setAttribute(Qt::WA_GroupLeader);
    ThemeControl::instance()->registerThemedView(m_context, "Context");
    // Set width now to avoid relayout later.
    m_context->resize(QApplication::desktop()->screenGeometry().width(),
                    m_context->height());
}

void E3ServerInterface::keyPressEvent(QKeyEvent *e)
{
    IdleKeys::ConstIterator iter = m_idleKeys.find(e->key());
    if(iter != m_idleKeys.end()) {
        QtopiaServiceRequest req = *iter;
        req.send();
        e->accept();
    } else {
        QAbstractServerInterface::keyPressEvent(e);
    }
}

void E3ServerInterface::loadTheme()
{
    QDesktopWidget *desktop = QApplication::desktop();
    QRect desktopRect = desktop->screenGeometry(desktop->primaryScreen());
   
    WindowManagement::dockWindow(m_header, WindowManagement::Top, 
                                 m_header->reservedSize());
    WindowManagement::dockWindow(m_context, WindowManagement::Bottom, 
                                 m_context->reservedSize());
}

void E3ServerInterface::quickApps()
{
    QSettings settings("Trolltech", "E3");
    settings.beginGroup("QuickApps");

    int count = settings.value("Count", 0).toInt();
    if(!count) return;

    int apps = 0;

    QContentSet set(QContentFilter::Role, "Applications");

    for(int ii = 0; ii < count; ++ii) {
        QString app = settings.value("Application" + QString::number(ii), QString()).toString();
        QContent c = set.findExecutable(app);
        if(c.isValid()) {
            ++apps;
            m_view->addItem(&c);
        }
    }

    m_view->setColumns(apps);
}

void E3ServerInterface::idleContext()
{
    QSettings settings("Trolltech", "E3");
    settings.beginGroup("IdleButtons");
    settings.beginGroup("Context1");

    ButtonBinding bb = buttonBinding(settings);
    if(!bb.first.isEmpty()) {
        m_idleKeys.insert(Qt::Key_Context1, bb.second);
        QSoftMenuBar::setLabel(this, Qt::Key_Context1, QString(), bb.first);
    }

    settings.endGroup();
    settings.beginGroup("Context2");

    bb = buttonBinding(settings);
    if(!bb.first.isEmpty()) {
        m_idleKeys.insert(Qt::Key_Back, bb.second);
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), bb.first);
    } else {
        QSoftMenuBar::setLabel(this, Qt::Key_Back, QString(), QString());
    }
}

E3ServerInterface::ButtonBinding E3ServerInterface::buttonBinding(QSettings & settings) const
{
    QPair<QString, QtopiaServiceRequest> rv;
    QString text = settings.value("Text").toString();
    QString service = settings.value("Service").toString();
    QString message = settings.value("Message").toString();

    if(!text.isEmpty() && !service.isEmpty() && !message.isEmpty()) {
        rv.first = text;
        rv.second.setService(service);
        rv.second.setMessage(message);
    }

    return rv;
}

void E3ServerInterface::launch(QContent c)
{
    c.execute();
}

#include "e3_launcher.moc"
