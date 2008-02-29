/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
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

#include <qcontent.h>
#include <qmimetype.h>

#include <qtopiaipcenvelope.h>
#include <qtopiaservices.h>
#include <qtopiaapplication.h>
#include <qtopialog.h>

#ifndef QTOPIA_MEDIA
# include <qtopia/pim/qcontact.h>
# include <qtopia/pim/qappointment.h>
# include <qtopia/pim/qtask.h>
#endif

#include <QTableView>
#include <QHeaderView>
#include <QString>
#include <QTableView>
#include <QAbstractTableModel>
#include <QItemDelegate>
#include <QMessageBox>

#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

class ReceivedFile
{
public:

    enum Direction { Sending, Receiving };

    ReceivedFile(int id, Direction dir, const QString &filename, const QString &mimetype);

    void setProgress(qint64 completed, qint64 total);
    qint64 total() const { return m_total; }
    qint64 completed() const { return m_completed; }

    const QString filename() const { return m_filename; }
    const QString mimetype() const { return m_mimetype; }

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
    qint64 m_completed;
    qint64 m_total;
    Direction m_direction;
    int m_id;
};

ReceivedFile::ReceivedFile(int id, Direction dir, const QString &filename, const QString &mimetype)
{
    m_id = id;
    m_filename = filename;
    m_mimetype = mimetype;
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
        if ( str.length() > 12 ) {
            str.truncate(9);
            str += "...";
        }

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

    if (m_list[idx].direction() == ReceivedFile::Sending) {
        removeFile(idx);
    }
    else {
        emit dataChanged(index(idx, 0), index(idx, 2));
    }
}

static QString pretty_print_size(int fsize)
{
    static const char *size_suffix[] = {"", "K", "M", "G"};

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
        if (index.column() != 2) {
            QItemDelegate::paint(painter, option, index);
            return;
        }

        // Set up a QStyleOptionProgressBar to precisely mimic the
        // environment of a progress bar.
        QStyleOptionProgressBar progressBarOption;
        progressBarOption.state = QStyle::State_Enabled;
        progressBarOption.direction = QApplication::layoutDirection();
        progressBarOption.rect = option.rect;
        progressBarOption.fontMetrics = QApplication::fontMetrics();
        progressBarOption.minimum = 0;
        int max = m_model->file(index.row()).total();
        progressBarOption.maximum =  max;
        progressBarOption.textAlignment = Qt::AlignCenter;
        progressBarOption.textVisible = true;

        // Set the progress and text values of the style option.
        int progress = m_model->file(index.row()).completed();
        if (progress > max) {
            progressBarOption.progress = 0;
            progressBarOption.text = QString("%1/??").arg(pretty_print_size(progress));
        }
        else {
            progressBarOption.progress = progress;
            progressBarOption.text = QString("%1/%2").
                    arg(pretty_print_size(progress)).
                    arg(pretty_print_size(max));
        }

        // Draw the progress bar onto the view.
        QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption, painter);
    }

private:
    ReceivedFilesModel *m_model;
};

ReceiveWindow::ReceiveWindow(QWidget *parent) : QMainWindow(parent)
{
    m_files = new QTableView(this);
    m_model = new ReceivedFilesModel(this);

    m_files->setModel(m_model);
    m_files->setItemDelegate(new ReceiveWindowItemDelegate(this, m_model));
    m_files->setAlternatingRowColors(true);
    m_files->verticalHeader()->hide();
    m_files->setSelectionBehavior(QAbstractItemView::SelectRows);

    QHeaderView *header = m_files->horizontalHeader();
    QFontMetrics fm = fontMetrics();

    connect(m_files, SIGNAL(activated(const QModelIndex &)),
            SLOT(fileSelected(const QModelIndex &)));

    setWindowTitle(QObject::tr("Send/Receive Files"));
    setCentralWidget(m_files);

    header->setResizeMode(0, QHeaderView::Custom);
    header->resizeSection(0, fm.width(">"));                  // no tr
    header->setResizeMode(1, QHeaderView::Custom);
    header->resizeSection(1, fm.width("12345678.ext"));       // no tr
    header->setResizeMode(2, QHeaderView::Stretch);
}

ReceiveWindow::~ReceiveWindow()
{
    delete m_files;
    delete m_model;
    m_files = 0;
    m_model = 0;
}

