/****************************************************************************
**
** Copyright (C) 2000-2008 TROLLTECH ASA. All rights reserved.
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

#include "genericviewer.h"
#include "attachmentoptions.h"
#include "browser.h"
#include <qtopiaservices.h>
#include <qprintingsupport.h>
#include <QAction>
#include <QMailMessage>
#include <QMenu>
#include <QtopiaApplication>

#include <qtopialog.h>

GenericViewer::GenericViewer( QWidget* parent )
    : QMailViewerInterface( parent ),
      browser( new Browser(parent) ),
      printAction( 0 ),
      message( 0 ),
      plainTextMode( false ),
      containsNumbers( false )
{
    connect(browser, SIGNAL(anchorClicked(QUrl)), this, SLOT(linkClicked(QUrl)));
    connect(browser, SIGNAL(highlighted(QUrl)), this, SLOT(linkHighlighted(QUrl)));
    connect(browser, SIGNAL(finished()), this, SIGNAL(finished()));

    plainTextModeAction = new QAction(QIcon(":icon/txt"), tr("Plain text"), this);
    plainTextModeAction->setVisible(!plainTextMode);
    plainTextModeAction->setWhatsThis(tr("Display the message contents in Plain text format."));

    richTextModeAction = new QAction(QIcon(":icon/txt"), tr("Rich text"), this);
    richTextModeAction->setVisible(plainTextMode);
    richTextModeAction->setWhatsThis(tr("Display the message contents in Rich text format."));

    if (QPrintingSupport::hasPrintingSupport()) {
        printAction = new QAction(QIcon(":icon/print"), tr("Print"), this);
        printAction->setVisible(false);
        printAction->setWhatsThis(tr("Print the message contents."));
    }

    dialAction = new QAction(this);
    dialAction->setVisible(false);
}

GenericViewer::~GenericViewer()
{
}

void GenericViewer::scrollToAnchor(const QString& a)
{
    browser->scrollToAnchor(a);
}

QWidget* GenericViewer::widget() const
{
    return browser;
}

void GenericViewer::addActions(QMenu* menu) const
{
    // Ensure we don't receive menu events multiple times
    disconnect(menu, SIGNAL(triggered(QAction*)),
               this, SLOT(action(QAction*)));

    menu->addAction(plainTextModeAction);
    menu->addAction(richTextModeAction);

    if (printAction)
        menu->addAction(printAction);

    if (containsNumbers) {
        menu->addSeparator();
        menu->addAction(dialAction);
    }

    connect(menu, SIGNAL(triggered(QAction*)),
            this, SLOT(action(QAction*)));
}

bool GenericViewer::setMessage(const QMailMessage& mail)
{
    message = &mail;

    setPlainTextMode(plainTextMode);
    if (printAction)
        printAction->setVisible(true);

    containsNumbers = !browser->embeddedNumbers().isEmpty();
    dialAction->setVisible(false);

    return true;
}

void GenericViewer::setResource(const QUrl& name, QVariant var)
{
    browser->setResource(name, var);
}

void GenericViewer::clear()
{
    plainTextMode = false;
    browser->setPlainText("");
    browser->clearResources();
}

void GenericViewer::action(QAction* action)
{
    if (action == plainTextModeAction) {
        setPlainTextMode(true);
    } else if (action == richTextModeAction) {
        setPlainTextMode(false);
    } else if (printAction && action == printAction) {
        print();
    } else if (action == dialAction) {
        emit anchorClicked(action->data().toString());
    }
}

void GenericViewer::setPlainTextMode(bool plainTextMode)
{
    this->plainTextMode = plainTextMode;

    browser->setMessage(*message, plainTextMode);

    plainTextModeAction->setVisible(!plainTextMode && message->messageType() != QMailMessage::Mms);
    richTextModeAction->setVisible(plainTextMode);
}

void GenericViewer::print() const
{
    QtopiaServiceRequest srv2( "Print", "printHtml(QString)" );
    srv2 << browser->toHtml();
    srv2.send();
}

void GenericViewer::linkClicked(const QUrl& link)
{
    QString command = link.toString();

    if (command.startsWith("attachment")) {
        QRegExp splitter("attachment;([^;]+)(?:;(\\d*))?");
        if (splitter.exactMatch(command)) {
            QString cmd = splitter.cap(1);
            QString number = splitter.cap(2);
            if (!number.isEmpty()) {
                uint partNumber = number.toUInt();

                // Show the attachment dialog
                AttachmentOptions options(widget());

                // TODO: either part-detaching is permitted on a const part, or we will
                // have to accept only non-const messages to the viewer...
                QMailMessagePart& part = const_cast<QMailMessagePart&>(message->partAt(partNumber));
                options.setAttachment(part);
                QtopiaApplication::execDialog(&options);
                return;
            }
        }
    }

    emit anchorClicked(link);
}

void GenericViewer::linkHighlighted(const QUrl& link)
{
    QString command = link.toString();

    if (command.startsWith("dial")) {
        QRegExp splitter("dial;(.+)");
        if (splitter.exactMatch(command)) {
            QString number = splitter.cap(1);
            if (!number.isEmpty()) {
                dialAction->setText(tr("Dial %1", "%1=number").arg(number));
                dialAction->setData(QVariant(QString("dial;%1").arg(number)));
                dialAction->setVisible(true);
                return;
            }
        }
    }

    dialAction->setVisible(false);
}


QTOPIA_EXPORT_PLUGIN( GenericViewerPlugin )

GenericViewerPlugin::GenericViewerPlugin()
    : QMailViewerPlugin()
{
}

QString GenericViewerPlugin::key() const
{
    return "GenericViewer";
}

bool GenericViewerPlugin::isSupported( QMailViewerFactory::ContentType type ) const
{
    return ( type == QMailViewerFactory::StaticContent || type == QMailViewerFactory::AnyContent );
}

QMailViewerInterface* GenericViewerPlugin::create( QWidget *parent )
{
    return new GenericViewer( parent );
}

