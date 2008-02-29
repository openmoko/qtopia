/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the Qt Designer of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "qdesigner_resource.h"
#include "formwindow.h"
#include "qdesigner_tabwidget_p.h"
#include "qdesigner_toolbox_p.h"
#include "qdesigner_stackedbox_p.h"
#include "qdesigner_toolbar_p.h"
#include "qdesigner_dockwidget_p.h"
#include "qdesigner_menu_p.h"
#include "qdesigner_menubar_p.h"
#include "ui4_p.h"

// shared
#include <widgetdatabase_p.h>
#include <layout_p.h>
#include <layoutinfo_p.h>
#include <spacer_widget_p.h>
#include <resourcefile_p.h>
#include <pluginmanager_p.h>
#include <metadatabase_p.h>
#include <widgetfactory_p.h>
#include <abstractlanguage.h>

#include <qdesigner_widget_p.h>
#include <qlayout_widget_p.h>
#include <qdesigner_promotedwidget_p.h>
#include <qdesigner_utils_p.h>
#include <ui4_p.h>

// sdk
#include <QtDesigner/QtDesigner>

#include <QtGui/QMenu>
#include <QtGui/QMessageBox>
#include <QtGui/QLayout>
#include <QtGui/QTabWidget>
#include <QtGui/QToolBox>
#include <QtGui/QTabBar>
#include <QtGui/QAction>
#include <QtGui/QActionGroup>
#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include <QtGui/QSplitter>
#include <QtGui/QMenuBar>
#include <QtGui/QFileDialog>

#include <QtCore/QBuffer>
#include <QtCore/QDir>
#include <QtCore/QProcess>
#include <QtCore/QLibraryInfo>
#include <QtCore/QMetaProperty>
#include <QtCore/qdebug.h>

#include <QtXml/QDomDocument>


using namespace qdesigner_internal;

QDesignerResource::QDesignerResource(FormWindow *formWindow)
   : QSimpleResource(formWindow->core()), m_formWindow(formWindow)
{
    setWorkingDirectory(formWindow->absoluteDir());

    m_topLevelSpacerCount = 0;
    m_copyWidget = false;
    m_selected = 0;

    // ### generalise
    m_internal_to_qt.insert(QLatin1String("QLayoutWidget"), QLatin1String("QWidget"));
    m_internal_to_qt.insert(QLatin1String("QDesignerWidget"), QLatin1String("QWidget"));
    m_internal_to_qt.insert(QLatin1String("QDesignerStackedWidget"), QLatin1String("QStackedWidget"));
    m_internal_to_qt.insert(QLatin1String("QDesignerTabWidget"), QLatin1String("QTabWidget"));
    m_internal_to_qt.insert(QLatin1String("QDesignerDialog"), QLatin1String("QDialog"));
    m_internal_to_qt.insert(QLatin1String("QDesignerLabel"), QLatin1String("QLabel"));
    m_internal_to_qt.insert(QLatin1String("QDesignerToolBox"), QLatin1String("QToolBox"));
    m_internal_to_qt.insert(QLatin1String("QDesignerToolBar"), QLatin1String("QToolBar"));
    m_internal_to_qt.insert(QLatin1String("QDesignerMenuBar"), QLatin1String("QMenuBar"));
    m_internal_to_qt.insert(QLatin1String("QDesignerMenu"), QLatin1String("QMenu"));
    m_internal_to_qt.insert(QLatin1String("QDesignerDockWidget"), QLatin1String("QDockWidget"));
    m_internal_to_qt.insert(QLatin1String("QDesignerQ3WidgetStack"), QLatin1String("Q3WidgetStack"));

    // invert
    QHashIterator<QString, QString> it(m_internal_to_qt);
    while (it.hasNext()) {
        it.next();

        if (it.value() == QLatin1String("QDesignerWidget")
                || it.value() == QLatin1String("QLayoutWidget"))
            continue;

        m_qt_to_internal.insert(it.value(), it.key());
    }
}

QDesignerResource::~QDesignerResource()
{
}

void QDesignerResource::save(QIODevice *dev, QWidget *widget)
{
    m_topLevelSpacerCount = 0;

    QAbstractFormBuilder::save(dev, widget);

    if (m_topLevelSpacerCount != 0) {
        QMessageBox::warning(widget->window(), QApplication::translate("Designer", "Qt Designer"),
               QApplication::translate("Designer", "This file contains top level spacers.<br>"
                           "They have <b>NOT</b> been saved into the form.<br>"
                           "Perhaps you forgot to create a layout?"),
                           QMessageBox::Ok, 0);
    }
}

void QDesignerResource::saveDom(DomUI *ui, QWidget *widget)
{
    QAbstractFormBuilder::saveDom(ui, widget);

    QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), widget);
    Q_ASSERT(sheet != 0);

    ui->setElementClass(sheet->property(sheet->indexOf(QLatin1String("objectName"))).toString());

    for (int index = 0; index < m_formWindow->toolCount(); ++index) {
        QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
        Q_ASSERT(tool != 0);
        tool->saveToDom(ui, widget);
    }

    QString author = m_formWindow->author();
    if (!author.isEmpty()) {
        ui->setElementAuthor(author);
    }

    QString comment = m_formWindow->comment();
    if (!comment.isEmpty()) {
        ui->setElementComment(comment);
    }

    QString exportMacro = m_formWindow->exportMacro();
    if (!exportMacro.isEmpty()) {
        ui->setElementExportMacro(exportMacro);
    }

    if (!m_formWindow->includeHints().isEmpty()) {
        QList<DomInclude*> ui_includes;
        foreach (QString includeHint, m_formWindow->includeHints()) {
            if (includeHint.isEmpty())
                continue;

            DomInclude *incl = new DomInclude;
            QString location = QLatin1String("local");
            if (includeHint.at(0) == QLatin1Char('<'))
                location = QLatin1String("global");

            includeHint = includeHint
                .replace(QLatin1Char('"'), "")
                .replace(QLatin1Char('<'), "")
                .replace(QLatin1Char('>'), "");

            incl->setAttributeLocation(location);
            incl->setText(includeHint);
            ui_includes.append(incl);
        }

        DomIncludes *includes = new DomIncludes;
        includes->setElementInclude(ui_includes);
        ui->setElementIncludes(includes);
    }

    int defaultMargin = INT_MIN, defaultSpacing = INT_MIN;
    m_formWindow->layoutDefault(&defaultMargin, &defaultSpacing);

    if (defaultMargin != INT_MIN || defaultSpacing != INT_MIN) {
        DomLayoutDefault *def = new DomLayoutDefault;
        if (defaultMargin != INT_MIN)
            def->setAttributeMargin(defaultMargin);
        if (defaultSpacing != INT_MIN)
            def->setAttributeSpacing(defaultSpacing);
        ui->setElementLayoutDefault(def);
    }

    QString marginFunction, spacingFunction;
    m_formWindow->layoutFunction(&marginFunction, &spacingFunction);
    if (!marginFunction.isEmpty() || !spacingFunction.isEmpty()) {
        DomLayoutFunction *def = new DomLayoutFunction;

        if (!marginFunction.isEmpty())
            def->setAttributeMargin(marginFunction);
        if (!spacingFunction.isEmpty())
            def->setAttributeSpacing(spacingFunction);
        ui->setElementLayoutFunction(def);
    }

    QString pixFunction = m_formWindow->pixmapFunction();
    if (!pixFunction.isEmpty()) {
        ui->setElementPixmapFunction(pixFunction);
    }
}

