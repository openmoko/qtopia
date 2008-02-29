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

#include "newform.h"
#include "qdesigner.h"
#include "qdesigner_workbench.h"
#include "qdesigner_actions.h"
#include "qdesigner_formwindow.h"
#include "qdesigner_settings.h"

#include <qdesigner_formbuilder_p.h>
#include <sheet_delegate_p.h>

#include <QtDesigner/abstractformwindow.h>

#include <QtCore/QDir>
#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtGui/QHeaderView>
#include <QtGui/QPainter>
#include <QtGui/QPushButton>

#include <QtCore/qdebug.h>

enum NewForm_CustomRole
{
    TemplateNameRole = Qt::UserRole + 100
};

NewForm::NewForm(QDesignerWorkbench *workbench, QWidget *parentWidget)
    : QDialog(parentWidget,
#ifdef Q_WS_MAC
            Qt::Tool |
#endif
            Qt::WindowTitleHint | Qt::WindowSystemMenuHint),
      m_workbench(workbench)
{
    ui.setupUi(this);
    ui.treeWidget->setItemDelegate(new qdesigner_internal::SheetDelegate(ui.treeWidget, this));
    ui.treeWidget->header()->hide();
    ui.treeWidget->header()->setStretchLastSection(true);
    ui.lblPreview->setBackgroundRole(QPalette::Base);
    ui.chkShowOnStartup->setChecked(QDesignerSettings().showNewFormOnStartup());
    ui.buttonBox->clear();
    ui.buttonBox->addButton(QApplication::translate("NewForm", "&Close", 0,
                                        QApplication::UnicodeUTF8), QDialogButtonBox::RejectRole);
    createButton = static_cast<QPushButton *>(ui.buttonBox->addButton(QApplication::translate("NewForm", "C&reate", 0,
                           QApplication::UnicodeUTF8), QDialogButtonBox::AcceptRole));
    createButton->setEnabled(false);
    ui.buttonBox->addButton(QApplication::translate("NewForm", "&Open...", 0,
                                    QApplication::UnicodeUTF8), QDialogButtonBox::ActionRole);

    loadFrom(QLatin1String(":/trolltech/designer/templates/forms"), true);

    QDesignerSettings settings;
    foreach(QString path, settings.formTemplatePaths())
        loadFrom(path, false);
}

NewForm::~NewForm()
{
    QDesignerSettings().setShowNewFormOnStartup(ui.chkShowOnStartup->isChecked());
}

void NewForm::on_treeWidget_currentItemChanged(QTreeWidgetItem *current, QTreeWidgetItem *)
{
    if (current && current->parent()) {
        QIcon icon = formPreviewIcon(current->data(0, TemplateNameRole).toString());
        if (icon.isNull()) {
            createButton->setEnabled(false);
            ui.lblPreview->setText(tr("Error loading form"));
        } else {
            createButton->setEnabled(true);
            createButton->setDefault(true);
            ui.lblPreview->setPixmap(icon.pixmap(QSize(256, 256)));
        }
    } else {
        createButton->setEnabled(false);
        ui.lblPreview->setText(tr("Choose a template for a preview"));
    }
}

void NewForm::on_treeWidget_itemActivated(QTreeWidgetItem *item)
{
    if (item->data(0, TemplateNameRole).isValid())
        createButton->animateClick(0);
}

void NewForm::on_buttonBox_clicked(QAbstractButton *btn)
{
    int role = ui.buttonBox->buttonRole(btn);
    switch (role) {
    case QDialogButtonBox::RejectRole:
        close();
        break;
    case QDialogButtonBox::ActionRole:
        hide();
        if (m_workbench->actionManager()->openForm())
            close();
        else
            show();
        break;
    case QDialogButtonBox::AcceptRole:
        if (QTreeWidgetItem *item = ui.treeWidget->currentItem()) {
            close();

            int maxUntitled = 0;
            int totalWindows = m_workbench->formWindowCount();
            // This will cause some problems with i18n, but for now I need the string to be "static"
            QRegExp rx(QLatin1String("untitled( (\\d+))?"));
            for (int i = 0; i < totalWindows; ++i) {
                QString title = m_workbench->formWindow(i)->windowTitle();
                title = title.replace(QLatin1String("[*]"), QLatin1String(""));
                if (rx.indexIn(title) != 1) {
                    if (maxUntitled == 0)
                        ++maxUntitled;
                    if (rx.numCaptures() > 1)
                        maxUntitled = qMax(rx.cap(2).toInt(), maxUntitled);
                }
            }

            QDesignerFormWindow *formWindow = workbench()->createFormWindow();
            if (QDesignerFormWindowInterface *editor = formWindow->editor()) {
                QString formTemplateName = item->data(0, TemplateNameRole).toString();
                QFile f(formTemplateName);
                if (f.open(QFile::ReadOnly)) {
                    editor->setContents(&f);
                    f.close();
                } else {
                    editor->setContents(QString());
                }

                if (QWidget *container = editor->mainContainer())
                    formWindow->resize(container->size());
            }
            QString newTitle = QLatin1String("untitled");
            if (maxUntitled)
                newTitle += QLatin1String(" ") + QString::number(maxUntitled + 1);

            newTitle.append(QLatin1String("[*]"));
            formWindow->setWindowTitle(newTitle);
            formWindow->editor()->setFileName("");
            formWindow->show();
        }
        break;
    default:
        break;
    }
}

