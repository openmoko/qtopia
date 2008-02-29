
#include "symbolpicker.h"
#include <qtopiaipcenvelope.h>
#include <qtranslatablesettings.h>
#include <qtopiaapplication.h>

#include <qapplication.h>
#include <qwsdisplay_qws.h>
#include <qtimer.h>
#include <qstyle.h>
#include <QPainter>

#define cr_width 12
#define cr_height 12
static unsigned char cr_bits[] = {
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x02, 0x00, 0x02, 0x08, 0x02,
    0x0c, 0x02, 0xfe, 0x03, 0x0c, 0x00, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00 };


SymbolPicker::SymbolPicker(QWidget *parent)
    : Picker(parent)
{
    QTranslatableSettings cfg(Qtopia::defaultButtonsFile(), QSettings::IniFormat); // No tr
    cfg.beginGroup("IMethod"); // No tr

    int charsPerRow = cfg.value("picker_width",13).toInt();
    setNumCols(charsPerRow);
    chars = cfg.value("picker_chars").toString();
    if ( chars.isEmpty() ) {
	// Default: toLatin1 non-letter printable characters, and newline
	int ch;
	chars += QChar('\n');
	for (ch='!'; ch<='/'; ch++) chars += QChar(ch);
	for (ch=':'; ch<='@'; ch++) chars += QChar(ch);
	for (ch='['; ch<='`'; ch++) chars += QChar(ch);
	for (ch='{'; ch<='~'; ch++) chars += QChar(ch);
	for (ch=161; ch<=191; ch++) chars += QChar(ch);
	chars += QChar(215);
	chars += QChar(247);
	chars += QChar(0x20A6);
	chars += QChar(0x20A9);
	chars += QChar(0x20AC);
	chars += QChar(0x2022);
    }

    QFont fnt(font());
    int cw=0;
    int tw;
    do {
	if ( cw ) // not first time round
	    fnt.setPointSize(fnt.pointSize()-2);
	QFontMetrics fm(fnt);
	cw = 1;
	for ( int i=0; i<(int)chars.length(); i++ ) {
	    int w = fm.width(chars[i]);
	    if ( w > cw ) cw = w;
	}
	tw = (cw+2)*charsPerRow;
//	tw += style()->scrollBarExtent().width();
//	tw += frameWidth() * 2;
    } while ( /*tw > qApp->desktop()->width() &&*/ fnt.pointSize() > 6 );

    setFont(fnt);

    QFontMetrics fm( font() );
    setCellHeight( fm.lineSpacing() + 3 );
//    setCellWidth( qMin(cw+1, qApp->desktop()->width() / (charsPerRow+1) ));
//    verticalScrollBar()->setLineStep(cellHeight());

    setNumRows((chars.length()+charsPerRow-1)/charsPerRow);

    symbols = new QChar* [numRows()];
    int ch=0;
    for (int i = 0; i < numRows(); i++) {
	symbols[i] = new QChar [charsPerRow];
	for (int c = 0; c < charsPerRow; c++)
	    symbols[i][c] = (ch < (int)chars.length()) ? QChar(chars[ch++]) : QChar();
    }

    connect(this, SIGNAL(cellClicked(int,int)), this, SLOT(sendSymbolForPos(int,int)));
}

SymbolPicker::~SymbolPicker()
{
    for (int i = 0; i < numRows(); i++)
	delete [] symbols[i];
    delete [] symbols;
}

// do press and release for proper click handling.
void SymbolPicker::sendSymbolForPos(int row, int col)
{
    // TODO also send Qt::Key_...
    if (!symbols[row][col].isNull())
	emit symbolClicked(symbols[row][col].unicode(), 0);
}

void SymbolPicker::drawCell(QPainter *p, int row, int col, bool selected)
{
    QChar u;
    QFontMetrics fm(font());
    p->setBrush(Qt::SolidPattern);
    if ( selected ) {
	p->setPen( palette().highlightedText().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().highlight());
    } else {
	p->setPen( palette().text().color() );
	p->fillRect( 0, 0, cellWidth(), cellHeight(), palette().base() );
    }
    if (symbols[row][col] == '\n') {
	QBitmap bm = QBitmap::fromData(QSize(cr_width, cr_height), cr_bits);
	bm.setMask(bm);
	p->drawPixmap((cellWidth()-bm.width())/2,
		(cellHeight()-bm.height())/2, bm);
    } else {
	u = QChar(symbols[row][col]);
	if (!u.isNull() && fm.inFont(u))
	    p->drawText(0, 0, cellWidth(), cellHeight(), Qt::AlignCenter, QString(u));
    }
}

