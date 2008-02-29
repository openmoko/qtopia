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

#include <QtGui>
#include <QDateTime>
#include <QColorSelector>
#include <QColorSelectorDialog>
#include <QSpinBox>
#include <QDialog>
#include <qmessagebox.h>

#include <qtopia/qtopiaapplication.h>
#include <qtopia/qsoftmenubar.h>
#include <QDocumentSelectorService>
#include <QMimeType>

#include "mainwindow.h"
#include "scribblearea.h"

MainWindow::MainWindow( QWidget *parent , Qt::WFlags f )
        :  QMainWindow( parent, f )
        , selector( 0 )

{
    scribbleArea = new ScribbleArea(this);
    setCentralWidget(scribbleArea);
    createActions();
    createMenus();

    setWindowTitle(tr("Scribble"));

}

void MainWindow::closeEvent(QCloseEvent *event)
{
    if (maybeSave()) {
        event->accept();
    } else {
        event->ignore();
    }
}

void MainWindow::open()
{
    createSelector();

    if (selector->openDocument(this)) {
        QByteArray format = QMimeType::fromId(selector->selectedDocument().type()).extension().toUpper().toLatin1();

        scribbleArea->openImage(selector->selectedDocumentData(), format.constData());
    }
}

bool MainWindow::save()
{
    createSelector();

    QContent document = selector->selectedDocument();

    if (!document.isNull() && saveTypes.contains(document.type()) ) {
        if (selector->saveDocument(this)) {
            QByteArray format = QMimeType::fromId(selector->selectedDocument().type()).extension().toUpper().toLatin1();

            return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
        }
    } else {
        return saveAs();
    }
    return false;
}

bool MainWindow::saveAs()
{
    createSelector();

    QContent document = selector->selectedDocument();

    if (document.isNull()) {
        QString date;
        QDateTime dt = QDateTime::currentDateTime();
        date = dt.toString( );
        date.replace(QRegExp("'"),"");
        date.replace(QRegExp(" "),"");
        date.replace(QRegExp(":"),"");
        date.replace(QRegExp(","),"");

        if (selector->newDocument(date,saveTypes,this)) {
            QByteArray format = QMimeType::fromId( selector->selectedDocument().type() ).extension().toUpper().toLatin1();

            return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
        }
    } else if (selector->newDocument(tr("Copy of %1").arg(document.name()),saveTypes,this)) {
        QByteArray format = QMimeType::fromId( selector->selectedDocument().type() ).extension().toUpper().toLatin1();

        return scribbleArea->saveImage(selector->selectedDocumentData(), format.constData());
    }
    return false;
}

void MainWindow::penColor()
{
  cselect = new QColorSelectorDialog(this);
  connect(cselect,SIGNAL(selected(const QColor &)),
          this,SLOT(colorSelected(const QColor &)));

  cselect->showMaximized();
}

void MainWindow::penWidth()
{
  dialog = new QDialog(this);
  dialog->setWindowTitle(tr("Pen Width"));
  dialog->resize(100,75);

  spinbox = new QSpinBox(dialog);
  spinbox->setMinimum(1);

  connect(dialog,SIGNAL(accepted()),this,SLOT(setPenWidth()));

   dialog->show();
   dialog->raise();
   dialog->activateWindow();
}

void MainWindow::about()
{
    QMessageBox::about(this, tr("About Scribble"),
            tr("<p><b>Scribble</b> is kewl</p>"));
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("Open..."), this);
    connect(openAct, SIGNAL(triggered()), this, SLOT(open()));

    saveAct = new QAction(tr("Save"), this);
    connect(saveAct, SIGNAL(triggered()), this, SLOT(save()));

    saveAsAct = new QAction(tr("Save As..."), this);
    connect(saveAsAct, SIGNAL(triggered()), this, SLOT(saveAs()));

//     printPdfAct = new QAction(tr("&Print as PDF"), this);
//     connect(printPdfAct, SIGNAL(triggered()), scribbleArea, SLOT(printPdf()));

    exitAct = new QAction(tr("Exit"), this);

    connect(exitAct, SIGNAL(triggered()), this, SLOT(close()));

    penColorAct = new QAction(tr("Pen Color..."), this);
    connect(penColorAct, SIGNAL(triggered()), this, SLOT(penColor()));

    penWidthAct = new QAction(tr("Pen Width..."), this);
    connect(penWidthAct, SIGNAL(triggered()), this, SLOT(penWidth()));

    clearScreenAct = new QAction(tr("Clear Screen"), this);

    connect(clearScreenAct, SIGNAL(triggered()),
            scribbleArea, SLOT(clearImage()));

//     aboutAct = new QAction(tr("About"), this);
//     connect(aboutAct, SIGNAL(triggered()), this, SLOT(about()));
}

void MainWindow::createMenus()
{
    QMenu *contextMenu;
    contextMenu = QSoftMenuBar::menuFor(this);

    contextMenu->addAction(openAct);
    contextMenu->addAction(saveAct);
    contextMenu->addAction(saveAsAct);
    contextMenu->addSeparator();

      //   optionMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addAction(penColorAct);
    contextMenu->addAction(penWidthAct);
    contextMenu->addSeparator();
    contextMenu->addAction(clearScreenAct);

      //   fileMenu = QSoftMenuBar::menuFor(this);
    contextMenu->addSeparator();
    contextMenu->addAction(exitAct);
}

bool MainWindow::maybeSave()
{
    if (scribbleArea->isModified()) {
         QMessageBox box( tr("Save?"),
                          tr("<qt>The image changed.<br>"
                             "Save it?</qt>"),
                          QMessageBox::Warning,
                          QMessageBox::Yes | QMessageBox::Default,
                          QMessageBox::No,
                          QMessageBox::Cancel | QMessageBox::Escape, this);
        box.resize(box.sizeHint()); //needs this otherwise shows offscreen

        switch(box.exec()) {
            case QMessageBox::Yes:
                  return save();
                  break;
            case QMessageBox::Cancel:
                 return false;
                 break;
          };
    }
    return true;
}

void MainWindow::createSelector()
{
    if (!selector) {
        selector = new QDocumentSelectorService(this);

        QContentFilter filter;
        foreach (QByteArray format, QImageReader::supportedImageFormats()) {
            filter |= QContentFilter(QMimeType::fromExtension(QString(format)));
        }
        selector->setFilter( filter );

        foreach (QByteArray format, QImageWriter::supportedImageFormats()) {
            QString type = QMimeType::fromExtension( QString( format ) ).id();

            if (!type.isEmpty() && !saveTypes.contains(type)) {
                if (type == QLatin1String("image/png"))
                    saveTypes.prepend(type);
                else
                    saveTypes.append(type);
            }
        }
    }
}

void MainWindow::colorSelected(const QColor &newColor)
{
  cselect->hide();
  scribbleArea->setPenColor(newColor);
  delete cselect;
  cselect = 0;

}


void MainWindow::setPenWidth()
{
  dialog->hide();
  scribbleArea->setPenWidth( spinbox->value());

  delete dialog;
  dialog = 0;

}
