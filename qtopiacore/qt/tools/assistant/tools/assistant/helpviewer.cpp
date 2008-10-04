/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Assistant of the Qt Toolkit.
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
** http://www.gnu.org/copyleft/gpl.html.  In addition, as a special
** exception, Nokia gives you certain additional rights. These rights
** are described in the Nokia Qt GPL Exception version 1.2, included in
** the file GPL_EXCEPTION.txt in this package.
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

#include "helpviewer.h"
#include "centralwidget.h"

#include <QtCore/QEvent>
#include <QtCore/QVariant>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>
#include <QtCore/QTimer>

#include <QtGui/QMenu>
#include <QtGui/QKeyEvent>
#include <QtGui/QClipboard>
#include <QtGui/QApplication>
#include <QtGui/QMessageBox>
#include <QtGui/QDesktopServices>

#include <QtHelp/QHelpEngine>

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

QT_BEGIN_NAMESPACE

#if defined(USE_WEBKIT)

class HelpNetworkReply : public QNetworkReply
{
public:
    HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData);

    virtual void abort();

    virtual qint64 bytesAvailable() const { return data.length() + QNetworkReply::bytesAvailable(); }

protected:
    virtual qint64 readData(char *data, qint64 maxlen);

private:
    QByteArray data;
    qint64 origLen;
};

HelpNetworkReply::HelpNetworkReply(const QNetworkRequest &request, const QByteArray &fileData)
    : data(fileData), origLen(fileData.length())
{
    setRequest(request);
    setOpenMode(QIODevice::ReadOnly);

    setHeader(QNetworkRequest::ContentTypeHeader, "text/html");
    setHeader(QNetworkRequest::ContentLengthHeader, QByteArray::number(fileData.length()));
    QTimer::singleShot(0, this, SIGNAL(metaDataChanged()));
    QTimer::singleShot(0, this, SIGNAL(readyRead()));
}

void HelpNetworkReply::abort()
{
    // nothing to do
}

qint64 HelpNetworkReply::readData(char *buffer, qint64 maxlen)
{
    qint64 len = qMin(qint64(data.length()), maxlen);
    if (len) {
        qMemCopy(buffer, data.constData(), len);
        data.remove(0, len);
    }
    if (!data.length())
        QTimer::singleShot(0, this, SIGNAL(finished()));
    return len;
}

class HelpNetworkAccessManager : public QNetworkAccessManager
{
public:
    HelpNetworkAccessManager(QHelpEngine *engine, QObject *parent);

protected:
    virtual QNetworkReply *createRequest(Operation op, const QNetworkRequest &request,
                                         QIODevice *outgoingData = 0);

private:
    QHelpEngine *helpEngine;
};

HelpNetworkAccessManager::HelpNetworkAccessManager(QHelpEngine *engine, QObject *parent)
    : QNetworkAccessManager(parent), helpEngine(engine)
{
}

QNetworkReply *HelpNetworkAccessManager::createRequest(Operation op, const QNetworkRequest &request,
                                                       QIODevice *outgoingData)
{
    const QString scheme = request.url().scheme();
    if (scheme == QLatin1String("qthelp") || scheme == QLatin1String("about")) {
        return new HelpNetworkReply(request, helpEngine->fileData(request.url()));
    }
    return QNetworkAccessManager::createRequest(op, request, outgoingData);
}

class HelpPage : public QWebPage
{
public:
    HelpPage(CentralWidget *central, QObject *parent);

protected:
    virtual QWebPage *createWindow(QWebPage::WebWindowType);

    virtual bool acceptNavigationRequest(QWebFrame *frame, const QNetworkRequest &request, NavigationType type);

private:
    CentralWidget *centralWidget;
};

HelpPage::HelpPage(CentralWidget *central, QObject *parent)
    : QWebPage(parent), centralWidget(central)
{
}

QWebPage *HelpPage::createWindow(QWebPage::WebWindowType)
{
    return centralWidget->newEmptyTab()->page();
}

