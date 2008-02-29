/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include "receivewindow.h"
#include "phone/runningapplicationsviewitem.h"

#include <qcontent.h>
#include <qmimetype.h>
#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qtopiaipcadaptor.h>
#include <qtopiaapplication.h>
#include <qsoftmenubar.h>
#include <qtopialog.h>

#include <qtopia/pim/qcontact.h>
#include <qtopia/pim/qappointment.h>
#include <qtopia/pim/qtask.h>

#include <QHeaderView>
#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPainter>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

class ReceivedFile
{
public:

    enum Direction { Sending, Receiving };

    ReceivedFile(int id, Direction dir, const QString &filename, const QString &mimetype, const QString &description);

    void setProgress(qint64 completed, qint64 total);
    qint64 total() const { return m_total; }
    qint64 completed() const { return m_completed; }

    const QString filename() const { return m_filename; }
    const QString mimetype() const { return m_mimetype; }
    const QString description() const { return m_description; }

    void setCompleted(bool failed) { m_complete = true; m_failed = failed; }
    bool failed() const { return m_failed; }
    bool complete() const { return m_complete; }

    Direction direction() const { return m_direction; }

    int id() const { return m_id; }

private:
    bool m_failed;
    bool m_complete;
    QString m_filename;
    QString m_mimetype;
    QString m_description;
    qint64 m_completed;
    qint64 m_total;
    Direction m_direction;
    int m_id;
};

ReceivedFile::ReceivedFile(int id, Direction dir, const QString &filename, const QString &mimetype, const QString &description)
{
    m_id = id;
    m_filename = filename;
    m_mimetype = mimetype;
    m_description = description;
    m_direction = dir;
    m_failed = false;
    m_complete = false;
    m_completed = 0;
    m_total = 0;
}

void ReceivedFile::setProgress(qint64 completed, qint64 total)
{
    m_total = total;
    m_completed = completed;
}

class ReceivedFilesModel : public QAbstractTableModel
{
public:
    ReceivedFilesModel(QObject *parent = 0);

    virtual int rowCount(const QModelIndex & parent = QModelIndex() ) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex() ) const;
    virtual QVariant headerData(int section, Qt::Orientation orientation,
                                int role = Qt::DisplayRole) const;
    virtual QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    void addFile(const ReceivedFile &file);
    void removeFile(int index);
    const ReceivedFile & file(int index) const;
    ReceivedFile & file(int index);
    int findFile(int id) const;
    void updateProgress(int index, qint64 completed, qint64 total);
    void setCompleted(int index, bool err);
    int size() const { return m_list.size(); }

private:
    QList<ReceivedFile> m_list;
};

ReceivedFilesModel::ReceivedFilesModel(QObject *parent) : QAbstractTableModel(parent)
{

}

int ReceivedFilesModel::rowCount(const QModelIndex &) const
{
    return m_list.size();
}

int ReceivedFilesModel::columnCount(const QModelIndex &) const
{
    return 3;
}

QVariant ReceivedFilesModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Vertical)
        return QVariant();

    if (role != Qt::DisplayRole)
        return QVariant();

    if (section == 0)
        return QVariant::fromValue(QString(" "));

    if (section == 1)
        return QVariant::fromValue(QObject::tr("File"));

    if (section == 2)
        return QVariant::fromValue(QObject::tr("Progress"));

    return QVariant();
}

QVariant ReceivedFilesModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();
    else if (role != Qt::DisplayRole)
        return QVariant();

    if (index.column() == 0) {
        if (m_list[index.row()].direction() == ReceivedFile::Sending)
            return QVariant::fromValue(QString("<"));
        else
            return QVariant::fromValue(QString(">"));
    }

    if (index.column() == 1) {
        QString str = m_list[index.row()].filename();
        int pos = str.lastIndexOf( QDir::separator() );
        if ( pos != -1 )
            str = str.mid( pos + 1 );
        return QVariant::fromValue(str);
    }

    if (index.column() == 2) {
        return QVariant(m_list[index.row()].completed());
    }

    return QVariant();
}

void ReceivedFilesModel::addFile(const ReceivedFile &file)
{
    beginInsertRows(QModelIndex(), m_list.size(), m_list.size());
    m_list.push_back(file);
    endInsertRows();
}

