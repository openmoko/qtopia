/*
 * RTSP/SDP client
 * Copyright (c) 2002 Fabrice Bellard.
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

#include <sys/time.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <ctype.h>
#ifndef __BEOS__
# include <arpa/inet.h>
#else
# include "barpainet.h"
#endif

//#define DEBUG

typedef struct RTSPState {
    URLContext *rtsp_hd; /* RTSP TCP connexion handle */
    ByteIOContext rtsp_gb;
    int seq;        /* RTSP command sequence number */
    char session_id[512];
    enum RTSPProtocol protocol;
    char last_reply[2048]; /* XXX: allocate ? */
} RTSPState;

typedef struct RTSPStream {
    AVFormatContext *ic;
    int interleaved_min, interleaved_max;  /* interleave ids, if TCP transport */
    char control_url[1024]; /* url for this stream (from SDP) */

    int sdp_port; /* port (from SDP content - not used in RTSP) */
    struct in_addr sdp_ip; /* IP address  (from SDP content - not used in RTSP) */
    int sdp_ttl;  /* IP TTL (from SDP content - not used in RTSP) */
    int sdp_payload_type; /* payload type - only used in SDP */
} RTSPStream;

/* suppress this hack */
int rtsp_abort_req = 0;

/* XXX: currently, the only way to change the protocols consists in
   changing this variable */
int rtsp_default_protocols = (1 << RTSP_PROTOCOL_RTP_TCP) | (1 << RTSP_PROTOCOL_RTP_UDP) | (1 << RTSP_PROTOCOL_RTP_UDP_MULTICAST);

/* if non zero, then set a range for RTP ports */
int rtsp_rtp_port_min = 0;
int rtsp_rtp_port_max = 0;

FFRTSPCallback *ff_rtsp_callback = NULL;

static int rtsp_probe(AVProbeData *p)
{
    if (strstart(p->filename, "rtsp:", NULL))
        return AVPROBE_SCORE_MAX;
    return 0;
}

static int redir_isspace(int c)
{
    return (c == ' ' || c == '\t' || c == '\n' || c == '\r');
}

static void skip_spaces(const char **pp)
{
    const char *p;
    p = *pp;
    while (redir_isspace(*p))
        p++;
    *pp = p;
}

