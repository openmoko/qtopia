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

#ifndef Q3TOOLBAR_H
#define Q3TOOLBAR_H

#include <Qt3Support/q3dockwindow.h>

QT_BEGIN_HEADER

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

QT_END_HEADER

#endif // Q3TOOLBAR_H
