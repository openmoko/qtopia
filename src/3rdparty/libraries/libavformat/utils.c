/*
 * Various utilities for ffmpeg system
 * Copyright (c) 2000, 2001, 2002 Fabrice Bellard
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#include "avformat.h"

AVInputFormat *first_iformat;
AVOutputFormat *first_oformat;
AVImageFormat *first_image_format;

void av_register_input_format(AVInputFormat *format)
{
    AVInputFormat **p;
    p = &first_iformat;
    while (*p != NULL) p = &(*p)->next;
    *p = format;
    format->next = NULL;
}

void av_register_output_format(AVOutputFormat *format)
{
    AVOutputFormat **p;
    p = &first_oformat;
    while (*p != NULL) p = &(*p)->next;
    *p = format;
    format->next = NULL;
}

int match_ext(const char *filename, const char *extensions)
{
    const char *ext, *p;
    char ext1[32], *q;

    ext = strrchr(filename, '.');
    if (ext) {
        ext++;
        p = extensions;
        for(;;) {
            q = ext1;
            while (*p != '\0' && *p != ',') 
                *q++ = *p++;
            *q = '\0';
            if (!strcasecmp(ext1, ext)) 
                return 1;
            if (*p == '\0') 
                break;
            p++;
        }
    }
    return 0;
}

AVOutputFormat *guess_format(const char *short_name, const char *filename, 
                             const char *mime_type)
{
    AVOutputFormat *fmt, *fmt_found;
    int score_max, score;

    /* specific test for image sequences */
    if (!short_name && filename && 
        filename_number_test(filename) >= 0 &&
        guess_image_format(filename)) {
        return guess_format("image", NULL, NULL);
    }

    /* find the proper file type */
    fmt_found = NULL;
    score_max = 0;
    fmt = first_oformat;
    while (fmt != NULL) {
        score = 0;
        if (fmt->name && short_name && !strcmp(fmt->name, short_name))
            score += 100;
        if (fmt->mime_type && mime_type && !strcmp(fmt->mime_type, mime_type))
            score += 10;
        if (filename && fmt->extensions && 
            match_ext(filename, fmt->extensions)) {
            score += 5;
        }
        if (score > score_max) {
            score_max = score;
            fmt_found = fmt;
        }
        fmt = fmt->next;
    }
    return fmt_found;
}   

AVOutputFormat *guess_stream_format(const char *short_name, const char *filename, 
                             const char *mime_type)
{
    AVOutputFormat *fmt = guess_format(short_name, filename, mime_type);

    if (fmt) {
        AVOutputFormat *stream_fmt;
        char stream_format_name[64];

        snprintf(stream_format_name, sizeof(stream_format_name), "%s_stream", fmt->name);
        stream_fmt = guess_format(stream_format_name, NULL, NULL);

        if (stream_fmt)
            fmt = stream_fmt;
    }

    return fmt;
}

AVInputFormat *av_find_input_format(const char *short_name)
{
    AVInputFormat *fmt;
    for(fmt = first_iformat; fmt != NULL; fmt = fmt->next) {
        if (!strcmp(fmt->name, short_name))
            return fmt;
    }
    return NULL;
}

/* memory handling */

/**
 * Default packet destructor 
 */
static void av_destruct_packet(AVPacket *pkt)
{
    av_free(pkt->data);
    pkt->data = NULL; pkt->size = 0;
}

/**
 * Allocate the payload of a packet and intialized its fields to default values.
 *
 * @param pkt packet
 * @param size wanted payload size
 * @return 0 if OK. AVERROR_xxx otherwise.
 */
int av_new_packet(AVPacket *pkt, int size)
{
    void *data = av_malloc(size + FF_INPUT_BUFFER_PADDING_SIZE);
    if (!data)
        return AVERROR_NOMEM;
    memset(data + size, 0, FF_INPUT_BUFFER_PADDING_SIZE);

    av_init_packet(pkt);
    pkt->data = data; 
    pkt->size = size;
    pkt->destruct = av_destruct_packet;
    return 0;
}

/* fifo handling */

int fifo_init(FifoBuffer *f, int size)
{
    f->buffer = av_malloc(size);
    if (!f->buffer)
        return -1;
    f->end = f->buffer + size;
    f->wptr = f->rptr = f->buffer;
    return 0;
}

void fifo_free(FifoBuffer *f)
{
    av_free(f->buffer);
}

int fifo_size(FifoBuffer *f, uint8_t *rptr)
{
    int size;

    if (f->wptr >= rptr) {
        size = f->wptr - rptr;
    } else {
        size = (f->end - rptr) + (f->wptr - f->buffer);
    }
    return size;
}

/* get data from the fifo (return -1 if not enough data) */
int fifo_read(FifoBuffer *f, uint8_t *buf, int buf_size, uint8_t **rptr_ptr)
{
    uint8_t *rptr = *rptr_ptr;
    int size, len;

    if (f->wptr >= rptr) {
        size = f->wptr - rptr;
    } else {
        size = (f->end - rptr) + (f->wptr - f->buffer);
    }
    
    if (size < buf_size)
        return -1;
    while (buf_size > 0) {
        len = f->end - rptr;
        if (len > buf_size)
            len = buf_size;
        memcpy(buf, rptr, len);
        buf += len;
        rptr += len;
        if (rptr >= f->end)
            rptr = f->buffer;
        buf_size -= len;
    }
    *rptr_ptr = rptr;
    return 0;
}

void fifo_write(FifoBuffer *f, uint8_t *buf, int size, uint8_t **wptr_ptr)
{
    int len;
    uint8_t *wptr;
    wptr = *wptr_ptr;
    while (size > 0) {
        len = f->end - wptr;
        if (len > size)
            len = size;
        memcpy(wptr, buf, len);
        wptr += len;
        if (wptr >= f->end)
            wptr = f->buffer;
        buf += len;
        size -= len;
    }
    *wptr_ptr = wptr;
}

int filename_number_test(const char *filename)
{
    char buf[1024];
    return get_frame_filename(buf, sizeof(buf), filename, 1);
}

