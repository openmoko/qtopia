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

#ifndef QLABEL_P_H
#define QLABEL_P_H

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Qt API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

#include "qlabel.h"

#include "../text/qtextdocumentlayout_p.h"
#include "private/qtextcontrol_p.h"
#include "qtextdocumentfragment.h"
#include "qframe_p.h"
#include "qtextdocument.h"
#include "qmovie.h"
#include "qimage.h"
#include "qbitmap.h"
#include "qpicture.h"
#include "qmenu.h"

class QLabelPrivate : public QFramePrivate
{
    Q_DECLARE_PUBLIC(QLabel)
public:
    QLabelPrivate() {}

    void init();
    void clearContents();
    void updateLabel();
    QSize sizeForWidth(int w) const;

    mutable QSize sh;
    mutable QSize msh;
    mutable bool valid_hints;
    mutable QSizePolicy sizePolicy;
    int margin;
    QString text;
    QPixmap  *pixmap;
    QPixmap *scaledpixmap;
    QImage *cachedimage;
#ifndef QT_NO_PICTURE
    QPicture *picture;
#endif
#ifndef QT_NO_MOVIE
    QPointer<QMovie> movie;
    void _q_movieUpdated(const QRect&);
    void _q_movieResized(const QSize&);
#endif
#ifndef QT_NO_SHORTCUT
    void updateShortcut();
#endif
    bool isRichText() const {
        return textformat == Qt::RichText
               || (textformat == Qt::AutoText && Qt::mightBeRichText(text));
    }
#ifndef QT_NO_SHORTCUT
    QPointer<QWidget> buddy;
    int shortcutId;
#endif
    ushort align;
    short indent;
    uint scaledcontents :1;
    mutable uint textDirty : 1;
    Qt::TextFormat textformat;
    QTextDocument* doc;
    QTextControl *control;
    QTextCursor shortcutCursor;
    Qt::TextInteractionFlags textInteractionFlags;

    void ensureTextLayouted() const;
    void ensureTextControl();
    void sendControlEvent(QEvent *e);

    void _q_linkHovered(const QString &link);

    QRect layoutRect() const;
    QRect documentRect() const;
    QPoint layoutPoint(const QPoint& p) const;
#ifndef QT_NO_CONTEXTMENU
    QMenu *createStandardContextMenu(const QPoint &pos);
#endif

    bool openExternalLinks;

    bool hasCustomCursor;
#ifndef QT_NO_CURSOR
    QCursor cursor;
#endif

    friend class QMessageBoxPrivate;
};

#endif // QLABEL_P_H