QWidget *QDesignerResource::create(DomUI *ui, QWidget *parentWidget)
{
    QString version = ui->attributeVersion();
    if (version != QLatin1String("4.0")) {

        // Try to convert it ourselves.
        QProcess uic3;
        uic3.start(QLibraryInfo::location(QLibraryInfo::BinariesPath)
                   + QDir::separator() + QLatin1String("uic3"), QStringList()
                   << QLatin1String("-convert") << m_formWindow->fileName());
        bool allOK = uic3.waitForFinished();
        if (allOK) {
            QByteArray ba = uic3.readAll();
            QBuffer buffer(&ba);
            m_formWindow->setFileName(QString());
            QWidget *w = load(&buffer, parentWidget);
            if (!w) {
                allOK = false;
            } else {
                QMessageBox::information(parentWidget->window(), QApplication::translate("Designer", "Qt Designer"),
                   QApplication::translate("Designer", "This file was created using Designer from Qt-%1 and"
                               " will be converted to a new form by Qt Designer.\n"
                               "The old form has been untouched, but you will have to save this form"
                               " under a new name.").arg(version), QMessageBox::Ok, 0);
                return w;
            }
        }

        if (!allOK) {
            QMessageBox::warning(parentWidget->window(), QApplication::translate("Designer", "Qt Designer"),
               QApplication::translate("Designer", "This file was created using designer from Qt-%1 and "
                           "could not be read. "
                           "Please run it through <b>uic3 -convert</b> to convert "
                           "it to Qt-4's ui format.").arg(version),
                               QMessageBox::Ok, 0);
            return 0;
        }
    }

    qdesigner_internal::WidgetFactory *factory = qobject_cast<qdesigner_internal::WidgetFactory*>(core()->widgetFactory());
    Q_ASSERT(factory != 0);

    QDesignerFormWindowInterface *previousFormWindow = factory->currentFormWindow(m_formWindow);

    m_isMainWidget = true;
    QWidget *mainWidget = QAbstractFormBuilder::create(ui, parentWidget);

    if (mainWidget && m_formWindow) {
        m_formWindow->setAuthor(ui->elementAuthor());
        m_formWindow->setComment(ui->elementComment());
        m_formWindow->setExportMacro(ui->elementExportMacro());
        m_formWindow->setPixmapFunction(ui->elementPixmapFunction());

        if (DomLayoutDefault *def = ui->elementLayoutDefault()) {
            m_formWindow->setLayoutDefault(def->attributeMargin(), def->attributeSpacing());
        }

        if (DomLayoutFunction *fun = ui->elementLayoutFunction()) {
            m_formWindow->setLayoutFunction(fun->attributeMargin(), fun->attributeSpacing());
        }

        if (DomIncludes *includes = ui->elementIncludes()) {
            QStringList includeHints;
            foreach (DomInclude *incl, includes->elementInclude()) {
                QString text = incl->text();

                if (text.isEmpty())
                    continue;

                if (incl->hasAttributeLocation() && incl->attributeLocation() == QLatin1String("global")) {
                    text = text.prepend('<').append('>');
                } else {
                    text = text.prepend('"').append('"');
                }

                includeHints.append(text);
            }

            m_formWindow->setIncludeHints(includeHints);
        }

        for (int index = 0; index < m_formWindow->toolCount(); ++index) {
            QDesignerFormWindowToolInterface *tool = m_formWindow->tool(index);
            Q_ASSERT(tool != 0);
            tool->loadFromDom(ui, mainWidget);
        }
    }

    factory->currentFormWindow(previousFormWindow);

    return mainWidget;
}