/* guess file format */
AVInputFormat *av_probe_input_format(AVProbeData *pd, int is_opened)
{
    AVInputFormat *fmt1, *fmt;
    int score, score_max;

    fmt = NULL;
    score_max = 0;
    for(fmt1 = first_iformat; fmt1 != NULL; fmt1 = fmt1->next) {
        if (!is_opened && !(fmt1->flags & AVFMT_NOFILE))
            continue;
        score = 0;
        if (fmt1->read_probe) {
            score = fmt1->read_probe(pd);
        } else if (fmt1->extensions) {
            if (match_ext(pd->filename, fmt1->extensions)) {
                score = 50;
            }
        } 
        if (score > score_max) {
            score_max = score;
            fmt = fmt1;
        }
    }
    return fmt;
}

/************************************************************/
/* input media file */

/**
 * open a media file from an IO stream. 'fmt' must be specified.
 */
int av_open_input_stream(AVFormatContext **ic_ptr, 
                         ByteIOContext *pb, const char *filename, 
                         AVInputFormat *fmt, AVFormatParameters *ap)
{
    int err;
    AVFormatContext *ic;

    ic = av_mallocz(sizeof(AVFormatContext));
    if (!ic) {
        err = AVERROR_NOMEM;
        goto fail;
    }
    ic->iformat = fmt;
    if (pb)
        ic->pb = *pb;
    ic->duration = AV_NOPTS_VALUE;
    ic->start_time = AV_NOPTS_VALUE;
    pstrcpy(ic->filename, sizeof(ic->filename), filename);

    /* allocate private data */
    if (fmt->priv_data_size > 0) {
        ic->priv_data = av_mallocz(fmt->priv_data_size);
        if (!ic->priv_data) {
            err = AVERROR_NOMEM;
            goto fail;
        }
    } else {
        ic->priv_data = NULL;
    }

    /* default pts settings is MPEG like */
    av_set_pts_info(ic, 33, 1, 90000);

    err = ic->iformat->read_header(ic, ap);
    if (err < 0)
        goto fail;
    *ic_ptr = ic;
    return 0;
 fail:
    if (ic) {
        av_freep(&ic->priv_data);
    }
    av_free(ic);
    *ic_ptr = NULL;
    return err;
}

#define PROBE_BUF_SIZE 2048

/**
 * Open a media file as input. The codec are not opened. Only the file
 * header (if present) is read.
 *
 * @param ic_ptr the opened media file handle is put here
 * @param filename filename to open.
 * @param fmt if non NULL, force the file format to use
 * @param buf_size optional buffer size (zero if default is OK)
 * @param ap additionnal parameters needed when opening the file (NULL if default)
 * @return 0 if OK. AVERROR_xxx otherwise.
 */
int av_open_input_file(AVFormatContext **ic_ptr, const char *filename, 
                       AVInputFormat *fmt,
                       int buf_size,
                       AVFormatParameters *ap)
{
    int err, must_open_file, file_opened;
    uint8_t buf[PROBE_BUF_SIZE];
    AVProbeData probe_data, *pd = &probe_data;
    ByteIOContext pb1, *pb = &pb1;
    
    file_opened = 0;
    pd->filename = "";
    if (filename)
        pd->filename = filename;
    pd->buf = buf;
    pd->buf_size = 0;

    if (!fmt) {
        /* guess format if no file can be opened  */
        fmt = av_probe_input_format(pd, 0);
    }

    /* do not open file if the format does not need it. XXX: specific
       hack needed to handle RTSP/TCP */
    must_open_file = 1;
    if (fmt && (fmt->flags & AVFMT_NOFILE)) {
        must_open_file = 0;
    }

    if (!fmt || must_open_file) {
        /* if no file needed do not try to open one */
        if (url_fopen(pb, filename, URL_RDONLY) < 0) {
            err = AVERROR_IO;
            goto fail;
        }
        file_opened = 1;
        if (buf_size > 0) {
            url_setbufsize(pb, buf_size);
        }
        if (!fmt) {
            /* read probe data */
            pd->buf_size = get_buffer(pb, buf, PROBE_BUF_SIZE);
            url_fseek(pb, 0, SEEK_SET);
        }
    }
    
    /* guess file format */
    if (!fmt) {
        fmt = av_probe_input_format(pd, 1);
    }

    /* if still no format found, error */
    if (!fmt) {
        err = AVERROR_NOFMT;
        goto fail;
    }
        
    /* XXX: suppress this hack for redirectors */
#ifdef CONFIG_NETWORK
    if (fmt == &redir_demux) {
        err = redir_open(ic_ptr, pb);
        url_fclose(pb);
        return err;
    }
#endif

    /* check filename in case of an image number is expected */
    if (fmt->flags & AVFMT_NEEDNUMBER) {
        if (filename_number_test(filename) < 0) { 
            err = AVERROR_NUMEXPECTED;
            goto fail;
        }
    }
    err = av_open_input_stream(ic_ptr, pb, filename, fmt, ap);
    if (err)
        goto fail;
    return 0;
 fail:
    if (file_opened)
        url_fclose(pb);
    *ic_ptr = NULL;
    return err;
    
}

/*******************************************************/

/**
 * Read a packet from a media file. Use it only for low level file
 * reading. It is almost always better to use av_read_frame().
 * 
 * @param s media file handle
 * @param pkt is filled 
 * @return 0 if OK. AVERROR_xxx if error.  
 */
int av_read_packet(AVFormatContext *s, AVPacket *pkt)
{
    AVPacketList *pktl;

    pktl = s->packet_buffer;
    if (pktl) {
        /* read packet from packet buffer, if there is data */
        *pkt = pktl->pkt;
        s->packet_buffer = pktl->next;
        av_free(pktl);
        return 0;
    } else {
        return s->iformat->read_packet(s, pkt);
    }
}

/*******************************************************/

/* return TRUE if the stream has accurate timings for at least one component */
static int av_has_timings(AVFormatContext *ic)
{
    int i;
    AVStream *st;

    for(i = 0;i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time != AV_NOPTS_VALUE &&
            st->duration != AV_NOPTS_VALUE)
            return 1;
    }
    return 0;
}

/* estimate the stream timings from the one of each components. Also
   compute the global bitrate if possible */
