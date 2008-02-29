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

#include "qsoftmenubarprovider.h"
#include "windowmanagement.h"
#include <QSharedData>
#include <QIcon>
#include <QDebug>
#include <Qtopia>
#include <QPixmapCache>
#include <qwindowsystem_qws.h>
#include <QStyle>
#include <QApplication>
#include <QtopiaChannel>

#include <QSoftMenuBar>

// declare QSoftMenuBarProviderPrivate
struct QSoftMenuBarProviderPrivate
{
    QSoftMenuBarProviderPrivate() : activeWin(-1), blockUpdates(false) {}

    QList<QSoftMenuBarProvider::MenuButton> buttons;
    int activeWin;
    bool blockUpdates;

    int keyToIdx(int key) const;
};

// define QSoftMenuBarProviderPrivate
int QSoftMenuBarProviderPrivate::keyToIdx(int key) const
{
    for(int ii = 0; ii < buttons.count(); ++ii)
        if(buttons.at(ii).key() == key)
            return ii;

    return -1;
}

// declare QSoftMenuBarProvider::MenuButtonPrivate
struct QSoftMenuBarProvider::MenuButtonPrivate : public QSharedData
{
    MenuButtonPrivate()
        : key(0), pixStale(false), index(-1) {}
    MenuButtonPrivate(const MenuButtonPrivate &other)
        : QSharedData(other), key(other.key), text(other.text), pixName(other.pixName),
          pixStale(other.pixStale), pix(other.pix), index(other.index) {}

    int key;
    QString text;
    QString pixName;
    mutable bool pixStale;
    mutable QPixmap pix;
    int index;
};

/*!
  \class QSoftMenuBarProvider::MenuButton
  \brief The MenuButton class encapsulates the current information for a configured soft button.
  \ingroup QtopiaServer

  QSoftMenuBarProvider::MenuButton instances can only be created and returned
  from a QSoftMenuBarProvider instance.
 */
// define QSoftMenuBarProvider::MenuButton

/*!
  Create a new, null MenuButton.
  */
QSoftMenuBarProvider::MenuButton::MenuButton()
: d(new QSoftMenuBarProvider::MenuButtonPrivate)
{
}

/*!
  Create a copy of \a other.
  */
QSoftMenuBarProvider::MenuButton::MenuButton(const MenuButton &other)
: d(other.d)
{
}

/*!
  Assign \a other to this instance.
  */
QSoftMenuBarProvider::MenuButton &QSoftMenuBarProvider::MenuButton::operator=(const MenuButton &other)
{
    d = other.d;
    return *this;
}

/*!
  Destroys the MenuButton instance.
  */
QSoftMenuBarProvider::MenuButton::~MenuButton()
{
}

/*!
  Return the index of this button within the list of configured buttons.  For
  example,

  \code
  QSoftMenuBarProvider prov;
  prov.key(2).index() == 2;
  \endcode

  For null buttons, the return value will be -1.
 */
int QSoftMenuBarProvider::MenuButton::index() const
{
    return d->index;
}

/*!
  Return the Qt key code for the key this soft button is configured for.
  */
int QSoftMenuBarProvider::MenuButton::key() const
{
    return d->key;
}

/*!
  Return the current text to display for this soft button.  The text may be
  empty.
  */
QString QSoftMenuBarProvider::MenuButton::text() const
{
    return d->text;
}

/*!
  Return the current pixmap to display for this soft button, scaled to the
  current style's small icon size metric.  The pixmap may be null.
  */
QPixmap QSoftMenuBarProvider::MenuButton::pixmap() const
{
    if(d->pixStale) {
        QPixmap pix;
        if(!QPixmapCache::find(d->pixName, pix)) {
            pix = QIcon(":icon/"+d->pixName).pixmap(QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize));
            QPixmapCache::insert(d->pixName, pix);
        }
        d->pix = pix;
        d->pixStale = false;
    }

    return d->pix;
}

