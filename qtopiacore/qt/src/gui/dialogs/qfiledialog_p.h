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

#ifndef QFILEDIALOG_P_H
#define QFILEDIALOG_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "private/qdialog_p.h"
#include "QtGui/qitemselectionmodel.h"
#include "QtGui/qabstractitemview.h"
#include "QtGui/qheaderview.h"
#include "QtGui/qlistview.h"
#include "QtGui/qtreeview.h"
#include "QtGui/qtoolbutton.h"
#include "QtGui/qevent.h"
#include "QtGui/qdialogbuttonbox.h"

class QDirModel;
class QComboBox;
class QAction;
class QPushButton;
class QFileDialogLineEdit;
class QFileDialogModeButton;
class QGridLayout;
class QLabel;
class QFileDialogListView;
class QFileDialogTreeView;

class QFileDialogPrivate : public QDialogPrivate
{
    Q_DECLARE_PUBLIC(QFileDialog)
public:
    QFileDialogPrivate();

    // private slots
    void _q_reload();
    void _q_navigateToPrevious();
    void _q_navigateToParent();
    void _q_enterDirectory(const QModelIndex &index);
    void _q_enterDirectory(const QString &path);
    void _q_enterDirectory();
    void _q_showList();
    void _q_showDetails();
    void _q_showHidden();
    void _q_useFilter(const QString &filter);
    void _q_updateFileName(const QItemSelection &selection);
    void _q_autoCompleteFileName(const QString &text);
    void _q_autoCompleteDirectory(const QString &text);
    void _q_showContextMenu(const QPoint &pos);
    void _q_createDirectory();
    void _q_renameCurrent();
    void _q_deleteCurrent();
    void _q_sortByName();
    void _q_sortBySize();
    void _q_sortByDate();
    void _q_setUnsorted();
    void _q_sortByColumn(int column);
    void _q_currentChanged(const QModelIndex &index);

    // setup
    void setup(const QString &directory, const QStringList &nameFilter);
    void setupActions();
    void setupListView(const QModelIndex &index, QGridLayout *grid);
    void setupTreeView(const QModelIndex &index, QGridLayout *grid);
    void setupToolButtons(const QModelIndex &index, QGridLayout *grid);
    void setupWidgets(QGridLayout *grid);

    // other
    void updateButtons(const QModelIndex &index);
    void setRootIndex(const QModelIndex &index);
    QModelIndex rootIndex() const;
    void setDirSorting(QDir::SortFlags sort);
    void setDirFilter(QDir::Filters filter);
    QDir::Filters filterForMode(QFileDialog::FileMode mode);
    QAbstractItemView::SelectionMode selectionMode(QFileDialog::FileMode mode);
    QModelIndex matchDir(const QString &text, const QModelIndex &first) const;
    QModelIndex matchName(const QString &name, const QModelIndex &first) const;

    bool itemViewKeyboardEvent(QKeyEvent *e);

    static QString getEnvironmentVariable(const QString &str);

    // inlined stuff
    inline QString tr(const char *text) const { return QObject::tr(text); }
    inline QString toNative(const QString &path) const
        { return QDir::toNativeSeparators(path); }
    inline QString toInternal(const QString &path) const
        {
#if defined(Q_FS_FAT) || defined(Q_OS_OS2EMX)
            QString n(path);
            for (int i = 0; i < (int)n.length(); ++i)
                if (n[i] == '\\') n[i] = '/';
            return n;
#else // the compile should optimize away this
            return path;
#endif
        }
    QFileDialog::ViewMode viewMode() const;

    // static stuff
    static QString workingDirectory(const QString &path);
    static QString initialSelection(const QString &path);

    // data
    QDirModel *model;
    QItemSelectionModel *selections;
    QFileDialogListView *listView;
    QFileDialogTreeView *treeView;

    QFileDialog::FileMode fileMode;
    QFileDialog::AcceptMode acceptMode;
    bool confirmOverwrite;
    QString defaultSuffix;

    QStringList history;

    QComboBox *lookInCombo;
    QFileDialogLineEdit *fileNameEdit;
    QFileDialogLineEdit *lookInEdit;
    QComboBox *fileTypeCombo;

    QAction *openAction;
    QAction *renameAction;
    QAction *deleteAction;

    QAction *reloadAction;
    QAction *sortByNameAction;
    QAction *sortBySizeAction;
    QAction *sortByDateAction;
    QAction *unsortedAction;
    QAction *showHiddenAction;

    QDialogButtonBox *buttonBox;

    QToolButton *backButton;
    QToolButton *toParentButton;
    QToolButton *newFolderButton;
    QFileDialogModeButton *detailModeButton;
    QFileDialogModeButton *listModeButton;

    QLabel *lookInLabel;
    QLabel *fileNameLabel;
    QLabel *fileTypeLabel;
};

struct QFileDialogArgs
{
    QFileDialogArgs() : parent(0), mode(QFileDialog::AnyFile) {}

    QWidget *parent;
    QString caption;
    QString directory;
    QString selection;
    QString filter;
    QFileDialog::FileMode mode;
    QFileDialog::Options options;
};

class QFileDialogListView : public QListView
{
public:
    QFileDialogListView(QFileDialogPrivate *d_pointer)
        : QListView(qobject_cast<QWidget*>(d_pointer->q_ptr)), d_ptr(d_pointer)
    {}
protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (d_ptr->itemViewKeyboardEvent(e)) {
            e->accept();
        } else {
            QListView::keyPressEvent(e);
        }
    }
private:
    QFileDialogPrivate *d_ptr;
};

class QFileDialogTreeView : public QTreeView
{
public:
    QFileDialogTreeView(QFileDialogPrivate *d_pointer)
        : QTreeView(qobject_cast<QWidget*>(d_pointer->q_ptr)), d_ptr(d_pointer)
    {
        // we want to handle this our selves in QFileDialog
        disconnect(header(), SIGNAL(sectionClicked(int)), this, SLOT(sortByColumn(int)));
    }
protected:
    void keyPressEvent(QKeyEvent *e)
    {
        if (d_ptr->itemViewKeyboardEvent(e)) {
            e->accept();
        } else {
            QTreeView::keyPressEvent(e);
        }
    }
private:
    QFileDialogPrivate *d_ptr;
};

#endif // QFILEDIALOG_P_H
