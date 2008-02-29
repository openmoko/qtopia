/****************************************************************************
**
** Copyright (C) 2005-2007 TROLLTECH ASA. All rights reserved.
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
TRANSLATOR qdesigner_internal::ResourceEditor
*/

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/qdebug.h>
#include <QtGui/QTabWidget>
#include <QtGui/QVBoxLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QAction>
#include <QtGui/QComboBox>
#include <QtGui/QLabel>
#include <QtGui/QStackedWidget>
#include <QtGui/QTreeView>
#include <QtGui/QItemSelectionModel>
#include <QtGui/QHeaderView>
#include <QtGui/QPushButton>
#include <QtGui/QLineEdit>
#include <QtGui/QFileDialog>
#include <QtGui/QMessageBox>
#include <QtGui/QToolButton>
#include <QtGui/QItemDelegate>
#include <QtGui/QKeyEvent>
#include <QtGui/QDrag>
#include <QtGui/QDockWidget>

#include <QtDesigner/QtDesigner>

#include <resourcefile_p.h>
#include <iconloader_p.h>

#include "ui_resourceeditor.h"
#include "resourceeditor_p.h"

#define COMBO_EMPTY_DATA 0
#define COMBO_NEW_DATA 1
#define COMBO_OPEN_DATA 2

namespace qdesigner_internal {

/******************************************************************************
** QrcItemDelegate
*/

class QrcItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    QrcItemDelegate(QObject *parent = 0);
    virtual void setModelData(QWidget *editor,
                                QAbstractItemModel *model,
                                const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
};

QrcItemDelegate::QrcItemDelegate(QObject *parent)
    : QItemDelegate(parent)
{
}

void QrcItemDelegate::setModelData(QWidget *_editor,
                            QAbstractItemModel *_model,
                            const QModelIndex &index) const
{
    QLineEdit *editor = qobject_cast<QLineEdit*>(_editor);
    if (editor == 0)
        return;

    ResourceModel *model = qobject_cast<ResourceModel*>(_model);
    if (model == 0)
        return;

    QString new_prefix = ResourceFile::fixPrefix(editor->text());
    QString old_prefix, file;
    model->getItem(index, old_prefix, file);

    if (old_prefix.isEmpty())
        return;

    if (old_prefix == new_prefix)
        return;

    model->changePrefix(index, new_prefix);
    if (!model->save())
        model->changePrefix(index, old_prefix);
}

void QrcItemDelegate::setEditorData(QWidget *_editor, const QModelIndex &index) const
{
    const ResourceModel *model = qobject_cast<const ResourceModel*>(index.model());
    if (model == 0)
        return;

    QLineEdit *editor = qobject_cast<QLineEdit*>(_editor);
    if (editor == 0)
        return;

    QString prefix, file;
    model->getItem(index, prefix, file);
    editor->setText(prefix);
}

/******************************************************************************
** QrcView
*/

class QrcView : public QTreeView
{
    Q_OBJECT
public:
    QrcView(QWidget *parent = 0);

protected:
    virtual void dragEnterEvent(QDragEnterEvent *);
    virtual void dragMoveEvent(QDragMoveEvent *);
    virtual void dragLeaveEvent(QDragLeaveEvent *);
    virtual void dropEvent(QDropEvent *);

private:
    bool acceptDrop(QDropEvent *event);
    QStringList mimeFileList(const QMimeData *mime);
    const QMimeData *m_last_mime_data;
    QStringList m_last_file_list;
};

QrcView::QrcView(QWidget *parent)
    : QTreeView(parent)
{
    setItemDelegate(new QrcItemDelegate(this));
    setEditTriggers(QTreeView::DoubleClicked
                            | QTreeView::AnyKeyPressed);
    header()->hide();
    setAcceptDrops(true);

    m_last_mime_data = 0;
}

QStringList QrcView::mimeFileList(const QMimeData *mime)
{
    if (mime == m_last_mime_data)
        return m_last_file_list;

    m_last_mime_data = mime;
    m_last_file_list.clear();

    if (!mime->hasFormat(QLatin1String("text/uri-list")))
        return m_last_file_list;

    QByteArray data_list_str = mime->data(QLatin1String("text/uri-list"));
    QList<QByteArray> data_list = data_list_str.split('\n');

    foreach (QByteArray data, data_list) {
        QString uri = QFile::decodeName(data.trimmed());
        if (uri.startsWith(QLatin1String("file:")))
            m_last_file_list.append(uri.mid(5));
    }

    return m_last_file_list;
}

bool QrcView::acceptDrop(QDropEvent *e)
{
    if (!(e->proposedAction() & Qt::CopyAction)) {
        e->ignore();
        return false;
    }

    if (mimeFileList(e->mimeData()).isEmpty()) {
        e->ignore();
        return false;
    }

    e->acceptProposedAction();

    return true;
}

void QrcView::dragEnterEvent(QDragEnterEvent *e)
{
    if (!acceptDrop(e))
        return;
}

void QrcView::dragMoveEvent(QDragMoveEvent *e)
{
    if (!acceptDrop(e))
        return;

    QModelIndex index = indexAt(e->pos());
    if (!index.isValid()) {
        e->ignore();
        return;
    }

    selectionModel()->setCurrentIndex(index, QItemSelectionModel::ClearAndSelect);
}

void QrcView::dragLeaveEvent(QDragLeaveEvent*)
{
    m_last_mime_data = 0;
}

void QrcView::dropEvent(QDropEvent *e)
{
    if (!acceptDrop(e)) {
        m_last_mime_data = 0;
        return;
    }

    QStringList file_list = mimeFileList(e->mimeData());
    m_last_mime_data = 0;

    QModelIndex index = indexAt(e->pos());
    if (!index.isValid()) {
        e->ignore();
        return;
    }

    ResourceModel *model = qobject_cast<ResourceModel*>(this->model());
    Q_ASSERT(model != 0);

    QModelIndex prefix_index = model->prefixIndex(index);
    QModelIndex last_added_idx = model->addFiles(prefix_index, file_list);
    setExpanded(prefix_index, true);
    selectionModel()->setCurrentIndex(last_added_idx, QItemSelectionModel::ClearAndSelect);
}

/******************************************************************************
** EditableResourceModel
*/

class EditableResourceModel : public ResourceModel
{
    Q_OBJECT

public:
    EditableResourceModel(const ResourceFile &resource_file, QObject *parent = 0);
    virtual Qt::ItemFlags flags(const QModelIndex &index) const;
    virtual QModelIndex addFiles(const QModelIndex &idx, const QStringList &file_list);
    virtual bool reload();
    virtual bool save();

private slots:
    void showWarning(const QString &caption, const QString &message);
};

EditableResourceModel::EditableResourceModel(const ResourceFile &resource_file,
                                             QObject *parent)
    : ResourceModel(resource_file, parent)
{
}

Qt::ItemFlags EditableResourceModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags result = Qt::ItemIsSelectable | Qt::ItemIsEnabled;

