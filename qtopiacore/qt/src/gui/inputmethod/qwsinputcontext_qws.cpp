/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
**
** Commercial Usage
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License versions 2.0 or 3.0 as published by the Free
** Software Foundation and appearing in the file LICENSE.GPL included in
** the packaging of this file.  Please review the following information
** to ensure GNU General Public Licensing requirements will be met:
** http://www.fsf.org/licensing/licenses/info/GPLv2.html and
** http://www.gnu.org/copyleft/gpl.html.
**
** Qt for Windows(R) Licensees
** As a special exception, Nokia, as the sole copyright holder for Qt
** Designer, grants users of the Qt/Eclipse Integration plug-in the
** right for the Qt/Eclipse Integration to link to functionality
** provided by Qt Designer and its related libraries.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at qt-sales@nokia.com.
**
****************************************************************************/

#include "qwsinputcontext_p.h"
#include "qinputcontext_p.h"
#include "qwsdisplay_qws.h"
#include "qwsevent_qws.h"
#include "private/qwscommand_qws_p.h"
#include "qwindowsystem_qws.h"
#include "qevent.h"
#include "qtextformat.h"

#include <qbuffer.h>

#include <qdebug.h>

#ifndef QT_NO_QWS_INPUTMETHODS

QT_BEGIN_NAMESPACE

static QWidget* activeWidget = 0;

//#define EXTRA_DEBUG

QWSInputContext::QWSInputContext(QObject *parent)
    :QInputContext(parent)
{
}

void QWSInputContext::reset()
{
    QPaintDevice::qwsDisplay()->resetIM();
}


void QWSInputContext::setFocusWidget( QWidget *w )
{
  QWidget *oldFocus = focusWidget();
    if (oldFocus == w)
        return;

    if (oldFocus) {
        QWidget *tlw = oldFocus->window();
        int winid = tlw->internalWinId();

        int widgetid = oldFocus->internalWinId();
        QPaintDevice::qwsDisplay()->sendIMUpdate(QWSInputMethod::FocusOut, winid, widgetid);
    }

    QInputContext::setFocusWidget(w);

    if (!w)
        return;

    QWidget *tlw = w->window();
    int winid = tlw->winId();

    int widgetid = w->winId();
    QPaintDevice::qwsDisplay()->sendIMUpdate(QWSInputMethod::FocusIn, winid, widgetid);

    //setfocus ???

    update();
}


void QWSInputContext::widgetDestroyed(QWidget *w)
{
    if (w == ::activeWidget)
        ::activeWidget = 0;
    QInputContext::widgetDestroyed(w);
}

void QWSInputContext::update()
{
    QWidget *w = focusWidget();
    if (!w)
        return;

    QWidget *tlw = w->window();
    int winid = tlw->winId();

    int widgetid = w->winId();
    QPaintDevice::qwsDisplay()->sendIMUpdate(QWSInputMethod::Update, winid, widgetid);

}

void QWSInputContext::mouseHandler( int x, QMouseEvent *event)
{
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease)
        QPaintDevice::qwsDisplay()->sendIMMouseEvent( x, event->type() == QEvent::MouseButtonPress );
}

QWidget *QWSInputContext::activeWidget()
{
    return ::activeWidget;
}


bool QWSInputContext::isComposing() const
{
    return ::activeWidget != 0;
}

bool QWSInputContext::translateIMQueryEvent(QWidget *w, const QWSIMQueryEvent *e)
{
    Qt::InputMethodQuery type = static_cast<Qt::InputMethodQuery>(e->simpleData.property);
    QVariant result = w->inputMethodQuery(type);
    QWidget *tlw = w->window();
    int winId = tlw->winId();

    if ( type == Qt::ImMicroFocus ) {
        // translate to relative to tlw
        QRect mf = result.toRect();
        mf.moveTopLeft(w->mapTo(tlw,mf.topLeft()));
        result = mf;
    }

    QPaintDevice::qwsDisplay()->sendIMResponse(winId, e->simpleData.property, result);

    return false;
}

bool QWSInputContext::translateIMInitEvent(const QWSIMInitEvent *e)
{
    Q_UNUSED(e);
    qDebug("### QWSInputContext::translateIMInitEvent not implemented ###");
    return false;
}

bool QWSInputContext::translateIMEvent(QWidget *w, const QWSIMEvent *e)
{
    QDataStream stream(e->streamingData);
    QString preedit;
    QString commit;

    stream >> preedit;
    stream >> commit;

    if (preedit.isEmpty() && ::activeWidget)
        w = ::activeWidget;

    QInputContext *qic = w->inputContext();
    if (!qic)
        return false;

    QList<QInputMethodEvent::Attribute> attrs;


    while (!stream.atEnd()) {
        int type = -1;
        int start = -1;
        int length = -1;
        QVariant data;
        stream >> type >> start >> length >> data;
        if (stream.status() != QDataStream::Ok) {
            qWarning("corrupted QWSIMEvent");
            //qic->reset(); //???
            return false;
        }
        if (type == QInputMethodEvent::TextFormat)
            data = qic->standardFormat(static_cast<QInputContext::StandardFormat>(data.toInt()));
        attrs << QInputMethodEvent::Attribute(static_cast<QInputMethodEvent::AttributeType>(type), start, length, data);
    }
#ifdef EXTRA_DEBUG
    qDebug() << "preedit" << preedit << "len" << preedit.length() <<"commit" << commit << "len" << commit.length()
             << "n attr" << attrs.count();
#endif

    if (preedit.isEmpty())
        ::activeWidget = 0;
    else
        ::activeWidget = w;


    QInputMethodEvent ime(preedit, attrs);
    if (!commit.isEmpty() || e->simpleData.replaceLength > 0)
        ime.setCommitString(commit, e->simpleData.replaceFrom, e->simpleData.replaceLength);


    extern bool qt_sendSpontaneousEvent(QObject *, QEvent *); //qapplication_qws.cpp
    qt_sendSpontaneousEvent(w, &ime);

    return true;
}

Q_GUI_EXPORT void (*qt_qws_inputMethodStatusChanged)(QWidget*) = 0;

void QInputContextPrivate::updateImeStatus(QWidget *w, bool hasFocus)
{
    Q_UNUSED(hasFocus);

    if (!w || !qt_qws_inputMethodStatusChanged)
        return;
    qt_qws_inputMethodStatusChanged(w);
}


QT_END_NAMESPACE

#endif // QT_NO_QWS_INPUTMETHODS