static void av_update_stream_timings(AVFormatContext *ic)
{
    int64_t start_time, end_time, end_time1;
    int i;
    AVStream *st;

    start_time = MAXINT64;
    end_time = MININT64;
    for(i = 0;i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time != AV_NOPTS_VALUE) {
            if (st->start_time < start_time)
                start_time = st->start_time;
            if (st->duration != AV_NOPTS_VALUE) {
                end_time1 = st->start_time + st->duration;
                if (end_time1 > end_time)
                    end_time = end_time1;
            }
        }
    }
    if (start_time != MAXINT64) {
        ic->start_time = start_time;
        if (end_time != MAXINT64) {
            ic->duration = end_time - start_time;
            if (ic->file_size > 0) {
                /* compute the bit rate */
                ic->bit_rate = (double)ic->file_size * 8.0 * AV_TIME_BASE / 
                    (double)ic->duration;
            }
        }
    }

}

static void fill_all_stream_timings(AVFormatContext *ic)
{
    int i;
    AVStream *st;

    av_update_stream_timings(ic);
    for(i = 0;i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time == AV_NOPTS_VALUE) {
            st->start_time = ic->start_time;
            st->duration = ic->duration;
        }
    }
}

static void av_estimate_timings_from_bit_rate(AVFormatContext *ic)
{
    int64_t filesize, duration;
    int bit_rate, i;
    AVStream *st;

    /* if bit_rate is already set, we believe it */
    if (ic->bit_rate == 0) {
        bit_rate = 0;
        for(i=0;i<ic->nb_streams;i++) {
            st = ic->streams[i];
            bit_rate += st->codec.bit_rate;
        }
        ic->bit_rate = bit_rate;
    }

    /* if duration is already set, we believe it */
    if (ic->duration == AV_NOPTS_VALUE && 
        ic->bit_rate != 0 && 
        ic->file_size != 0)  {
        filesize = ic->file_size;
        if (filesize > 0) {
            duration = (int64_t)((8 * AV_TIME_BASE * (double)filesize) / (double)ic->bit_rate);
            for(i = 0; i < ic->nb_streams; i++) {
                st = ic->streams[i];
                if (st->start_time == AV_NOPTS_VALUE ||
                    st->duration == AV_NOPTS_VALUE) {
                    st->start_time = 0;
                    st->duration = duration;
                }
            }
        }
    }
}

static void flush_packet_queue(AVFormatContext *s)
{
    AVPacketList *pktl;

    for(;;) {
        pktl = s->packet_buffer;
        if (!pktl) 
            break;
        s->packet_buffer = pktl->next;
        av_free_packet(&pktl->pkt);
        av_free(pktl);
    }
}

#define DURATION_MAX_READ_SIZE 250000

/* only usable for MPEG-PS streams */
static void av_estimate_timings_from_pts(AVFormatContext *ic)
{
    AVPacket pkt1, *pkt = &pkt1;
    AVStream *st;
    int read_size, i, ret;
    int64_t start_time, end_time, end_time1;
    int64_t filesize, offset, duration;
    
    /* we read the first packets to get the first PTS (not fully
       accurate, but it is enough now) */
    url_fseek(&ic->pb, 0, SEEK_SET);
    read_size = 0;
    for(;;) {
        if (read_size >= DURATION_MAX_READ_SIZE)
            break;
        /* if all info is available, we can stop */
        for(i = 0;i < ic->nb_streams; i++) {
            st = ic->streams[i];
            if (st->start_time == AV_NOPTS_VALUE)
                break;
        }
        if (i == ic->nb_streams)
            break;

        ret = av_read_packet(ic, pkt);
        if (ret != 0)
            break;
        read_size += pkt->size;
        st = ic->streams[pkt->stream_index];
        if (pkt->pts != AV_NOPTS_VALUE) {
            if (st->start_time == AV_NOPTS_VALUE)
                st->start_time = (int64_t)((double)pkt->pts * ic->pts_num * (double)AV_TIME_BASE / ic->pts_den);
        }
        av_free_packet(pkt);
    }

    /* we compute the minimum start_time and use it as default */
    start_time = MAXINT64;
    for(i = 0; i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->start_time != AV_NOPTS_VALUE &&
            st->start_time < start_time)
            start_time = st->start_time;
    }
    if (start_time != MAXINT64)
        ic->start_time = start_time;
    
    /* estimate the end time (duration) */
    /* XXX: may need to support wrapping */
    filesize = ic->file_size;
    offset = filesize - DURATION_MAX_READ_SIZE;
    if (offset < 0)
        offset = 0;

    /* flush packet queue */
    flush_packet_queue(ic);

    url_fseek(&ic->pb, offset, SEEK_SET);
    read_size = 0;
    for(;;) {
        if (read_size >= DURATION_MAX_READ_SIZE)
            break;
        /* if all info is available, we can stop */
        for(i = 0;i < ic->nb_streams; i++) {
            st = ic->streams[i];
            if (st->duration == AV_NOPTS_VALUE)
                break;
        }
        if (i == ic->nb_streams)
            break;
        
        ret = av_read_packet(ic, pkt);
        if (ret != 0)
            break;
        read_size += pkt->size;
        st = ic->streams[pkt->stream_index];
        if (pkt->pts != AV_NOPTS_VALUE) {
            end_time = (int64_t)((double)pkt->pts * ic->pts_num * (double)AV_TIME_BASE / ic->pts_den);
            duration = end_time - st->start_time;
            if (duration > 0) {
                if (st->duration == AV_NOPTS_VALUE ||
                    st->duration < duration)
                    st->duration = duration;
            }
        }
        av_free_packet(pkt);
    }
    
    /* estimate total duration */
    end_time = MININT64;
    for(i = 0;i < ic->nb_streams; i++) {
        st = ic->streams[i];
        if (st->duration != AV_NOPTS_VALUE) {
            end_time1 = st->start_time + st->duration;
            if (end_time1 > end_time)
                end_time = end_time1;
        }
    }
    
    /* update start_time (new stream may have been created, so we do
       it at the end */
    if (ic->start_time != AV_NOPTS_VALUE) {
        for(i = 0; i < ic->nb_streams; i++) {
            st = ic->streams[i];
            if (st->start_time == AV_NOPTS_VALUE)
                st->start_time = ic->start_time;
        }
    }

    if (end_time != MININT64) {
        /* put dummy values for duration if needed */
        for(i = 0;i < ic->nb_streams; i++) {
            st = ic->streams[i];
            if (st->duration == AV_NOPTS_VALUE && 
                st->start_time != AV_NOPTS_VALUE)
                st->duration = end_time - st->start_time;
        }
        ic->duration = end_time - ic->start_time;
    }

    url_fseek(&ic->pb, 0, SEEK_SET);
}

