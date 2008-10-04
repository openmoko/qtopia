/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the Qt3Support module of the Qt Toolkit.
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

#ifndef Q3TOOLBAR_H
#define Q3TOOLBAR_H

#include <Qt3Support/q3dockwindow.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Qt3SupportLight)

#ifndef QT_NO_TOOLBAR

class Q3MainWindow;
class Q3ToolBarPrivate;

class Q_COMPAT_EXPORT Q3ToolBar: public Q3DockWindow
{
    Q_OBJECT
    Q_PROPERTY(QString label READ label WRITE setLabel)

public:
    Q3ToolBar(const QString &label,
              Q3MainWindow *, Qt::ToolBarDock = Qt::DockTop,
              bool newLine = false, const char* name=0);
    Q3ToolBar(const QString &label, Q3MainWindow *, QWidget *,
              bool newLine = false, const char* name=0, Qt::WindowFlags f = 0);
    Q3ToolBar(Q3MainWindow* parent=0, const char* name=0);
    ~Q3ToolBar();

    void addSeparator();

    void setVisible(bool visible);

    Q3MainWindow * mainWindow() const;

    virtual void setStretchableWidget(QWidget *);

    bool event(QEvent * e);

    virtual void setLabel(const QString &);
    QString label() const;

    virtual void clear();

    QSize minimumSize() const;
    QSize minimumSizeHint() const;

    void setOrientation(Qt::Orientation o);
    void setMinimumSize(int minw, int minh);

protected:
    void resizeEvent(QResizeEvent *e);
    void styleChange(QStyle &);
    void actionEvent(QActionEvent *);

private Q_SLOTS:
    void createPopup();

private:
    void init();
    void checkForExtension(const QSize &sz);
    Q3ToolBarPrivate * d;
    Q3MainWindow * mw;
    QWidget * sw;
    QString l;

    friend class Q3MainWindow;
    friend class Q3DockAreaLayout;

private:
    Q_DISABLE_COPY(Q3ToolBar)
};

#endif // QT_NO_TOOLBAR

QT_END_NAMESPACE

QT_END_HEADER

#endif // Q3TOOLBAR_H
