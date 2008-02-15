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
** http://trolltech.com/products/qt/licenses/licensing/opensource/
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://trolltech.com/products/qt/licenses/licensing/licensingoverview
** or contact the sales department at sales@trolltech.com.
**
** In addition, as a special exception, Trolltech gives you certain
** additional rights. These rights are described in the Trolltech GPL
** Exception version 1.0, which can be found at
** http://www.trolltech.com/products/qt/gplexception/ and in the file
** GPL_EXCEPTION.txt in this package.
**
** In addition, as a special exception, Trolltech, as the sole copyright
** holder for Qt Designer, grants users of the Qt/Eclipse Integration
** plug-in the right for the Qt/Eclipse Integration to link to
** functionality provided by Qt Designer and its related libraries.
**
** Trolltech reserves all rights not expressly granted herein.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef QTEXTDOCUMENT_H
#define QTEXTDOCUMENT_H

#include <QtCore/qobject.h>
#include <QtCore/qsize.h>
#include <QtCore/qrect.h>
#include <QtGui/qfont.h>

QT_BEGIN_HEADER

QT_MODULE(Gui)

class QTextFormatCollection;
class QTextListFormat;
class QRect;
class QPainter;
class QPrinter;
class QAbstractTextDocumentLayout;
class QPoint;
class QTextCursor;
class QTextObject;
class QTextFormat;
class QTextFrame;
class QTextBlock;
class QTextCodec;
class QUrl;
class QVariant;
class QRectF;
class QTextOption;

template<typename T> class QVector;

namespace Qt
{
    enum HitTestAccuracy { ExactHit, FuzzyHit };
    enum WhiteSpaceMode {
        WhiteSpaceNormal,
        WhiteSpacePre,
        WhiteSpaceNoWrap,
        WhiteSpaceModeUndefined = -1
    };

    Q_GUI_EXPORT bool mightBeRichText(const QString&);
    Q_GUI_EXPORT QString escape(const QString& plain);
    Q_GUI_EXPORT QString convertFromPlainText(const QString &plain, WhiteSpaceMode mode = WhiteSpacePre);

#ifndef QT_NO_TEXTCODEC
    Q_GUI_EXPORT QTextCodec *codecForHtml(const QByteArray &ba);
#endif
}

class Q_GUI_EXPORT QAbstractUndoItem
{
public:
    virtual ~QAbstractUndoItem() = 0;
    virtual void undo() = 0;
    virtual void redo() = 0;
};

inline QAbstractUndoItem::~QAbstractUndoItem()
{
}

class QTextDocumentPrivate;

class Q_GUI_EXPORT QTextDocument : public QObject
{
    Q_OBJECT

    Q_PROPERTY(bool undoRedoEnabled READ isUndoRedoEnabled WRITE setUndoRedoEnabled)
    Q_PROPERTY(bool modified READ isModified WRITE setModified DESIGNABLE false)
    Q_PROPERTY(QSizeF pageSize READ pageSize WRITE setPageSize)
    Q_PROPERTY(QFont defaultFont READ defaultFont WRITE setDefaultFont)
    Q_PROPERTY(bool useDesignMetrics READ useDesignMetrics WRITE setUseDesignMetrics)
    Q_PROPERTY(QSizeF size READ size)
    Q_PROPERTY(qreal textWidth READ textWidth WRITE setTextWidth)
    Q_PROPERTY(int blockCount READ blockCount)
    Q_PROPERTY(QString defaultStyleSheet READ defaultStyleSheet WRITE setDefaultStyleSheet)
    Q_PROPERTY(int maximumBlockCount READ maximumBlockCount WRITE setMaximumBlockCount)
    QDOC_PROPERTY(QTextOption defaultTextOption READ defaultTextOption WRITE setDefaultTextOption)

public:
    explicit QTextDocument(QObject *parent = 0);
    explicit QTextDocument(const QString &text, QObject *parent = 0);
    ~QTextDocument();

    QTextDocument *clone(QObject *parent = 0) const;

    bool isEmpty() const;
    virtual void clear();

