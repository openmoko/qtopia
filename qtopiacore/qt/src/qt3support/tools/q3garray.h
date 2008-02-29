/****************************************************************************
**
** Copyright (C) 1992-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qt Toolkit.
**
** $TROLLTECH_DUAL_LICENSE$
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#ifndef Q3GARRAY_H
#define Q3GARRAY_H

#include <Qt3Support/q3shared.h>

QT_BEGIN_HEADER

QT_MODULE(Qt3SupportLight)

class Q_COMPAT_EXPORT Q3GArray					// generic array
{
    friend class QBuffer;
public:
    // do not use this, even though this is public
    struct array_data : public Q3Shared {	// shared array
	array_data():data(0),len(0)
#ifdef QT_QGARRAY_SPEED_OPTIM
		    ,maxl(0)
#endif
	    {}
	char *data;				// actual array data
	uint  len;
#ifdef QT_QGARRAY_SPEED_OPTIM
	uint maxl;
#endif
    };
    Q3GArray();
    enum Optimization { MemOptim, SpeedOptim };
protected:
    Q3GArray(int, int);			// dummy; does not alloc
    Q3GArray(int size);			// allocate 'size' bytes
    Q3GArray(const Q3GArray &a);		// shallow copy
    virtual ~Q3GArray();

    Q3GArray    &operator=(const Q3GArray &a) { return assign(a); }

    virtual void detach()	{ duplicate(*this); }

    // ### Qt 4.0: maybe provide two versions of data(), at(), etc.
    char       *data()	 const	{ return shd->data; }
    uint	nrefs()	 const	{ return shd->count; }
    uint	size()	 const	{ return shd->len; }
    bool	isEqual(const Q3GArray &a) const;

    bool	resize(uint newsize, Optimization optim);
    bool	resize(uint newsize);

    bool	fill(const char *d, int len, uint sz);

    Q3GArray    &assign(const Q3GArray &a);
    Q3GArray    &assign(const char *d, uint len);
    Q3GArray    &duplicate(const Q3GArray &a);
    Q3GArray    &duplicate(const char *d, uint len);
    void	store(const char *d, uint len);

    array_data *sharedBlock()	const		{ return shd; }
    void	setSharedBlock(array_data *p) { shd=(array_data*)p; }

    Q3GArray    &setRawData(const char *d, uint len);
    void	resetRawData(const char *d, uint len);

    int		find(const char *d, uint index, uint sz) const;
    int		contains(const char *d, uint sz) const;

    void	sort(uint sz);
    int		bsearch(const char *d, uint sz) const;

    char       *at(uint index) const;

    bool	setExpand(uint index, const char *d, uint sz);

protected:
    virtual array_data *newData();
    virtual void deleteData(array_data *p);

private:
    static void msg_index(uint);
    array_data *shd;
};


inline char *Q3GArray::at(uint index) const
{
#if defined(QT_CHECK_RANGE)
    if (index >= size()) {
	msg_index(index);
	index = 0;
    }
#endif
    return &shd->data[index];
}

QT_END_HEADER

#endif // Q3GARRAY_H
