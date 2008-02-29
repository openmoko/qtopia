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

#ifndef Q3GROUPBOX_H
#define Q3GROUPBOX_H

#include <QtGui/qgroupbox.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q3GroupBoxPrivate;

class Q_COMPAT_EXPORT Q3GroupBox : public QGroupBox
{
    Q_OBJECT
    Q_PROPERTY(Qt::Orientation orientation READ orientation WRITE setOrientation DESIGNABLE false)
    Q_PROPERTY(int columns READ columns WRITE setColumns DESIGNABLE false)
public:
    explicit Q3GroupBox(QWidget* parent=0, const char* name=0);
    explicit Q3GroupBox(const QString &title,
	       QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o,
	       QWidget* parent=0, const char* name=0);
    Q3GroupBox(int strips, Qt::Orientation o, const QString &title,
	       QWidget* parent=0, const char* name=0);
    ~Q3GroupBox();

    virtual void setColumnLayout(int strips, Qt::Orientation o);

    int columns() const;
    void setColumns(int);

    Qt::Orientation orientation() const;
    void setOrientation(Qt::Orientation);

    int insideMargin() const;
    int insideSpacing() const;
    void setInsideMargin(int m);
    void setInsideSpacing(int s);

    void addSpace(int);

    void setFrameRect(QRect) {}
    QRect frameRect() const { return QRect(); }
    enum DummyFrame { Box, Sunken, Plain, Raised, MShadow, NoFrame, Panel, StyledPanel, 
                      HLine, VLine, GroupBoxPanel, WinPanel, ToolBarPanel, MenuBarPanel, 
                      PopupPanel, LineEditPanel, TabWidgetPanel, MShape };
    void setFrameShadow(DummyFrame) {}
    DummyFrame frameShadow() const { return Plain; }
    void setFrameShape(DummyFrame) {}
    DummyFrame frameShape() const { return NoFrame; }
    void setFrameStyle(int) {}
    int frameStyle() const  { return 0; }
    int frameWidth() const { return 0; }
    void setLineWidth(int) {}
    int lineWidth() const { return 0; }    
    void setMargin(int margin) { setContentsMargins(margin, margin, margin, margin); }
    int margin() const 
    { int margin; int dummy; getContentsMargins(&margin, &dummy, &dummy, &dummy);  return margin; }    
    void setMidLineWidth(int) {}
    int midLineWidth() const { return 0; }

protected:
    void childEvent(QChildEvent *);
    void resizeEvent(QResizeEvent *);
    void changeEvent(QEvent *);

private:
    void skip();
    void init();
    void calculateFrame();
    void insertWid(QWidget*);
    Q3GroupBoxPrivate * d;

    Q_DISABLE_COPY(Q3GroupBox)
};

QT_END_HEADER

#endif // Q3GROUPBOX_H
