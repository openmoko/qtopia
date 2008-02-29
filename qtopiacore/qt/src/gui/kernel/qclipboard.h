/****************************************************************************
**
** Copyright (C) 1992-2007 Trolltech ASA. All rights reserved.
**
** This file is part of the QtGui module of the Qt Toolkit.
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

#ifndef QCLIPBOARD_H
#define QCLIPBOARD_H

#include <QtCore/qobject.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

#ifndef QT_NO_CLIPBOARD

class QMimeSource;
class QMimeData;
class QImage;
class QPixmap;

class QClipboardPrivate;

class Q_GUI_EXPORT QClipboard : public QObject
{
    Q_OBJECT
    Q_DECLARE_PRIVATE(QClipboard)
private:
    QClipboard(QObject *parent);
    ~QClipboard();

public:
    enum Mode { Clipboard, Selection, FindBuffer, LastMode = FindBuffer };

    void clear(Mode mode = Clipboard);

    bool supportsSelection() const;
    bool supportsFindBuffer() const;

    bool ownsSelection() const;
    bool ownsClipboard() const;
    bool ownsFindBuffer() const;

    QString text(Mode mode = Clipboard) const;
    QString text(QString& subtype, Mode mode = Clipboard) const;
    void setText(const QString &, Mode mode = Clipboard);

#ifdef QT3_SUPPORT
    QT3_SUPPORT QMimeSource *data(Mode mode = Clipboard) const;
    QT3_SUPPORT void setData(QMimeSource*, Mode mode = Clipboard);
#endif
    const QMimeData *mimeData(Mode mode = Clipboard ) const;
    void setMimeData(QMimeData *data, Mode mode = Clipboard);

    QImage image(Mode mode = Clipboard) const;
    QPixmap pixmap(Mode mode = Clipboard) const;
    void setImage(const QImage &, Mode mode  = Clipboard);
    void setPixmap(const QPixmap &, Mode mode  = Clipboard);

Q_SIGNALS:
    void changed(QClipboard::Mode mode);
    void selectionChanged();
    void findBufferChanged();
    void dataChanged();
private Q_SLOTS:
    void ownerDestroyed();

protected:
    void connectNotify(const char *);
    bool event(QEvent *);

    friend class QApplication;
    friend class QApplicationPrivate;
    friend class QBaseApplication;
    friend class QDragManager;
    friend class QMimeSource;

private:
    Q_DISABLE_COPY(QClipboard)

    bool supportsMode(Mode mode) const;
    bool ownsMode(Mode mode) const;
    void emitChanged(Mode mode);
};

#endif // QT_NO_CLIPBOARD

QT_END_HEADER

#endif // QCLIPBOARD_H