static void get_word_sep(char *buf, int buf_size, const char *sep, 
                         const char **pp)
{
    const char *p;
    char *q;

    p = *pp;
    skip_spaces(&p);
    q = buf;
    while (!strchr(sep, *p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
            *q++ = *p;
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}

static void get_word(char *buf, int buf_size, const char **pp)
{
    const char *p;
    char *q;

    p = *pp;
    skip_spaces(&p);
    q = buf;
    while (!redir_isspace(*p) && *p != '\0') {
        if ((q - buf) < buf_size - 1)
            *q++ = *p;
        p++;
    }
    if (buf_size > 0)
        *q = '\0';
    *pp = p;
}

/* parse the rtpmap description: <codec_name>/<clock_rate>[/<other
   params>] */
static int sdp_parse_rtpmap(AVCodecContext *codec, const char *p)
{
    char buf[256];

    /* codec name */
    get_word_sep(buf, sizeof(buf), "/", &p);
    if (!strcmp(buf, "MP4V-ES")) {
        codec->codec_id = CODEC_ID_MPEG4;
        return 0;
    } else {
        return -1;
    }
}

/* return the length and optionnaly the data */
static int hex_to_data(uint8_t *data, const char *p)
{
    int c, len, v;

    len = 0;
    v = 1;
    for(;;) {
        skip_spaces(&p);
        if (p == '\0')
            break;
        c = toupper((unsigned char)*p++);
        if (c >= '0' && c <= '9')
            c = c - '0';
        else if (c >= 'A' && c <= 'F')
            c = c - 'A' + 10;
        else
            break;
        v = (v << 4) | c;
        if (v & 0x100) {
            if (data)
                data[len] = v;
            len++;
            v = 1;
        }
    }
    return len;
}

static void sdp_parse_fmtp(AVCodecContext *codec, const char *p)
{
    char attr[256];
    char value[4096];
    int len;

    /* loop on each attribute */
    for(;;) {
        skip_spaces(&p);
        if (*p == '\0')
            break;
        get_word_sep(attr, sizeof(attr), "=", &p);
        if (*p == '=') 
            p++;
        get_word_sep(value, sizeof(value), ";", &p);
        if (*p == ';')
            p++;
        /* handle MPEG4 video */
        switch(codec->codec_id) {
        case CODEC_ID_MPEG4:
            if (!strcmp(attr, "config")) {
                /* decode the hexa encoded parameter */
                len = hex_to_data(NULL, value);
                codec->extradata = av_mallocz(len);
                if (!codec->extradata)
                    goto fail;
                codec->extradata_size = len;
                hex_to_data(codec->extradata, value);
            }
            break;
        default:
            /* ignore data for other codecs */
            break;
        }
    fail: ;
        //        printf("'%s' = '%s'\n", attr, value);
    }
}

typedef struct SDPParseState {
    /* SDP only */
    struct in_addr default_ip;
    int default_ttl;
} SDPParseState;

static void sdp_parse_line(AVFormatContext *s, SDPParseState *s1,
                           int letter, const char *buf)
{
    char buf1[64], st_type[64];
    const char *p;
    int codec_type, payload_type, i;
    AVStream *st;
    RTSPStream *rtsp_st;
    struct in_addr sdp_ip;
    int ttl;

#ifdef DEBUG
    printf("sdp: %c='%s'\n", letter, buf);
#endif

    p = buf;
    switch(letter) {
    case 'c':
        get_word(buf1, sizeof(buf1), &p);
        if (strcmp(buf1, "IN") != 0)
            return;
        get_word(buf1, sizeof(buf1), &p);
        if (strcmp(buf1, "IP4") != 0)
            return;
        get_word_sep(buf1, sizeof(buf1), "/", &p);
        if (inet_aton(buf1, &sdp_ip) == 0)
            return;
        ttl = 16;
        if (*p == '/') {
            p++;
            get_word_sep(buf1, sizeof(buf1), "/", &p);
            ttl = atoi(buf1);
        }
        if (s->nb_streams == 0) {
            s1->default_ip = sdp_ip;
            s1->default_ttl = ttl;
        } else {
            st = s->streams[s->nb_streams - 1];
            rtsp_st = st->priv_data;
            rtsp_st->sdp_ip = sdp_ip;
            rtsp_st->sdp_ttl = ttl;
        }
        break;
    case 's':
        pstrcpy(s->title, sizeof(s->title), p);
        break;
    case 'i':
        if (s->nb_streams == 0) {
            pstrcpy(s->comment, sizeof(s->comment), p);
            break;
        }
        break;
    case 'm':
        /* new stream */
        get_word(st_type, sizeof(st_type), &p);
        if (!strcmp(st_type, "audio")) {
            codec_type = CODEC_TYPE_AUDIO;
        } else if (!strcmp(st_type, "video")) {
            codec_type = CODEC_TYPE_VIDEO;
        } else {
            return;
        }
        rtsp_st = av_mallocz(sizeof(RTSPStream));
        if (!rtsp_st)
            return;
        st = av_new_stream(s, s->nb_streams);
        if (!st) 
            return;
        st->priv_data = rtsp_st;

        rtsp_st->sdp_ip = s1->default_ip;
        rtsp_st->sdp_ttl = s1->default_ttl;

        st->codec.codec_type = codec_type;

        get_word(buf1, sizeof(buf1), &p); /* port */
        rtsp_st->sdp_port = atoi(buf1);

        get_word(buf1, sizeof(buf1), &p); /* protocol (ignored) */
        
        /* XXX: handle list of formats */
        get_word(buf1, sizeof(buf1), &p); /* format list */
        rtsp_st->sdp_payload_type = atoi(buf1);
        if (rtsp_st->sdp_payload_type < 96) {
            /* if standard payload type, we can find the codec right now */
            rtp_get_codec_info(&st->codec, rtsp_st->sdp_payload_type);
        }

        /* put a default control url */
        pstrcpy(rtsp_st->control_url, sizeof(rtsp_st->control_url), s->filename);
        break;
    case 'a':
        if (strstart(p, "control:", &p) && s->nb_streams > 0) {
            char proto[32];
            /* get the control url */
            st = s->streams[s->nb_streams - 1];
            rtsp_st = st->priv_data;
            
            /* XXX: may need to add full url resolution */
            url_split(proto, sizeof(proto), NULL, 0, NULL, NULL, 0, p);
            if (proto[0] == '\0') {
                /* relative control URL */
                pstrcat(rtsp_st->control_url, sizeof(rtsp_st->control_url), "/");
                pstrcat(rtsp_st->control_url, sizeof(rtsp_st->control_url), p);
            } else {
                pstrcpy(rtsp_st->control_url, sizeof(rtsp_st->control_url), p);
            }
        } else if (strstart(p, "rtpmap:", &p)) {
            /* NOTE: rtpmap is only supported AFTER the 'm=' tag */
            get_word(buf1, sizeof(buf1), &p); 
            payload_type = atoi(buf1);
            for(i = 0; i < s->nb_streams;i++) {
                st = s->streams[i];
                rtsp_st = st->priv_data;
                if (rtsp_st->sdp_payload_type == payload_type) {
                    sdp_parse_rtpmap(&st->codec, p);
                }
            }
        } else if (strstart(p, "fmtp:", &p)) {
            /* NOTE: fmtp is only supported AFTER the 'a=rtpmap:xxx' tag */
            get_word(buf1, sizeof(buf1), &p); 
            payload_type = atoi(buf1);
            for(i = 0; i < s->nb_streams;i++) {
                st = s->streams[i];
                rtsp_st = st->priv_data;
                if (rtsp_st->sdp_payload_type == payload_type) {
                    sdp_parse_fmtp(&st->codec, p);
                }
            }
        }
        break;
    }
}

int sdp_parse(AVFormatContext *s, const char *content)
{
    const char *p;
    int letter;
    char buf[1024], *q;
    SDPParseState sdp_parse_state, *s1 = &sdp_parse_state;
    
    memset(s1, 0, sizeof(SDPParseState));
    p = content;
    for(;;) {
        skip_spaces(&p);
        letter = *p;
        if (letter == '\0')
            break;
        p++;
        if (*p != '=')
            goto next_line;
        p++;
        /* get the content */
        q = buf;
        while (*p != '\n' && *p != '\0') {
            if ((q - buf) < sizeof(buf) - 1)
                *q++ = *p;
            p++;
        }
        *q = '\0';
        sdp_parse_line(s, s1, letter, buf);
    next_line:
        while (*p != '\n' && *p != '\0')
            p++;
        if (*p == '\n')
            p++;
    }
    return 0;
}

static void rtsp_parse_range(int *min_ptr, int *max_ptr, const char **pp)
{
    const char *p;
    int v;

    p = *pp;
    skip_spaces(&p);
    v = strtol(p, (char **)&p, 10);
    if (*p == '-') {
        p++;
        *min_ptr = v;
        v = strtol(p, (char **)&p, 10);
        *max_ptr = v;
    } else {
        *min_ptr = v;
        *max_ptr = v;
    }
    *pp = p;
}

/* XXX: only one transport specification is parsed */
static void rtsp_parse_transport(RTSPHeader *reply, const char *p)
{
    char transport_protocol[16];
    char profile[16];
    char lower_transport[16];
    char parameter[16];
    RTSPTransportField *th;
    char buf[256];
    
    reply->nb_transports = 0;
    
    for(;;) {
        skip_spaces(&p);
        if (*p == '\0')
            break;

        th = &reply->transports[reply->nb_transports];

        get_word_sep(transport_protocol, sizeof(transport_protocol), 
                     "/", &p);
        if (*p == '/')
            p++;
        get_word_sep(profile, sizeof(profile), "/;,", &p);
        lower_transport[0] = '\0';
        if (*p == '/') {
            get_word_sep(lower_transport, sizeof(lower_transport), 
                         ";,", &p);
        }
        if (!strcmp(lower_transport, "TCP"))
            th->protocol = RTSP_PROTOCOL_RTP_TCP;
        else
            th->protocol = RTSP_PROTOCOL_RTP_UDP;
        
        if (*p == ';')
            p++;
        /* get each parameter */
        while (*p != '\0' && *p != ',') {
            get_word_sep(parameter, sizeof(parameter), "=;,", &p);
            if (!strcmp(parameter, "port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&th->port_min, &th->port_max, &p);
                }
            } else if (!strcmp(parameter, "client_port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&th->client_port_min, 
                                     &th->client_port_max, &p);
                }
            } else if (!strcmp(parameter, "server_port")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&th->server_port_min, 
                                     &th->server_port_max, &p);
                }
            } else if (!strcmp(parameter, "interleaved")) {
                if (*p == '=') {
                    p++;
                    rtsp_parse_range(&th->interleaved_min, 
                                     &th->interleaved_max, &p);
                }
            } else if (!strcmp(parameter, "multicast")) {
                if (th->protocol == RTSP_PROTOCOL_RTP_UDP)
                    th->protocol = RTSP_PROTOCOL_RTP_UDP_MULTICAST;
            } else if (!strcmp(parameter, "ttl")) {
                if (*p == '=') {
                    p++;
                    th->ttl = strtol(p, (char **)&p, 10);
                }
            } else if (!strcmp(parameter, "destination")) {
                struct in_addr ipaddr;

                if (*p == '=') {
                    p++;
                    get_word_sep(buf, sizeof(buf), ";,", &p);
                    if (inet_aton(buf, &ipaddr)) 
                        th->destination = ntohl(ipaddr.s_addr);
                }
            }
            while (*p != ';' && *p != '\0' && *p != ',')
                p++;
            if (*p == ';')
                p++;
        }
        if (*p == ',')
            p++;

        reply->nb_transports++;
    }
}

