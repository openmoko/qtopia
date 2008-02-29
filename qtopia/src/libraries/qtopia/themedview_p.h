/**********************************************************************
** Copyright (C) 2000-2005 Trolltech AS.  All rights reserved.
**
** This file is part of the Qtopia Environment.
** 
** This program is free software; you can redistribute it and/or modify it
** under the terms of the GNU General Public License as published by the
** Free Software Foundation; either version 2 of the License, or (at your
** option) any later version.
** 
** A copy of the GNU GPL license version 2 is included in this package as 
** LICENSE.GPL.
**
** This program is distributed in the hope that it will be useful, but
** WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
** See the GNU General Public License for more details.
**
** In addition, as a special exception Trolltech gives permission to link
** the code of this program with Qtopia applications copyrighted, developed
** and distributed by Trolltech under the terms of the Qtopia Personal Use
** License Agreement. You must comply with the GNU General Public License
** in all respects for all of the code used other than the applications
** licensed under the Qtopia Personal Use License Agreement. If you modify
** this file, you may extend this exception to your version of the file,
** but you are not obligated to do so. If you do not wish to do so, delete
** this exception statement from your version.
** 
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**********************************************************************/

#ifndef THEMEDVIEW_H
#define THEMEDVIEW_H

#include <qwidget.h>
#include <qpixmap.h>
#include <qlist.h>
#include <qmap.h>
#include <qfile.h>

class ThemedView;
class ThemedViewPrivate;
class ThemedItemInterface;
class ThemeAnimationItemPrivate;
class PluginLoader;
class QPainter;
class QXmlAttributes;
class QSimpleRichText;
class QDataStream;
class ThemePageItem;
class ThemeItem;

class ThemeCache 
{
public:
    ThemeCache();

    void setThemeFile( const QString &themeFile );

    bool load( ThemedViewPrivate *data );
    bool save( ThemedViewPrivate *data );

    void remove();

    ~ThemeCache();

    void setPalette( const QPalette &p );

    bool loaded() const;

private:
    ThemePageItem *loadHelper( ThemedViewPrivate *data );
    void save( ThemeItem *item );
    bool paletteColorsEqual( const QPalette &a, const QPalette &b ) const;

    const QString baseDir() const;

    QString m_cacheFileName, m_themeFile;
    QFile m_cacheFile;
    QDataStream *m_cacheStream;
    bool m_loaded;
    QPalette m_palette;
#ifdef DEBUG
    bool m_paletteBeenSet;
#endif
};

/* 
   THEME ITEM BASE CLASS
 */
class ThemeItem
{
    friend class CallItem;
public:
    ThemeItem(ThemeItem *p, ThemedView *ir);
    virtual void readData( QDataStream &s );
    virtual void writeData( QDataStream &s );
    void init();

    enum State
    {
	Default = 1,
	Focus = 2,
	Pressed = 6, // Pressed | Focus 
	All = 7, // Default | Pressed | Focus
    };

    ThemeItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
    virtual ~ThemeItem();

    virtual void setActive(bool a);
    bool active() const { return actv; }

    QRect rect() const;
    QRect geometry()const;
    QRect geometryHint()const;
    void setGeometry(const QRect&);
    bool isVisible() const;

    virtual int rtti() const;

    QString itemName() const;

    bool isInteractive() const { return interactive; }
    bool pressed() const { return press; }
    bool hasFocus() const { return focus; }

    virtual void setPressed( bool p );
    virtual void setFocus( bool p );

    int state() const;
    ThemeItem *parentItem() const { return parent; }

protected:
    void setAttribute( const QString &key, const int &val, int st = ThemeItem::Default );
    int attribute( const QString &key, int st = ThemeItem::Default ) const;
    void setAttribute( const QString &key, const QString &val, int st = ThemeItem::Default );
    QString strAttribute( const QString &key, int st = ThemeItem::Default );

    void paletteChange(const QPalette &);
    virtual void paint(QPainter *p, const QRect &r);
    virtual void layout();

    enum RMode { Rect, Coords };