static void av_estimate_timings(AVFormatContext *ic)
{
    URLContext *h;
    int64_t file_size;

    /* get the file size, if possible */
    if (ic->iformat->flags & AVFMT_NOFILE) {
        file_size = 0;
    } else {
        h = url_fileno(&ic->pb);
        file_size = url_filesize(h);
        if (file_size < 0)
            file_size = 0;
    }
    ic->file_size = file_size;

    if (ic->iformat == &mpegps_demux) {
        /* get accurate estimate from the PTSes */
        av_estimate_timings_from_pts(ic);
    } else if (av_has_timings(ic)) {
        /* at least one components has timings - we use them for all
           the components */
        fill_all_stream_timings(ic);
    } else {
        /* less precise: use bit rate info */
        av_estimate_timings_from_bit_rate(ic);
    }
    av_update_stream_timings(ic);

#if 0
    {
        int i;
        AVStream *st;
        for(i = 0;i < ic->nb_streams; i++) {
            st = ic->streams[i];
        printf("%d: start_time: %0.3f duration: %0.3f\n", 
               i, (double)st->start_time / AV_TIME_BASE, 
               (double)st->duration / AV_TIME_BASE);
        }
        printf("stream: start_time: %0.3f duration: %0.3f bitrate=%d kb/s\n", 
               (double)ic->start_time / AV_TIME_BASE, 
               (double)ic->duration / AV_TIME_BASE,
               ic->bit_rate / 1000);
    }
#endif
}

/* state for codec information */
#define CSTATE_NOTFOUND    0
#define CSTATE_DECODING    1
#define CSTATE_FOUND       2

static int has_codec_parameters(AVCodecContext *enc)
{
    int val;
    switch(enc->codec_type) {
    case CODEC_TYPE_AUDIO:
        val = enc->sample_rate;
        break;
    case CODEC_TYPE_VIDEO:
        val = enc->width;
        break;
    default:
        val = 1;
        break;
    }
    return (val != 0);
}

/**
 * Read the beginning of a media file to get stream information. This
 * is useful for file formats with no headers such as MPEG. This
 * function also compute the real frame rate in case of mpeg2 repeat
 * frame mode.
 *
 * @param ic media file handle
 * @return >=0 if OK. AVERROR_xxx if error.  
 */
int av_find_stream_info(AVFormatContext *ic)
{
    int i, count, ret, got_picture, size, read_size;
    AVCodec *codec;
    AVStream *st;
    AVPacket *pkt;
    AVFrame picture;
    AVPacketList *pktl=NULL, **ppktl;
    short samples[AVCODEC_MAX_AUDIO_FRAME_SIZE / 2];
    uint8_t *ptr;
    int min_read_size, max_read_size;

    /* typical mpeg ts rate is 40 Mbits. DVD rate is about 10
       Mbits. We read at most 0.2 second of file to find all streams */

    /* XXX: base it on stream bitrate when possible */
    if (ic->iformat == &mpegts_demux) {
        /* maximum number of bytes we accept to read to find all the streams
           in a file */
        min_read_size = 6000000;
    } else {
        min_read_size = 250000;
    }
    /* max read size is 2 seconds of video max */
    max_read_size = min_read_size * 10;

    /* set initial codec state */
    for(i=0;i<ic->nb_streams;i++) {
        st = ic->streams[i];
        if (has_codec_parameters(&st->codec))
            st->codec_info_state = CSTATE_FOUND;
        else
            st->codec_info_state = CSTATE_NOTFOUND;
        st->codec_info_nb_repeat_frames = 0;
        st->codec_info_nb_real_frames = 0;
    }

    count = 0;
    read_size = 0;
    ppktl = &ic->packet_buffer;
    for(;;) {
        /* check if one codec still needs to be handled */
        for(i=0;i<ic->nb_streams;i++) {
            st = ic->streams[i];
            if (st->codec_info_state != CSTATE_FOUND)
                break;
        }
        if (i == ic->nb_streams) {
            /* NOTE: if the format has no header, then we need to read
               some packets to get most of the streams, so we cannot
               stop here */
            if (!(ic->ctx_flags & AVFMTCTX_NOHEADER) ||
                read_size >= min_read_size) {
                /* if we found the info for all the codecs, we can stop */
                ret = count;
                break;
            }
        } else {
            /* we did not get all the codec info, but we read too much data */
            if (read_size >= max_read_size) {
                ret = count;
                break;
            }
        }

        pktl = av_mallocz(sizeof(AVPacketList));
        if (!pktl) {
            ret = AVERROR_NOMEM;
            break;
        }

        /* add the packet in the buffered packet list */
        *ppktl = pktl;
        ppktl = &pktl->next;

        /* NOTE: a new stream can be added there if no header in file
           (AVFMTCTX_NOHEADER) */
        pkt = &pktl->pkt;
        if (ic->iformat->read_packet(ic, pkt) < 0) {
            /* EOF or error */
            ret = -1; /* we could not have all the codec parameters before EOF */
            if ((ic->ctx_flags & AVFMTCTX_NOHEADER) &&
                i == ic->nb_streams)
                ret = 0;
            break;
        }
        read_size += pkt->size;

        /* open new codecs */
        for(i=0;i<ic->nb_streams;i++) {
            st = ic->streams[i];
            if (st->codec_info_state == CSTATE_NOTFOUND) {
                /* set to found in case of error */
                st->codec_info_state = CSTATE_FOUND; 
                codec = avcodec_find_decoder(st->codec.codec_id);
                if (codec) {
                    if(codec->capabilities & CODEC_CAP_TRUNCATED)
                        st->codec.flags |= CODEC_FLAG_TRUNCATED;

                    ret = avcodec_open(&st->codec, codec);
                    if (ret >= 0)
                        st->codec_info_state = CSTATE_DECODING;
                }
            }
        }

        st = ic->streams[pkt->stream_index];
        if (st->codec_info_state == CSTATE_DECODING) {
            /* decode the data and update codec parameters */
            ptr = pkt->data;
            size = pkt->size;
            while (size > 0) {
                switch(st->codec.codec_type) {
                case CODEC_TYPE_VIDEO:
                    ret = avcodec_decode_video(&st->codec, &picture, 
                                               &got_picture, ptr, size);
                    break;
                case CODEC_TYPE_AUDIO:
                    ret = avcodec_decode_audio(&st->codec, samples, 
                                               &got_picture, ptr, size);
                    break;
                default:
                    ret = -1;
                    break;
                }
                if (ret < 0) {
                    /* if error, simply ignore because another packet
                       may be OK */
                    break;
                }
                if (got_picture) {
                    /* we got the parameters - now we can stop
                       examining this stream */
                    /* XXX: add a codec info so that we can decide if
                       the codec can repeat frames */
                    if (st->codec.codec_id == CODEC_ID_MPEG1VIDEO && 
                        ic->iformat != &mpegts_demux &&
                        st->codec.sub_id == 2) {
                        /* for mpeg2 video, we want to know the real
                           frame rate, so we decode 40 frames. In mpeg
                           TS case we do not do it because it would be
                           too long */
                        st->codec_info_nb_real_frames++;
                        st->codec_info_nb_repeat_frames += st->codec.coded_frame->repeat_pict;
#if 0
                        /* XXX: testing */
                        if ((st->codec_info_nb_real_frames % 24) == 23) {
                            st->codec_info_nb_repeat_frames += 2;
                        }
#endif
                        /* stop after 40 frames */
                        if (st->codec_info_nb_real_frames >= 40) {
                            av_reduce(
                                &st->r_frame_rate,
                                &st->r_frame_rate_base,
                                (int64_t)st->codec.frame_rate * st->codec_info_nb_real_frames,
                                (st->codec_info_nb_real_frames + (st->codec_info_nb_repeat_frames >> 1)) * st->codec.frame_rate_base,
                                1<<30);
                            goto close_codec;
                        }
                    } else {
                    close_codec:
                        st->codec_info_state = CSTATE_FOUND;
                        avcodec_close(&st->codec);
                        break;
                    }
                }
                ptr += ret;
                size -= ret;
            }
        }
        count++;
    }

    /* close each codec if there are opened */
    for(i=0;i<ic->nb_streams;i++) {
        st = ic->streams[i];
        if (st->codec_info_state == CSTATE_DECODING)
            avcodec_close(&st->codec);
    }

    /* set real frame rate info */
    for(i=0;i<ic->nb_streams;i++) {
        st = ic->streams[i];
        if (st->codec.codec_type == CODEC_TYPE_VIDEO) {
            if (!st->r_frame_rate){
                st->r_frame_rate      = st->codec.frame_rate;
                st->r_frame_rate_base = st->codec.frame_rate_base;
            }
        }
    }


    av_estimate_timings(ic);
    return ret;
}

