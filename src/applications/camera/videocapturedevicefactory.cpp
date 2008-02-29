/****************************************************************************
**
** Copyright (C) 2000-2006 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Phone Edition of the Qtopia Toolkit.
**
** This software is licensed under the terms of the GNU General Public
** License (GPL) version 2.
**
** See http://www.trolltech.com/gpl/ for GPL licensing information.
**
** Contact info@trolltech.com if any conditions of this licensing are
** not clear to you.
**
**
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**
****************************************************************************/

#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <linux/videodev.h>

#include "videocapturedevicefactory.h"
#include "videocapturedevice.h"
#ifdef HAVE_V4L2
#include "v4l2videocapturedevice.h"
#include "nodevice.h"
#endif
#include "v4l1videocapturedevice.h"
#ifdef _CAMERA_DEBUG
#include "dummyvideocapturedevice.h"
#endif

#define VIDEO_DEVICE    "/dev/video"    // TODO: fetched from settings

namespace camera
{

VideoCaptureDevice* VideoCaptureDeviceFactory::createVideoCaptureDevice()
{
    VideoCaptureDevice* rc;

#ifdef QTOPIA_CAMERA_DEBUG
    rc = new DummyVideoCaptureDevice;
#else

#ifdef HAVE_V4L2

    int fd;

    if ((fd = open(VIDEO_DEVICE, O_RDWR)) != -1)
    {
        v4l2_capability     capability;

        if (ioctl(fd, VIDIOC_QUERYCAP, &capability) == -1)
        {
//            if (errno == EINVAL)      // standard says this is what happens, but a (some) driver(s?) don't do this
//            {   // Failed v4l2 ioctl, assume v4l1
                close(fd);  // see below
                rc = new V4L1VideoCaptureDevice;
//            }
        }
        else
        {
            rc = new V4L2VideoCaptureDevice(fd);
        }

//        close(fd);    // XXX: asymmetric, problem with a driver which
                        // will not allow a second open after the closee
    }
    else
    {
        rc = new NoDevice;
    }

#else
    rc = new V4L1VideoCaptureDevice;
#endif  // HAVE_V4L2
#endif  // QTOPIA_CAMERA_DEBUG

    return rc;
}

}   // ns camera

