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

#ifndef TEXTEDIT_H
#define TEXTEDIT_H

#include <qmainwindow.h>
#include <QContent>
#include <QList>

class QpeEditor;
class QLineEdit;
class QToolBar;
class QToolButton;
class QAction;
class QStackedWidget;
class QDocumentSelector;
class SimpleToolTip;
class QVBoxLayout;
class QLabel;

class TextEdit : public QMainWindow
{
    Q_OBJECT

public:
    TextEdit( QWidget *parent = 0, Qt::WFlags f = 0 );
    ~TextEdit();

    bool eventFilter(QObject *obj, QEvent *event);

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

    void search(const QString&);
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

    void clipboardChanged();

    void contentChanged(const QContentIdList& id,const QContent::ChangeType type);
    void print();

private:
    void colorChanged( const QColor &c );
    bool save();
    void clear();
    void updateCaption( const QString &name=QString() );
    void setFontSize(qreal size);
    void setupFontSizes(void);
    void setReadOnly(bool);

private:
    QStackedWidget *editorStack;
    QDocumentSelector *fileSelector;
    QpeEditor* editor;
    QAction *pasteAction;
    QAction *findAction;
    QContent *doc;
    SimpleToolTip *mToolTip;
    QWidget *mFindTextWidget;
    QLineEdit *mFindTextEntry;
    QLabel *mFindIcon;
    QVBoxLayout *mEditorLayout;

    QString backup;
    bool qCopActivated, canceled, saved;

    bool wasCreated;
    bool searchVisible;
    QAction *zin, *zout;
    bool zinE, zoutE;
    qreal defaultFontSize;
    qreal originalFontSize;
    QList<qreal> fontSizes;
    QString calculateName(QString);
};

#endif
