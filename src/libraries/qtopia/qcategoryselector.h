/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
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

#ifndef __CATEGORYCOMBO_H__
#define __CATEGORYCOMBO_H__

#include <qtopiaglobal.h>
#include <qcategorymanager.h>

#include <QWidget>
#include <QDialog>
#include <QItemSelection>

class QCategoryEditorData;
class QCategoryEditor : public QDialog
{
    Q_OBJECT

public:
    explicit QCategoryEditor(QWidget *parent);
    ~QCategoryEditor();

    QString name() const;
    bool global() const;

    void setName(const QString &name);
    void setGlobal(bool global);

public slots:
    void accept();

private:
    QCategoryEditorData *d;
};

class QCategorySelectData;
class QTOPIA_EXPORT QCategorySelector : public QWidget
{
    Q_OBJECT

public:
    enum ContentFlag
    {
        IncludeAll = 0x04,     // Adds "All" option
        IncludeUnfiled = 0x08, // Adds "Unfiled" option
        ListView = 0x10,      // Forces it to appear as a list
        ComboView = 0x20,     // Forces it to appear as a combobox
        DialogView = 0x40,    // Forces it to appear as a button that displays a dialog.

        Filter = IncludeAll | IncludeUnfiled,
        Editor = IncludeUnfiled,
        ViewMask = ListView | ComboView | DialogView
    };
    Q_DECLARE_FLAGS(ContentFlags, ContentFlag)

    explicit QCategorySelector(QWidget *parent = 0);
    explicit QCategorySelector(const QString &scope, ContentFlags f = Editor, QWidget *parent = 0);
    virtual ~QCategorySelector();

    QStringList selectedCategories() const;
    QCategoryFilter selectedFilter() const;

    virtual QSize sizeHint () const;

    void setListFrameStyle(int style);

public slots:
    void selectCategories(const QString &id);
    void selectCategories(const QStringList &id);
    void selectFilter(const QCategoryFilter &);

    void selectAll();
    void selectUnfiled();

signals:
    void categoriesSelected(const QList<QString> &);
    void filterSelected(const QCategoryFilter &);
    void listActivated(int pos);

private slots:
    void comboSelection(int index);
    void listActivated(const QModelIndex &);

    void showDialog();

private:
    QCategorySelectData *d;
};

class QCategoryDialogData;
class QTOPIA_EXPORT QCategoryDialog : public QDialog
{
    Q_OBJECT

public:
    enum ContentFlag
    {
        IncludeAll = 0x04,
        IncludeUnfiled = 0x08,
        Filter = IncludeAll | IncludeUnfiled,
        Editor = IncludeUnfiled
    };
    Q_DECLARE_FLAGS(ContentFlags, ContentFlag)

    explicit QCategoryDialog(const QString &scope, ContentFlags f = Filter, QWidget *parent = 0);
    ~QCategoryDialog();

    void setText(const QString &);
    QString text() const;

    QList<QString> selectedCategories() const;
    QCategoryFilter selectedFilter() const;

public slots:
    void selectCategories(const QString &id);
    void selectCategories(const QList<QString> &id);
    void selectFilter(const QCategoryFilter &);

    void selectAll();
    void selectUnfiled();

protected:
    virtual QSize sizeHint() const;
#ifdef QTOPIA_KEYPAD_NAVIGATION
    void keyPressEvent(QKeyEvent* e);
#endif

private:
    QCategoryDialogData *d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QCategorySelector::ContentFlags)

Q_DECLARE_OPERATORS_FOR_FLAGS(QCategoryDialog::ContentFlags)

#endif