QWidget *QDesignerResource::create(DomWidget *ui_widget, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QString className = ui_widget->attributeClass();
    if (!m_isMainWidget && className == QLatin1String("QWidget") && ui_widget->elementLayout().size() &&
                !ui_widget->hasAttributeNative()) {
        // ### check if elementLayout.size() == 1

        QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget);

        if (container == 0) {
            // generate a QLayoutWidget iff the parent is not an QDesignerContainerExtension.
            ui_widget->setAttributeClass(QLatin1String("QLayoutWidget"));
        }
    }

    // save the actions
    QList<DomActionRef*> actionRefs = ui_widget->elementAddAction();
    ui_widget->setElementAddAction(QList<DomActionRef*>());

    QWidget *w = QAbstractFormBuilder::create(ui_widget, parentWidget);

    // restore the actions
    ui_widget->setElementAddAction(actionRefs);

    if (w == 0)
        return 0;

    // ### generalize using the extension manager
    QDesignerMenu *menu = qobject_cast<QDesignerMenu*>(w);
    QDesignerMenuBar *menuBar = qobject_cast<QDesignerMenuBar*>(w);
    QDesignerToolBar *toolBar = qobject_cast<QDesignerToolBar*>(w);

    if (menu) {
        menu->interactive(false);
        menu->hide();
    } else if (menuBar) {
        menuBar->interactive(false);
    } else if (toolBar) {
        toolBar->interactive(false);
    }

    foreach (DomActionRef *ui_action_ref, actionRefs) {
        QString name = ui_action_ref->attributeName();
        if (name == QLatin1String("separator")) {
            QAction *sep = new QAction(w);
            sep->setSeparator(true);
            w->addAction(sep);
            addMenuAction(sep);
        } else if (QAction *a = m_actions.value(name)) {
            w->addAction(a);
        } else if (QActionGroup *g = m_actionGroups.value(name)) {
            w->addActions(g->actions());
        } else if (QMenu *menu = qFindChild<QMenu*>(w, name)) {
            w->addAction(menu->menuAction());
            addMenuAction(menu->menuAction());
        }
    }

    if (menu) {
        menu->interactive(true);
        menu->adjustSpecialActions();
    } else if (menuBar) {
        menuBar->interactive(true);
        menuBar->adjustSpecialActions();
    } else if (toolBar) {
        toolBar->interactive(true);
        toolBar->adjustSpecialActions();
    }


    ui_widget->setAttributeClass(className); // fix the class name

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core()->extensionManager(), w)) {
        extra->loadWidgetExtraInfo(ui_widget);
    }

    return w;
}

QLayout *QDesignerResource::create(DomLayout *ui_layout, QLayout *layout, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QLayout *l = QAbstractFormBuilder::create(ui_layout, layout, parentWidget);

    if (QGridLayout *gridLayout = qobject_cast<QGridLayout*>(l))
        QLayoutSupport::createEmptyCells(gridLayout);

    return l;
}

QLayoutItem *QDesignerResource::create(DomLayoutItem *ui_layoutItem, QLayout *layout, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    if (ui_layoutItem->kind() == DomLayoutItem::Spacer) {
        QHash<QString, DomProperty*> properties = propertyMap(ui_layoutItem->elementSpacer()->elementProperty());

        Spacer *spacer = (Spacer*) core()->widgetFactory()->createWidget(QLatin1String("Spacer"), parentWidget);
        core()->metaDataBase()->add(spacer);

        spacer->setInteraciveMode(false);
        applyProperties(spacer, ui_layoutItem->elementSpacer()->elementProperty());
        spacer->setInteraciveMode(true);

        if (m_formWindow) {
            m_formWindow->manageWidget(spacer);
            if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), spacer))
                sheet->setChanged(sheet->indexOf(QLatin1String("orientation")), true);
        }

        return new QWidgetItem(spacer);
    } else if (ui_layoutItem->kind() == DomLayoutItem::Layout && parentWidget) {
        DomLayout *ui_layout = ui_layoutItem->elementLayout();
        QLayoutWidget *layoutWidget = new QLayoutWidget(m_formWindow, parentWidget);
        core()->metaDataBase()->add(layoutWidget);
        applyProperties(layoutWidget, ui_layout->elementProperty());

        if (m_formWindow) {
            m_formWindow->manageWidget(layoutWidget);
        }

        (void) create(ui_layout, 0, layoutWidget);
        return new QWidgetItem(layoutWidget);
    }
    return QAbstractFormBuilder::create(ui_layoutItem, layout, parentWidget);
}

void QDesignerResource::changeObjectName(QObject *o, QString objName)
{
    m_formWindow->unify(o, objName, true);

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(o)) {
        if (objName.startsWith(QLatin1String("__qt__promoted_"))) {
            promoted->setObjectName(objName);
            promoted->child()->setObjectName(objName.mid(15));
        } else {
            promoted->child()->setObjectName(objName);
            promoted->setObjectName(QLatin1String("__qt__promoted_") + objName);
        }
    } else {
        o->setObjectName(objName);
    }

}

void QDesignerResource::applyProperties(QObject *o, const QList<DomProperty*> &properties)
{
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), o)) {

        for (int i=0; i<properties.size(); ++i) {
            DomProperty *p = properties.at(i);
            QString propertyName = p->attributeName();

            int index = sheet->indexOf(propertyName);
            if (index != -1) {
                QObject *realObject = o;
                if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(o)) {
                    realObject = promoted->child();
                }
                const QMetaObject *meta = realObject->metaObject();
                QVariant v = toVariant(meta, p);

                MetaDataBaseItem *item = 0;
                if (core()->metaDataBase())
                    item = static_cast<MetaDataBaseItem*>(core()->metaDataBase()->item(realObject));

                if (!item) {
                    qWarning() << "** WARNING no ``meta database item'' for object:" << realObject;
                }

                if (p->kind() == DomProperty::String && item) {
                    DomString *str = p->elementString();
                    if (str->hasAttributeComment()) {
                        item->setPropertyComment(propertyName, str->attributeComment());
                    }
                }

                // ### move me
                if (QLayout *layout = qobject_cast<QLayout*>(o)) {
                    if (propertyName == QLatin1String("margin") && qobject_cast<QLayoutWidget*>(layout->parentWidget()))
                        v = v.toInt() + 1;
                }

                sheet->setProperty(index, v);
                sheet->setChanged(index, true);
            }

            if (propertyName == QLatin1String("objectName"))
                changeObjectName(o, o->objectName());
        }
        QSplitter *splitter = qobject_cast<QSplitter *>(o);
        if (splitter) {
            QDesignerWidgetFactoryInterface *widgetFactory = core()->widgetFactory();
            widgetFactory->createLayout(splitter, 0, splitter->orientation() == Qt::Horizontal ?
                        LayoutInfo::HBox : LayoutInfo::VBox);
        }
    }
}

