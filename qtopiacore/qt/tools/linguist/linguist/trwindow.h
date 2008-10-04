/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt Linguist of the Qt Toolkit.
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

#ifndef TRWINDOW_H
#define TRWINDOW_H

#include "phrase.h"
#include "messagestreeview.h"
#include "ui_mainwindow.h"
#include "recentfiles.h"
#include <QMainWindow>
#include <QHash>
#include <QPrinter>
#include <QtCore/QPointer>
#include <QtCore/QLocale>

QT_BEGIN_NAMESPACE

class QModelIndex;
class QStringList;
class QPixmap;
class QAction;
class QDialog;
class QLabel;
class QMenu;
class QProcess;
class QIcon;
class QSortFilterProxyModel;
class QTableView;
class QTreeView;

class BatchTranslationDialog;
class ContextItem;
class ErrorsView;
class FindDialog;
class LanguagesDialog;
class LanguagesManager;
class MessageEditor;
class MessageModel;
class MessageItem;
class PhraseView;
class SourceCodeView;
class Statistics;
class TranslateDialog;
class TranslationSettingsDialog;
class TrPreviewTool;

class TrWindow : public QMainWindow
{
    Q_OBJECT
public:
    enum {PhraseCloseMenu, PhraseEditMenu, PhrasePrintMenu};

    TrWindow();
    ~TrWindow();

    void openFile(const QString &name);
    static RecentFiles& recentFiles();
    static const QString& resourcePrefix();
    static QString friendlyString(const QString &str);

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
    void findAgain();
    void showTranslateDialog();
    void showBatchTranslateDialog();
    void showTranslationSettings();
    void translateAndFindNext(const QString& findWhat, const QString &translateTo,
                              int matchOption, int mode, bool markFinished);
    void translate(int mode);
    void newPhraseBook();
    void openPhraseBook();
    void closePhraseBook(QAction *action);
    void editPhraseBook(QAction *action);
    void printPhraseBook(QAction *action);
    void addToPhraseBook();
    void showLanguagesDialog();
    void manual();
    void resetSorting();
    void about();
    void aboutQt();
    void updateViewMenu();

    void showContextDock();
    void showMessagesDock();
    void showPhrasesDock();
    void showSourceCodeDock();
    void showErrorDock();

    void setupPhrase();
    bool maybeSave();
    void updateCaption();
    void selectedContextChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);
    void selectedMessageChanged(const QModelIndex &sortedIndex, const QModelIndex &oldIndex);
    void refreshCurrentMessage();

    // To synchronize from the contextmodel to the MetaTranslator...
    // Operates on the selected item
    void updateTranslation(const QStringList &translations);
    void updateFinished(bool finished);
    // Operates on the given item
    void updateTranslation(int context, int message, const QString &translation);
    void updateFinished(int context, int message, bool finished);

    void resetContextView();
    void toggleFinished(const QModelIndex &index);
    void prevUnfinished();
    void nextUnfinished();
    void findNext(const QString &text, int where, bool matchCase, bool ignoreAccelerators);
    void revalidate();
    void toggleStatistics();
    void updateStatistics();
    void onWhatsThis();
    void finishedBatchTranslation();
    void previewForm();
    void updateLanguage(QLocale::Language);
    void updatePhraseDict();

private:
    int findCurrentContextRow();
    QModelIndex nextContext(const QModelIndex &index) const;
    QModelIndex prevContext(const QModelIndex &index) const;
    QModelIndex nextMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
    QModelIndex prevMessage(const QModelIndex &currentIndex, bool checkUnfinished = false) const;
    bool next(bool checkUnfinished);
    bool prev(bool checkUnfinished);
    QStringList findFormFilesInCurrentTranslationFile();

    void setupMenuBar();
    void setupToolBars();
    void setCurrentContext(const QModelIndex &index);
    void setCurrentContextRow(int row);
    void setCurrentMessage(const QModelIndex &index);
    QModelIndex currentContextIndex() const;
    QModelIndex currentMessageIndex() const;
    PhraseBook *openPhraseBook(const QString &name);
    bool phraseBooksContains(QString name);
    bool savePhraseBook(QString &name, PhraseBook &pb);
    bool maybeSavePhraseBook(PhraseBook *phraseBook);
    bool closePhraseBooks();
    QString pickTranslationFile();
    void updateProgress();
    void updatePhraseBookActions();
    bool danger(const MessageItem *message, bool verbose = false);

    void printDanger(MessageItem *m);
    bool updateDanger(MessageItem *m, bool verbose = false);

    bool searchItem(const QString &searchWhat);

    MessageModel *mainModel() const;

    QProcess *m_assistantProcess;
    MessagesTreeView *m_contextView;
    QTreeView *m_messageView;
    QSortFilterProxyModel *m_sortedContextsModel;
    QSortFilterProxyModel *m_sortedMessagesModel;
    MessageEditor *me;
    PhraseView *m_phraseView;
    SourceCodeView *m_sourceCodeView;
    ErrorsView *m_errorsView;
    QLabel *progress;
    QLabel *modified;
    QString m_filename;
    QString phraseBookDir;
    // keyword -> list of appropriate phrases in the phrasebooks
    QHash<QString, QList<Phrase *> > m_phraseDict;
    QList<PhraseBook *> m_phraseBooks;
    QMap<QAction *, PhraseBook *> m_phraseBookMenu[3];
    QPrinter printer;

    FindDialog *m_findDialog;
    QString findText;
    int findWhere;
    bool findMatchCase;
    bool findIgnoreAccelerators;
    int foundWhere;
    TranslateDialog *m_translateDialog;
    BatchTranslationDialog *m_batchTranslateDialog;
    TranslationSettingsDialog *m_translationSettingsDialog;
    LanguagesDialog *m_languagesDialog;
    QString m_translateTo;
    bool m_findMatchSubstring;
    bool m_markFinished;

    // used by the preview tool
    QPointer<TrPreviewTool> m_previewTool;

    QDockWidget *m_contextDock;
    QDockWidget *m_messagesDock;
    QDockWidget *m_phrasesDock;
    QDockWidget *m_sourceCodeDock;
    QDockWidget *m_errorsDock;

    Ui::MainWindow m_ui;    // menus and actions
    Statistics *stats;
    LanguagesManager *m_languagesManager;

    static const QString m_resourcePrefix;
    static RecentFiles m_recentFiles;
};

QT_END_NAMESPACE

#endif