void rtsp_parse_line(RTSPHeader *reply, const char *buf)
{
    const char *p;

    /* NOTE: we do case independent match for broken servers */
    p = buf;
    if (stristart(p, "Session:", &p)) {
        get_word_sep(reply->session_id, sizeof(reply->session_id), ";", &p);
    } else if (stristart(p, "Content-Length:", &p)) {
        reply->content_length = strtol(p, NULL, 10);
    } else if (stristart(p, "Transport:", &p)) {
        rtsp_parse_transport(reply, p);
    } else if (stristart(p, "CSeq:", &p)) {
        reply->seq = strtol(p, NULL, 10);
    }
}


static void rtsp_send_cmd(AVFormatContext *s, 
                          const char *cmd, RTSPHeader *reply, 
                          unsigned char **content_ptr)
{
    RTSPState *rt = s->priv_data;
    char buf[4096], buf1[1024], *q;
    unsigned char ch;
    const char *p;
    int content_length, line_count;
    unsigned char *content = NULL;

    memset(reply, 0, sizeof(RTSPHeader));

    rt->seq++;
    pstrcpy(buf, sizeof(buf), cmd);
    snprintf(buf1, sizeof(buf1), "CSeq: %d\n", rt->seq);
    pstrcat(buf, sizeof(buf), buf1);
    if (rt->session_id[0] != '\0' && !strstr(cmd, "\nIf-Match:")) {
        snprintf(buf1, sizeof(buf1), "Session: %s\n", rt->session_id);
        pstrcat(buf, sizeof(buf), buf1);
    }
    pstrcat(buf, sizeof(buf), "\n");
#ifdef DEBUG
    printf("Sending:\n%s--\n", buf);
#endif
    url_write(rt->rtsp_hd, buf, strlen(buf));

    /* parse reply (XXX: use buffers) */
    line_count = 0;
    rt->last_reply[0] = '\0';
    for(;;) {
        q = buf;
        for(;;) {
            if (url_read(rt->rtsp_hd, &ch, 1) == 0)
                break;
            if (ch == '\n')
                break;
            if (ch != '\r') {
                if ((q - buf) < sizeof(buf) - 1)
                    *q++ = ch;
            }
        }
        *q = '\0';
#ifdef DEBUG
        printf("line='%s'\n", buf);
#endif
        /* test if last line */
        if (buf[0] == '\0')
            break;
        p = buf;
        if (line_count == 0) {
            /* get reply code */
            get_word(buf1, sizeof(buf1), &p);
            get_word(buf1, sizeof(buf1), &p);
            reply->status_code = atoi(buf1);
        } else {
            rtsp_parse_line(reply, p);
            pstrcat(rt->last_reply, sizeof(rt->last_reply), p);
            pstrcat(rt->last_reply, sizeof(rt->last_reply), "\n");
        }
        line_count++;
    }
    
    if (rt->session_id[0] == '\0' && reply->session_id[0] != '\0')
        pstrcpy(rt->session_id, sizeof(rt->session_id), reply->session_id);
    
    content_length = reply->content_length;
    if (content_length > 0) {
        /* leave some room for a trailing '\0' (useful for simple parsing) */
        content = av_malloc(content_length + 1);
        url_read(rt->rtsp_hd, content, content_length);
        content[content_length] = '\0';
    }
    if (content_ptr)
        *content_ptr = content;
}

