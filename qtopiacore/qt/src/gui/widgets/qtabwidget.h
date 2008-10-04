/****************************************************************************
**
** Copyright (C) 2008 Nokia Corporation and/or its subsidiary(-ies).
** Contact: Qt Software Information (qt-info@nokia.com)
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QTABWIDGET_H
#define QTABWIDGET_H

#include <QtGui/qwidget.h>
#include <QtGui/qicon.h>

QT_BEGIN_HEADER

QT_BEGIN_NAMESPACE

QT_MODULE(Gui)

#ifndef QT_NO_TABWIDGET

class QTabBar;
class QTabWidgetPrivate;
class QStyleOptionTabWidgetFrame;

class Q_GUI_EXPORT QTabWidget : public QWidget
{
    Q_OBJECT
    Q_ENUMS(TabPosition TabShape)
    Q_PROPERTY(TabPosition tabPosition READ tabPosition WRITE setTabPosition)
    Q_PROPERTY(TabShape tabShape READ tabShape WRITE setTabShape)
    Q_PROPERTY(int currentIndex READ currentIndex WRITE setCurrentIndex NOTIFY currentChanged)
    Q_PROPERTY(int count READ count)
    Q_PROPERTY(QSize iconSize READ iconSize WRITE setIconSize)
    Q_PROPERTY(Qt::TextElideMode elideMode READ elideMode WRITE setElideMode)
    Q_PROPERTY(bool usesScrollButtons READ usesScrollButtons WRITE setUsesScrollButtons)

public:
    explicit QTabWidget(QWidget *parent = 0);
    ~QTabWidget();

    int addTab(QWidget *widget, const QString &);
    int addTab(QWidget *widget, const QIcon& icon, const QString &label);

    int insertTab(int index, QWidget *widget, const QString &);
    int insertTab(int index, QWidget *widget, const QIcon& icon, const QString &label);

    void removeTab(int index);

    bool isTabEnabled(int index) const;
    void setTabEnabled(int index, bool);

    QString tabText(int index) const;
    void setTabText(int index, const QString &);

    QIcon tabIcon(int index) const;
    void setTabIcon(int index, const QIcon & icon);

#ifndef QT_NO_TOOLTIP
    void setTabToolTip(int index, const QString & tip);
    QString tabToolTip(int index) const;
#endif

#ifndef QT_NO_WHATSTHIS
    void setTabWhatsThis(int index, const QString &text);
    QString tabWhatsThis(int index) const;
#endif

    int currentIndex() const;
    QWidget *currentWidget() const;
    QWidget *widget(int index) const;
    int indexOf(QWidget *widget) const;
    int count() const;

    enum TabPosition { North, South, West, East
#if defined(QT3_SUPPORT) && !defined(Q_MOC_RUN)
        , Top = North, Bottom = South
#endif
    };
    TabPosition tabPosition() const;
    void setTabPosition(TabPosition);

    enum TabShape { Rounded, Triangular };
    TabShape tabShape() const;
    void setTabShape(TabShape s);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;

    void setCornerWidget(QWidget * w, Qt::Corner corner = Qt::TopRightCorner);
    QWidget * cornerWidget(Qt::Corner corner = Qt::TopRightCorner) const;

    Qt::TextElideMode elideMode() const;
    void setElideMode(Qt::TextElideMode);

    QSize iconSize() const;
    void setIconSize(const QSize &size);

    bool usesScrollButtons() const;
    void setUsesScrollButtons(bool useButtons);

    void clear();

public Q_SLOTS:
    void setCurrentIndex(int index);
    void setCurrentWidget(QWidget *widget);

Q_SIGNALS:
    void currentChanged(int index);

protected:
    virtual void tabInserted(int index);
    virtual void tabRemoved(int index);

    void showEvent(QShowEvent *);
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void paintEvent(QPaintEvent *);
    void setTabBar(QTabBar *);
    QTabBar* tabBar() const;
    void changeEvent(QEvent *);
    bool event(QEvent *);
    void initStyleOption(QStyleOptionTabWidgetFrame *option) const;

#ifdef QT3_SUPPORT
public:
    QT3_SUPPORT_CONSTRUCTOR QTabWidget(QWidget *parent, const char *name, Qt::WindowFlags f = 0);

    inline QT3_SUPPORT void insertTab(QWidget * w, const QString &s, int index = -1) { insertTab(index, w, s); }
    inline QT3_SUPPORT void insertTab(QWidget *child, const QIcon& icon,
                                    const QString &label, int index = -1) { insertTab(index, child, icon, label); }

    inline QT3_SUPPORT void changeTab(QWidget *w, const QString &s) {setTabText(indexOf(w), s); }
    inline QT3_SUPPORT void changeTab(QWidget *w, const QIcon& icon,
                                    const QString &label) { int idx = indexOf(w); setTabText(idx, label); setTabIcon(idx, icon); }

    inline QT3_SUPPORT bool isTabEnabled( QWidget *w) const {return isTabEnabled(indexOf(w)); }
    inline QT3_SUPPORT void setTabEnabled(QWidget *w, bool b) { setTabEnabled(indexOf(w), b); }

    inline QT3_SUPPORT QString tabLabel(QWidget *w) const  {return tabText(indexOf(w)); }
    inline QT3_SUPPORT void setTabLabel(QWidget *w, const QString &l) { setTabText(indexOf(w), l); }

    inline QT3_SUPPORT QIcon tabIconSet(QWidget * w) const  {return tabIcon(indexOf(w)); }
    inline QT3_SUPPORT void setTabIconSet(QWidget * w, const QIcon & icon) { setTabIcon(indexOf(w), icon); }

    inline QT3_SUPPORT void removeTabToolTip(QWidget * w) {
#ifndef QT_NO_TOOLTIP
        setTabToolTip(indexOf(w), QString());
#else
        Q_UNUSED(w);
#endif
    }
    inline QT3_SUPPORT void setTabToolTip(QWidget * w, const QString & tip) {
#ifndef QT_NO_TOOLTIP
        setTabToolTip(indexOf(w), tip);
#else
        Q_UNUSED(w);
        Q_UNUSED(tip);
#endif
    }

    inline QT3_SUPPORT QString tabToolTip(QWidget * w) const {
#ifndef QT_NO_TOOLTIP
        return tabToolTip(indexOf(w));
#else
        Q_UNUSED(w);
        return QString();
#endif
    }

    inline QT3_SUPPORT QWidget * currentPage() const { return currentWidget(); }
    inline QT3_SUPPORT QWidget *page(int index) const { return widget(index); }
    inline QT3_SUPPORT QString label(int index) const { return tabText(index); }
    inline QT3_SUPPORT int currentPageIndex() const { return currentIndex(); }

    inline QT3_SUPPORT int margin() const { return 0; }
    inline QT3_SUPPORT void setMargin(int) {}

public Q_SLOTS:
    inline QT_MOC_COMPAT void setCurrentPage(int index) { setCurrentIndex(index); }
    inline QT_MOC_COMPAT void showPage(QWidget *w) { setCurrentIndex(indexOf(w)); }
    inline QT_MOC_COMPAT void removePage(QWidget *w) { removeTab(indexOf(w)); }

Q_SIGNALS:
    QT_MOC_COMPAT void currentChanged(QWidget *);
    QT_MOC_COMPAT void selected(const QString&);
#endif // QT3_SUPPORT

private:
    Q_DECLARE_PRIVATE(QTabWidget)
    Q_DISABLE_COPY(QTabWidget)
    Q_PRIVATE_SLOT(d_func(), void _q_showTab(int))
    Q_PRIVATE_SLOT(d_func(), void _q_removeTab(int))
    void setUpLayout(bool = false);
    friend class Q3TabDialog;
};

#endif // QT_NO_TABWIDGET

QT_END_NAMESPACE

QT_END_HEADER

#endif // QTABWIDGET_H