    QString prefix, file;
    getItem(index, prefix, file);

    if (file.isEmpty())
        result |= Qt::ItemIsEditable;
    else if (!iconFileExtension(file))
        result &= ~Qt::ItemIsEnabled & ~Qt::ItemIsSelectable;

    return result;
}

QModelIndex EditableResourceModel::addFiles(const QModelIndex &idx,
                                            const QStringList &file_list)
{
    QModelIndex result;

    QStringList good_file_list;
    foreach (QString file, file_list) {
        if (!relativePath(file).startsWith(QLatin1String("..")))
            good_file_list.append(file);
    }

    if (good_file_list.size() == file_list.size()) {
        result = ResourceModel::addFiles(idx, good_file_list);
    } else if (good_file_list.size() > 0) {
        int answer =
            QMessageBox::warning(0, tr("Invalid files"),
                                    tr("Files referenced in a qrc must be in the qrc's "
                                        "directory or one of its subdirectories:<p><b>%1</b><p>"
                                        "Some of the selected files do not comply with this.")
                                            .arg(absolutePath(QString())),
                                    tr("Cancel"), tr("Only insert files which comply"),
                                    QString(), 1);

        if (answer != 0)
            result = ResourceModel::addFiles(idx, good_file_list);
    } else {
        QMessageBox::warning(0, tr("Invalid files"),
                                tr("Files referenced in a qrc must be in the qrc's "
                                    "directory or one of its subdirectories:<p><b>%1</b><p>"
                                    "The selected files do not comply with this.")
                                        .arg(absolutePath(QString())),
                                    QMessageBox::Cancel, QMessageBox::NoButton);
    }

    return result;
}