/*!
  Return the name of the pixmap to display for this soft button.  This will be
  the same as that passed to QSoftMenuBar::setLabel() and is only provided for
  completeness.  Generally applications should use pixmap() to retrieve the
  pixmap directly.
  */
QString QSoftMenuBarProvider::MenuButton::pixmapName() const
{
    return d->pixName;
}

/*!
  \class QSoftMenuBarProvider
  \brief The QSoftMenuBarProvider class provides the backend for the QSoftMenuBar API.
  \ingroup QtopiaServer

  The QSoftMenuBar class allows applications to control a context specific soft
  key bar.  The QSoftMenuBarProvider manages the state of the soft bar on behalf
  of a specific UI implementation.

  Internally the QSoftMenuBar class sends QCop messages to communicate with
  the QSoftMenuBarProvider instance.  The "context specific" aspects of the
  bar are managed in the QSoftMenuBar instance and messages are only sent when
  the bar UI needs to update.  Three QCop messages are used on the
  \c {QPE/QSoftMenuBar} channel.

  \table
  \header \o Message \o Description
  \row \o \c {setLabelText(int,int,QString)}
       \o \i {setLabelText(int winId, int key, QString text)}

       Sets the button bound to \i {key} to \i {text}.  \i {winId} is the
       integer identifier of the active window from the application's
       perspective.  This is to resolve the race condition where application
       \c {A} sends an update, application \c {B} becomes active and sends its
       update but the provider receives application \c {A}'s update after
       application \c {B}'s incorrectly applying it.  The label will only be
       updated if \i {winId} is equal to the currently active windows id.

  \row \o \c {setLabelPixmap(int,int,QString)}
       \o \i {setLabelPixmap(int winId, int key, QString pixmapName)}

       Sets the button for \i {key} to display the given \i {pixmapName}.
       \i {winId} is used to resolve the race condition outlined above.  The
       pixmap returned from MenuButton::pixmap() will be resolved as
       \c {":icon/" + pixmapName}.

  \row \o \c {clearLabel(int,int)}
       \o \i {clearLabel(int winId, int key)}

       Clears the button for \i {key}.  The button text will be set to a null
       string, and the image a null image.  \i {winId} is used to resolve the
       race condition outlined above.
  \endtable

  The soft keys available on a device are configured in the \c {SoftKeys} group
  of the \c {defaultbuttons.conf} configuration file.  For example, the
  following snippet configures three softkeys - \c {Context1}, \c {Select} and
  \c {Back}.

  \code
  [SoftKeys]
  Count=3
  Key0=Context1
  Key1=Select
  Key2=Back
  \endcode

  Valid key entries are those understood by the QKeySequence class.  The
  QSoftMenuBarProvider class will only respond to keys configured in this way.
 */

// define QSoftMenuBarProvider
/*!
  Create a new QSoftMenuBarProvider instance, with the specified \a parent.
 */
QSoftMenuBarProvider::QSoftMenuBarProvider(QObject *parent)
: QObject(parent)
{
    d = new QSoftMenuBarProviderPrivate;

    QtopiaChannel *channel = new QtopiaChannel("QPE/QSoftMenuBar", this);
    QObject::connect(channel,
                     SIGNAL(received(const QString&,const QByteArray&)),
                     this, SLOT(message(const QString&,const QByteArray&)));

    QSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat);
    cfg.beginGroup("SoftKeys");
    int buttonCount = cfg.value("Count", 0).toInt();

    for(int ii = 0; ii < buttonCount; ++ii) {
        int key =
            QKeySequence(cfg.value("Key" + QString::number(ii)).toString())[0];
        MenuButton button;
        button.d->index = ii;
        button.d->key = key;
        d->buttons.append(button);
    }
    cfg.endGroup();

    //swap menu/back key (when RTL)
    if ( QApplication::layoutDirection() == Qt::RightToLeft ) {
        int backIdx = d->keyToIdx( Qt::Key_Back );
        int menuIdx = d->keyToIdx( QSoftMenuBar::menuKey() );
        d->buttons[backIdx].d->index = menuIdx;
        d->buttons[menuIdx].d->index = backIdx;
        d->buttons.swap( backIdx, menuIdx );
    }

    WindowManagement *man = new WindowManagement(this);
    QObject::connect(man, SIGNAL(windowActive(QString,QRect,QWSWindow *)),
                     this, SLOT(activeChanged(QString,QRect,QWSWindow *)));
}