/* useful for modules: set RTSP callback function */

void rtsp_set_callback(FFRTSPCallback *rtsp_cb)
{
    ff_rtsp_callback = rtsp_cb;
}


static int rtsp_read_header(AVFormatContext *s,
                            AVFormatParameters *ap)
{
    RTSPState *rt = s->priv_data;
    char host[1024], path[1024], tcpname[1024], cmd[2048];
    URLContext *rtsp_hd;
    int port, i, ret, err;
    RTSPHeader reply1, *reply = &reply1;
    unsigned char *content = NULL;
    AVStream *st;
    RTSPStream *rtsp_st;
    int protocol_mask;

    rtsp_abort_req = 0;
    
    /* extract hostname and port */
    url_split(NULL, 0,
              host, sizeof(host), &port, path, sizeof(path), s->filename);
    if (port < 0)
        port = RTSP_DEFAULT_PORT;

    /* open the tcp connexion */
    snprintf(tcpname, sizeof(tcpname), "tcp://%s:%d", host, port);
    if (url_open(&rtsp_hd, tcpname, URL_RDWR) < 0)
        return AVERROR_IO;
    rt->rtsp_hd = rtsp_hd;
    rt->seq = 0;
    
    /* describe the stream */
    snprintf(cmd, sizeof(cmd), 
             "DESCRIBE %s RTSP/1.0\n"
             "Accept: application/sdp\n",
             s->filename);
    rtsp_send_cmd(s, cmd, reply, &content);
    if (!content) {
        err = AVERROR_INVALIDDATA;
        goto fail;
    }
    if (reply->status_code != RTSP_STATUS_OK) {
        err = AVERROR_INVALIDDATA;
        goto fail;
    }
        
    /* now we got the SDP description, we parse it */
    ret = sdp_parse(s, (const char *)content);
    av_freep(&content);
    if (ret < 0) {
        err = AVERROR_INVALIDDATA;
        goto fail;
    }
    
    protocol_mask = rtsp_default_protocols;

    /* for each stream, make the setup request */
    /* XXX: we assume the same server is used for the control of each
       RTSP stream */
    for(i=0;i<s->nb_streams;i++) {
        char transport[2048];
        AVInputFormat *fmt;

        st = s->streams[i];
        rtsp_st = st->priv_data;

        /* compute available transports */
        transport[0] = '\0';

        /* RTP/UDP */
        if (protocol_mask & (1 << RTSP_PROTOCOL_RTP_UDP)) {
            char buf[256];
            int j;

            /* first try in specified port range */
            if (rtsp_rtp_port_min != 0) {
                for(j=rtsp_rtp_port_min;j<=rtsp_rtp_port_max;j++) {
                    snprintf(buf, sizeof(buf), "rtp://?localport=%d", j);
                    if (!av_open_input_file(&rtsp_st->ic, buf, 
                                            &rtp_demux, 0, NULL))
                        goto rtp_opened;
                }
            }

            /* then try on any port */
            if (av_open_input_file(&rtsp_st->ic, "rtp://", 
                                       &rtp_demux, 0, NULL) < 0) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
            }

        rtp_opened:
            port = rtp_get_local_port(url_fileno(&rtsp_st->ic->pb));
            if (transport[0] != '\0')
                pstrcat(transport, sizeof(transport), ",");
            snprintf(transport + strlen(transport), sizeof(transport) - strlen(transport) - 1,
                     "RTP/AVP/UDP;unicast;client_port=%d-%d",
                     port, port + 1);
        }

        /* RTP/TCP */
        if (protocol_mask & (1 << RTSP_PROTOCOL_RTP_TCP)) {
            if (transport[0] != '\0')
                pstrcat(transport, sizeof(transport), ",");
            snprintf(transport + strlen(transport), sizeof(transport) - strlen(transport) - 1,
                     "RTP/AVP/TCP");
        }

        if (protocol_mask & (1 << RTSP_PROTOCOL_RTP_UDP_MULTICAST)) {
            if (transport[0] != '\0')
                pstrcat(transport, sizeof(transport), ",");
            snprintf(transport + strlen(transport), 
                     sizeof(transport) - strlen(transport) - 1,
                     "RTP/AVP/UDP;multicast");
        }
        
        snprintf(cmd, sizeof(cmd), 
                 "SETUP %s RTSP/1.0\n"
                 "Transport: %s\n",
                 rtsp_st->control_url, transport);
        rtsp_send_cmd(s, cmd, reply, NULL);
        if (reply->status_code != RTSP_STATUS_OK ||
            reply->nb_transports != 1) {
            err = AVERROR_INVALIDDATA;
            goto fail;
        }

        /* XXX: same protocol for all streams is required */
        if (i > 0) {
            if (reply->transports[0].protocol != rt->protocol) {
                err = AVERROR_INVALIDDATA;
                goto fail;
            }
        } else {
            rt->protocol = reply->transports[0].protocol;
        }

        /* close RTP connection if not choosen */
        if (reply->transports[0].protocol != RTSP_PROTOCOL_RTP_UDP &&
            (protocol_mask & (1 << RTSP_PROTOCOL_RTP_UDP))) {
            av_close_input_file(rtsp_st->ic);
            rtsp_st->ic = NULL;
        }

        switch(reply->transports[0].protocol) {
        case RTSP_PROTOCOL_RTP_TCP:
            fmt = &rtp_demux;
            if (av_open_input_file(&rtsp_st->ic, "null", fmt, 0, NULL) < 0) {
                err = AVERROR_INVALIDDATA;
                goto fail;
            }
            rtsp_st->interleaved_min = reply->transports[0].interleaved_min;
            rtsp_st->interleaved_max = reply->transports[0].interleaved_max;
            break;
            
        case RTSP_PROTOCOL_RTP_UDP:
            {
                char url[1024];
                
                /* XXX: also use address if specified */
                snprintf(url, sizeof(url), "rtp://%s:%d", 
                         host, reply->transports[0].server_port_min);
                if (rtp_set_remote_url(url_fileno(&rtsp_st->ic->pb), url) < 0) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
                }
            }
            break;
        case RTSP_PROTOCOL_RTP_UDP_MULTICAST:
            {
                char url[1024];
                int ttl;

                fmt = &rtp_demux;
                ttl = reply->transports[0].ttl;
                if (!ttl)
                    ttl = 16;
                snprintf(url, sizeof(url), "rtp://%s:%d?multicast=1&ttl=%d", 
                         host, 
                         reply->transports[0].server_port_min,
                         ttl);
                if (av_open_input_file(&rtsp_st->ic, url, fmt, 0, NULL) < 0) {
                    err = AVERROR_INVALIDDATA;
                    goto fail;
                }
            }
            break;
        }
    }

    /* use callback if available to extend setup */
    if (ff_rtsp_callback) {
        if (ff_rtsp_callback(RTSP_ACTION_CLIENT_SETUP, rt->session_id, 
                             NULL, 0, rt->last_reply) < 0) {
            err = AVERROR_INVALIDDATA;
            goto fail;
        }
    }
                         
    /* start playing */
    snprintf(cmd, sizeof(cmd), 
             "PLAY %s RTSP/1.0\n",
             s->filename);
    rtsp_send_cmd(s, cmd, reply, NULL);
    if (reply->status_code != RTSP_STATUS_OK) {
        err = AVERROR_INVALIDDATA;
        goto fail;
    }

    /* open TCP with bufferized input */
    if (rt->protocol == RTSP_PROTOCOL_RTP_TCP) {
        if (url_fdopen(&rt->rtsp_gb, rt->rtsp_hd) < 0) {
            err = AVERROR_NOMEM;
            goto fail;
        }
    }

    return 0;
 fail:
    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        if (rtsp_st) {
            if (rtsp_st->ic)
                av_close_input_file(rtsp_st->ic);
        }
        av_free(rtsp_st);
    }
    av_freep(&content);
    url_close(rt->rtsp_hd);
    return err;
}