void ReceiveWindow::receiveInitiated(int id, const QString &filename, const QString &mime)
{
    qLog(Obex) << "recvInitiated: " << filename << "(" << mime << ")";

    if ((mime.toLower() == "text/x-vcard") || (mime.toLower() == "text/x-vcalendar")) {
        qLog(Obex) << "Looks like a Vcard or VCal";
        handleSupportedFormatRecv(id, filename, mime);
    }
    else {
        qLog(Obex) << "Looks like a regular file...";
        m_model->addFile(ReceivedFile(id, ReceivedFile::Receiving, filename, mime));
        showMaximized();
    }
}

void ReceiveWindow::sendInitiated(int id, const QString &filename, const QString &mime)
{
    qLog(Obex) << "sendInitiated";

    // Don't do anything here
    if ((mime.toLower() == "text/x-vcard") || (mime.toLower() == "text/x-vcalendar")) {
    }
    else {
        m_model->addFile(ReceivedFile(id, ReceivedFile::Sending, filename, mime));
        showMaximized();
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
        if (m_model->size() == 0)
            close();
    }
}

void ReceiveWindow::fileSelected(const QModelIndex &index)
{
    if (m_model->size() == 0)
        return;

    ReceivedFile file = m_model->file(index.row());
    if (!file.completed())
        return;

    // This should never happen!
    if (file.direction() == ReceivedFile::Sending)
        return;

    QMessageBox *box = new QMessageBox(QObject::tr("Save/Discard File?"),
                                       QObject::tr("<P>Save or Discard the file?"),
                                       QMessageBox::Question,
                                       QMessageBox::Yes|QMessageBox::Default,
                                       QMessageBox::No,
                                       QMessageBox::Cancel|QMessageBox::Escape);
    box->setButtonText(QMessageBox::Yes, QObject::tr("Save"));
    box->setButtonText(QMessageBox::No, QObject::tr("Discard"));

#ifdef QTOPIA_PHONE
    int result = QtopiaApplication::execDialog(box);
#else
    int result = box->exec();
#endif

    delete box;

    qLog(Obex) << "Result: " << result;

    if (result == QMessageBox::Cancel)
        return;

    QString saveAs = m_model->file(index.row()).filename();
    int pos = saveAs.lastIndexOf( "/" );
    if ( pos != -1 )
        saveAs = saveAs.mid( pos );

    QString tmpFile = Qtopia::tempDir() + "obex/in/" + saveAs;

    if (result == QMessageBox::No) {
        qLog(Obex) << "Discarding file!";
        ::unlink(tmpFile.toLocal8Bit().data());
        m_model->removeFile(index.row());
        if (m_model->size() == 0)
            close();
        return;
    }

    QFile f(tmpFile);
    if ( !f.open(QIODevice::ReadOnly) ) {
        qLog(Obex) << "Couldn't open file to read!!";
        return;
    }

    QContent doc;
    doc.setType( QMimeType( saveAs ).id() );
    // strip off extension
    pos = saveAs.lastIndexOf( "." );
    if ( pos != -1 )
        saveAs = saveAs.left( pos );
    doc.setName( saveAs );
    QIODevice *device = doc.open(QIODevice::WriteOnly);

    qLog(Obex) << "File for QContent is: " << doc.file();

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
        qLog(Obex) << "Failed to save file" << doc.file();
    }

    f.close();
    ::unlink(tmpFile.toLocal8Bit().data());

    m_model->removeFile(index.row());
    if (m_model->size() == 0)
        close();
}

void ReceiveWindow::handleSupportedFormatRecv(int id, const QString &file, const QString &mime)
{
    m_list.push_back(ReceivedFile(id, ReceivedFile::Receiving, file, mime));
}

#ifndef QTOPIA_MEDIA
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
#endif

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

    QMimeType mt(file.mimetype());

    qLog(Obex) << "Mimetype is: " << mt.id();
    QString service = "Receive/"+mt.id();
    qLog(Obex) << "Service is: " << service;
    QString receiveChannel = QtopiaService::channel(service);
    qLog(Obex) << "receive Channel is: " << receiveChannel;

    if ( receiveChannel.isEmpty() ) {
        // Special cases...
        // ##### should split file, or some other full fix
#ifndef QTOPIA_MEDIA
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
#endif
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