/**
 * Close a media file (but not its codecs)
 *
 * @param s media file handle
 */
void av_close_input_file(AVFormatContext *s)
{
    int i, must_open_file;
    AVStream *st;

    if (s->iformat->read_close)
        s->iformat->read_close(s);
    for(i=0;i<s->nb_streams;i++) {
        /* free all data in a stream component */
        st = s->streams[i];
        av_free(st);
    }
    if (s->packet_buffer) {
        AVPacketList *p, *p1;
        p = s->packet_buffer;
        while (p != NULL) {
            p1 = p->next;
            av_free_packet(&p->pkt);
            av_free(p);
            p = p1;
        }
        s->packet_buffer = NULL;
    }
    must_open_file = 1;
    if (s->iformat->flags & AVFMT_NOFILE) {
        must_open_file = 0;
    }
    if (must_open_file) {
        url_fclose(&s->pb);
    }
    av_freep(&s->priv_data);
    av_free(s);
}

/**
 * Add a new stream to a media file. Can only be called in the
 * read_header function. If the flag AVFMTCTX_NOHEADER is in the
 * format context, then new streams can be added in read_packet too.
 *
 *
 * @param s media file handle
 * @param id file format dependent stream id 
 */
AVStream *av_new_stream(AVFormatContext *s, int id)
{
    AVStream *st;

    if (s->nb_streams >= MAX_STREAMS)
        return NULL;

    st = av_mallocz(sizeof(AVStream));
    if (!st)
        return NULL;
    avcodec_get_context_defaults(&st->codec);
    if (s->iformat) {
        /* no default bitrate if decoding */
        st->codec.bit_rate = 0;
    }
    st->index = s->nb_streams;
    st->id = id;
    st->start_time = AV_NOPTS_VALUE;
    st->duration = AV_NOPTS_VALUE;
    s->streams[s->nb_streams++] = st;
    return st;
}

/************************************************************/
/* output media file */

int av_set_parameters(AVFormatContext *s, AVFormatParameters *ap)
{
    int ret;
    
    if (s->oformat->priv_data_size > 0) {
        s->priv_data = av_mallocz(s->oformat->priv_data_size);
        if (!s->priv_data)
            return AVERROR_NOMEM;
    } else
        s->priv_data = NULL;
	
    if (s->oformat->set_parameters) {
        ret = s->oformat->set_parameters(s, ap);
        if (ret < 0)
            return ret;
    }
    return 0;
}

/**
 * allocate the stream private data and write the stream header to an
 * output media file
 *
 * @param s media file handle
 * @return 0 if OK. AVERROR_xxx if error.  
 */