static int tcp_read_packet(AVFormatContext *s,
                           AVPacket *pkt)
{
    RTSPState *rt = s->priv_data;
    ByteIOContext *rtsp_gb = &rt->rtsp_gb;
    int c, id, len, i, ret;
    AVStream *st;
    RTSPStream *rtsp_st;
    char buf[RTP_MAX_PACKET_LENGTH];

 redo:
    for(;;) {
        c = url_fgetc(rtsp_gb);
        if (c == URL_EOF)
            return AVERROR_IO;
        if (c == '$')
            break;
    }
    id = get_byte(rtsp_gb);
    len = get_be16(rtsp_gb);
    if (len > RTP_MAX_PACKET_LENGTH || len < 12)
        goto redo;
    /* get the data */
    get_buffer(rtsp_gb, buf, len);
    
    /* find the matching stream */
    for(i = 0; i < s->nb_streams; i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        if (i >= rtsp_st->interleaved_min && 
            i <= rtsp_st->interleaved_max) 
            goto found;
    }
    goto redo;
 found:
    ret = rtp_parse_packet(rtsp_st->ic, pkt, buf, len);
    if (ret < 0)
        goto redo;
    pkt->stream_index = i;
    return ret;
}

/* NOTE: output one packet at a time. May need to add a small fifo */
static int udp_read_packet(AVFormatContext *s,
                           AVPacket *pkt)
{
    AVFormatContext *ic;
    AVStream *st;
    RTSPStream *rtsp_st;
    fd_set rfds;
    int fd1, fd2, fd_max, n, i, ret;
    char buf[RTP_MAX_PACKET_LENGTH];
    struct timeval tv;

    for(;;) {
        if (rtsp_abort_req)
            return -EIO;
        FD_ZERO(&rfds);
        fd_max = -1;
        for(i = 0; i < s->nb_streams; i++) {
            st = s->streams[i];
            rtsp_st = st->priv_data;
            ic = rtsp_st->ic;
            /* currently, we cannot probe RTCP handle because of blocking restrictions */
            rtp_get_file_handles(url_fileno(&ic->pb), &fd1, &fd2);
            if (fd1 > fd_max)
                fd_max = fd1;
            FD_SET(fd1, &rfds);
        }
        /* XXX: also add proper API to abort */
        tv.tv_sec = 0;
        tv.tv_usec = 500000;
        n = select(fd_max + 1, &rfds, NULL, NULL, &tv);
        if (n > 0) {
            for(i = 0; i < s->nb_streams; i++) {
                st = s->streams[i];
                rtsp_st = st->priv_data;
                ic = rtsp_st->ic;
                rtp_get_file_handles(url_fileno(&ic->pb), &fd1, &fd2);
                if (FD_ISSET(fd1, &rfds)) {
                    ret = url_read(url_fileno(&ic->pb), buf, sizeof(buf));
                    if (ret >= 0 && 
                        rtp_parse_packet(ic, pkt, buf, ret) == 0) {
                        pkt->stream_index = i;
                        return ret;
                    }
                }
            }
        }
    }
}

