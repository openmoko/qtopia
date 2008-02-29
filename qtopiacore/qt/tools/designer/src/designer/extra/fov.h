/****************************************************************************
**
** Copyright (C) 2005-2006 TROLLTECH ASA. All rights reserved.
** Copyright (C) 2002-2006 Bjoern Bergstroem
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
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
