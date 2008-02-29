/****************************************************************************
**
** Copyright (C) 1992-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"
#include "messagestreeview.h"
#include "ui_mainwindow.h"
#include <QMainWindow>
#include <QHash>
#include <QPrinter>
#include <QtCore/QPointer>
#include <QtCore/QLocale>
class QModelIndex;
class QStringList;
class QPixmap;
class QAction;
class QDialog;
class QLabel;
class QMenu;
class QAssistantClient;
template <typename T> class QList;
class QIcon;

class TrPreviewTool;

class QTreeView;
class PhraseModel;
class PhraseItem;
class MessageModel;
class MessageItem;
class ContextItem;
class FindDialog;
class TranslateDialog;
class BatchTranslationDialog;
class TranslationSettingsDialog;
class MessageEditor;
class Statistics;

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum {PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};

    static QPixmap *pxOn;
    static QPixmap *pxOff;
    static QPixmap *pxObsolete;
    static QPixmap *pxDanger;
    static QPixmap *pxWarning;
    static QPixmap *pxEmpty;
    static const QPixmap pageCurl();

    TrWindow();
    ~TrWindow();

    void openFile(const QString &name);

protected:
    void readConfig();
    void writeConfig();
    void closeEvent(QCloseEvent *);

private slots:
    void doneAndNext();
    void prev();
    void next();
    void recentFileActivated(QAction *action);
    void setupRecentFilesMenu();
    void open();
    void save();
    void saveAs();
    void release();
    void releaseAs();
    void print();
    void find();
    void findAgain();
    void showTranslateDialog();
    void showBatchTranslateDialog();
    void showTranslationSettings();
    void translateAndFindNext(const QString& findWhat, const QString &translateTo, int matchOption, int mode, bool markFinished);
    void translate(int mode);
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook(QAction *action);
    void editPhraseBook(QAction *action);
    void printPhraseBook(QAction *action);
    void manual();
    void revertSorting();
    void about();
    void aboutQt();
    void updateViewMenu();

    void setupPhrase();
    bool maybeSave();
    void updateCaption();
    void showNewCurrent(const QModelIndex &current, const QModelIndex &old);
    
    // To synchronize from the contextmodel to the MetaTranslator...
    // Operates on the selected item
    void updateTranslation(const QStringList &translations);
    void updateFinished(bool finished);
    // Operates on the given item
    void updateTranslation(int context, int message, const QString &translation);
    void updateFinished(int context, int message, bool finished);

    void toggleFinished(const QModelIndex &index);
    void prevUnfinished();
    void nextUnfinished();
    void findNext(const QString &text, int where, bool matchCase);
    void revalidate();
    void toggleGuessing();
    void focusSourceList();
    void focusPhraseList();
    void toggleStatistics();
    void updateStatistics();
    void onWhatsThis();
    void finishedBatchTranslation();
    void previewForm();
    void updateLanguage(QLocale::Language);
private:

    typedef QHash<QString, PhraseBook> PBD;

    static QString friendlyString(const QString &str);

    int findCurrentContextRow();
    //bool setNextMessage(int *currentrow, bool checkUnfinished);
    bool setNextMessage(QModelIndex *currentIndex, bool checkUnfinished);
    bool setPrevMessage(QModelIndex *currentIndex, bool checkUnfinished);
    bool setNextContext(int *currentrow, bool checkUnfinished);
    bool setPrevContext(int *currentrow, bool checkUnfinished);
    bool next(bool checkUnfinished);
    bool prev(bool checkUnfinished);
    QStringList findFormFilesInCurrentTranslationFile();

    void addRecentlyOpenedFile(const QString &fn, QStringList &lst);
    void setupMenuBar();
    void setupToolBars();
    void setCurrentContextRow(int row);
    void setCurrentContext(const QModelIndex &indx);
    void setCurrentMessage(const QModelIndex &indx);
    PhraseBook phraseBookFromFileName(QString name) const;
    bool openPhraseBook(const QString &name);
    bool phraseBooksContains(QString name);
    bool savePhraseBook(QString &name, const PhraseBook &pb);
    void updateProgress();
    void updatePhraseDict();
    PhraseBook getPhrases(const QString &source);
    bool danger(const MessageItem *message, bool verbose = false);

    void printDanger(MessageItem *m);
    bool updateDanger(MessageItem *m, bool verbose = false);

    bool searchItem(const QString &searchWhat, int c, int m);

    QAssistantClient *ac;
    MessagesTreeView *tv;
    MessageModel *cmdl;
    QTreeView *stv;
    QTreeView *ptv;
    PhraseModel *pmdl;
    MessageEditor * me;
    QLabel        * progress;
    QLabel        * modified;
    QStringList recentFiles;
    QString     filename;
    PBD phraseDict;
    QMap<QAction *, PhraseBook> phraseBooks[3];
    QPrinter printer;

    FindDialog *finddlg;
    QString findText;
    int findWhere;
    bool findMatchCase;
    int foundWhere;
    int foundOffset;
    TranslateDialog *m_translatedlg;
    BatchTranslationDialog *m_batchTranslateDlg;
    TranslationSettingsDialog *m_translationSettingsDialog;
    QString m_translateTo;
    bool m_findMatchSubstring;
    bool m_markFinished;
    
    // used by the preview tool
    QPointer<TrPreviewTool> m_previewTool;

    QDockWidget *dwScope;
    Ui::MainWindow m_ui;    // menus and actions
    Statistics *stats;
};

#endif