bool EditableResourceModel::reload()
{
    if (!ResourceModel::reload()) {
        QMetaObject::invokeMethod(this, "showWarning", Qt::QueuedConnection,
                                    Q_ARG(QString, tr("Error loading resource file")),
                                    Q_ARG(QString, tr("Failed to open \"%1\":\n%2")
                                                        .arg(fileName())
                                                        .arg(errorMessage())));
        return false;
    }
    return true;
}

bool EditableResourceModel::save()
{
    if (!ResourceModel::save()) {
        QMetaObject::invokeMethod(this, "showWarning", Qt::QueuedConnection,
                                    Q_ARG(QString, tr("Error saving resource file")),
                                    Q_ARG(QString, tr("Failed to save \"%1\":\n%2")
                                        .arg(fileName())
                                        .arg(errorMessage())));
        return false;
    }
    return true;
}

void EditableResourceModel::showWarning(const QString &caption, const QString &message)
{
    QMessageBox::warning(0, caption, message, QMessageBox::Ok, QMessageBox::NoButton);
}

/******************************************************************************
** ModelCache
*/

class ModelCache
{
public:
    ResourceModel *model(const QString &file);

private:
    QList<ResourceModel*> m_model_list;
};

Q_GLOBAL_STATIC(ModelCache, g_model_cache)

ResourceModel *ModelCache::model(const QString &file)
{
    if (file.isEmpty()) {
        ResourceModel *model = new EditableResourceModel(ResourceFile());
        m_model_list.append(model);
        return model;
    }

    for (int i = 0; i < m_model_list.size(); ++i) {
        ResourceModel *model = m_model_list.at(i);
        if (model->fileName() == file)
            return model;
    }

    ResourceFile rf(file);
    if (!rf.load()) {
        QMessageBox::warning(0, QApplication::translate("Designer", "Error opening resource file"),
                                QApplication::translate("Designer", "Failed to open \"%1\":\n%2")
                                    .arg(file).arg(rf.errorMessage()),
                                QMessageBox::Ok, QMessageBox::NoButton);
        return 0;
    }

    ResourceModel *model = new EditableResourceModel(rf);
    m_model_list.append(model);
    return model;
}

/******************************************************************************
** ResourceEditor
*/

ResourceEditor::ResourceEditor(QDesignerFormEditorInterface *core, QWidget *parent)
    : QWidget(parent)
{
    m_ignore_update = false;
    Ui::ResourceEditor ui;
    ui.setupUi(this);

    m_qrc_combo = ui.m_qrc_combo;
    m_qrc_stack = ui.m_qrc_stack;
    m_add_button = ui.m_add_button;
    m_remove_button = ui.m_remove_button;
    m_add_files_button = ui.m_add_files_button;
    m_remove_qrc_button = ui.m_remove_qrc_button;

    m_form = 0;
    setEnabled(false);

    connect(core->formWindowManager(),
            SIGNAL(activeFormWindowChanged(QDesignerFormWindowInterface*)),
            this, SLOT(setActiveForm(QDesignerFormWindowInterface*)));
    connect(m_qrc_combo, SIGNAL(activated(int)),
            this, SLOT(setCurrentIndex(int)));

    m_remove_qrc_button->setIcon(createIconSet(QLatin1String("editdelete.png")));
    connect(m_remove_qrc_button, SIGNAL(clicked()), this, SLOT(removeCurrentView()));

    m_add_button->setIcon(createIconSet(QLatin1String("plus.png")));
    connect(m_add_button, SIGNAL(clicked()), this, SLOT(addPrefix()));
    m_remove_button->setIcon(createIconSet(QLatin1String("minus.png")));
    connect(m_remove_button, SIGNAL(clicked()), this, SLOT(deleteItem()));
    m_add_files_button->setIcon(createIconSet(QLatin1String("fileopen.png")));
    connect(m_add_files_button, SIGNAL(clicked()), this, SLOT(addFiles()));

    updateQrcStack();
}

void ResourceEditor::insertEmptyComboItem()
{
    if (m_qrc_combo->count() == 0)
        return;
    QVariant v = m_qrc_combo->itemData(0);
    if (v.type() == QVariant::Int && v.toInt() == COMBO_EMPTY_DATA)
        return;
    m_qrc_combo->insertItem(0, QIcon(), tr("<no resource files>"), QVariant(COMBO_EMPTY_DATA));
    m_qrc_combo->setCurrentIndex(0);
}

