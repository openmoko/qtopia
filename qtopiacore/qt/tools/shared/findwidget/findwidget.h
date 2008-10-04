/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the tools applications of the Qt Toolkit.
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

#ifndef FINDWIDGET_H
#define FINDWIDGET_H

#include <QtGui/QIcon>
#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE

class QCheckBox;
class QEvent;
class QKeyEvent;
class QLabel;
class QLineEdit;
class QObject;
class QTextEdit;
class QToolButton;

class FindWidget : public QWidget
{
    Q_OBJECT

public:
    FindWidget(QWidget *parent = 0);
    ~FindWidget();

    QTextEdit *textEdit() const
    { return m_textEdit; }

    void setTextEdit(QTextEdit *textEdit);
    bool eventFilter(QObject *object, QEvent *e);

    static QIcon findIconSet();

public slots:
    void activate();
    void deactivate();
    void findNext();
    void findPrevious();
    void findCurrentText(const QString &text);

protected:
    void keyPressEvent(QKeyEvent *event);

private slots:
    void updateButtons();

private:
    void find(const QString &ttf, bool forward, bool backward);

    QLineEdit *m_editFind;
    QCheckBox *m_checkCase;
    QLabel *m_labelWrapped;
    QToolButton *m_toolNext;
    QToolButton *m_toolClose;
    QToolButton *m_toolPrevious;
    QCheckBox *m_checkWholeWords;
    QTextEdit *m_textEdit;
};

QT_END_NAMESPACE

#endif  // FINDWIDGET_H