QWidget *QDesignerResource::createWidget(const QString &widgetName, QWidget *parentWidget, const QString &_name)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parentWidget))
        parentWidget = promoted->child();

    QString name = _name;
    QString className = widgetName;
    if (m_isMainWidget)
        m_isMainWidget = false;

    QWidget *w = core()->widgetFactory()->createWidget(className, parentWidget);
    if (!w)
        return 0;

    if (name.isEmpty()) {
        QDesignerWidgetDataBaseInterface *db = core()->widgetDataBase();
        if (QDesignerWidgetDataBaseItemInterface *item = db->item(db->indexOfObject(w)))
            name = qtify(item->name());
    }

    changeObjectName(w, name);

    QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget);
    if (!qobject_cast<QMenu*>(w) && (!parentWidget || !container)) {
        m_formWindow->manageWidget(w);
    } else {
        core()->metaDataBase()->add(w);
    }

    w->setWindowFlags(w->windowFlags() & ~Qt::Window);

    return w;
}

QLayout *QDesignerResource::createLayout(const QString &layoutName, QObject *parent, const QString &name)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(parent))
        parent = promoted->child();

    QWidget *layoutBase = 0;
    QLayout *layout = qobject_cast<QLayout*>(parent);

    if (parent->isWidgetType())
        layoutBase = static_cast<QWidget*>(parent);
    else {
        Q_ASSERT( layout != 0 );
        layoutBase = layout->parentWidget();
    }

    LayoutInfo::Type layoutType = LayoutInfo::Grid;
    if (layoutName == QLatin1String("QVBoxLayout"))
        layoutType = LayoutInfo::VBox;
    else if (layoutName == QLatin1String("QHBoxLayout"))
        layoutType = LayoutInfo::HBox;
    else if (layoutName == QLatin1String("QStackedLayout"))
        layoutType = LayoutInfo::Stacked;

    QLayout *lay = core()->widgetFactory()->createLayout(layoutBase, layout, layoutType);
    if (lay != 0)
        changeObjectName(lay, name);

    return lay;
}

// save
DomWidget *QDesignerResource::createDom(QWidget *widget, DomWidget *ui_parentWidget, bool recursive)
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(widget);
    if (!item)
        return 0;

    if (qobject_cast<Spacer*>(widget) && m_copyWidget == false) {
        ++m_topLevelSpacerCount;
        return 0;
    }

    QDesignerWidgetDataBaseItemInterface *widgetInfo =  0;
    int widgetInfoIndex = core()->widgetDataBase()->indexOfObject(widget, false);
    if (widgetInfoIndex != -1) {
        widgetInfo = core()->widgetDataBase()->item(widgetInfoIndex);

        if (widgetInfo->isCustom()) {
            if (widgetInfo->extends().isEmpty()) {
                const QMetaObject *mo = widget->metaObject()->superClass();
                while (mo != 0) {
                    if (core()->widgetDataBase()->indexOfClassName(QLatin1String(mo->className())) != -1) {
                        widgetInfo->setExtends(QLatin1String(mo->className()));
                        break;
                    }
                    mo = mo->superClass();
                }
            }
            m_usedCustomWidgets.insert(widgetInfo, true);
        }
    }

    DomWidget *w = 0;

    if (QDesignerTabWidget *tabWidget = qobject_cast<QDesignerTabWidget*>(widget))
        w = saveWidget(tabWidget, ui_parentWidget);
    else if (QDesignerStackedWidget *stackedWidget = qobject_cast<QDesignerStackedWidget*>(widget))
        w = saveWidget(stackedWidget, ui_parentWidget);
    else if (QDesignerToolBox *toolBox = qobject_cast<QDesignerToolBox*>(widget))
        w = saveWidget(toolBox, ui_parentWidget);
    else if (QDesignerToolBar *toolBar = qobject_cast<QDesignerToolBar*>(widget))
        w = saveWidget(toolBar, ui_parentWidget);
    else if (QDesignerDockWidget *dockWidget = qobject_cast<QDesignerDockWidget*>(widget))
        w = saveWidget(dockWidget, ui_parentWidget);
    else if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget))
        w = saveWidget(widget, container, ui_parentWidget);
    else if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget))
        w = createDom(promoted->child(), ui_parentWidget, recursive);
    else
        w = QAbstractFormBuilder::createDom(widget, ui_parentWidget, recursive);

    Q_ASSERT( w != 0 );

    if (!qobject_cast<QLayoutWidget*>(widget) && w->attributeClass() == QLatin1String("QWidget")) {
        w->setAttributeNative(true);
    }

    QString className = w->attributeClass();
    if (m_internal_to_qt.contains(className))
        w->setAttributeClass(m_internal_to_qt.value(className));

    w->setAttributeName(widget->objectName());

    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget)) {
        Q_ASSERT(widgetInfo != 0);

        w->setAttributeName(promoted->child()->objectName());
        w->setAttributeClass(widgetInfo->name());

        QList<DomProperty*> prop_list = w->elementProperty();
        foreach (DomProperty *prop, prop_list) {
            if (prop->attributeName() == QLatin1String("geometry")) {
                if (DomRect *rect = prop->elementRect()) {
                    rect->setElementX(widget->x());
                    rect->setElementY(widget->y());
                }
                break;
            }
        }
    } else if (widgetInfo != 0 && m_usedCustomWidgets.contains(widgetInfo)) {
        if (widgetInfo->name() != w->attributeClass())
            w->setAttributeClass(widgetInfo->name());
    }

    if (QDesignerExtraInfoExtension *extra = qt_extension<QDesignerExtraInfoExtension*>(core()->extensionManager(), widget)) {
        extra->saveWidgetExtraInfo(w);
    }

    return w;
}

DomLayout *QDesignerResource::createDom(QLayout *layout, DomLayout *ui_parentLayout, DomWidget *ui_parentWidget)
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(layout);

    if (item == 0) {
        layout = qFindChild<QLayout*>(layout);
        // refresh the meta database item
        item = core()->metaDataBase()->item(layout);
    }

    if (item == 0) {
        // nothing to do.
        return 0;
    }

    if (qobject_cast<QSplitter*>(layout->parentWidget()) != 0) {
        // nothing to do.
        return 0;
    }

    m_chain.push(layout);

    DomLayout *l = QAbstractFormBuilder::createDom(layout, ui_parentLayout, ui_parentWidget);
    Q_ASSERT(l != 0);

    m_chain.pop();

    return l;
}