/*!
  Destroys the QSoftMenuBarProvider instance.
  */
QSoftMenuBarProvider::~QSoftMenuBarProvider()
{
    delete d;
    d = 0;
}

/*!
  Returns the number of configured keys.
  */
int QSoftMenuBarProvider::keyCount() const
{
    return d->buttons.count();
}

/*!
  Returns a key description for the \a index 'th key.  \a index must be less
  than keyCount().
 */
QSoftMenuBarProvider::MenuButton QSoftMenuBarProvider::key(int index) const
{
    Q_ASSERT(index < keyCount());
    return d->buttons.at(index);
}

/*!
  Returns a list of key descriptions for all configured keys.
 */
QList<QSoftMenuBarProvider::MenuButton> QSoftMenuBarProvider::keys() const
{
    return d->buttons;
}

/*! \internal */
void QSoftMenuBarProvider::message(const QString &msg, const QByteArray &data)
{
    QDataStream stream(data);
    if (msg == "setLabelText(int,int,QString)") {
        int btn;
        int win;
        QString label;
        stream >> win;
        if(win == d->activeWin) {
            stream >> btn;
            stream >> label;

            int idx = d->keyToIdx(btn);
            if(-1 != idx) {
                d->buttons[idx].d->text = label;
                d->buttons[idx].d->pix = QPixmap();
                d->buttons[idx].d->pixName = QString();
                d->buttons[idx].d->pixStale = false;
                if(!d->blockUpdates)
                    emit keyChanged(key(idx));
            }
        }
    } else if (msg == "setLabelPixmap(int,int,QString)") {
        int btn;
        int win;
        QString label;
        stream >> win;
        if(win == d->activeWin) {
            stream >> btn;
            stream >> label;

            int idx = d->keyToIdx(btn);
            if(-1 != idx) {
                d->buttons[idx].d->text = QString();
                d->buttons[idx].d->pix = QPixmap();
                d->buttons[idx].d->pixName = label;
                d->buttons[idx].d->pixStale = true;
                if(!d->blockUpdates)
                    emit keyChanged(key(idx));
            }
        }
    } else if (msg == "clearLabel(int,int)") {
        int btn;
        int win;
        stream >> win;
        if(win == d->activeWin) {
            stream >> btn;

            int idx = d->keyToIdx(btn);
            if(-1 != idx) {
                d->buttons[idx].d->text = QString();
                d->buttons[idx].d->pix = QPixmap();
                d->buttons[idx].d->pixName = QString();
                d->buttons[idx].d->pixStale = false;

                if(!d->blockUpdates)
                    emit keyChanged(key(idx));
            }
        }
    } else if (msg == "blockUpdates(int)") {
        // Very dangerous.  Undocumented for good reason.  User apps should
        // never need this.
        int block;
        stream >> block;

        if(d->blockUpdates && !block) {
            d->blockUpdates = false;
            for(int ii = 0; ii < d->buttons.count(); ++ii)
                emit keyChanged(key(ii));
        } else {
            d->blockUpdates = block;
        }
    }
}

/*!  \internal */
void QSoftMenuBarProvider::activeChanged(const QString &, const QRect &, QWSWindow *win)
{
    d->activeWin = win->winId();
}

/*!
  \fn void QSoftMenuBarProvider::keyChanged(const QSoftMenuBarProvider::MenuButton &button)

  Emitted whenever the text or image for \a button has changed.
 */
