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

#ifndef QDYNAMICDOCKWIDGET_H
#define QDYNAMICDOCKWIDGET_H

#include <QtGui/qwidget.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_DOCKWIDGET

class QDockWidgetLayout;
class QDockWidgetPrivate;
class QMainWindow;

class Q_GUI_EXPORT QDockWidget : public QWidget
{
    Q_OBJECT

    Q_FLAGS(DockWidgetFeatures)
    Q_PROPERTY(bool floating READ isFloating WRITE setFloating)
    Q_PROPERTY(DockWidgetFeatures features READ features WRITE setFeatures NOTIFY featuresChanged)
    Q_PROPERTY(Qt::DockWidgetAreas allowedAreas READ allowedAreas
               WRITE setAllowedAreas NOTIFY allowedAreasChanged)
    Q_PROPERTY(QString windowTitle READ windowTitle WRITE setWindowTitle DESIGNABLE true)

public:
    explicit QDockWidget(const QString &title, QWidget *parent = 0, Qt::WindowFlags flags = 0);
    explicit QDockWidget(QWidget *parent = 0, Qt::WindowFlags flags = 0);
    ~QDockWidget();

    QWidget *widget() const;
    void setWidget(QWidget *widget);

    enum DockWidgetFeature {
        DockWidgetClosable    = 0x01,
        DockWidgetMovable     = 0x02,
        DockWidgetFloatable   = 0x04,

        DockWidgetFeatureMask = 0x07,
        AllDockWidgetFeatures = DockWidgetFeatureMask,
        NoDockWidgetFeatures  = 0x00,

        Reserved              = 0xff
    };
    Q_DECLARE_FLAGS(DockWidgetFeatures, DockWidgetFeature)

    void setFeatures(DockWidgetFeatures features);
    DockWidgetFeatures features() const;

    void setFloating(bool floating);
    inline bool isFloating() const { return isWindow(); }

    void setAllowedAreas(Qt::DockWidgetAreas areas);
    Qt::DockWidgetAreas allowedAreas() const;

    inline bool isAreaAllowed(Qt::DockWidgetArea area) const
    { return (allowedAreas() & area) == area; }

#ifndef QT_NO_ACTION
    QAction *toggleViewAction() const;
#endif

Q_SIGNALS:
    void featuresChanged(QDockWidget::DockWidgetFeatures features);
    void topLevelChanged(bool topLevel);
    void allowedAreasChanged(Qt::DockWidgetAreas allowedAreas);

protected:
    void changeEvent(QEvent *event);
    void closeEvent(QCloseEvent *event);
    void paintEvent(QPaintEvent *event);
    bool event(QEvent *event);

private:
    Q_DECLARE_PRIVATE(QDockWidget)
    Q_DISABLE_COPY(QDockWidget)
    Q_PRIVATE_SLOT(d_func(), void _q_toggleView(bool))
    Q_PRIVATE_SLOT(d_func(), void _q_toggleTopLevel())
    friend class QDockWidgetLayout;
    friend class QDockWidgetItem;
    friend class QMainWindowLayout;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QDockWidget::DockWidgetFeatures)

#endif // QT_NO_DOCKWIDGET

QT_END_HEADER

#endif // QDYNAMICDOCKWIDGET_H