DomLayoutItem *QDesignerResource::createDom(QLayoutItem *item, DomLayout *ui_layout, DomWidget *ui_parentWidget)
{
    DomLayoutItem *ui_item = 0;

    if (Spacer *s = qobject_cast<Spacer*>(item->widget())) {
        if (!core()->metaDataBase()->item(s))
            return 0;

        DomSpacer *spacer = new DomSpacer();
        QList<DomProperty*> properties = computeProperties(item->widget());
        // ### filter the properties
        spacer->setElementProperty(properties);

        ui_item = new DomLayoutItem();
        ui_item->setElementSpacer(spacer);
        m_laidout.insert(item->widget(), true);
    } else if (QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(item->widget())) {
        Q_ASSERT(layoutWidget->layout());
        DomLayout *l = createDom(layoutWidget->layout(), ui_layout, ui_parentWidget);
        ui_item = new DomLayoutItem();
        ui_item->setElementLayout(l);
        m_laidout.insert(item->widget(), true);
    } else if (!item->spacerItem()) { // we use spacer as fake item in the Designer
        ui_item = QAbstractFormBuilder::createDom(item, ui_layout, ui_parentWidget);
    } else {
        return 0;
    }

    if (m_chain.size() && item->widget()) {
        if (QGridLayout *grid = qobject_cast<QGridLayout*>(m_chain.top())) {
            int index = Utils::indexOfWidget(grid, item->widget());

            int row, column, rowspan, colspan;
            grid->getItemPosition(index, &row, &column, &rowspan, &colspan);
            ui_item->setAttributeRow(row);
            ui_item->setAttributeColumn(column);

            if (colspan != 1)
                ui_item->setAttributeColSpan(colspan);

            if (rowspan != 1)
                ui_item->setAttributeRowSpan(rowspan);
        }
    }

    return ui_item;
}

void QDesignerResource::createCustomWidgets(DomCustomWidgets *dom_custom_widgets)
{
    if (dom_custom_widgets == 0)
        return;
    QList<DomCustomWidget*> custom_widget_list = dom_custom_widgets->elementCustomWidget();
    QDesignerWidgetDataBaseInterface *db = m_formWindow->core()->widgetDataBase();
    foreach(DomCustomWidget *custom_widget, custom_widget_list) {
        WidgetDataBaseItem *item
            = new WidgetDataBaseItem(custom_widget->elementClass());
        QString base_class = custom_widget->elementExtends();
        item->setExtends(base_class);
        item->setPromoted(!base_class.isEmpty());
        item->setGroup(base_class.isEmpty() ? QApplication::translate("Designer", "Custom Widgets")
                                                : QApplication::translate("Designer", "Promoted Widgets"));
        if (DomHeader *header = custom_widget->elementHeader())
            item->setIncludeFile(header->text());
        item->setContainer(custom_widget->elementContainer());
        item->setCustom(true);
        db->append(item);
    }
}

DomTabStops *QDesignerResource::saveTabStops()
{
    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);

    QStringList tabStops;
    foreach (QWidget *widget, item->tabOrder()) {
        if (m_formWindow->mainContainer()->isAncestorOf(widget))
            tabStops.append(widget->objectName());
    }

    if (tabStops.count()) {
        DomTabStops *dom = new DomTabStops;
        dom->setElementTabStop(tabStops);
        return dom;
    }

    return 0;
}

void QDesignerResource::applyTabStops(QWidget *widget, DomTabStops *tabStops)
{
    if (!tabStops)
        return;

    QList<QWidget*> tabOrder;
    foreach (QString widgetName, tabStops->elementTabStop()) {
        if (QWidget *w = qFindChild<QWidget*>(widget, widgetName)) {
            tabOrder.append(w);
        }
    }

    QDesignerMetaDataBaseItemInterface *item = core()->metaDataBase()->item(m_formWindow);
    Q_ASSERT(item);
    item->setTabOrder(tabOrder);
}

DomWidget *QDesignerResource::saveWidget(QWidget *widget, QDesignerContainerExtension *container, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    for (int i=0; i<container->count(); ++i) {
        QWidget *page = container->widget(i);
        Q_ASSERT(page);

        DomWidget *ui_page = createDom(page, ui_widget);
        Q_ASSERT( ui_page != 0 );

        ui_widget_list.append(ui_page);
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerStackedWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;
    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerToolBar *toolBar, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(toolBar, ui_parentWidget, false);
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(toolBar->parentWidget())) {
        Qt::ToolBarArea area = mainWindow->toolBarArea(toolBar);
        DomProperty *attr = new DomProperty();
        attr->setAttributeName(QLatin1String("toolBarArea"));
        attr->setElementNumber(int(area));
        ui_widget->setElementAttribute(ui_widget->elementAttribute() << attr);
    }

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerDockWidget *dockWidget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(dockWidget, ui_parentWidget, true);
    if (QMainWindow *mainWindow = qobject_cast<QMainWindow*>(dockWidget->parentWidget())) {
        Qt::DockWidgetArea area = mainWindow->dockWidgetArea(dockWidget);
        DomProperty *attr = new DomProperty();
        attr->setAttributeName(QLatin1String("dockWidgetArea"));
        attr->setElementNumber(int(area));
        ui_widget->setElementAttribute(ui_widget->elementAttribute() << attr);
    }

    return ui_widget;
}

DomProperty *QDesignerResource::createIconProperty(const QVariant &v) const
{
    DomProperty *dom_prop = new DomProperty();

    DomResourcePixmap *r = new DomResourcePixmap;
    QString icon_path;
    QString qrc_path;
    if (v.type() == QVariant::Icon) {
        QIcon icon = qvariant_cast<QIcon>(v);
        icon_path = iconToFilePath(icon);
        qrc_path = iconToQrcPath(icon);
    } else {
        QPixmap pixmap = qvariant_cast<QPixmap>(v);
        icon_path = pixmapToFilePath(pixmap);
        qrc_path = pixmapToQrcPath(pixmap);
    }

    if (qrc_path.isEmpty())
        icon_path = workingDirectory().relativeFilePath(icon_path);
    else
        qrc_path = workingDirectory().relativeFilePath(qrc_path);

    r->setText(icon_path);
    if (!qrc_path.isEmpty())
        r->setAttributeResource(qrc_path);

    if (v.type() == QVariant::Icon)
        dom_prop->setElementIconSet(r);
    else
        dom_prop->setElementPixmap(r);

    return dom_prop;
}