int av_write_header(AVFormatContext *s)
{
    int ret, i;
    AVStream *st;

    /* default pts settings is MPEG like */
    av_set_pts_info(s, 33, 1, 90000);
    ret = s->oformat->write_header(s);
    if (ret < 0)
        return ret;

    /* init PTS generation */
    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];

        switch (st->codec.codec_type) {
        case CODEC_TYPE_AUDIO:
            av_frac_init(&st->pts, 0, 0, 
                         (int64_t)s->pts_num * st->codec.sample_rate);
            break;
        case CODEC_TYPE_VIDEO:
            av_frac_init(&st->pts, 0, 0, 
                         (int64_t)s->pts_num * st->codec.frame_rate);
            break;
        default:
            break;
        }
    }
    return 0;
}

/**
 * Write a packet to an output media file. The packet shall contain
 * one audio or video frame.
 *
 * @param s media file handle
 * @param stream_index stream index
 * @param buf buffer containing the frame data
 * @param size size of buffer
 * @return < 0 if error, = 0 if OK, 1 if end of stream wanted.
 */
int av_write_frame(AVFormatContext *s, int stream_index, const uint8_t *buf, 
                   int size)
{
    AVStream *st;
    int64_t pts_mask;
    int ret, frame_size;

    st = s->streams[stream_index];
    pts_mask = (1LL << s->pts_wrap_bits) - 1;
    ret = s->oformat->write_packet(s, stream_index, (uint8_t *)buf, size, 
                                   st->pts.val & pts_mask);
    if (ret < 0)
        return ret;

    /* update pts */
    switch (st->codec.codec_type) {
    case CODEC_TYPE_AUDIO:
        if (st->codec.frame_size <= 1) {
            frame_size = size / st->codec.channels;
            /* specific hack for pcm codecs because no frame size is provided */
            switch(st->codec.codec_id) {
            case CODEC_ID_PCM_S16LE:
            case CODEC_ID_PCM_S16BE:
            case CODEC_ID_PCM_U16LE:
            case CODEC_ID_PCM_U16BE:
                frame_size >>= 1;
                break;
            default:
                break;
            }
        } else {
            frame_size = st->codec.frame_size;
        }
        av_frac_add(&st->pts, 
                    (int64_t)s->pts_den * frame_size);
        break;
    case CODEC_TYPE_VIDEO:
        av_frac_add(&st->pts, 
                    (int64_t)s->pts_den * st->codec.frame_rate_base);
        break;
    default:
        break;
    }
    return ret;
}

/**
 * write the stream trailer to an output media file and and free the
 * file private data.
 *
 * @param s media file handle
 * @return 0 if OK. AVERROR_xxx if error.  */
int av_write_trailer(AVFormatContext *s)
{
    int ret;
    ret = s->oformat->write_trailer(s);
    av_freep(&s->priv_data);
    return ret;
}

/* "user interface" functions */

void dump_format(AVFormatContext *ic,
                 int index, 
                 const char *url,
                 int is_output)
{
    int i, flags;
    char buf[256];

    fprintf(stderr, "%s #%d, %s, %s '%s':\n", 
            is_output ? "Output" : "Input",
            index, 
            is_output ? ic->oformat->name : ic->iformat->name, 
            is_output ? "to" : "from", url);
    if (!is_output) {
        fprintf(stderr, "  Duration: ");
        if (ic->duration != AV_NOPTS_VALUE) {
            int hours, mins, secs, us;
            secs = ic->duration / AV_TIME_BASE;
            us = ic->duration % AV_TIME_BASE;
            mins = secs / 60;
            secs %= 60;
            hours = mins / 60;
            mins %= 60;
            fprintf(stderr, "%02d:%02d:%02d.%01d", hours, mins, secs, 
                   (10 * us) / AV_TIME_BASE);
        } else {
            fprintf(stderr, "N/A");
        }
        fprintf(stderr, ", bitrate: ");
        if (ic->bit_rate) {
            fprintf(stderr,"%d kb/s", ic->bit_rate / 1000);
        } else {
            fprintf(stderr, "N/A");
        }
        fprintf(stderr, "\n");
    }
    for(i=0;i<ic->nb_streams;i++) {
        AVStream *st = ic->streams[i];
        avcodec_string(buf, sizeof(buf), &st->codec, is_output);
        fprintf(stderr, "  Stream #%d.%d", index, i);
        /* the pid is an important information, so we display it */
        /* XXX: add a generic system */
        if (is_output)
            flags = ic->oformat->flags;
        else
            flags = ic->iformat->flags;
        if (flags & AVFMT_SHOW_IDS) {
            fprintf(stderr, "[0x%x]", st->id);
        }
        fprintf(stderr, ": %s\n", buf);
    }
}

typedef struct {
    const char *abv;
    int width, height;
    int frame_rate, frame_rate_base;
} AbvEntry;

static AbvEntry frame_abvs[] = {
    { "ntsc",      720, 480, 30000, 1001 },
    { "pal",       720, 576,    25,    1 },
    { "qntsc",     352, 240, 30000, 1001 }, /* VCD compliant ntsc */
    { "qpal",      352, 288,    25,    1 }, /* VCD compliant pal */
    { "sntsc",     640, 480, 30000, 1001 }, /* square pixel ntsc */
    { "spal",      768, 576,    25,    1 }, /* square pixel pal */
    { "film",      352, 240,    24,    1 },
    { "ntsc-film", 352, 240, 24000, 1001 },
    { "sqcif",     128,  96,     0,    0 },
    { "qcif",      176, 144,     0,    0 },
    { "cif",       352, 288,     0,    0 },
    { "4cif",      704, 576,     0,    0 },
};

int parse_image_size(int *width_ptr, int *height_ptr, const char *str)
{
    int i;
    int n = sizeof(frame_abvs) / sizeof(AbvEntry);
    const char *p;
    int frame_width = 0, frame_height = 0;

    for(i=0;i<n;i++) {
        if (!strcmp(frame_abvs[i].abv, str)) {
            frame_width = frame_abvs[i].width;
            frame_height = frame_abvs[i].height;
            break;
        }
    }
    if (i == n) {
        p = str;
        frame_width = strtol(p, (char **)&p, 10);
        if (*p)
            p++;
        frame_height = strtol(p, (char **)&p, 10);
    }
    if (frame_width <= 0 || frame_height <= 0)
        return -1;
    *width_ptr = frame_width;
    *height_ptr = frame_height;
    return 0;
}

