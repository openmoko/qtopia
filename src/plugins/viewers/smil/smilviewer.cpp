/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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

#include "smilviewer.h"

#include <qtopia/smil/element.h>
#include <qtopia/smil/smil.h>
#include <qtopia/smil/timing.h>
#include <qtopia/smil/transfer.h>
#include <qtopialog.h>

#include <QBuffer>
#include <QKeyEvent>
#include <QMailMessage>

SmilViewer::SmilViewer( QWidget* parent )
    : QMailViewerInterface( parent ),
      view( new SmilView( parent ) ),
      mail( 0 )
{
    view->setGeometry(parent->rect());
    view->installEventFilter(this);
    view->setFocusPolicy(Qt::StrongFocus);

    connect(view, SIGNAL(transferRequested(SmilDataSource*,QString)),
            this, SLOT(requestTransfer(SmilDataSource*,QString)));
    connect(view, SIGNAL(transferCancelled(SmilDataSource*,QString)),
            this, SLOT(cancelTransfer(SmilDataSource*,QString)));

    connect(view, SIGNAL(finished()),
            this, SIGNAL(finished()));
}

SmilViewer::~SmilViewer()
{
}

QWidget* SmilViewer::widget() const
{
    return view;
}

QString SmilViewer::findStartMarker() const
{
    QMailMessageContentType type(mail->headerField("X-qtmail-internal-original-content-type"));
    if (type.isNull()) {
        type = QMailMessageContentType(mail->headerField("Content-Type"));
    }
    if (!type.isNull()) {
        QString startElement = type.parameter("start");
        if (!startElement.isEmpty())
            return startElement;
    }

    return QString("<presentation-part>");
}

bool SmilViewer::setMessage(const QMailMessage& msg)
{
    mail = &msg;
    QString start = findStartMarker();

    uint smilPartIndex = 0;
    for ( uint i = 1; i < mail->partCount(); i++ ) {
        const QMailMessagePart &part = mail->partAt( i );
        if (part.contentID() == start) {
            smilPartIndex = i;
            break;
        }
    }

    const QMailMessagePart &part = mail->partAt( smilPartIndex );
    QString smil(part.body().data());

    if (view->setSource(smil) && view->rootElement()) {
        tweakView();
        view->play();
        return true;
    }

    return false;
}

bool SmilViewer::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == view && event->type() == QEvent::KeyPress) {
        if (QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event)) {
            if (keyEvent->key() == Qt::Key_Select) {
                advanceSlide();
                return true;
            } else if (keyEvent->key() == Qt::Key_Back) {
                emit finished();
                return true;
            }
        }
    }

    return false;
}

void SmilViewer::clear()
{
    view->reset();

    mail = 0;
    QMap<SmilDataSource*,QIODevice*>::ConstIterator it = transfers.begin(), end = transfers.end();
    for ( ; it != end; ++it)
        (*it)->deleteLater();

    transfers.clear();
}

void SmilViewer::tweakView()
{
    // Try to make sure the layout works on our display
    SmilElement *layout = view->rootElement()->findChild(QString(), "layout", true);
    if (!layout)
        return;

    QRect rl = view->rect();
    SmilElement *rootLayout = layout->findChild(QString(), "root-layout");
    if (rootLayout) {
        if (rootLayout->rect().width() > view->width() || 
            rootLayout->rect().height() > view->height()) {
            rootLayout->setRect(QRect(0, 0, view->width(), view->height()));
        }
        rl = rootLayout->rect();
    }

    SmilElement *imageLayout = layout->findChild("Image", "region");
    if (!imageLayout)
        imageLayout = layout->findChild("image", "region");

    SmilElement *textLayout = layout->findChild("Text", "region");
    if (!textLayout)
        textLayout = layout->findChild("text", "region");

    if (imageLayout && textLayout) {
        QRect il = imageLayout->rect();
        QRect tl = textLayout->rect();
        if (il.bottom() > tl.top() ||
            il.right() > rl.right() ||
            tl.right() > rl.right() ||
            il.bottom() > rl.bottom() ||
            tl.bottom() > rl.bottom()) {
            // Not going to fit - use our preferred sizes.
            il = tl = rl;
            il.setBottom(il.top() + rl.height() * 2 / 3);
            tl.setTop(il.bottom() + 1);
            tl.setHeight(rl.height() - il.height());
            imageLayout->setRect(il);
            textLayout->setRect(tl);
        }
    }
}

void SmilViewer::advanceSlide()
{
    // Try to advance to the next slide
    SmilElement *smil = view->rootElement();
    if (!smil)
        return;

    SmilElement *body = smil->findChild(QString(), "body", true);
    if (!body)
        return;

    SmilElementList::ConstIterator it;
    for (it = body->children().begin(); it != body->children().end(); ++it) {
        SmilElement *e = *it;
        if (e->name() == "par") {
            if (e->state() == SmilElement::Active) {
                // This should be the current active slide
                SmilTimingAttribute *at = static_cast<SmilTimingAttribute*>(e->module("Timing"));
                Duration d(at->startTime.elapsed());    // i.e. end now.
                e->setCurrentEnd(d);
                break;
            }
        }
    }
}

void SmilViewer::requestTransfer(SmilDataSource* dataSource, const QString &src)
{
    bool isId = false;
    QString source = src;
    if (source.startsWith("cid:")) {
        source = source.mid(4);
        isId = true;
    }

    for ( uint i = 0; i < mail->partCount(); i++ ) {
        const QMailMessagePart &part = mail->partAt( i );
        if ((isId && part.contentID() == source) ||
            (part.displayName() == source) ||
            (part.contentLocation() == source)) 
        {
            dataSource->setMimeType(part.contentType().content());

            const QString filename(part.attachmentPath());
            if (filename.isEmpty()) {
                QBuffer *data = new QBuffer();
                data->setData(part.body().data(QMailMessageBody::Decoded));
                data->open(QIODevice::ReadOnly);
                dataSource->setDevice(data);
                transfers[dataSource] = data;
            } else {
                QFile *file = new QFile(filename);
                file->open(QIODevice::ReadOnly);
                dataSource->setDevice(file);
                transfers[dataSource] = file;
            }
            break;
        }
    }
}

void SmilViewer::cancelTransfer(SmilDataSource *dataSource, const QString &src)
{
    if (transfers.contains(dataSource))
        transfers.take(dataSource)->deleteLater();

    Q_UNUSED(src)
}

QTOPIA_EXPORT_PLUGIN( SmilViewerPlugin )

SmilViewerPlugin::SmilViewerPlugin()
    : QMailViewerPlugin()
{
}

QString SmilViewerPlugin::key() const
{
    return "SmilViewer";
}

bool SmilViewerPlugin::isSupported( QMailViewerFactory::ContentType type ) const
{
    return ( type == QMailViewerFactory::SmilContent || type == QMailViewerFactory::AnyContent );
}

QMailViewerInterface* SmilViewerPlugin::create( QWidget *parent )
{
    return new SmilViewer( parent );
}

