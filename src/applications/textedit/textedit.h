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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <qdocumentselector.h>
#ifdef QTOPIA_PHONE
#include <qsoftmenubar.h>
#include <QToolButton>
#endif

#include <qmainwindow.h>

class QpeEditor;
class QLineEdit;
class QAction;
class QToolBar;
class QStackedWidget;

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~TextEdit();

protected:
    void closeEvent( QCloseEvent* );

public slots:
    void setDocument(const QString&);

private slots:
    void message(const QString& msg, const QByteArray& data);

    void fileNew();
    void fileRevert();
    void fileOpen();
    void fileName();

    void editCut();
    void editCopy();
    void editPaste();
    void editFind(bool);

    void search();
    void searchNext();
    void findNotFound();
    void findWrapped();

    void accept();

    void newFile();
    void openFile( const QContent & );
    void showEditTools();

    void zoomIn();
    void zoomOut();
    void setWordWrap(bool y);
    void setFixedWidth(bool y);

    void clipboardChanged();

    void contentChanged(const QContentIdList& id,const QContent::ChangeType type);
    void print();

private:
    void colorChanged( const QColor &c );
    bool save();
    void clear();
    void updateCaption( const QString &name=QString() );
    void setFontSize(int sz, bool round_down_not_up);
    void setupFontSizes(void);
    void setReadOnly(bool);

private:
    QStackedWidget *editorStack;
    QDocumentSelector *fileSelector;
    QpeEditor* editor;
#ifndef QTOPIA_PHONE
    QToolBar *menu, *editBar;
#else
    QToolButton *findTb;
#endif
    QToolBar *searchBar;
    QLineEdit *searchEdit;
    QAction *pasteAction;
    QAction *fixedAction;
    QAction *findAction;
    QContent *doc;

#ifdef QTOPIA_PHONE
    QString backup;
    bool qCopActivated, canceled, saved;
#endif

    bool wasCreated;
    bool searchVisible;
    QAction *zin, *zout;
    bool zinE,zoutE,zoomOutLast;
    int variableFontSize;
    QString calculateName(QString);
#ifdef QTOPIA_PHONE
    QMenu *contextMenu;
#endif
};

#endif
