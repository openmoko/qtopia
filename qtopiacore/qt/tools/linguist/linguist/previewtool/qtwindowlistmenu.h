/****************************************************************************
**
** Copyright (C) 2006-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTWINDOWLISTMENU_H
#define QTWINDOWLISTMENU_H

#include <QMap>
#include <QMenu>
#include <QActionGroup>

class QMenuBar;
class QWidget;
class QString;
class QWorkspace;
class QAction;

#if defined(Q_WS_WIN)
#  if !defined(QT_QTWINDOWLISTMENU_EXPORT) && !defined(QT_QTWINDOWLISTMENU_IMPORT)
#    define QT_QTWINDOWLISTMENU_EXPORT
#  elif defined(QT_QTWINDOWLISTMENU_IMPORT)
#    if defined(QT_QTWINDOWLISTMENU_EXPORT)
#      undef QT_QTWINDOWLISTMENU_EXPORT
#    endif
#    define QT_QTWINDOWLISTMENU_EXPORT __declspec(dllimport)
#  elif defined(QT_QTWINDOWLISTMENU_EXPORT)
#    undef QT_QTWINDOWLISTMENU_EXPORT
#    define QT_QTWINDOWLISTMENU_EXPORT __declspec(dllexport)
#  endif
#else
#  define QT_QTWINDOWLISTMENU_EXPORT
#endif

class QT_QTWINDOWLISTMENU_EXPORT QtWindowListMenu : public QMenu
{
    Q_OBJECT

public:
	QtWindowListMenu(QWorkspace *workspace, QWidget *parent = 0, const char *name = 0);
	QAction *addTo(const QString &text, QMenuBar *menubar, int idx = -1);
	void removeWindow(QWidget *w, bool windowDestroyed = false);

	virtual bool eventFilter(QObject *obj, QEvent *e);

    void setWindowIcon(QWidget *widget, const QIcon &icon);
    void setDefaultIcon(const QIcon &icon);    

    void setCloseIcon(const QIcon &icon);
    void setCloseAllIcon(const QIcon &icon);
    void setCascadeIcon(const QIcon &icon);
    void setTileIcon(const QIcon &icon);

public slots:
	void addWindow(QWidget *w);
    void addWindow(QWidget *w, const QIcon &icon);
	virtual void setEnabled(bool b);
	void windowDestroyed(QObject *obj);

private slots:
    void setSenderChecked(bool checked);
   
private:
	QMenuBar *m_menubar;
	QAction *m_my_action;
    QAction *m_close_current_action;
    QAction *m_close_all_action;
    QAction *m_cascade_action;
    QAction *m_tile_action;

    QIcon m_default_icon;

	/* A list of window/QAction* pairs. If the QAction-pointer is 0, we are keeping
	   track of the window, but it's hidden so it's not in the menu. */
	typedef QMap<QWidget *, QAction *> WindowList;
	WindowList m_window_list;
	QWorkspace *m_workspace;
    QActionGroup groupWindows;

	bool isEmpty();
    void setChecked(bool checked, QAction *a);    
};

#endif
