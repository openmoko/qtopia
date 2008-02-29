/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

/*
TRANSLATOR qdesigner_internal::WidgetFactory
*/

#include "widgetfactory_p.h"
#include "widgetdatabase_p.h"
#include "qlayout_widget_p.h"
#include "qdesigner_widget_p.h"
#include "qdesigner_tabwidget_p.h"
#include "qdesigner_toolbox_p.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_toolbar_p.h"
#include "qdesigner_menubar_p.h"
#include "qdesigner_menu_p.h"
#include "qdesigner_dockwidget_p.h"
#include "qdesigner_promotedwidget_p.h"
#include "abstractformwindow.h"

// shared
#include "layoutinfo_p.h"
#include "spacer_widget_p.h"
#include "layout_p.h"

// sdk
#include <QtDesigner/QtDesigner>

#include <QtGui/QtGui>
#include <QtCore/qdebug.h>

namespace qdesigner_internal {

QPointer<QWidget> *WidgetFactory::m_lastPassiveInteractor = new QPointer<QWidget>();
bool WidgetFactory::m_lastWasAPassiveInteractor = false;


WidgetFactory::WidgetFactory(QDesignerFormEditorInterface *core, QObject *parent)
    : QDesignerWidgetFactoryInterface(parent),
      m_core(core),
      m_formWindow(0)
{
}

WidgetFactory::~WidgetFactory()
{
}

QDesignerFormWindowInterface *WidgetFactory::currentFormWindow(QDesignerFormWindowInterface *fw)
{
    QDesignerFormWindowInterface *was = m_formWindow;
    m_formWindow = fw;
    return was;
}

void WidgetFactory::loadPlugins()
{
    m_customFactory.clear();

    QDesignerPluginManager *pluginManager = m_core->pluginManager();

    QList<QDesignerCustomWidgetInterface*> lst = pluginManager->registeredCustomWidgets();
    foreach (QDesignerCustomWidgetInterface *c, lst) {
        m_customFactory.insert(c->name(), c);
    }
}

QWidget *WidgetFactory::createWidget(const QString &widgetName, QWidget *parentWidget) const
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QDesignerFormWindowInterface *fw = m_formWindow;
    if (! fw)
        fw = QDesignerFormWindowInterface::findFormWindow(parentWidget);

    QWidget *w = 0;

// ### cleanup
    if (QDesignerCustomWidgetInterface *f = m_customFactory.value(widgetName)) {
        w = f->createWidget(parentWidget);
    } else if (widgetName == QLatin1String("Line")) {
        w = new Line(parentWidget);
    } else if (widgetName == QLatin1String("QLabel")) {
        w = new QDesignerLabel(parentWidget);
    } else if (widgetName == QLatin1String("QDockWidget")) {
        w = new QDesignerDockWidget(parentWidget);
    } else if (widgetName == QLatin1String("QTabWidget")) {
        w = new QDesignerTabWidget(parentWidget);
    } else if (widgetName == QLatin1String("QStackedWidget")) {
        w = new QDesignerStackedWidget(parentWidget);
    } else if (widgetName == QLatin1String("QToolBox")) {
        w = new QDesignerToolBox(parentWidget);
    } else if (widgetName == QLatin1String("QToolBar")) {
        w = new QDesignerToolBar(parentWidget);
    } else if (widgetName == QLatin1String("QMenuBar")) {
        w = new QDesignerMenuBar(parentWidget);
    } else if (widgetName == QLatin1String("QMenu")) {
        w = new QDesignerMenu(parentWidget);
    } else if (widgetName == QLatin1String("Spacer")) {
        w = new Spacer(parentWidget);
    } else if (widgetName == QLatin1String("QDockWidget")) {
        w = new QDesignerDockWidget(parentWidget);
    } else if (widgetName == QLatin1String("QLayoutWidget")) {
        w = fw ? new QLayoutWidget(fw, parentWidget) : new QWidget(parentWidget);
    } else if (widgetName == QLatin1String("QDialog")) {
        if (fw) {
            w = new QDesignerDialog(fw, parentWidget);
        } else {
            w = new QDialog(parentWidget);
        }
    } else if (widgetName == QLatin1String("QWidget")) {
        if (fw && parentWidget &&
             (qobject_cast<QDesignerFormWindowInterface*>(parentWidget) || qt_extension<QDesignerContainerExtension*>(m_core->extensionManager(), parentWidget))) {
             w = new QDesignerWidget(fw, qobject_cast<QDesignerFormWindowInterface*>(parentWidget) ? parentWidget : 0);
        } else {
            w = new QWidget(parentWidget);
        }
    }

#define DECLARE_LAYOUT(L, C)
#define DECLARE_COMPAT_WIDGET(W, C) /*DECLARE_WIDGET(W, C)*/
#define DECLARE_WIDGET(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(parentWidget); }
#define DECLARE_WIDGET_1(W, C) else if (widgetName == QLatin1String(#W)) { Q_ASSERT(w == 0); w = new W(0, parentWidget); }

#include "widgets.table"

#undef DECLARE_COMPAT_WIDGET
#undef DECLARE_LAYOUT
#undef DECLARE_WIDGET
#undef DECLARE_WIDGET_1