void ReceivedFilesModel::removeFile(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    m_list.removeAt(index);
    endRemoveRows();
}

const ReceivedFile & ReceivedFilesModel::file(int index) const
{
    return m_list[index];
}

ReceivedFile & ReceivedFilesModel::file(int index)
{
    return m_list[index];
}

int ReceivedFilesModel::findFile(int id) const
{
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list[i].id() == id)
            return i;
    }

    return -1;
}

void ReceivedFilesModel::updateProgress(int idx, qint64 completed, qint64 total)
{
    m_list[idx].setProgress(completed, total);
    emit dataChanged(index(idx, 0), index(idx, 2));
}

void ReceivedFilesModel::setCompleted(int idx, bool err)
{
    m_list[idx].setCompleted(err);

    if (m_list[idx].direction() != ReceivedFile::Sending) {
        emit dataChanged(index(idx, 0), index(idx, 2));
    }
}

static QString pretty_print_size(int fsize)
{
    static const char *size_suffix[] = {"B", "K", "M", "G"};

    double max = fsize;

    int i = 0;
    for (; i < 4; i++) {
        if (max > 1024.0) {
            max /= 1024.0;
        }
        else {
            break;
        }
    }

    // REALLY big file?
    if (i == 4)
        i = 0;

    QString size = QString::number(max, 'f', 1);
    size += size_suffix[i];

    return size;
}

class ReceiveWindowItemDelegate : public QItemDelegate
{
    Q_OBJECT

public:
    ReceiveWindowItemDelegate(ReceiveWindow *mainWindow, ReceivedFilesModel *model)
    : QItemDelegate(mainWindow) { m_model = model; }

    virtual void paint(QPainter *painter, const QStyleOptionViewItem &option,
                        const QModelIndex &index ) const
    {
        switch (index.column()) {
        case 0:
            QItemDelegate::paint(painter, option, index);
            break;
        case 1:
        {
            const ReceivedFile &receivedFile = m_model->file(index.row());
            QString s = option.fontMetrics.elidedText(
                    index.model()->data(index, Qt::DisplayRole).toString(),
                    Qt::ElideRight,
                    option.rect.width());
            painter->drawText(option.rect, Qt::AlignCenter, s);
            break;
        }
        case 2:
        {
            const ReceivedFile &receivedFile = m_model->file(index.row());

            // Set up a QStyleOptionProgressBar to precisely mimic the
            // environment of a progress bar.
            QStyleOptionProgressBar progressBarOption;
            progressBarOption.state = QStyle::State_Enabled;
            progressBarOption.direction = QApplication::layoutDirection();
            progressBarOption.rect = option.rect;
            progressBarOption.fontMetrics = option.fontMetrics;
            progressBarOption.minimum = 0;
            int max = receivedFile.total();
            progressBarOption.maximum = max;
            progressBarOption.textAlignment = Qt::AlignCenter;
            progressBarOption.textVisible = true;

            // Set the progress and text values of the style option.
            if (receivedFile.complete() && receivedFile.failed()) {
                progressBarOption.text = tr("Transfer error.");
            } else {
                int progress = receivedFile.completed();
                if (progress > max || max == 0) {
                    progressBarOption.progress = 0;
                    progressBarOption.text = QString("%1/?").arg(pretty_print_size(progress));
                }
                else if (progress == max) {
                    progressBarOption.progress = progress;
                    progressBarOption.text = tr("Done (%1)", "%1 = transferred file size").
                            arg(pretty_print_size(progress));
                }
                else {
                    progressBarOption.progress = progress;
                    progressBarOption.text = QString("%1/%2").
                            arg(pretty_print_size(progress)).
                            arg(pretty_print_size(max));
                }
            }

            // Draw the progress bar onto the view.
            QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                &progressBarOption, painter);
            break;
        }
        }
    }

private:
    ReceivedFilesModel *m_model;
};


//===================================================================