    void update();
    void update(int x, int y, int w, int h);
    QMap<QString,QString> parseSubAtts( const QString &subatts ) const;
    QRect parseRect(const QXmlAttributes &atts, RMode &rmode, const QString &name=QString::null);
    int parseAlignment(const QXmlAttributes &atts, const QString &name=QString::null, int defAlign=Qt::AlignLeft|Qt::AlignTop);

    virtual void addCharacters(const QString &ch);

protected:
    ThemeItem *parent;
    ThemedView *view;
    QString name;
    QRect sr;
    QRect br;
    RMode rmode;
    bool actv;
    bool transient;
    bool visible;
    QList<ThemeItem> chldn;
    bool press;
    bool focus;
    bool interactive;
    QMap<QString,int> intattributes[3];
    QMap<QString,QString> strattributes[3];

    friend class ThemeCache;
    friend class ThemedView;
    friend class ThemedViewPrivate;
    friend class ThemeExclusiveItem;
    friend class ThemeLayoutItem;
    friend class ThemeGroupItem;
};

/*
   NON-VISUAL THEME ITEMS
*/

class ThemePageItem : public ThemeItem
{
public:
    ThemePageItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemePageItem(ThemedView *ir, const QXmlAttributes &atts);

    const QString &base() const { return bd; }

    int rtti() const;

    QSize sizeHint() const;

protected:
    virtual void paint(QPainter *p, const QRect &r);
    virtual void layout();

private:
    void applyMask();

protected:
    QPixmap bg;
    QString bd;
    bool stretch;
    QString maskImg;
    int offs[2];
};

class ThemeGroupItem : public ThemeItem
{
public:
    ThemeGroupItem(ThemeItem *p, ThemedView *ir);
    ThemeGroupItem( ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts );

    int rtti() const;

protected:
    virtual void setPressed( bool p );
};

#ifndef QTOPIA_DESKTOP

class ThemePluginItem : public ThemeItem
{
public:
    ThemePluginItem(ThemeItem *p, ThemedView *ir);
    ThemePluginItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
    ~ThemePluginItem();
    void init();

    void setPlugin(const QString &);
    void setBuiltin(ThemedItemInterface *);

    int rtti() const;

protected:
    virtual void paint(QPainter *p, const QRect &r);
    virtual void layout();

private:
    void releasePlugin();

protected:
    PluginLoader *loader;
    ThemedItemInterface *iface;
    bool builtin;
};

#endif

class ThemeLayoutItem : public ThemeItem
{
public:
    ThemeLayoutItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );
    void init();

    ThemeLayoutItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    int rtti() const;

protected:
    virtual void layout();

private:
    Qt::Orientation orient;
    int spacing;
    int align;
    bool stretch;
};

class ThemeExclusiveItem : public ThemeItem
{
public:
    ThemeExclusiveItem(ThemeItem *p, ThemedView *ir);
    ThemeExclusiveItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    int rtti() const;

protected:
    virtual void layout();
};

/* GRAPHICAL THEME ITEMS */
class ThemeGraphicItem : public ThemeItem
{
public:
    ThemeGraphicItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemeGraphicItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
    
protected:
    void setColor( const QString &key, const QColor &val, int st = ThemeItem::Default );
    QColor color( const QString &key, int st = ThemeItem::Default ) const;
    QFont font( const QString &key, int st = ThemeItem::Default ) const;
    void setFont( const QString &key, const QFont &val, int st = ThemeItem::Default );

    QFont parseFont(const QFont &defFont, const QString &size, const QString &bold );
    int parseColor(const QString &val, QColor &col);
    QColor getColor(const QColor &col, int role) const;
private:
    QMap<QString,QColor> colors[3];
    QMap<QString,QFont> fonts[3];
};

class ThemeInputItem : public ThemeGraphicItem
{
public:
    ThemeInputItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );
    void init();

    ThemeInputItem(ThemedView *ir, const QXmlAttributes &atts);
    ThemeInputItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    QWidget *widget() const;