static bool isLocalUrl(const QUrl &url)
{
    const QString scheme = url.scheme();
    if (scheme.isEmpty()
        || scheme == QLatin1String("file")
        || scheme == QLatin1String("qrc")
        || scheme == QLatin1String("data")
        || scheme == QLatin1String("qthelp")
        || scheme == QLatin1String("about"))
        return true;
    return false;
}

bool HelpPage::acceptNavigationRequest(QWebFrame *, const QNetworkRequest &request, QWebPage::NavigationType)
{
    const QUrl url = request.url();
    if (isLocalUrl(url)) {
        return true;
    } else {
        QDesktopServices::openUrl(url);
        return false;
    }
}

HelpViewer::HelpViewer(QHelpEngine *engine, CentralWidget *parent)
    : QWebView(parent), helpEngine(engine), parentWidget(parent)
{
    setPage(new HelpPage(parent, this));

    page()->setNetworkAccessManager(new HelpNetworkAccessManager(engine, this));

    pageAction(QWebPage::OpenLinkInNewWindow)->setText(tr("Open Link in New Tab"));
    pageAction(QWebPage::DownloadLinkToDisk)->setVisible(false);
    pageAction(QWebPage::DownloadImageToDisk)->setVisible(false);
    pageAction(QWebPage::OpenImageInNewWindow)->setVisible(false);

    connect(pageAction(QWebPage::Copy), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(pageAction(QWebPage::Back), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(pageAction(QWebPage::Forward), SIGNAL(changed()), this, SLOT(actionChanged()));
    connect(page(), SIGNAL(linkHovered(const QString &, const QString &, const QString &)), this, SIGNAL(highlighted(const QString &)));
    connect(this, SIGNAL(urlChanged(const QUrl &)), this, SIGNAL(sourceChanged(const QUrl &)));
}

void HelpViewer::setSource(const QUrl &url)
{
    if (!homeUrl.isValid())
        homeUrl = url;
    load(url);
}

void HelpViewer::resetZoom()
{
    setTextSizeMultiplier(1.0);
}

void HelpViewer::zoomIn(int /*range*/)
{
    setTextSizeMultiplier(textSizeMultiplier() + .5);
}

void HelpViewer::zoomOut(int /*range*/)
{
    setTextSizeMultiplier(textSizeMultiplier() - .5);
}

void HelpViewer::home()
{
    if (homeUrl.isValid())
        setSource(homeUrl);
}

void HelpViewer::wheelEvent(QWheelEvent *e)
{
    if (e->modifiers() & Qt::ControlModifier) {
        const int delta = e->delta();
        if (delta > 0)
            zoomOut();
        else if (delta < 0)
            zoomIn();
        e->accept();
        return;
    }
    QWebView::wheelEvent(e);
}

void HelpViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::XButton1) {
        triggerPageAction(QWebPage::Back);
        return;
    }

    if (e->button() == Qt::XButton2) {
        triggerPageAction(QWebPage::Forward);
        return;
    }

    QWebView::mouseReleaseEvent(e);
}

void HelpViewer::actionChanged()
{
    QAction *a = qobject_cast<QAction *>(sender());
    if (a == pageAction(QWebPage::Copy))
        emit copyAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Back))
        emit backwardAvailable(a->isEnabled());
    else if (a == pageAction(QWebPage::Forward))
        emit backwardAvailable(a->isEnabled());
}

#else   // !defined(USE_WEBKIT)

HelpViewer::HelpViewer(QHelpEngine *engine, CentralWidget *parent)
    : QTextBrowser(parent)
    , zoomCount(0)
    , controlPressed(false)
    , lastAnchor(QString())
    , helpEngine(engine)
    , parentWidget(parent)
{
    // nothing todo
}