void ResourceEditor::removeEmptyComboItem()
{
    if (m_qrc_combo->count() == 0)
        return;
    QVariant v = m_qrc_combo->itemData(0);
    if (v.type() != QVariant::Int || v.toInt() != COMBO_EMPTY_DATA)
        return;
    m_qrc_combo->removeItem(0);
}

int ResourceEditor::qrcCount() const
{
    return m_qrc_stack->count();
}

QTreeView *ResourceEditor::view(int i) const
{
    if (i >= qrcCount() || i < 0)
        return 0;
    return qobject_cast<QTreeView*>(m_qrc_stack->widget(i));
}

ResourceModel *ResourceEditor::model(int i) const
{
    if (i >= qrcCount() || i < 0)
        return 0;
    return qobject_cast<ResourceModel*>(view(i)->model());
}

QTreeView *ResourceEditor::currentView() const
{
    int idx = currentIndex();
    if (idx == -1)
        return 0;
    return view(idx);
}

ResourceModel *ResourceEditor::currentModel() const
{
    int idx = currentIndex();
    if (idx == -1)
        return 0;
    return model(idx);
}

void ResourceEditor::getCurrentItem(QString &prefix, QString &file)
{
    prefix.clear();
    file.clear();

    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->getItem(view->currentIndex(), prefix, file);
}

void ResourceEditor::addPrefix()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QModelIndex idx = model->addNewPrefix();
    view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    model->save();
    updateUi();
}

void ResourceEditor::addFiles()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QStringList file_list = QFileDialog::getOpenFileNames(this, tr("Open file"),
                                                            model->lastResourceOpenDirectory(),
                                                            tr("All files (*)"));
    if (file_list.isEmpty())
        return;
    
    QModelIndex idx = model->addFiles(view->currentIndex(), file_list);
    if (idx.isValid()) {
        view->setExpanded(model->prefixIndex(view->currentIndex()), true);
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    }

    model->save();
    updateUi();
}

void ResourceEditor::deleteItem()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QModelIndex cur_idx = view->currentIndex();
    if (!cur_idx.isValid())
        return;

    QModelIndex idx = model->deleteItem(cur_idx);

    if (idx.isValid()) {
        QModelIndex pref_idx = model->prefixIndex(idx);
        if (pref_idx != idx)
            view->setExpanded(pref_idx, true);
        view->selectionModel()->setCurrentIndex(idx, QItemSelectionModel::ClearAndSelect);
    }

    model->save();
    updateUi();
}

void ResourceEditor::updateUi()
{
    QString prefix, file;
    getCurrentItem(prefix, file);

    m_add_button->setEnabled(currentModel() != 0);
    m_remove_button->setEnabled(!prefix.isEmpty());
    m_add_files_button->setEnabled(!prefix.isEmpty());
    m_remove_qrc_button->setEnabled(currentModel() != 0);

    QString name;
    if (m_form != 0)
        name = QFileInfo(m_form->fileName()).fileName();

    QString suffix;
    name.isEmpty() ? suffix = tr("Resource Editor") : suffix = tr("Resource Editor: %1").arg(name);

    QWidget* widget = 0;
    if (m_form != 0)
        widget = m_form->core()->topLevel();

    if (widget && (widget->objectName() == QLatin1String("MDIWindow"))) {
        QDockWidget* dockWidget = qFindChild<QDockWidget*>(widget, (parentWidget()->objectName() + QLatin1String("_dock")));
        if(dockWidget)
            dockWidget->setWindowTitle(suffix);
    }
    else if ((widget = parentWidget()) != 0 && widget->isWindow())
        widget->setWindowTitle(suffix);
    else
        setWindowTitle(suffix);
}

int ResourceEditor::currentIndex() const
{
    return m_qrc_stack->currentIndex();
}