static int rtsp_read_packet(AVFormatContext *s,
                            AVPacket *pkt)
{
    RTSPState *rt = s->priv_data;
    int ret;

    switch(rt->protocol) {
    default:
    case RTSP_PROTOCOL_RTP_TCP:
        ret = tcp_read_packet(s, pkt);
        break;
    case RTSP_PROTOCOL_RTP_UDP:
        ret = udp_read_packet(s, pkt);
        break;
    }
    return ret;
}

static int rtsp_read_close(AVFormatContext *s)
{
    RTSPState *rt = s->priv_data;
    AVStream *st;
    RTSPStream *rtsp_st;
    RTSPHeader reply1, *reply = &reply1;
    int i;
    char cmd[1024];

    /* NOTE: it is valid to flush the buffer here */
    if (rt->protocol == RTSP_PROTOCOL_RTP_TCP) {
        url_fclose(&rt->rtsp_gb);
    }

    snprintf(cmd, sizeof(cmd), 
             "TEARDOWN %s RTSP/1.0\n",
             s->filename);
    rtsp_send_cmd(s, cmd, reply, NULL);

    if (ff_rtsp_callback) {
        ff_rtsp_callback(RTSP_ACTION_CLIENT_TEARDOWN, rt->session_id, 
                         NULL, 0, NULL);
    }

    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        if (rtsp_st) {
            if (rtsp_st->ic)
                av_close_input_file(rtsp_st->ic);
        }
        av_free(rtsp_st);
    }
    url_close(rt->rtsp_hd);
    return 0;
}