void HelpViewer::setSource(const QUrl &url)
{
    bool help = url.toString() == QLatin1String("help");
    if (url.isValid() && !help) {
        if (url.scheme() == QLatin1String("http") || url.scheme() == QLatin1String("ftp") 
            || url.scheme() == QLatin1String("mailto") || url.path().endsWith(QLatin1String("pdf"))) {
            bool launched = QDesktopServices::openUrl(url);
            if (!launched) {
                QMessageBox::information(this, tr("Help"),
                             tr("Unable to launch external application.\n"),
                             tr("OK"));
            }
            return;
        } else {
            QUrl u = helpEngine->findFile(url);
            if (u.isValid()) {
                QTextBrowser::setSource(u);
                return;
            }
        }
    }

    if (help) {
        QTextBrowser::setSource(QUrl(
            QLatin1String("qthelp://com.trolltech.com.assistantinternal_1.0.0/assistant/assistant.html")));
    } else {
        QTextBrowser::setSource(url);
        setHtml(tr("<title>Error 404...</title><div align=\"center\"><br><br><h1>"
            "The page could not be found</h1><br><h3>'%1'</h3></div>").arg(url.toString()));
        emit sourceChanged(url);
    }
}

void HelpViewer::resetZoom()
{
    if (zoomCount == 0)
        return;
    
    QTextBrowser::zoomOut(zoomCount);
    zoomCount = 0;
}

void HelpViewer::zoomIn(int range)
{
    if (zoomCount == 10)
        return;

    QTextBrowser::zoomIn(range); 
    zoomCount++;
}

void HelpViewer::zoomOut(int range)
{
    if (zoomCount == -5)
        return;

    QTextBrowser::zoomOut(range);
    zoomCount--;
}

QVariant HelpViewer::loadResource(int type, const QUrl &name)
{
    QByteArray ba;
    if (type < 4) {
        ba = helpEngine->fileData(name);    
        if (name.toString().endsWith(QLatin1String(".svg"), Qt::CaseInsensitive)) {
            QImage image;
            image.loadFromData(ba, "svg");
            if (!image.isNull())
                return image;
        }
    }
    return ba;
}

void HelpViewer::openLinkInNewTab()
{
    if(lastAnchor.isEmpty())
        return;

    parentWidget->setSourceInNewTab(QUrl(lastAnchor));
    lastAnchor.clear();
}

void HelpViewer::openLinkInNewTab(const QString &link)
{
    lastAnchor = link;
    openLinkInNewTab();
}

bool HelpViewer::hasAnchorAt(const QPoint& pos)
{
    lastAnchor = anchorAt(pos);
    if (lastAnchor.isEmpty()) 
        return false;

    lastAnchor = source().resolved(lastAnchor).toString();
    if (lastAnchor.at(0) == QLatin1Char('#')) {
        QString src = source().toString();
        int hsh = src.indexOf(QLatin1Char('#'));
        lastAnchor = (hsh>=0 ? src.left(hsh) : src) + lastAnchor;
    }

    return true;
}

void HelpViewer::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu menu(QLatin1String(""), 0);

    QUrl link;
    QAction *copyAnchorAction = 0;
    if (hasAnchorAt(e->pos())) {
        link = anchorAt(e->pos());
        if (link.isRelative())            
            link = source().resolved(link);
        copyAnchorAction = menu.addAction(tr("Copy &Link Location"));
        copyAnchorAction->setEnabled(!link.isEmpty() && link.isValid());

        menu.addAction(tr("Open Link in New Tab\tCtrl+LMB"), this, SLOT(openLinkInNewTab()));
        menu.addSeparator();
    }
    menu.addActions(parentWidget->globalActions());
    QAction *action = menu.exec(e->globalPos());
    if (action == copyAnchorAction)
        QApplication::clipboard()->setText(link.toString());
}

void HelpViewer::mouseReleaseEvent(QMouseEvent *e)
{
    if (e->button() == Qt::XButton1) {
        QTextBrowser::backward();
        return;
    }

    if (e->button() == Qt::XButton2) {
        QTextBrowser::forward();
        return;
    }

    controlPressed = e->modifiers() & Qt::ControlModifier;
    if ((controlPressed && hasAnchorAt(e->pos())) ||
        (e->button() == Qt::MidButton && hasAnchorAt(e->pos()))) {
        openLinkInNewTab();
        return;
    }

    QTextBrowser::mouseReleaseEvent(e);
}

#endif  // !defined(USE_WEBKIT)

QT_END_NAMESPACE
