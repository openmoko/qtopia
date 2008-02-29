/****************************************************************************
**
** Copyright (C) 2005-2007 Trolltech ASA. All rights reserved.
** Copyright (C) 2002-2007 Bjoern Bergstroem
**
** This file is part of the Qt Designer of the Qt Toolkit.
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

#ifndef FOV_H
#define FOV_H

class OublietteLevel;

class FOV
{
protected:
	virtual ~FOV() {}

	virtual bool scanCell(OublietteLevel *map, int x, int y) = 0;
	virtual void applyCell(OublietteLevel *map, int x, int y) = 0;

	double slope(double x1, double y1, double x2, double y2);
	double invSlope(double x1, double y1, double x2, double y2);

	void scanNW2N(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNE2N(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNW2W(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSW2W(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanNE2E(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSE2E(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);

	void scanSW2S(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
	void scanSE2S(OublietteLevel *map, int xCenter, int yCenter, int distance, int maxRadius, double startSlope, double endSlope);
public:
	void start(OublietteLevel *map, unsigned int x, unsigned int y, int maxRadius);
};


class SIMPLEFOV : public FOV
{
private:
	bool scanCell(OublietteLevel *map, int x, int y);
	void applyCell(OublietteLevel *map, int x, int y);
};
#endif