protected:
    void paletteChange(const QPalette &);
    virtual void layout();
    virtual int rtti() const;

    void parseColorGroup( const QMap<QString,QString> &cgatts );

    void setupWidget();

private:
    QMap<QString,QString> colorGroupAtts;
    QWidget *mWidget;
};

class ThemeTextItem : public ThemeGraphicItem
{
public:
    ThemeTextItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemeTextItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
    ~ThemeTextItem();

    void setText(const QString &t);
    QString text() const { return txt; }
    bool shortLabel() const { return shortLbl; }
    void setTextFormat(Qt::TextFormat format);
    Qt::TextFormat textFormat() const { return format; }

    int rtti() const;

protected:
    void setupFont( const QFont &deffont, const QString &size, const QString &bold, const QString &col, 
	    const QString &outline, ThemeItem::State st = ThemeItem::Default );
    virtual void paint(QPainter *p, const QRect &r);
    void drawOutline(QPainter *p, const QRect &r, int flags, const QString &text);
    void drawOutline(QPainter *p, const QRect &r, const QPalette &pal, QSimpleRichText *rt);
    void addCharacters(const QString &ch);

protected:
    QString txt;
    bool shortLbl;
    int shadow;
    QImage *shadowImg;
    int align;
    Qt::TextFormat format;
    bool richText;
};

class ThemeRectItemPrivate;

class ThemeRectItem : public ThemeGraphicItem
{
public:
    ThemeRectItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemeRectItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    int rtti() const;

    QColor brushColor( ThemeItem::State st = ThemeItem::Default ) const;

protected:
    void setupColor( const QString &key, const QString &col, ThemeItem::State st = ThemeItem::Default );
    void setupAlpha(const QString &key, const QString &al, ThemeItem::State st=ThemeItem::Default);
    virtual void paint(QPainter *p, const QRect &r);

private:
    ThemeRectItemPrivate *d[3];
};

class ThemeLineItem : public ThemeGraphicItem
{
public:
    ThemeLineItem(ThemeItem *p, ThemedView *ir);
    ThemeLineItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    int rtti() const;

protected:
    virtual void paint(QPainter *p, const QRect &r);
};

/* PIXMAP THEME ITEMS */
class ThemePixmapItem : public ThemeGraphicItem
{
public:
    ThemePixmapItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemePixmapItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
protected:
    QPixmap loadImage(const QString &imgName, int colorRole, const QColor &color, int alpha);
    void setPixmap( const QString &key, const QPixmap &val, int st = ThemeItem::Default );
    QPixmap pixmap( const QString &key, int st = ThemeItem::Default ) const;
protected:
    QMap<QString,QPixmap> pixmaps[3];
};

class ThemeAnimationItem : public ThemePixmapItem
{
public:
    ThemeAnimationItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemeAnimationItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);
    ~ThemeAnimationItem();

    void setFrame(int);
    int frame() const { return currFrame; }
    int frameCount() const { return attribute("count", state()); }
    void start();
    void stop();

    int rtti() const;

protected:
    void stateChange( int bst, int ast );
    void setFocus( bool f );
    void setPressed( bool p );
    void setupAnimation( const QString &base, const QString &src, const QString &color, const QString &alpha, 
	    const QString &count, const QString &width, const QString &loop, const QString &looprev, 
	    const QString &delay, ThemeItem::State st = ThemeItem::Default );

    virtual void paint(QPainter *p, const QRect &r);
    virtual void layout();
    void advance();

protected:
    int currFrame;
    int inc;
    ThemeAnimationItemPrivate *d[3];

    friend class ThemeAnimationItemPrivate;
};

class ThemeLevelItem : public ThemeAnimationItem
{
public:
    ThemeLevelItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );
    ThemeLevelItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    void setValue(int v);
    void setRange(int min, int max);

    int value() const { return val; }
    int minValue() const { return minVal; }
    int maxValue() const { return maxVal; }

    int rtti() const;

protected:
    int minVal;
    int maxVal;
    int val;
};

class ThemeStatusItem : public ThemePixmapItem
{
public:
    ThemeStatusItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );

    ThemeStatusItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    void setEnabled(bool e);
    bool enabled() const { return enable; }

    int rtti() const;