DomWidget *QDesignerResource::saveWidget(QDesignerTabWidget *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            QList<DomProperty*> ui_attribute_list;

            DomProperty *p = 0;
            DomString *str = 0;

            // attribute `title'
            p = new DomProperty();
            p->setAttributeName(QLatin1String("title"));
            str = new DomString();
            str->setText(widget->tabText(i));
            p->setElementString(str);
            ui_attribute_list.append(p);

            // attribute `icon'
            if (!widget->tabIcon(i).isNull()) {
                p = createIconProperty(widget->tabIcon(i));
                p->setAttributeName(QLatin1String("icon"));
                ui_attribute_list.append(p);
            }

            // attribute `toolTip'
            if (!widget->tabToolTip(i).isEmpty()) {
                p = new DomProperty();
                p->setAttributeName(QLatin1String("toolTip"));
                str = new DomString();
                str->setText(widget->tabToolTip(i));
                p->setElementString(str);
                ui_attribute_list.append(p);
            }

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

DomWidget *QDesignerResource::saveWidget(QDesignerToolBox *widget, DomWidget *ui_parentWidget)
{
    DomWidget *ui_widget = QAbstractFormBuilder::createDom(widget, ui_parentWidget, false);
    QList<DomWidget*> ui_widget_list;

    if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), widget)) {
        for (int i=0; i<container->count(); ++i) {
            QWidget *page = container->widget(i);
            Q_ASSERT(page);

            DomWidget *ui_page = createDom(page, ui_widget);
            Q_ASSERT( ui_page != 0 );

            // attribute `label'
            DomProperty *p = new DomProperty();
            p->setAttributeName(QLatin1String("label"));
            DomString *str = new DomString();
            str->setText(widget->itemText(i));
            p->setElementString(str); // ### check f tb->indexOf(page) == i ??

            QList<DomProperty*> ui_attribute_list;
            ui_attribute_list.append(p);

            // attribute `icon'
            if (!widget->itemIcon(i).isNull()) {
                p = createIconProperty(widget->itemIcon(i));
                p->setAttributeName(QLatin1String("icon"));
                ui_attribute_list.append(p);
            }

            // attribute `toolTip'
            if (!widget->itemToolTip(i).isEmpty()) {
                p = new DomProperty();
                p->setAttributeName(QLatin1String("toolTip"));
                str = new DomString();
                str->setText(widget->itemToolTip(i));
                p->setElementString(str);
                ui_attribute_list.append(p);
            }

            ui_page->setElementAttribute(ui_attribute_list);

            ui_widget_list.append(ui_page);
        }
    }

    ui_widget->setElementWidget(ui_widget_list);

    return ui_widget;
}

bool QDesignerResource::checkProperty(QObject *obj, const QString &prop) const
{
    const QMetaObject *meta = obj->metaObject();
    int pindex = meta->indexOfProperty(prop.toLatin1());
    if (pindex != -1) {
        if (!meta->property(pindex).isStored(obj))
            return false;
    }

    if (prop == QLatin1String("objectName")) { // ### don't store the property objectName
        return false;
    } else if (prop == QLatin1String("geometry") && obj->isWidgetType()) {
        QWidget *check_widget = qobject_cast<QWidget*>(obj);
         if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(obj->parent()))
            check_widget = promoted;

         if (m_selected && m_selected == check_widget)
             return true;

        return !LayoutInfo::isWidgetLaidout(core(), check_widget);
    }

    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), obj)) {
        int pindex = sheet->indexOf(prop);
        if (sheet->isAttribute(pindex))
            return false;

        return sheet->isChanged(pindex);
    }

    return false;
}

bool QDesignerResource::addItem(DomLayoutItem *ui_item, QLayoutItem *item, QLayout *layout)
{
    if (item->widget() == 0) {
        return false;
    }

    QGridLayout *grid = qobject_cast<QGridLayout*>(layout);
    QBoxLayout *box = qobject_cast<QBoxLayout*>(layout);

    if (grid != 0) {
        int rowSpan = ui_item->hasAttributeRowSpan() ? ui_item->attributeRowSpan() : 1;
        int colSpan = ui_item->hasAttributeColSpan() ? ui_item->attributeColSpan() : 1;
        add_to_grid_layout(grid, item->widget(), ui_item->attributeRow(), ui_item->attributeColumn(),
                        rowSpan, colSpan, item->alignment());
        return true;
    } else if (box != 0) {
        add_to_box_layout(box, item->widget());
        return true;
    }

    return QAbstractFormBuilder::addItem(ui_item, item, layout);
}

bool QDesignerResource::addItem(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    core()->metaDataBase()->add(widget); // ensure the widget is in the meta database

    if (! QAbstractFormBuilder::addItem(ui_widget, widget, parentWidget) || qobject_cast<QMainWindow*> (parentWidget)) {
        if (QDesignerContainerExtension *container = qt_extension<QDesignerContainerExtension*>(core()->extensionManager(), parentWidget))
            container->addWidget(widget);
    }

    return true;
}

void QDesignerResource::copy(QIODevice *dev, const QList<QWidget*> &selection)
{
    m_copyWidget = true;

    DomUI *ui = copy(selection);
    QDomDocument doc;
    doc.appendChild(ui->write(doc));
    dev->write(doc.toString().toUtf8());

    m_laidout.clear();

    delete ui;
    m_copyWidget = false;
}