int parse_frame_rate(int *frame_rate, int *frame_rate_base, const char *arg)
{
    int i;
    char* cp;
   
    /* First, we check our abbreviation table */
    for (i = 0; i < sizeof(frame_abvs)/sizeof(*frame_abvs); ++i)
         if (!strcmp(frame_abvs[i].abv, arg)) {
	     *frame_rate = frame_abvs[i].frame_rate;
	     *frame_rate_base = frame_abvs[i].frame_rate_base;
	     return 0;
	 }

    /* Then, we try to parse it as fraction */
    cp = strchr(arg, '/');
    if (cp) {
        char* cpp;
	*frame_rate = strtol(arg, &cpp, 10);
	if (cpp != arg || cpp == cp) 
	    *frame_rate_base = strtol(cp+1, &cpp, 10);
	else
	   *frame_rate = 0;
    } 
    else {
        /* Finally we give up and parse it as double */
        *frame_rate_base = DEFAULT_FRAME_RATE_BASE;
        *frame_rate = (int)(strtod(arg, 0) * (*frame_rate_base) + 0.5);
    }
    if (!*frame_rate || !*frame_rate_base)
        return -1;
    else
        return 0;
}

/* Syntax:
 * - If not a duration:
 *  [{YYYY-MM-DD|YYYYMMDD}]{T| }{HH[:MM[:SS[.m...]]][Z]|HH[MM[SS[.m...]]][Z]}
 * Time is localtime unless Z is suffixed to the end. In this case GMT
 * Return the date in micro seconds since 1970 
 * - If duration:
 *  HH[:MM[:SS[.m...]]]
 *  S+[.m...]
 */
int64_t parse_date(const char *datestr, int duration)
{
    const char *p;
    int64_t t;
    struct tm dt;
    int i;
    static const char *date_fmt[] = {
        "%Y-%m-%d",
        "%Y%m%d",
    };
    static const char *time_fmt[] = {
        "%H:%M:%S",
        "%H%M%S",
    };
    const char *q;
    int is_utc, len;
    char lastch;
    time_t now = time(0);

    len = strlen(datestr);
    if (len > 0)
        lastch = datestr[len - 1];
    else
        lastch = '\0';
    is_utc = (lastch == 'z' || lastch == 'Z');

    memset(&dt, 0, sizeof(dt));

    p = datestr;
    q = NULL;
    if (!duration) {
        for (i = 0; i < sizeof(date_fmt) / sizeof(date_fmt[0]); i++) {
            q = small_strptime(p, date_fmt[i], &dt);
            if (q) {
                break;
            }
        }

        if (!q) {
            if (is_utc) {
                dt = *gmtime(&now);
            } else {
                dt = *localtime(&now);
            }
            dt.tm_hour = dt.tm_min = dt.tm_sec = 0;
        } else {
            p = q;
        }

        if (*p == 'T' || *p == 't' || *p == ' ')
            p++;

        for (i = 0; i < sizeof(time_fmt) / sizeof(time_fmt[0]); i++) {
            q = small_strptime(p, time_fmt[i], &dt);
            if (q) {
                break;
            }
        }
    } else {
        q = small_strptime(p, time_fmt[0], &dt);
        if (!q) {
            dt.tm_sec = strtol(p, (char **)&q, 10);
            dt.tm_min = 0;
            dt.tm_hour = 0;
        }
    }

    /* Now we have all the fields that we can get */
    if (!q) {
        if (duration)
            return 0;
        else
            return now * int64_t_C(1000000);
    }

    if (duration) {
        t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
    } else {
        dt.tm_isdst = -1;       /* unknown */
        if (is_utc) {
            t = mktimegm(&dt);
        } else {
            t = mktime(&dt);
        }
    }

    t *= 1000000;

    if (*q == '.') {
        int val, n;
        q++;
        for (val = 0, n = 100000; n >= 1; n /= 10, q++) {
            if (!isdigit(*q)) 
                break;
            val += n * (*q - '0');
        }
        t += val;
    }
    return t;
}

/* syntax: '?tag1=val1&tag2=val2...'. Little URL decoding is done. Return
   1 if found */
int find_info_tag(char *arg, int arg_size, const char *tag1, const char *info)
{
    const char *p;
    char tag[128], *q;

    p = info;
    if (*p == '?')
        p++;
    for(;;) {
        q = tag;
        while (*p != '\0' && *p != '=' && *p != '&') {
            if ((q - tag) < sizeof(tag) - 1)
                *q++ = *p;
            p++;
        }
        *q = '\0';
        q = arg;
        if (*p == '=') {
            p++;
            while (*p != '&' && *p != '\0') {
                if ((q - arg) < arg_size - 1) {
                    if (*p == '+')
                        *q++ = ' ';
                    else
                        *q++ = *p;
                }
                p++;
            }
            *q = '\0';
        }
        if (!strcmp(tag, tag1)) 
            return 1;
        if (*p != '&')
            break;
        p++;
    }
    return 0;
}

/* Return in 'buf' the path with '%d' replaced by number. Also handles
   the '%0nd' format where 'n' is the total number of digits and
   '%%'. Return 0 if OK, and -1 if format error */
int get_frame_filename(char *buf, int buf_size,
                       const char *path, int number)
{
    const char *p;
    char *q, buf1[20], c;
    int nd, len, percentd_found;

    q = buf;
    p = path;
    percentd_found = 0;
    for(;;) {
        c = *p++;
        if (c == '\0')
            break;
        if (c == '%') {
            do {
                nd = 0;
                while (isdigit(*p)) {
                    nd = nd * 10 + *p++ - '0';
                }
                c = *p++;
            } while (isdigit(c));

            switch(c) {
            case '%':
                goto addchar;
            case 'd':
                if (percentd_found)
                    goto fail;
                percentd_found = 1;
                snprintf(buf1, sizeof(buf1), "%0*d", nd, number);
                len = strlen(buf1);
                if ((q - buf + len) > buf_size - 1)
                    goto fail;
                memcpy(q, buf1, len);
                q += len;
                break;
            default:
                goto fail;
            }
        } else {
        addchar:
            if ((q - buf) < buf_size - 1)
                *q++ = c;
        }
    }
    if (!percentd_found)
        goto fail;
    *q = '\0';
    return 0;
 fail:
    *q = '\0';
    return -1;
}

