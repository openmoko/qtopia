#include "data.h"

#include <qapplication.h>

Data::Data(){clear();};
Data::~Data(){};

bool Data::push(char,bool) {
    return TRUE;
}

void Data::del() { }

void Data::clear() { }

QString Data::getType() {
    return QString("NONE");
}

QString Data::getFormattedOutput() {
    return formattedOutput;
}

#ifdef NEW_STYLE_DISPLAY
void Data::draw(QPainter *p) {
    int fontSize = p->font().pixelSize();
    // do some size checking here first...
    p->drawText(fontSize*16 - fontSize*formattedOutput.length(),
	    fontSize,getFormattedOutput());
}
#endif
