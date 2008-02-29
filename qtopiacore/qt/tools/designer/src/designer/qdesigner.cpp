/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <QtGui/QFileOpenEvent>
#include <QtGui/QCloseEvent>
#include <QtGui/QMessageBox>
#include <QtCore/QMetaObject>
#include <QtCore/QFile>
#include <QtCore/QLibraryInfo>
#include <QtCore/QLocale>
#include <QtCore/QTimer>
#include <QtCore/QTranslator>
#include <QtCore/qdebug.h>

#include <QtDesigner/QDesignerComponents>

// designer
#include "qdesigner.h"
#include "qdesigner_actions.h"
#include "qdesigner_server.h"
#include "qdesigner_settings.h"
#include "qdesigner_workbench.h"
#include "qdesigner_toolwindow.h"

QDesigner::QDesigner(int &argc, char **argv)
    : QApplication(argc, argv),
      m_server(0),
      m_client(0),
      m_workbench(0), suppressNewFormShow(false)
{
    setOrganizationName(QLatin1String("Trolltech"));
    setApplicationName(QLatin1String("Designer"));
    QDesignerComponents::initializeResources();

#ifndef Q_WS_MAC
    setWindowIcon(QIcon(QLatin1String(":/trolltech/designer/images/designer.png")));
#endif

    initialize();
}

QDesigner::~QDesigner()
{
    if (m_workbench)
        delete m_workbench;
    if (m_server)
        delete m_server;
    if (m_client)
        delete m_client;
}

QDesignerWorkbench *QDesigner::workbench() const
{
    return m_workbench;
}

QDesignerServer *QDesigner::server() const
{
    return m_server;
}

void QDesigner::initialize()
{
    // initialize the sub components
    QStringList files;

    QString resourceDir = QLibraryInfo::location(QLibraryInfo::TranslationsPath);

    for (int i = 1; i < argc(); ++i)
    {
        if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-server")) {
            m_server = new QDesignerServer();
            printf("%d\n", m_server->serverPort());
            fflush(stdout);
        } else if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-client")) {
            bool ok = true;
            if (i + 1 < argc()) {
                quint16 port = QString::fromLocal8Bit(argv()[++i]).toUShort(&ok);
                if (ok)
                    m_client = new QDesignerClient(port, this);
            }
        } else if (QString::fromLocal8Bit(argv()[i]) == QLatin1String("-resourcedir")) {
            if (i + 1 < argc()) {
                resourceDir = QFile::decodeName(argv()[++i]);
            } else {
                // issue a warning
            }
        } else {
            files.append(QString::fromLocal8Bit(argv()[i]));
        }
    }

    QTranslator *translator = new QTranslator;
    QTranslator *qtTranslator = new QTranslator;
    translator->load(QLatin1String("designer_") + QLocale::system().name().toLower(), resourceDir);
    qtTranslator->load(QLatin1String("qt_") + QLocale::system().name().toLower(), resourceDir);
    installTranslator(translator);
    installTranslator(qtTranslator);

    if (QLibraryInfo::licensedProducts() == QLatin1String("Console")) {
        QMessageBox::information(0, tr("Qt Designer"),
                tr("This application cannot be used for the Console edition of Qt"));
        QMetaObject::invokeMethod(this, "quit", Qt::QueuedConnection);
        return;
    }

    m_workbench = new QDesignerWorkbench();

    emit initialized();

    foreach (QString file, files) {
        if (m_workbench->readInForm(file) && !suppressNewFormShow)
            suppressNewFormShow = true;
    }
    if (QDesignerSettings().showNewFormOnStartup())
        QTimer::singleShot(100, this, SLOT(callCreateForm())); // won't show anything if suppressed
}

bool QDesigner::event(QEvent *ev)
{
    bool eaten;
    switch (ev->type()) {
    case QEvent::FileOpen:
        // set it true first since, if it's a Qt 3 form, the messagebox from convert will fire the timer.
        suppressNewFormShow = true;
        if (!m_workbench->readInForm(static_cast<QFileOpenEvent *>(ev)->file()))
            suppressNewFormShow = false;
        eaten = true;
        break;
    case QEvent::Close: {
        QCloseEvent *closeEvent = static_cast<QCloseEvent *>(ev);
        closeEvent->setAccepted(m_workbench->handleClose());
        if (closeEvent->isAccepted()) {
            // We're going down, make sure that we don't get our settings saved twice.
            if (m_mainWindow)
                m_mainWindow->setSaveSettingsOnClose(false);
            eaten = QApplication::event(ev);
        }
        eaten = true;
        break;
    }
    default:
        eaten = QApplication::event(ev);
        break;
    }
    return eaten;
}

void QDesigner::setMainWindow(QDesignerToolWindow *tw)
{
    m_mainWindow = tw;
}

QDesignerToolWindow *QDesigner::mainWindow() const
{
    return m_mainWindow;
}

void QDesigner::callCreateForm()
{
    if (!suppressNewFormShow)
        m_workbench->actionManager()->createForm();
}