void ResourceEditor::setCurrentIndex(int i)
{
    QVariant v = m_qrc_combo->itemData(i);
    if (v.type() == QVariant::Int) {
        switch (v.toInt()) {
            case COMBO_EMPTY_DATA: {
                    bool blocked = m_qrc_combo->blockSignals(true);
                    m_qrc_combo->setCurrentIndex(i);
                    m_qrc_combo->blockSignals(blocked);
                }
                break;
            case COMBO_OPEN_DATA:
                openView();
                break;
            case COMBO_NEW_DATA:
                newView();
                break;
            default:
                break;
        }
    } else {
        bool blocked = m_qrc_combo->blockSignals(true);
        m_qrc_combo->setCurrentIndex(i);
        m_qrc_combo->blockSignals(blocked);
        m_qrc_stack->setCurrentIndex(i);
    }

    QAbstractItemView *view = currentView();
    if (view != 0)
        itemChanged(view->currentIndex());

    updateUi();
}

void ResourceEditor::updateQrcStack()
{
    if (m_ignore_update)
        return;

    m_qrc_combo->clear();
    while (m_qrc_stack->count() > 0) {
        QWidget *w = m_qrc_stack->widget(0);
        m_qrc_stack->removeWidget(w);
        delete w;
    }

    bool empty_list = true;
    QStringList qrc_file_list;
    if (m_form != 0) {
        qrc_file_list = m_form->resourceFiles();
        foreach (QString qrc_file, qrc_file_list) {
            if (QFile::exists(qrc_file)) {
                addView(qrc_file);
                empty_list = false;
            }
        }
    }

    m_qrc_combo->addItem(QIcon(), tr("New..."), QVariant(COMBO_NEW_DATA));
    m_qrc_combo->addItem(QIcon(), tr("Open..."), QVariant(COMBO_OPEN_DATA));
    if (empty_list)
        insertEmptyComboItem();

    updateUi();
}

QString ResourceEditor::qrcName(const QString &path) const
{
    if (m_form == 0 || path.isEmpty())
        return tr("Untitled");
    return m_form->absoluteDir().relativeFilePath(path);
}

void ResourceEditor::updateQrcPaths()
{
    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        ResourceModel *model = this->model(i);
        m_qrc_combo->setItemText(i, model->fileName());
    }

    updateUi();
}

void ResourceEditor::addView(const QString &qrc_file)
{
    int idx = qrcCount();

    QTreeView *view = new QrcView;
    ResourceModel *model = g_model_cache()->model(qrc_file);
    if (model == 0)
        return;
    removeEmptyComboItem();

    view->setModel(model);
    m_qrc_combo->insertItem(idx, model->fileName());
    m_qrc_stack->addWidget(view);
    connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)),
            this, SLOT(updateUi()));
    connect(view, SIGNAL(activated(QModelIndex)), this, SLOT(itemActivated(QModelIndex)));
    connect(view->selectionModel(), SIGNAL(currentChanged(QModelIndex, QModelIndex)),
            this, SLOT(itemChanged(QModelIndex)));
//    connect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));

    setCurrentIndex(idx);

    m_ignore_update = true;
    if (m_form && !qrc_file.isEmpty())
        m_form->addResourceFile(qrc_file);
    m_ignore_update = false;

    updateUi();
}

void ResourceEditor::setCurrentFile(const QString &_qrc_path, const QString &file_path)
{
    QDir form_dir = m_form->absoluteDir();
    QString qrc_path = form_dir.relativeFilePath(_qrc_path);

    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        ResourceModel *model = this->model(i);
        if (qrc_path == form_dir.relativeFilePath(model->fileName())) {
            setCurrentIndex(i);
            QModelIndex index = model->getIndex(file_path);
            view(i)->setCurrentIndex(index);
            break;
        }
    }

    updateUi();
}

void ResourceEditor::itemChanged(const QModelIndex &index)
{
    QString file_name, qrc_path;

    if (ResourceModel *model = currentModel()) {
        QString prefix, file;
        model->getItem(index, prefix, file);
        if (!file.isEmpty()) {
            file_name = QLatin1Char(':')
                        + prefix
                        + QLatin1Char('/')
                        + file;
            file_name = QDir::cleanPath(file_name);
            qrc_path = m_form->absoluteDir().absoluteFilePath(model->fileName());
        }
    }

    emit currentFileChanged(qrc_path, file_name);
}

void ResourceEditor::itemActivated(const QModelIndex &index)
{
    ResourceModel *model = currentModel();
    if (!model)
        return;

    QString prefix, file;
    model->getItem(index, prefix, file);
    if (file.isEmpty())
        return;

    QString file_name = QLatin1Char(':')
                        + prefix
                        + QLatin1Char('/')
                        + file;
    file_name = QDir::cleanPath(file_name);
    QString qrc_path = m_form->absoluteDir().absoluteFilePath(model->fileName());

    emit fileActivated(qrc_path, file_name);
}