DomUI *QDesignerResource::copy(const QList<QWidget*> &selection)
{
    m_copyWidget = true;

    DomUI *ui = new DomUI();
    ui->setAttributeVersion(QLatin1String("4.0"));

    DomWidget *ui_widget = new DomWidget();
    QList<DomWidget*> ui_widget_list;
    ui_widget->setAttributeName(QLatin1String("__qt_fake_top_level"));

    for (int i=0; i<selection.size(); ++i) {
        QWidget *w = selection.at(i);
        m_selected = w;
        DomWidget *ui_child = createDom(w, ui_widget);
        m_selected = 0;
        if (!ui_child)
            continue;

        ui_widget_list.append(ui_child);
    }

    ui_widget->setElementWidget(ui_widget_list);
    ui->setElementWidget(ui_widget);

    m_laidout.clear();

    m_copyWidget = false;

    return ui;
}

QList<QWidget*> QDesignerResource::paste(DomUI *ui, QWidget *parentWidget)
{
    int saved = m_isMainWidget;
    m_isMainWidget = false;
    QList<QWidget*> createdWidgets;

    DomWidget *topLevel = ui->elementWidget();
    QList<DomWidget*> widgets = topLevel->elementWidget();
    for (int i=0; i<widgets.size(); ++i) {
        QWidget *w = create(widgets.at(i), parentWidget);
        if (!w)
            continue;

        w->move(w->pos() + m_formWindow->grid());
        // ### change the init properties of w
        createdWidgets.append(w);
    }

    m_isMainWidget = saved;

    return createdWidgets;
}

QList<QWidget*> QDesignerResource::paste(QIODevice *dev, QWidget *parentWidget)
{
    QDomDocument doc;
    if (!doc.setContent(dev))
        return QList<QWidget*>();

    QDomElement root = doc.firstChild().toElement();
    DomUI ui;
    ui.read(root);
    return paste(&ui, parentWidget);
}

void QDesignerResource::layoutInfo(DomLayout *layout, QObject *parent, int *margin, int *spacing)
{
    QAbstractFormBuilder::layoutInfo(layout, parent, margin, spacing);

    QLayoutWidget *layoutWidget = qobject_cast<QLayoutWidget*>(parent);
    if (layoutWidget && margin) {
        if (*margin == INT_MIN)
            *margin = 1;
        else
            *margin = *margin + 1;
    }
}

QString QDesignerResource::qtify(const QString &name)
{
    QString qname = name;

    if (qname.count() > 1 && qname.at(1).toUpper() == qname.at(1) && (qname.at(0) == QLatin1Char('Q') || qname.at(0) == QLatin1Char('K')))
        qname = qname.mid(1);

    int i=0;
    while (i < qname.length()) {
        if (qname.at(i).toLower() != qname.at(i))
            qname[i] = qname.at(i).toLower();
        else
            break;

        ++i;
    }

    return qname;
}

DomCustomWidgets *QDesignerResource::saveCustomWidgets()
{
    if (m_usedCustomWidgets.isEmpty())
        return 0;

    QList<DomCustomWidget*> custom_widget_list;
    foreach (QDesignerWidgetDataBaseItemInterface *item, m_usedCustomWidgets.keys()) {
        DomCustomWidget *custom_widget = new DomCustomWidget;
        custom_widget->setElementClass(item->name());
        if (item->isContainer())
            custom_widget->setElementContainer(item->isContainer());

        if (!item->includeFile().isEmpty()) {
            DomHeader *header = new DomHeader;
            header->setText(item->includeFile());
            custom_widget->setElementHeader(header);
            custom_widget->setElementExtends(item->extends());
        }

        custom_widget_list.append(custom_widget);
    }

    DomCustomWidgets *customWidgets = new DomCustomWidgets;
    customWidgets->setElementCustomWidget(custom_widget_list);
    return customWidgets;
}

QList<DomProperty*> QDesignerResource::computeProperties(QObject *object)
{
    QList<DomProperty*> properties;
    if (QDesignerPropertySheetExtension *sheet = qt_extension<QDesignerPropertySheetExtension*>(core()->extensionManager(), object)) {
        for (int index = 0; index < sheet->count(); ++index) {
            QString propertyName = sheet->propertyName(index);
            QVariant value = sheet->property(index);

            if (QLayout *layout = qobject_cast<QLayout*>(object)) {
                if (propertyName == QLatin1String("margin") && qobject_cast<QLayoutWidget*>(layout->parentWidget()))
                    value = value.toInt() - 1;
            }

            if (!sheet->isChanged(index))
                continue;

            if (DomProperty *p = createProperty(object, propertyName, value)) {
                if (p->kind() == DomProperty::String && qobject_cast<MetaDataBase*>(core()->metaDataBase())) {
                    MetaDataBaseItem *item = static_cast<MetaDataBaseItem*>(core()->metaDataBase()->item(object));

                    if (item && !item->propertyComment(propertyName).isEmpty()) {
                        p->elementString()->setAttributeComment(item->propertyComment(propertyName));
                    }
                }

                properties.append(p);
            }
        }
    }
    return properties;
}