    void setUndoRedoEnabled(bool enable);
    bool isUndoRedoEnabled() const;

    bool isUndoAvailable() const;
    bool isRedoAvailable() const;

    void setDocumentLayout(QAbstractTextDocumentLayout *layout);
    QAbstractTextDocumentLayout *documentLayout() const;

    enum MetaInformation {
        DocumentTitle
    };
    void setMetaInformation(MetaInformation info, const QString &);
    QString metaInformation(MetaInformation info) const;

    QString toHtml(const QByteArray &encoding = QByteArray()) const;
    void setHtml(const QString &html);

    QString toPlainText() const;
    void setPlainText(const QString &text);

    enum FindFlag
    {
        FindBackward        = 0x00001,
        FindCaseSensitively = 0x00002,
        FindWholeWords      = 0x00004
    };
    Q_DECLARE_FLAGS(FindFlags, FindFlag)

    QTextCursor find(const QString &subString, int from = 0, FindFlags options = 0) const;
    QTextCursor find(const QString &subString, const QTextCursor &from, FindFlags options = 0) const;

    QTextCursor find(const QRegExp &expr, int from = 0, FindFlags options = 0) const;
    QTextCursor find(const QRegExp &expr, const QTextCursor &from, FindFlags options = 0) const;

    QTextFrame *frameAt(int pos) const;
    QTextFrame *rootFrame() const;

    QTextObject *object(int objectIndex) const;
    QTextObject *objectForFormat(const QTextFormat &) const;

    QTextBlock findBlock(int pos) const;
    QTextBlock begin() const;
    QTextBlock end() const;

    void setPageSize(const QSizeF &size);
    QSizeF pageSize() const;

    void setDefaultFont(const QFont &font);
    QFont defaultFont() const;

    int pageCount() const;

    bool isModified() const;

#ifndef QT_NO_PRINTER
    void print(QPrinter *printer) const;
#endif

    enum ResourceType {
        HtmlResource  = 1,
        ImageResource = 2,
        StyleSheetResource = 3,

        UserResource  = 100
    };

    QVariant resource(int type, const QUrl &name) const;
    void addResource(int type, const QUrl &name, const QVariant &resource);

    QVector<QTextFormat> allFormats() const;

    void markContentsDirty(int from, int length);

    void setUseDesignMetrics(bool b);
    bool useDesignMetrics() const;

    void drawContents(QPainter *painter, const QRectF &rect = QRectF());

    void setTextWidth(qreal width);
    qreal textWidth() const;

    qreal idealWidth() const;

    void adjustSize();
    QSizeF size() const;

    int blockCount() const;

    void setDefaultStyleSheet(const QString &sheet);
    QString defaultStyleSheet() const;

    void undo(QTextCursor *cursor);
    void redo(QTextCursor *cursor);

    int maximumBlockCount() const;
    void setMaximumBlockCount(int maximum);

    QTextOption defaultTextOption() const;
    void setDefaultTextOption(const QTextOption &option);

Q_SIGNALS:
    void contentsChange(int from, int charsRemoves, int charsAdded);
    void contentsChanged();
    void undoAvailable(bool);
    void redoAvailable(bool);
    void modificationChanged(bool m);
    void cursorPositionChanged(const QTextCursor &cursor);
    void blockCountChanged(int newBlockCount);

public Q_SLOTS:
    void undo();
    void redo();
    void appendUndoItem(QAbstractUndoItem *);
    void setModified(bool m = true);

protected:
    virtual QTextObject *createObject(const QTextFormat &f);
    virtual QVariant loadResource(int type, const QUrl &name);

    QTextDocument(QTextDocumentPrivate &dd, QObject *parent);
public:
    QTextDocumentPrivate *docHandle() const;
private:
    Q_DISABLE_COPY(QTextDocument)
    Q_DECLARE_PRIVATE(QTextDocument)
};

Q_DECLARE_OPERATORS_FOR_FLAGS(QTextDocument::FindFlags)

QT_END_HEADER

#endif // QTEXTDOCUMENT_H