    if (w == 0) {
        QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
        QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfClassName(widgetName));

        if (item == 0) {
            item = new WidgetDataBaseItem(widgetName, tr("%1 Widget").arg(widgetName));
            item->setCustom(true);
            item->setPromoted(true);
            item->setExtends(QLatin1String("QWidget"));
            item->setContainer(true);
            item->setIncludeFile(widgetName.toLower() + QLatin1String(".h"));
            db->append(item);
        }

        QString baseClass = item->extends();
        if (baseClass.isEmpty())
            baseClass = QLatin1String("QWidget");

        QDesignerPromotedWidget *promoted = new QDesignerPromotedWidget(item, parentWidget);
        QWidget *child = createWidget(baseClass, promoted);
        promoted->setChildWidget(child);

        w = promoted;
    }

    Q_ASSERT(w != 0);

    if (fw != 0)
        initialize(w);

    return w;
}

const char *WidgetFactory::classNameOf(QObject* o)
{
    if (o == 0)
        return 0;

    if (qobject_cast<QDesignerTabWidget*>(o))
        return "QTabWidget";
    else if (qobject_cast<QDesignerStackedWidget*>(o))
        return "QStackedWidget";
    else if (qobject_cast<QDesignerMenuBar*>(o))
        return "QMenuBar";
    else if (qobject_cast<QDesignerToolBar*>(o))
        return "QToolBar";
    else if (qobject_cast<QDesignerDockWidget*>(o))
        return "QDockWidget";
    else if (qobject_cast<QDesignerToolBox*>(o))
        return "QToolBox";
    else if (qobject_cast<QDesignerDialog*>(o))
        return "QDialog";
    else if (qobject_cast<QDesignerWidget*>(o))
        return "QWidget";
    else if (qobject_cast<QDesignerLabel*>(o))
        return "QLabel";
    else if (qstrcmp(o->metaObject()->className(), "QAxBase") == 0)
        return "QAxWidget";
    else if (qstrcmp(o->metaObject()->className(), "QDesignerQ3WidgetStack") == 0)
        return "Q3WidgetStack";
    else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(o))
        return promoted->customClassName();

    return o->metaObject()->className();
}

/*!  Creates a layout on the widget \a widget of the type \a type
  which can be \c HBox, \c VBox or \c Grid.
*/

QLayout *WidgetFactory::createLayout(QWidget *widget, QLayout *parentLayout, int type) const // ### (sizepolicy)
{
    QDesignerMetaDataBaseInterface *metaDataBase = core()->metaDataBase();

    if (parentLayout == 0) {
        widget = containerOfWidget(widget);
    }

    Q_ASSERT(metaDataBase->item(widget) != 0); // ensure the widget is managed

    if (parentLayout == 0 && metaDataBase->item(widget->layout()) == 0) {
        parentLayout = widget->layout();
    }

    QWidget *parentWidget = parentLayout != 0 ? 0 : widget;

    QLayout *layout = 0;
    switch (type) {
    case LayoutInfo::HBox:
        layout = new QHBoxLayout(parentWidget);
        break;
    case LayoutInfo::VBox:
        layout = new QVBoxLayout(parentWidget);
        break;
    case LayoutInfo::Grid:
        layout = new QGridLayout(parentWidget);
        break;
    case LayoutInfo::Stacked:
        layout = new QStackedLayout(parentWidget);
        break;
    default:
        Q_ASSERT(0);
        return 0;
    } // end switch

    metaDataBase->add(layout); // add the layout in the MetaDataBase

    if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(widget)) {
        layoutWidget->setLayoutMargin(0);
    }

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), layout)) {
        sheet->setChanged(sheet->indexOf(QLatin1String("margin")), true);
        sheet->setChanged(sheet->indexOf(QLatin1String("spacing")), true);
        sheet->setChanged(sheet->indexOf(QLatin1String("alignment")), true);
    }

    if (widget && metaDataBase->item(widget->layout()) == 0) {
        Q_ASSERT(layout->parent() == 0);
        QBoxLayout *box = qobject_cast<QBoxLayout*>(widget->layout());
        Q_ASSERT(box != 0); // we support only unmanaged box layouts
        box->addLayout(layout);
    }

    return layout;
}

/*!  Returns the widget into which children should be inserted when \a
  w is a container known to the designer.

  Usually that is \a w itself, sometimes it is different (e.g. a
  tabwidget is known to the designer as a container but the child
  widgets should be inserted into the current page of the
  tabwidget. So in this case this function returns the current page of
  the tabwidget.)
 */
QWidget* WidgetFactory::containerOfWidget(QWidget *w) const
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w))
        return containerOfWidget(promoted->child());
    else if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), w))
        return container->widget(container->currentIndex());

    return w;
}

