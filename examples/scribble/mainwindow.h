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

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QList>
#include <QMainWindow>

class QIODevice;
class QDocumentSelectorService;
class QColorSelectorDialog;
class QSpinBox;
class ScribbleArea;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow( QWidget *parent = 0, Qt::WFlags f = 0 );

protected:
    void closeEvent(QCloseEvent *event);

private slots:
    void open();
    bool save();
    bool saveAs();
    void penColor();
    void penWidth();
    void about();
    void colorSelected(const QColor &);
    void setPenWidth();

private:
    void createActions();
    void createMenus();
    void createSelector();
    bool maybeSave();

    ScribbleArea *scribbleArea;


    QMenu *fileMenu;
    QMenu *optionMenu;
    QMenu *helpMenu;

    QAction *openAct;
    QAction *saveAct;
    QAction *saveAsAct;

    QAction *exitAct;
    QAction *penColorAct;
    QAction *penWidthAct;
    QAction *printPdfAct;
    QAction *clearScreenAct;
    QAction *aboutAct;
    QAction *aboutQtAct;

    QColorSelectorDialog *cselect;
    QSpinBox *spinbox;
    QDialog *dialog;
    QDocumentSelectorService *selector;
    QStringList saveTypes;


};

#endif