static AVInputFormat rtsp_demux = {
    "rtsp",
    "RTSP input format",
    sizeof(RTSPState),
    rtsp_probe,
    rtsp_read_header,
    rtsp_read_packet,
    rtsp_read_close,
    .flags = AVFMT_NOFILE,
};

static int sdp_probe(AVProbeData *p1)
{
    const char *p;

    /* we look for a line beginning "c=IN IP4" */
    p = p1->buf;
    while (*p != '\0') {
        if (strstart(p, "c=IN IP4", NULL))
            return AVPROBE_SCORE_MAX / 2;
        p = strchr(p, '\n');
        if (!p)
            break;
        p++;
        if (*p == '\r')
            p++;
    }
    return 0;
}

#define SDP_MAX_SIZE 8192

static int sdp_read_header(AVFormatContext *s,
                           AVFormatParameters *ap)
{
    AVStream *st;
    RTSPStream *rtsp_st;
    int size, i, err;
    char *content;
    char url[1024];

    /* read the whole sdp file */
    /* XXX: better loading */
    content = av_malloc(SDP_MAX_SIZE);
    size = get_buffer(&s->pb, content, SDP_MAX_SIZE - 1);
    if (size <= 0) {
        av_free(content);
        return AVERROR_INVALIDDATA;
    }
    content[size] ='\0';

    sdp_parse(s, content);
    av_free(content);

    /* open each RTP stream */
    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        
        snprintf(url, sizeof(url), "rtp://%s:%d?multicast=1&ttl=%d", 
                 inet_ntoa(rtsp_st->sdp_ip), 
                 rtsp_st->sdp_port,
                 rtsp_st->sdp_ttl);
        if (av_open_input_file(&rtsp_st->ic, url, &rtp_demux, 0, NULL) < 0) {
            err = AVERROR_INVALIDDATA;
            goto fail;
        }
    }
    return 0;
 fail:
    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        if (rtsp_st) {
            if (rtsp_st->ic)
                av_close_input_file(rtsp_st->ic);
        }
        av_free(rtsp_st);
    }
    return err;
}