/**
 *
 * Print on stdout a nice hexa dump of a buffer
 * @param buf buffer
 * @param size buffer size
 */
void av_hex_dump(uint8_t *buf, int size)
{
    int len, i, j, c;

    for(i=0;i<size;i+=16) {
        len = size - i;
        if (len > 16)
            len = 16;
        printf("%08x ", i);
        for(j=0;j<16;j++) {
            if (j < len)
                printf(" %02x", buf[i+j]);
            else
                printf("   ");
        }
        printf(" ");
        for(j=0;j<len;j++) {
            c = buf[i+j];
            if (c < ' ' || c > '~')
                c = '.';
            printf("%c", c);
        }
        printf("\n");
    }
}

void url_split(char *proto, int proto_size,
               char *hostname, int hostname_size,
               int *port_ptr,
               char *path, int path_size,
               const char *url)
{
    const char *p;
    char *q;
    int port;

    port = -1;

    p = url;
    q = proto;
    while (*p != ':' && *p != '\0') {
        if ((q - proto) < proto_size - 1)
            *q++ = *p;
        p++;
    }
    if (proto_size > 0)
        *q = '\0';
    if (*p == '\0') {
        if (proto_size > 0)
            proto[0] = '\0';
        if (hostname_size > 0)
            hostname[0] = '\0';
        p = url;
    } else {
        p++;
        if (*p == '/')
            p++;
        if (*p == '/')
            p++;
        q = hostname;
        while (*p != ':' && *p != '/' && *p != '?' && *p != '\0') {
            if ((q - hostname) < hostname_size - 1)
                *q++ = *p;
            p++;
        }
        if (hostname_size > 0)
            *q = '\0';
        if (*p == ':') {
            p++;
            port = strtoul(p, (char **)&p, 10);
        }
    }
    if (port_ptr)
        *port_ptr = port;
    pstrcpy(path, path_size, p);
}

/**
 * Set the pts for a given stream
 * @param s stream 
 * @param pts_wrap_bits number of bits effectively used by the pts
 *        (used for wrap control, 33 is the value for MPEG) 
 * @param pts_num numerator to convert to seconds (MPEG: 1) 
 * @param pts_den denominator to convert to seconds (MPEG: 90000)
 */
void av_set_pts_info(AVFormatContext *s, int pts_wrap_bits,
                     int pts_num, int pts_den)
{
    s->pts_wrap_bits = pts_wrap_bits;
    s->pts_num = pts_num;
    s->pts_den = pts_den;
}

/* fraction handling */

/**
 * f = val + (num / den) + 0.5. 'num' is normalized so that it is such
 * as 0 <= num < den.
 *
 * @param f fractional number
 * @param val integer value
 * @param num must be >= 0
 * @param den must be >= 1 
 */
void av_frac_init(AVFrac *f, int64_t val, int64_t num, int64_t den)
{
    num += (den >> 1);
    if (num >= den) {
        val += num / den;
        num = num % den;
    }
    f->val = val;
    f->num = num;
    f->den = den;
}

/* set f to (val + 0.5) */
void av_frac_set(AVFrac *f, int64_t val)
{
    f->val = val;
    f->num = f->den >> 1;
}

/**
 * Fractionnal addition to f: f = f + (incr / f->den)
 *
 * @param f fractional number
 * @param incr increment, can be positive or negative
 */
void av_frac_add(AVFrac *f, int64_t incr)
{
    int64_t num, den;

    num = f->num + incr;
    den = f->den;
    if (num < 0) {
        f->val += num / den;
        num = num % den;
        if (num < 0) {
            num += den;
            f->val--;
        }
    } else if (num >= den) {
        f->val += num / den;
        num = num % den;
    }
    f->num = num;
}

/**
 * register a new image format
 * @param img_fmt Image format descriptor
 */
void av_register_image_format(AVImageFormat *img_fmt)
{
    AVImageFormat **p;

    p = &first_image_format;
    while (*p != NULL) p = &(*p)->next;
    *p = img_fmt;
    img_fmt->next = NULL;
}

/* guess image format */
AVImageFormat *av_probe_image_format(AVProbeData *pd)
{
    AVImageFormat *fmt1, *fmt;
    int score, score_max;

    fmt = NULL;
    score_max = 0;
    for(fmt1 = first_image_format; fmt1 != NULL; fmt1 = fmt1->next) {
        if (fmt1->img_probe) {
            score = fmt1->img_probe(pd);
            if (score > score_max) {
                score_max = score;
                fmt = fmt1;
            }
        }
    }
    return fmt;
}

AVImageFormat *guess_image_format(const char *filename)
{
    AVImageFormat *fmt1;

    for(fmt1 = first_image_format; fmt1 != NULL; fmt1 = fmt1->next) {
        if (fmt1->extensions && match_ext(filename, fmt1->extensions))
            return fmt1;
    }
    return NULL;
}

/**
 * Read an image from a stream. 
 * @param gb byte stream containing the image
 * @param fmt image format, NULL if probing is required
 */
int av_read_image(ByteIOContext *pb, const char *filename,
                  AVImageFormat *fmt,
                  int (*alloc_cb)(void *, AVImageInfo *info), void *opaque)
{
    char buf[PROBE_BUF_SIZE];
    AVProbeData probe_data, *pd = &probe_data;
    offset_t pos;
    int ret;

    if (!fmt) {
        pd->filename = filename;
        pd->buf = buf;
        pos = url_ftell(pb);
        pd->buf_size = get_buffer(pb, buf, PROBE_BUF_SIZE);
        url_fseek(pb, pos, SEEK_SET);
        fmt = av_probe_image_format(pd);
    }
    if (!fmt)
        return AVERROR_NOFMT;
    ret = fmt->img_read(pb, alloc_cb, opaque);
    return ret;
}

/**
 * Write an image to a stream.
 * @param pb byte stream for the image output
 * @param fmt image format
 * @param img image data and informations
 */
int av_write_image(ByteIOContext *pb, AVImageFormat *fmt, AVImageInfo *img)
{
    return fmt->img_write(pb, img);
}