QDesignerWorkbench *NewForm::workbench() const
{
    return m_workbench;
}

QIcon NewForm::formPreviewIcon(const QString &fileName)
{
    QIcon result;

    QFile f(fileName);
    if (f.open(QFile::ReadOnly)) {
        qdesigner_internal::QDesignerFormBuilder formBuilder(workbench()->core());

        QWidget *fake = new QWidget(0);

        fake->createWinId();
        fake->setAttribute(Qt::WA_WState_Visible);

        if (QWidget *widget = formBuilder.load(&f, fake)) {
            widget->setParent(fake, 0);
            widget->show();
            f.close();

            int margin = 7;
            int shadow = 7;

            QPixmap pix = QPixmap::grabWidget(widget);
            QImage image = pix.toImage();
            image = image.scaled(256 - margin * 2,
                                 256 - margin * 2,
                                 Qt::KeepAspectRatio,
                                 Qt::SmoothTransformation);

            QImage dest(image.width() + margin * 2, image.height() + margin * 2, QImage::Format_ARGB32_Premultiplied);
            dest.fill(0);

            QPainter p(&dest);
            p.drawImage(margin, margin, image);

            p.setPen(QPen(palette().brush(QPalette::WindowText), 0));

            p.drawRect(margin-1, margin-1, image.width() + 1, image.height() + 1);

            QColor dark(Qt::darkGray);
            QColor light(Qt::transparent);

            // right shadow
            {
                QRect rect(margin + image.width() + 1, margin + shadow, shadow, image.height() - shadow + 1);
                QLinearGradient lg(rect.topLeft(), rect.topRight());
                lg.setColorAt(0, dark);
                lg.setColorAt(1, light);
                p.fillRect(rect, lg);
            }

            // bottom shadow
            {
                QRect rect(margin + shadow, margin + image.height() + 1, image.width() - shadow + 1, shadow);
                QLinearGradient lg(rect.topLeft(), rect.bottomLeft());
                lg.setColorAt(0, dark);
                lg.setColorAt(1, light);
                p.fillRect(rect, lg);
            }

            // bottom/right corner shadow
            {
                QRect rect(margin + image.width() + 1, margin + image.height() + 1, shadow, shadow);
                QRadialGradient g(rect.topLeft(), shadow);
                g.setColorAt(0, dark);
                g.setColorAt(1, light);
                p.fillRect(rect, g);
            }

            // top/right corner
            {
                QRect rect(margin + image.width() + 1, margin, shadow, shadow);
                QRadialGradient g(rect.bottomLeft(), shadow);
                g.setColorAt(0, dark);
                g.setColorAt(1, light);
                p.fillRect(rect, g);
            }

            // bottom/left corner
            {
                QRect rect(margin, margin + image.height() + 1, shadow, shadow);
                QRadialGradient g(rect.topRight(), shadow);
                g.setColorAt(0, dark);
                g.setColorAt(1, light);
                p.fillRect(rect, g);
            }

            p.end();

            result = QPixmap::fromImage(dest);
        }

        fake->deleteLater();
    }

    return result;
}

void NewForm::loadFrom(const QString &path, bool resourceFile)
{
    QDir dir(path);

    if (!dir.exists())
        return;

    // Iterate through the directory and add the templates
    QFileInfoList list = dir.entryInfoList(QStringList() << "*.ui", QDir::Files);

    if (list.isEmpty())
        return;

    QChar separator = resourceFile ? QChar(QLatin1Char('/'))
                                   : QChar(QDir::separator());
    QTreeWidgetItem *root = new QTreeWidgetItem(ui.treeWidget);
    // Try to get something that is easy to read.
    QString visiblePath = path;
    int index = visiblePath.lastIndexOf(separator);
    if (index != -1) {
        // try to find a second slash, just to be a bit better.
        int index2 = visiblePath.lastIndexOf(separator, index - 1);
        if (index2 != -1)
            index = index2;
        visiblePath = visiblePath.mid(index + 1);
        visiblePath = QDir::toNativeSeparators(visiblePath);
    }

    root->setText(0, visiblePath.replace(QLatin1String("_"), QLatin1String(" ")));
    root->setToolTip(0, path);

    foreach(QFileInfo fi, list) {
        if (!fi.isFile())
            continue;

        QTreeWidgetItem *item = new QTreeWidgetItem(root);
        item->setText(0, fi.baseName().replace(QLatin1String("_"), QLatin1String(" ")));
        item->setData(0, TemplateNameRole, fi.absoluteFilePath());

        QTreeWidgetItem *i = ui.treeWidget->currentItem();
        if (i == 0) {
            ui.treeWidget->setCurrentItem(item);
            ui.treeWidget->setItemSelected(item, true);
        }
    }
    ui.treeWidget->setItemExpanded(root, true);
}

void NewForm::on_treeWidget_itemPressed(QTreeWidgetItem *item)
{
    if (item && !item->parent())
        ui.treeWidget->setItemExpanded(item, !ui.treeWidget->isItemExpanded(item));
}
