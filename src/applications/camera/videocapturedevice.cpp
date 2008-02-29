/****************************************************************************
**
** Copyright (C) 2000-2007 TROLLTECH ASA. All rights reserved.
**
** This file is part of the Opensource Edition of the Qtopia Toolkit.
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
#include <unistd.h>
#include <fcntl.h>
#include <linux/videodev.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <custom.h>

#include "videocapturedevice.h"

#define VIDEO_DEFAULT_WIDTH     640
#define VIDEO_DEFAULT_HEIGHT    480

/*!
 * \class VideoCaptureDevice
 *
 * \brief  This class provides an interface for Capture Devices
 */

VideoCaptureDevice::VideoCaptureDevice():
    m_frames(NULL),
    m_width(VIDEO_DEFAULT_WIDTH),
    m_height(VIDEO_DEFAULT_HEIGHT),
    m_fd(-1)
{
    setupCamera();
}

VideoCaptureDevice::~VideoCaptureDevice()
{
    shutdown();
}

bool VideoCaptureDevice::isAvailable() const
{
    return m_fd != -1;
}

void VideoCaptureDevice::setCaptureSize(QSize size)
{
    m_size = size;
}

QSize VideoCaptureDevice::captureSize() const
{
    return m_size;
}

void VideoCaptureDevice::beginCapture(QImage*& image)
{
    v4l2_format     format;

    //
    image = NULL;

    // adjust camera
    memset(&format, 0, sizeof(format));

    format.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    format.fmt.pix.width = m_width;
    format.fmt.pix.height = m_height;
    format.fmt.pix.pixelformat = V4L2_PIX_FMT_RGB32;

    if (ioctl(m_fd, VIDIOC_S_FMT, &format) == 0)
    {
        v4l2_requestbuffers requestBuffers;

        memset(&requestBuffers, 0, sizeof(requestBuffers));

        requestBuffers.count = 1;
        requestBuffers.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
        requestBuffers.memory = V4L2_MEMORY_MMAP;

        if (ioctl(m_fd, VIDIOC_REQBUFS, &requestBuffers) == 0)
        {
            v4l2_buffer     buffer;

            memset(&buffer, 0, sizeof(buffer));

            buffer.index = 0;
            buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

            if (ioctl(m_fd, VIDIOC_QUERYBUF, &buffer) == 0)
            {
                m_imageBuffer  = (unsigned char*)mmap(NULL,
                                                      buffer.length,
                                                      PROT_READ | PROT_WRITE,
                                                      MAP_SHARED,
                                                      m_fd,
                                                      buffer.m.offset);

                if (m_imageBuffer != (unsigned char*) -1)
                {
                    v4l2_buf_type   type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

                    if (ioctl(fd, VIDIOC_QBUF, &buffer) == 0 &&
                        ioctl(fd, VIDIOC_STREAMON, &type) == 0)
                    {
                        m_imageBufferLength = buffer.length;

                        image = new QImage(m_imageBuffer,
                                            format.fmt.pix.width,
                                            format.fmt.pix.height,
                                            QImage::Format_RGB32);
                    }
                }
            }
        }
    }
}

void VideoCaptureDevice::endCapture(QImage* image)
{
    if (munmap(m_imageBuffer, m_imageBufferSize) != -1)
    {
        if (ioctl(m_fd, VIDIOC_STREAMOFF) != 0)
        {
            // panic?
        }

        delete image;
    }
}

void VideoCaptureDevice::sync()
{
    // sync on queued
    pollfd  fpolls;

    fpolls.fd = fd;
    fpolls.events = POLLIN;
    fpolls.revents = 0;

    poll(&fpolls, 1, -1);

    // queue next
    v4l2_buffer     buffer;

    buffer.index = 0;
    buffer.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
    ioctl(fd, VIDIOC_QBUF, &buffer);
}

void VideoCaptureDevice::setupCamera()
{
    bool                success = false;
    v4l2_capability     capability;

    // Open the video device.
    if (m_fd = open(V4L_VIDEO_DEVICE, O_RDWR) != -1) {

        memset(&capability, 0, sizeof(capability));

        if (ioctl(fd, VIDIOC_QUERYCAP, &capability) >= 0) {

            if ((capability.capabilities & V4L2_CAP_VIDEO_CAPTURE) != 0 &&
                (capability.capabilities & V4L2_CAP_STREAMING) != 0) {

                v4l2_input  input;

                memset(&input, 0, sizeof(input));

                // check for camera
                for (; ioctl(fd, VIDIOC_ENUMINPUT) == 0; ++input.index) {

                    if (input.type == V4L2_INPUT_TYPE_CAMERA) {
                        // select as input first camera type device

                        if (ioctl(fd, VIDIOC_S_INPUT, &input.index) != 0) {
                            success = true;
                        }
                    }
                }
            }
            else {
                qWarning("%s is not a suitable capture device", V4L_VIDEO_DEVICE);
            }
        }
        else {
            qWarning("%s: could not retrieve the video capabilities", V4L_VIDEO_DEVICE);
        }
    }
    else {
        qWarning("Unable to open %s: %s", V4L_VIDEO_DEVICE, strerror(errno));
    }

    if (!success && m_fd != -1) {
        close(m_fd);
        m_fd = -1;
    }
}

void VideoCaptureDevice::shutdown()
{
    close(m_fd);
}