protected:
    void createImage( const QString &key, const QString &filename, const QString &col, 
		const QString &alpha, ThemeItem::State st = ThemeItem::Default );
    virtual void paint(QPainter *p, const QRect &r);

protected:
    bool enable;
};

class ThemeImageItem : public ThemePixmapItem
{
public:
    ThemeImageItem(ThemeItem *p, ThemedView *ir);
    void writeData( QDataStream &s );
    void readData( QDataStream &s );
    ThemeImageItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    void setImage( const QPixmap &p, ThemeItem::State st = ThemeItem::Default ); 
    QPixmap image( ThemeItem::State st = ThemeItem::Default ) const;

    int rtti() const;

protected:
    void createImage( const QString &filename, const QString &col, const QString &al, ThemeItem::State st = ThemeItem::Default );
    QPixmap scaleImage( const QPixmap &pix, int width, int height );
    virtual void layout();
    virtual void paint(QPainter *p, const QRect &r);
    void paletteChange(const QPalette &);

protected:
    int offs[2];
    bool stretch;
    bool scale;
    bool tile;
    Qt::Orientation sorient;
    QString imgName, pressedImgName, focusImgName;
    int align;
};

class ThemeListItem : public ThemeGraphicItem
{
public:
    ThemeListItem(ThemeItem *p, ThemedView *ir);
    ThemeListItem(ThemeItem *p, ThemedView *ir, const QXmlAttributes &atts);

    int rtti() const;
};

class ThemedView : public QWidget
{
    Q_OBJECT
public:
    ThemedView(QWidget *parent=0, const char *name=0, WFlags f=0);
    ~ThemedView();

    bool sourceLoaded() const; // return whether a source has been loaded

    bool setSource(const QString &file = QString());
    void setSourceFile(const QString &file);
    QSize sizeHint() const;

    enum Type { Item, Page, Animation, Level, Status, Image, Text, Rect,
	        Line, Plugin, Exclusive, Layout, Group, Input, ListItem };
    QValueList<ThemeItem*> findItems(const QString &name, int type, int state = ThemeItem::All) const;
    ThemeItem *findItem(const QString &name, int type, int state = ThemeItem::All) const;

    const QString &base() const;
    const QString defaultPics() const;
    const QString cacheDir() const;
    ThemeCache *themeCache();

    ThemeItem *itemAt( const QPoint &pos ) const;
    void paint(QPainter *p, const QRect &clip, ThemeItem *item=0);
    virtual QWidget *newInputItemWidget(ThemeInputItem*, const QString&);
    void setGeometryAndLayout(int x, int y, int w, int h);

signals:
    void itemPressed( ThemeItem * );
    void itemClicked( ThemeItem * );

protected:
    ThemeItem *itemAt( const QPoint &pos, ThemeItem *item ) const;
    bool isEnabled( ThemeItem *item ) const;

    void mousePressEvent( QMouseEvent *e );
    void mouseReleaseEvent( QMouseEvent *e );
    void mouseMoveEvent( QMouseEvent *e );

    void paintEvent(QPaintEvent *);
    void paintItem(QPainter *p, ThemeItem *item, const QRect &clip);
    ThemeItem *findItem(ThemeItem *item, const QString &name, int type, int state = ThemeItem::All) const;
    void findItems(ThemeItem *item, const QString &name, int type, int pressed, QValueList<ThemeItem*> &list) const;
    void layout(ThemeItem *item=0);
    void paletteChange(const QPalette &);
    void focusInEvent(QFocusEvent *);
    void focusOutEvent(QFocusEvent *);
    void resizeEvent(QResizeEvent *);
    void showEvent(QShowEvent *);

    QString m_newSourceFile; // the next source as set by setSourceFile
    QString m_curSourceFile; // the one currently loaded

private:
    ThemedViewPrivate *d;
    QWidget *target;
    ThemeItem *pressedItem;
    ThemeCache *m_themeCache;

    friend class ThemeItem;
};


#endif
