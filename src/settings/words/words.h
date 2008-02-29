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
#ifndef WORDS_H
#define WORDS_H

#include <QDialog>
#include <QItemDelegate>

class QAction;
class QLineEdit;
class QListWidget;
class InputMatcher;
class QLabel;

class WordListDelegate : public QItemDelegate
{
    Q_OBJECT
public:
    WordListDelegate(QObject *parent);
    ~WordListDelegate() {};

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option,
               const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;
};


class Words : public QDialog
{
    Q_OBJECT

public:
    Words( QWidget* parent = 0, Qt::WFlags fl = 0 );
    ~Words();

    QSize sizeHint() const;

protected:
    void keyPressEvent(QKeyEvent*);
    void resizeEvent(QResizeEvent *resizeEvent);
    bool eventFilter(QObject* watched, QEvent* e);

private slots:
    void updateActions();
    void lookup();
    void lookup(const QString& in);
    void modeChanged();
    void addWord();
    void addWord(const QString& word);
    void deleteWord();
    void preferWord();
    void showAddedDict();
    void showPreferredDict();
    void showDeletedDict();

private:
    void showDict(const char* name);
    void search();
    void search(const QString&);
    QLineEdit *line;
    QListWidget *box;
    QAction *a_add, *a_del, *a_prefer, *a_pkim, *a_word;
    QAction *a_alllocal, *a_allpref, *a_alldel;
    InputMatcher *matcher;
    QLabel *tooltip;
    bool dummy;
};


#endif // WORDS_H