void ResourceEditor::saveCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    if (model->fileName().isEmpty()) {
        QString file_name = QFileDialog::getSaveFileName(this, tr("Save resource file"),
                                                            m_form->absoluteDir().absolutePath(),
                                                            tr("Resource files (*.qrc)"));
        if (file_name.isEmpty())
            return;

        if (QFileInfo(file_name).suffix() != QLatin1String("qrc"))
            file_name.append(QLatin1String(".qrc"));

        model->setFileName(file_name);
        m_ignore_update = true;
        m_form->addResourceFile(file_name);
        m_ignore_update = false;
        QString s = QFileInfo(file_name).fileName();
        bool blocked = m_qrc_combo->blockSignals(true);
        m_qrc_combo->setItemText(currentIndex(), model->fileName());
        m_qrc_combo->setCurrentIndex(-1);
        m_qrc_combo->setCurrentIndex(currentIndex());
        m_qrc_combo->blockSignals(blocked);
    }

    model->save();
    updateUi();
}

int ResourceEditor::indexOfView(QTreeView *view)
{
    for (int i = 0; i < m_qrc_stack->count(); ++i) {
        if (view == m_qrc_stack->widget(i))
            return i;
    }
    return -1;
}

void ResourceEditor::removeCurrentView()
{
    QTreeView *view = currentView();
    if (view == 0)
        return;

    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    QString file_name = model->fileName();

    int idx = indexOfView(view);
    if (idx == -1)
        return;

    m_qrc_combo->removeItem(idx);
    m_qrc_stack->removeWidget(view);
    delete view;

    disconnect(model, SIGNAL(dirtyChanged(bool)), this, SLOT(updateUi()));

    m_ignore_update = true;
    if (m_form != 0 && !file_name.isEmpty())
        m_form->removeResourceFile(file_name);
    m_ignore_update = false;

    if (qrcCount() == 0) {
        insertEmptyComboItem();
    } else {
        if (idx < qrcCount())
            setCurrentIndex(idx);
        else if (idx > 0)
            setCurrentIndex(idx - 1);
    }
    updateUi();
}

void ResourceEditor::reloadCurrentView()
{
    ResourceModel *model = currentModel();
    if (model == 0)
        return;

    model->reload();
    updateUi();
}

void ResourceEditor::newView()
{
    QString file_name = QFileDialog::getSaveFileName(this, tr("New resource file"),
                                                        m_form->absoluteDir().absolutePath(),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty()) {
        setCurrentIndex(m_qrc_stack->count() == 0 ? 0 : m_qrc_stack->currentIndex());
        return;
    }
        
    if (QFileInfo(file_name).suffix() != QLatin1String("qrc"))
		file_name.append(QLatin1String(".qrc"));

    ResourceFile rf(file_name);
    rf.save();

    addView(file_name);
}

void ResourceEditor::openView()
{
    QString file_name = QFileDialog::getOpenFileName(this, tr("Open resource file"),
                                                        m_form->absoluteDir().absolutePath(),
                                                        tr("Resource files (*.qrc)"));
    if (file_name.isEmpty()) {
        setCurrentIndex(m_qrc_stack->count() == 0 ? 0 : m_qrc_stack->currentIndex());
        return;
    }

    addView(file_name);
}

void ResourceEditor::setActiveForm(QDesignerFormWindowInterface *form)
{
    if (form == m_form)
        return;

    if (m_form != 0) {
        disconnect(m_form, SIGNAL(fileNameChanged(QString)),
                    this, SLOT(updateQrcPaths()));
        disconnect(m_form, SIGNAL(resourceFilesChanged()),
                    this, SLOT(updateQrcStack()));
    }

    m_form = form;
    updateQrcStack();
    
    if (m_form != 0) {
        connect(m_form, SIGNAL(fileNameChanged(QString)),
                    this, SLOT(updateQrcPaths()));
        connect(m_form, SIGNAL(resourceFilesChanged()),
                    this, SLOT(updateQrcStack()));
    }

    setEnabled(m_form != 0);
}

} // namespace qdesigner_internal

#include "resourceeditor.moc"
