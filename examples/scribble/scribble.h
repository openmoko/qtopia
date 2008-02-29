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

#ifndef SCRIBBLE_H
#define SCRIBBLE_H

#include <qdocumentselector.h>

#include <qmainwindow.h>

class Scribbler;

class QContent;

class QAction;
class QListView;
class QMenuBar;
class QString;
class QToolBar;

class Scribble: public QMainWindow
{
    Q_OBJECT

public:
    Scribble(QWidget *parent = 0, const char *name = 0, int wFlags = 0);
    ~Scribble(void);

private slots:
    void            newScribble(void);
    void            editScribble(const QContent &f);

private:
    void            updateActions(void);
    bool            load(const QString &filename, Scribbler &s);
    void            save(const QString &filename, const Scribbler &s);

    Scribbler       *scribbleWindow;

    QAction         *newAction;
    QMenuBar        *menubar;
    QToolBar        *toolbar;

    QDocumentSelector    *filelist;
};

#endif  // SCRIBBLE_H