DomProperty *QDesignerResource::createProperty(QObject *object, const QString &propertyName, const QVariant &value)
{
    if (!checkProperty(object, propertyName)) {
        return 0;
    }

    QExtensionManager *mgr = core()->extensionManager();
    QDesignerLanguageExtension *lang = qt_extension<QDesignerLanguageExtension*> (mgr, core());

    if (qVariantCanConvert<EnumType>(value)) {
        EnumType e = qvariant_cast<EnumType>(value);
        int v = e.value.toInt();
        QMapIterator<QString, QVariant> it(e.items);
        while (it.hasNext()) {
            if (it.next().value().toInt() != v)
                continue;

            DomProperty *p = new DomProperty;
            // check if we have a standard cpp set function
            const QMetaObject *meta = object->metaObject();
            int pindex = meta->indexOfProperty(propertyName.toLatin1());
            if (pindex != -1) {
                QMetaProperty meta_property = meta->property(pindex);
                if (!meta_property.hasStdCppSet())
                    p->setAttributeStdset(0);
            }
            p->setAttributeName(propertyName);

            QString id = it.key();
            if (lang)
                id = lang->neutralEnumerator(id);

            p->setElementEnum(id);
            return p;
        }

        return 0;
    } else if (qVariantCanConvert<FlagType>(value)) {
        FlagType f = qvariant_cast<FlagType>(value);
        uint v = f.value.toUInt();
        QMapIterator<QString, QVariant> it(f.items);
        QStringList keys;

        while (it.hasNext()) {
            uint x = it.next().value().toUInt();

            QString id = it.key();
            if (lang)
                id = lang->neutralEnumerator(id);

            if (v == x) {
                DomProperty *p = new DomProperty;
                // check if we have a standard cpp set function
                const QMetaObject *meta = object->metaObject();
                int pindex = meta->indexOfProperty(propertyName.toLatin1());
                if (pindex != -1) {
                    QMetaProperty meta_property = meta->property(pindex);
                    if (!meta_property.hasStdCppSet())
                        p->setAttributeStdset(0);
                }
                p->setAttributeName(propertyName);

                p->setElementSet(id);
                return p;
            }

            if ((v & x) == x)
                keys.push_back(id);
        }

        if (keys.isEmpty())
            return 0;

        DomProperty *p = new DomProperty;
        p->setAttributeName(propertyName);
        p->setElementSet(keys.join(QLatin1String("|")));
        return p;
    }

    return QAbstractFormBuilder::createProperty(object, propertyName, value);
}

void QDesignerResource::createResources(DomResources *resources)
{
    if (resources == 0)
        return;

    QList<DomResource*> dom_include = resources->elementInclude();
    foreach (DomResource *res, dom_include) {
        QString path = m_formWindow->absoluteDir().absoluteFilePath(res->attributeLocation());
        while (!QFile::exists(path)) {
            if (QMessageBox::warning(m_formWindow->core()->topLevel(), QApplication::translate("qdesigner_internal::QDesignerResource",
                "Loading qrc file", 0, QApplication::UnicodeUTF8),
                QApplication::translate("qdesigner_internal::QDesignerResource",
                "The specified qrc file <p><b>%1</b></p><p>could not be found. Do you want to update the file location?</p>", 0, QApplication::UnicodeUTF8).arg(path),
                QApplication::translate("qdesigner_internal::QDesignerResource", "&Yes", 0, QApplication::UnicodeUTF8),
                QApplication::translate("qdesigner_internal::QDesignerResource", "&No", 0, QApplication::UnicodeUTF8),
                QString(), 0, 1) == 0) {
                QFileInfo fi(path);
                path = QFileDialog::getOpenFileName(m_formWindow->core()->topLevel(),
                    QApplication::translate("qdesigner_internal::QDesignerResource",
                    "New location for %1", 0, QApplication::UnicodeUTF8).arg(fi.fileName()), fi.absolutePath(),
                    QApplication::translate("qdesigner_internal::QDesignerResource", "Resource files (*.qrc)", 0, QApplication::UnicodeUTF8));
                if (path.isEmpty())
                    break;
            } else {
                break;
            }
        }
        if (!path.isEmpty())
            m_formWindow->addResourceFile(path);
    }
}

DomResources *QDesignerResource::saveResources()
{
    QStringList res_list = m_formWindow->resourceFiles();
    QList<DomResource*> dom_include;
    foreach (QString res, res_list) {
        DomResource *dom_res = new DomResource;
        QString conv_path = m_formWindow->absoluteDir().relativeFilePath(res);
        dom_res->setAttributeLocation(conv_path.replace(QDir::separator(), QLatin1Char('/')));
        dom_include.append(dom_res);
    }

    DomResources *dom_resources = new DomResources;
    dom_resources->setElementInclude(dom_include);

    return dom_resources;
}

DomAction *QDesignerResource::createDom(QAction *action)
{
    if (!core()->metaDataBase()->item(action) || action->menu())
        return 0;

    return QAbstractFormBuilder::createDom(action);
}

DomActionGroup *QDesignerResource::createDom(QActionGroup *actionGroup)
{
    if (core()->metaDataBase()->item(actionGroup) != 0) {
        return QAbstractFormBuilder::createDom(actionGroup);
    }

    return 0;
}

QAction *QDesignerResource::create(DomAction *ui_action, QObject *parent)
{
    if (QAction *action = QAbstractFormBuilder::create(ui_action, parent)) {
        core()->metaDataBase()->add(action);
        return action;
    }

    return 0;
}

QActionGroup *QDesignerResource::create(DomActionGroup *ui_action_group, QObject *parent)
{
    if (QActionGroup *actionGroup = QAbstractFormBuilder::create(ui_action_group, parent)) {
        core()->metaDataBase()->add(actionGroup);
        return actionGroup;
    }

    return 0;
}

DomActionRef *QDesignerResource::createActionRefDom(QAction *action)
{
    if (!core()->metaDataBase()->item(action)
            || qobject_cast<SentinelAction*>(action)
            || (!action->isSeparator() && !action->menu() && action->objectName().isEmpty()))
        return 0;

    return QAbstractFormBuilder::createActionRefDom(action);
}

void QDesignerResource::addMenuAction(QAction *action)
{
    core()->metaDataBase()->add(action);
}

QAction *QDesignerResource::createAction(QObject *parent, const QString &name)
{
    if (QAction *action = QAbstractFormBuilder::createAction(parent, name)) {
        core()->metaDataBase()->add(action);
        return action;
    }

    return 0;
}

QActionGroup *QDesignerResource::createActionGroup(QObject *parent, const QString &name)
{
    if (QActionGroup *actionGroup = QAbstractFormBuilder::createActionGroup(parent, name)) {
        core()->metaDataBase()->add(actionGroup);
        return actionGroup;
    }

    return 0;
}

void QDesignerResource::loadExtraInfo(DomWidget *ui_widget, QWidget *widget, QWidget *parentWidget)
{
    if (QDesignerPromotedWidget *promoted = qobject_cast<QDesignerPromotedWidget*>(widget))
        widget = promoted->child();
    QAbstractFormBuilder::loadExtraInfo(ui_widget, widget, parentWidget);
}
