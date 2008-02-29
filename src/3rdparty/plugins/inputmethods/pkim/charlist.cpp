
#include "charlist.h"
#include <qapplication.h>
#include <qpainter.h>
#include <qtimer.h>
#include <QDesktopWidget>

CharList::CharList(QWidget *parent, const char *name, Qt::WFlags f)
    : QWidget(parent, f | Qt::Tool | Qt::FramelessWindowHint)
{
    setObjectName(name);

    setBackgroundRole( QPalette::Base );

    fm = new QFontMetrics( font() );
    cellHeight = fm->lineSpacing() + 3;
    cellWidth = fm->width("W")+1;

    resize(50, cellHeight);
}

CharList::~CharList()
{
}


void CharList::setMicroFocus( int x, int y )
{
    QPoint p;
    if (y < 30 + appFont.pointSize() + height() + 5)
	p = QPoint(x, y+2);
    else
	p = QPoint(x, y-appFont.pointSize()-4-height());

    int dw = QApplication::desktop()->availableGeometry().width();
    if (p.x() + width() > dw)
	p.setX(dw-width());

    move(p);
}

void CharList::setChars(const QStringList &ch)
{
    chars = ch;
    int c = chars.indexOf(current);
    current = c >= 0 ? ch[c] : ch[0];
    cellWidth = 5;
    for (int i=0; i < (int)chars.count(); i++) {
	if (fm->width(chars[i]) > cellWidth)
	    cellWidth = fm->width(chars[i]);
    }
    resize(cellWidth*chars.count()+2, height());
    update();
}

void CharList::setCurrent(const QString &ch)
{
    current = ch;
    update();
}

void CharList::paintEvent(QPaintEvent *)
{
    QPainter p(this);
    p.setBrush(Qt::NoBrush);
    p.drawRect(0, 0, width(), height());
    int pos = 1;
    for (int i=0; i < (int)chars.count(); i++) {
	QString ch = chars[i];
	if (current == ch) {
            p.setPen(palette().highlightedText().color());
            p.fillRect(pos, 1, cellWidth, height()-2, palette().highlight());
	} else {
	    p.setPen(palette().text().color());
	    p.fillRect(pos, 1, cellWidth, height()-2, palette().base());
	}
	int cw = fm->width(ch);
	p.drawText(pos+(cellWidth-cw)/2, fm->ascent()+1, ch);
	pos += cellWidth;
    }
}