static int sdp_read_packet(AVFormatContext *s,
                            AVPacket *pkt)
{
    return udp_read_packet(s, pkt);
}

static int sdp_read_close(AVFormatContext *s)
{
    AVStream *st;
    RTSPStream *rtsp_st;
    int i;

    for(i=0;i<s->nb_streams;i++) {
        st = s->streams[i];
        rtsp_st = st->priv_data;
        if (rtsp_st) {
            if (rtsp_st->ic)
                av_close_input_file(rtsp_st->ic);
        }
        av_free(rtsp_st);
    }
    return 0;
}


static AVInputFormat sdp_demux = {
    "sdp",
    "SDP",
    sizeof(RTSPState),
    sdp_probe,
    sdp_read_header,
    sdp_read_packet,
    sdp_read_close,
};


/* dummy redirector format (used directly in av_open_input_file now) */
static int redir_probe(AVProbeData *pd)
{
    const char *p;
    p = pd->buf;
    while (redir_isspace(*p))
        p++;
    if (strstart(p, "http://", NULL) ||
        strstart(p, "rtsp://", NULL))
        return AVPROBE_SCORE_MAX;
    return 0;
}

/* called from utils.c */
int redir_open(AVFormatContext **ic_ptr, ByteIOContext *f)
{
    char buf[4096], *q;
    int c;
    AVFormatContext *ic = NULL;

    /* parse each URL and try to open it */
    c = url_fgetc(f);
    while (c != URL_EOF) {
        /* skip spaces */
        for(;;) {
            if (!redir_isspace(c))
                break;
            c = url_fgetc(f);
        }
        if (c == URL_EOF)
            break;
        /* record url */
        q = buf;
        for(;;) {
            if (c == URL_EOF || redir_isspace(c))
                break;
            if ((q - buf) < sizeof(buf) - 1)
                *q++ = c;
            c = url_fgetc(f);
        }
        *q = '\0';
        //printf("URL='%s'\n", buf);
        /* try to open the media file */
        if (av_open_input_file(&ic, buf, NULL, 0, NULL) == 0)
            break;
    }
    *ic_ptr = ic;
    if (!ic)
        return AVERROR_IO;
    else
        return 0;
}

AVInputFormat redir_demux = {
    "redir",
    "Redirector format",
    0,
    redir_probe,
    NULL,
    NULL,
    NULL,
};

int rtsp_init(void)
{
    av_register_input_format(&rtsp_demux);
    av_register_input_format(&redir_demux);
    av_register_input_format(&sdp_demux);
    return 0;
}