ReceiveWindow::ReceiveWindow(QWidget *parent) : QMainWindow(parent)
{
    m_files = new QTableView(this);
    m_model = new ReceivedFilesModel(this);

    m_files->setModel(m_model);
    m_files->setItemDelegate(new ReceiveWindowItemDelegate(this, m_model));
    m_files->setAlternatingRowColors(true);
    m_files->verticalHeader()->hide();
    m_files->setSelectionMode(QAbstractItemView::NoSelection);
    m_files->setFrameStyle(QFrame::NoFrame);

    QHeaderView *header = m_files->horizontalHeader();
    QFontMetrics fm = fontMetrics();
    header->setResizeMode(0, QHeaderView::Custom);
    header->resizeSection(0, fm.width(">"));                  // no tr
    header->setResizeMode(1, QHeaderView::Custom);
    header->resizeSection(1, fm.width("12345678.ext"));       // no tr
    header->setResizeMode(2, QHeaderView::Stretch);

    // items aren't selectable, so remove 'select' label
    QSoftMenuBar::setLabel(m_files, Qt::Key_Select, QSoftMenuBar::NoLabel);

    setWindowTitle(QObject::tr("Send/Receive Files"));
    setCentralWidget(m_files);

    m_runningAppsItem = new RunningApplicationsViewItem(windowTitle(), "sync", this);
    connect(m_runningAppsItem, SIGNAL(activated()), SLOT(showWindow()));
}

ReceiveWindow::~ReceiveWindow()
{
    delete m_files;
    delete m_model;
    m_files = 0;
    m_model = 0;
}

void ReceiveWindow::closeEvent(QCloseEvent *event)
{
    // remove all completed transfers
    bool hasUnfinishedTransfer = false;
    for (int i=0; i<m_model->size(); i++) {
        if (m_model->file(i).complete()) {
            m_model->removeFile(i);
        } else {
            hasUnfinishedTransfer = true;
        }
    }
    if (!hasUnfinishedTransfer)
        m_runningAppsItem->hide();

    QMainWindow::closeEvent(event);
}

void ReceiveWindow::receiveInitiated(int id, const QString &filename, const QString &mime, const QString &description)
{
    QString mimeType = ( mime.isEmpty() ? QMimeType(filename).id() : mime );

    qLog(Obex) << "recvInitiated: " << filename << "(" << mime << ") ->" << mimeType << description;

    if ((mimeType.toLower() == "text/x-vcard") || (mimeType.toLower() == "text/x-vcalendar")) {
        qLog(Obex) << "Looks like a Vcard or VCal";
        handleSupportedFormatRecv(id, filename, mimeType, description);
    }
    else {
        qLog(Obex) << "Looks like a regular file...";
        m_model->addFile(ReceivedFile(id, ReceivedFile::Receiving, filename, mimeType, description));
        showWindow();

        m_runningAppsItem->show();
    }
}

void ReceiveWindow::sendInitiated(int id, const QString &filename, const QString &mime)
{
    qLog(Obex) << "sendInitiated";

    // Don't do anything here
    if ((mime.toLower() == "text/x-vcard") || (mime.toLower() == "text/x-vcalendar")) {
        qLog(Obex) << "Looks like a Vcard or VCal";
    }
    else {
        qLog(Obex) << "Looks like a regular file...";
        m_model->addFile(ReceivedFile(id, ReceivedFile::Sending, filename, mime, QString()));
        showWindow();

        m_runningAppsItem->show();
    }
}

void ReceiveWindow::progress(int id, qint64 bytes, qint64 total)
{
    qLog(Obex) << "Got Progress " << id << " " << bytes << "/" << total;

    int index = m_model->findFile(id);
    if (index != -1) {
        m_model->updateProgress(index, bytes, total);
    }
}

void ReceiveWindow::completed(int id, bool error)
{
    qLog(Obex) << "Got completion message " << id << "(" << error << ")";

    if (handleSupportedFormatComplete(id, error))
        return;

    int index = m_model->findFile(id);
    if (index != -1) {
        m_model->setCompleted(index, error);
        update();
        if (m_model->size() != 0) {
            if (!error) {
                if (m_model->file(index).direction() == ReceivedFile::Receiving)
                    saveFile(index);
            }
        }
    }
}