/*!  Returns the actual designer widget of the container \a w. This is
  normally \a w itself, but might be a parent or grand parent of \a w
  (e.g. when working with a tabwidget and \a w is the container which
  contains and layouts childs, but the actual widget known to the
  designer is the tabwidget which is the parent of \a w. So this
  function returns the tabwidget then.)
*/

QWidget* WidgetFactory::widgetOfContainer(QWidget *w) const
{
    // ### cleanup
    if (!w)
        return 0;

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(w))
        return widgetOfContainer(promoted->child());

    if (w->parentWidget() && w->parentWidget()->parentWidget() &&
         w->parentWidget()->parentWidget()->parentWidget() &&
         qobject_cast<QToolBox*>(w->parentWidget()->parentWidget()->parentWidget()))
        return w->parentWidget()->parentWidget()->parentWidget();

    while (w != 0) {
        if (core()->widgetDataBase()->isContainer(w) ||
             w && qobject_cast<QDesignerFormWindowInterface*>(w->parentWidget()))
            return w;

        w = w->parentWidget();
    }

    return w;
}

QDesignerFormEditorInterface *WidgetFactory::core() const
{
    return m_core;
}

void WidgetFactory::initialize(QObject *object) const
{
    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(m_core->extensionManager(), object);

    if (object->metaObject()->indexOfProperty("focusPolicy") != -1)
        object->setProperty("focusPolicy", Qt::NoFocus);

    if (!sheet)
        return;

    sheet->setChanged(sheet->indexOf(QLatin1String("objectName")), true);
    sheet->setChanged(sheet->indexOf(QLatin1String("geometry")), true);

    if (qobject_cast<Spacer*>(object))
        sheet->setChanged(sheet->indexOf(QLatin1String("sizeHint")), true);

    int o = sheet->indexOf(QLatin1String("orientation"));
    if (o != -1)
        sheet->setChanged(o, true);

    if (QWidget *widget = qobject_cast<QWidget*>(object)) {
        QSize sz = widget->sizeHint();

        if (qobject_cast<QFrame*>(object) && sz.width() <= 0 && sz.height() <= 0)
            widget->setMinimumSize(QSize(16, 16));

        widget->setAttribute(Qt::WA_TransparentForMouseEvents, false);

        if (!(qobject_cast<QDesignerWidget*>(widget) || qobject_cast<QDesignerDialog*>(widget)))
            widget->setAttribute(Qt::WA_TintedBackground);
    }

    if (qobject_cast<QDockWidget*>(object) || qobject_cast<QToolBar*>(object)) {
        sheet->setVisible(sheet->indexOf(QLatin1String("windowTitle")), true);

        if (qobject_cast<QDockWidget*>(object)) {
            sheet->setVisible(sheet->indexOf(QLatin1String("windowIcon")), true);
        }
    }

    if (qobject_cast<QAction*>(object)) {
        sheet->setChanged(sheet->indexOf(QLatin1String("text")), true);
    }

    if (qobject_cast<QMenu*>(object)) {
        sheet->setChanged(sheet->indexOf(QLatin1String("geometry")), false);
        sheet->setChanged(sheet->indexOf(QLatin1String("title")), true);
    }

    if (qobject_cast<QMenu*>(object) || qobject_cast<QMenuBar*>(object)) {
        qobject_cast<QWidget*>(object)->setFocusPolicy(Qt::StrongFocus);
    }
}

bool WidgetFactory::isPassiveInteractor(QWidget *widget)
{
    if (m_lastPassiveInteractor != 0 && (QWidget*)(*m_lastPassiveInteractor) == widget)
        return m_lastWasAPassiveInteractor;

    m_lastWasAPassiveInteractor = false;
    (*m_lastPassiveInteractor) = widget;

    if (QApplication::activePopupWidget()) // if a popup is open, we have to make sure that this one is closed, else X might do funny things
        return (m_lastWasAPassiveInteractor = true);
    else if (widget == 0)
        return m_lastWasAPassiveInteractor;

    if (qobject_cast<QTabBar*>(widget))
        return (m_lastWasAPassiveInteractor = true);
    else if (qobject_cast<QSizeGrip*>(widget))
        return (m_lastWasAPassiveInteractor = true);
    else if (qobject_cast<QAbstractButton*>(widget) && (qobject_cast<QTabBar*>(widget->parent()) || qobject_cast<QToolBox*>(widget->parent())))
        return (m_lastWasAPassiveInteractor = true);
    else if (qobject_cast<QMenuBar*>(widget))
        return (m_lastWasAPassiveInteractor = true);
    else if (qstrcmp(widget->metaObject()->className(), "QDockWidgetTitle") == 0)
        return (m_lastWasAPassiveInteractor = true);
    else if (qstrcmp(widget->metaObject()->className(), "QToolBarHandle") == 0)
        return (m_lastWasAPassiveInteractor = true);
    else if (widget->objectName().startsWith(QLatin1String("__qt__passive_")))
        return (m_lastWasAPassiveInteractor = true);

    return m_lastWasAPassiveInteractor;
}

} // namespace qdesigner_internal