void ReceiveWindow::saveFile(int index)
{
    if (m_model->size() == 0)
        return;

    ReceivedFile file = m_model->file(index);
    if (!file.completed())
        return;

    if (file.direction() == ReceivedFile::Sending)
        return;

    QString saveAs = file.filename();
    int pos = saveAs.lastIndexOf( QDir::separator() );
    if ( pos != -1 )
        saveAs = saveAs.mid( pos + 1 );

    QString tmpFile = Qtopia::tempDir() + "obex/in/" + saveAs;

    QFile f(tmpFile);
    if ( !f.open(QIODevice::ReadOnly) ) {
        qLog(Obex) << "Couldn't open file to read!!";
        return;
    }

    QContent doc;
    doc.setType(file.mimetype());
    doc.setName(file.filename());

    QIODevice *device = doc.open(QIODevice::WriteOnly);

    qLog(Obex) << "File for QContent is:" << doc.name()
            << "with filename:" << doc.fileName();

    // If there's a Description value, use that as the QContent name instead
    // of using the Name value, cos it's probably a more user-friendly name
    // whereas the Name is probably a filename.
    // Must do this after QContent::open() so that the file has already been
    // created under the filename in the Name value.
    QString desc = file.description();
    if (!desc.isEmpty()) {
        qLog(Obex) << "Renaming QContent name for" << doc.name()
                << "to" << desc;
        doc.setName(desc);
    }

    if (device) {
        char buf[4096];
        int size;
        while ( (size = f.read(buf, 4096)) > 0) {
            device->write(buf, size);
        }
        device->close();
        delete device;
        doc.commit();
    }
    else {
        qLog(Obex) << "Failed to save file" << doc.fileName();
    }

    f.close();
    ::unlink(tmpFile.toLocal8Bit().data());
}

void ReceiveWindow::handleSupportedFormatRecv(int id, const QString &file, const QString &mime, const QString &description)
{
    m_list.push_back(ReceivedFile(id, ReceivedFile::Receiving, file, mime, description));
}

static void vcalInfo( const QString &filename, bool *todo, bool *cal )
{
    *cal = *todo = false;

    QList<QAppointment> events = QAppointment::readVCalendar( filename );

    if ( events.count() ) {
        *cal = true;
    }

    QList<QTask> tasks = QTask::readVCalendar( filename );

    if ( tasks.count() ) {
        *todo = true;
    }
}

void ReceiveWindow::showWindow()
{
    showMaximized();
    activateWindow();
    raise();
}

// Returns whether the file was handled
bool ReceiveWindow::handleSupportedFormatComplete(int id, bool err)
{
    int index = -1;
    for (int i = 0; i < m_list.size(); i++) {
        if (m_list[i].id() == id) {
            index = i;
        }
    }

    if (index == -1)
        return false;

    qLog(Obex) << "Found file...";

    ReceivedFile file = m_list.takeAt(index);

    if (err) {
        qLog(Obex) << "Error occurred, deleting incoming file...";
        QString fn = Qtopia::tempDir() + "obex/in/" + file.filename();
        ::unlink(fn.toLocal8Bit().data());
        return true;
    }

    qLog(Obex) << "Figuring out mimetype and app to send to";

    QString incoming = Qtopia::tempDir() + "obex/in/" + file.filename();
    qLog(Obex) << "Temp file:" << incoming << "exists?" << QFile::exists(incoming);

    QMimeType mt(file.mimetype());

    qLog(Obex) << "Mimetype is: " << mt.id();
    QString service = "Receive/"+mt.id();
    qLog(Obex) << "Service is: " << service;
    QString receiveChannel = QtopiaService::channel(service);
    qLog(Obex) << "receive Channel is: " << receiveChannel;

    if ( receiveChannel.isEmpty() ) {
        // Special cases...
        // ##### should split file, or some other full fix
        if ( mt.id().toLower() == "text/x-vcalendar" ) {
            bool calendar, todo;
            vcalInfo(Qtopia::tempDir() + "obex/in/" + file.filename(), &todo, &calendar);
            if ( calendar ) {
                receiveChannel = QtopiaService::channel(service+"-Events");
            }
            else if ( todo ) {
                receiveChannel = QtopiaService::channel(service+"-Tasks");
            }
        }
    }

    if ( !receiveChannel.isEmpty() ) {
        // Send immediately
        QContent lnk(QtopiaService::app(service));
        qLog(Obex) << "Sending QtopiaIpcEnvelope";
        QtopiaIpcEnvelope e( receiveChannel, "receiveData(QString,QString)");
        e << Qtopia::tempDir() + "obex/in/" + file.filename() << mt.id();
    }

    return true;
}

#include "receivewindow.moc"
