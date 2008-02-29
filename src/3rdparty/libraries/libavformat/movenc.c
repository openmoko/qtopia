/*
 * MOV, 3GP, MP4 encoder.
 * Copyright (c) 2003 Thomas Raivio.
 * Enhancements by Gildas Bazin <gbazin@netcourrier.com>
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
#include "avi.h"
#include "avio.h"

#undef NDEBUG
#include <assert.h>

#define MOV_INDEX_CLUSTER_SIZE 16384
#define globalTimescale 1000

typedef struct MOVIentry {
    unsigned int flags, pos, size;
    unsigned int samplesInChunk;
    char         key_frame;
    unsigned int entries;
} MOVIentry;

typedef struct MOVIndex {
    int         entry;
    int         mdat_size;
    int         ents_allocated;
    long        timescale;
    long        time;
    long        trackDuration;
    long        sampleCount;
    long        sampleDuration;
    int         hasKeyframes;
    int         trackID;
    AVCodecContext *enc;

    int         vosLen;
    uint8_t     *vosData;
    MOVIentry** cluster;
} MOVTrack;

typedef struct {
    long    time;
    int     nb_streams;
    int     mdat_written;
    offset_t mdat_pos;
    offset_t movi_list;
    long    timescale;
    MOVTrack tracks[MAX_STREAMS];
} MOVContext;

static int mov_write_esds_tag(ByteIOContext *pb, MOVTrack* track);

//FIXME supprt 64bit varaint with wide placeholders
static int updateSize (ByteIOContext *pb, int pos)
{
    long curpos = url_ftell(pb);
    url_fseek(pb, pos, SEEK_SET);
    put_be32(pb, curpos - pos); /* rewrite size */
    url_fseek(pb, curpos, SEEK_SET);

    return curpos - pos;
}

/* Chunk offset atom */
static int mov_write_stco_tag(ByteIOContext *pb, MOVTrack* track)
{
    int i;
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "stco");
    put_be32(pb, 0); /* version & flags */
    put_be32(pb, track->entry); /* entry count */
    for (i=0; i<track->entry; i++) {
        int cl = i / MOV_INDEX_CLUSTER_SIZE;
        int id = i % MOV_INDEX_CLUSTER_SIZE;
        put_be32(pb, track->cluster[cl][id].pos);
    }
    return updateSize (pb, pos);
}

/* Sample size atom */
static int mov_write_stsz_tag(ByteIOContext *pb, MOVTrack* track)
{
    int equalChunks = 1;
    int i, j, entries = 0, tst = -1, oldtst = -1;

    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "stsz");
    put_be32(pb, 0); /* version & flags */

    for (i=0; i<track->entry; i++) {
        int cl = i / MOV_INDEX_CLUSTER_SIZE;
        int id = i % MOV_INDEX_CLUSTER_SIZE;
        tst = track->cluster[cl][id].size/track->cluster[cl][id].entries;
        if(oldtst != -1 && tst != oldtst) {
            equalChunks = 0;
        }
        oldtst = tst;
        entries += track->cluster[cl][id].entries;
    }
    if (equalChunks) {
        int sSize = track->cluster[0][0].size/track->cluster[0][0].entries;
        put_be32(pb, sSize); // sample size 
        put_be32(pb, entries); // sample count
    }
    else {
        put_be32(pb, 0); // sample size 
        put_be32(pb, entries); // sample count 
        for (i=0; i<track->entry; i++) {
            int cl = i / MOV_INDEX_CLUSTER_SIZE;
            int id = i % MOV_INDEX_CLUSTER_SIZE;
            for ( j=0; j<track->cluster[cl][id].entries; j++) {
                put_be32(pb, track->cluster[cl][id].size /
                         track->cluster[cl][id].entries);
            }
        }
    }
    return updateSize (pb, pos);
}

/* Sample to chunk atom */
static int mov_write_stsc_tag(ByteIOContext *pb, MOVTrack* track)
{
    int index = 0, oldval = -1, i, entryPos, curpos;

    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "stsc");
    put_be32(pb, 0); // version & flags 
    entryPos = url_ftell(pb);
    put_be32(pb, track->entry); // entry count 
    for (i=0; i<track->entry; i++) {
        int cl = i / MOV_INDEX_CLUSTER_SIZE;
        int id = i % MOV_INDEX_CLUSTER_SIZE;
        if(oldval != track->cluster[cl][id].samplesInChunk)
        {
            put_be32(pb, i+1); // first chunk 
            put_be32(pb, track->cluster[cl][id].samplesInChunk); // samples per chunk
            put_be32(pb, 0x1); // sample description index 
            oldval = track->cluster[cl][id].samplesInChunk;
            index++;
        }
    }
    curpos = url_ftell(pb);
    url_fseek(pb, entryPos, SEEK_SET);
    put_be32(pb, index); // rewrite size 
    url_fseek(pb, curpos, SEEK_SET);

    return updateSize (pb, pos);
}

/* Sync sample atom */
static int mov_write_stss_tag(ByteIOContext *pb, MOVTrack* track)
{
    long curpos;
    int i, index = 0, entryPos;
    int pos = url_ftell(pb);
    put_be32(pb, 0); // size 
    put_tag(pb, "stss");
    put_be32(pb, 0); // version & flags 
    entryPos = url_ftell(pb);
    put_be32(pb, track->entry); // entry count 
    for (i=0; i<track->entry; i++) {
        int cl = i / MOV_INDEX_CLUSTER_SIZE;
        int id = i % MOV_INDEX_CLUSTER_SIZE;
        if(track->cluster[cl][id].key_frame == 1) {
            put_be32(pb, i+1);
            index++;
        }
    }
    curpos = url_ftell(pb);
    url_fseek(pb, entryPos, SEEK_SET);
    put_be32(pb, index); // rewrite size 
    url_fseek(pb, curpos, SEEK_SET);
    return updateSize (pb, pos);
}

static int mov_write_damr_tag(ByteIOContext *pb)
{
    put_be32(pb, 0x11); /* size */
    put_tag(pb, "damr");
    put_tag(pb, "FFMP");
    put_byte(pb, 0);

    put_be16(pb, 0x80); /* Mode set (all modes for AMR_NB) */
    put_be16(pb, 0xa); /* Mode change period (no restriction) */
    //put_be16(pb, 0x81ff); /* Mode set (all modes for AMR_NB) */
    //put_be16(pb, 1); /* Mode change period (no restriction) */
    return 0x11;
}

static int mov_write_audio_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */

    if(track->enc->codec_id == CODEC_ID_PCM_MULAW)
      put_tag(pb, "ulaw");
    else if(track->enc->codec_id == CODEC_ID_PCM_ALAW)
      put_tag(pb, "alaw");
    else if(track->enc->codec_id == CODEC_ID_ADPCM_IMA_QT)
      put_tag(pb, "ima4");
    else if(track->enc->codec_id == CODEC_ID_MACE3)
      put_tag(pb, "MAC3");
    else if(track->enc->codec_id == CODEC_ID_MACE6)
      put_tag(pb, "MAC6");
    else if(track->enc->codec_id == CODEC_ID_AAC)
      put_tag(pb, "mp4a");
    else if(track->enc->codec_id == CODEC_ID_AMR_NB)
      put_tag(pb, "samr");
    else
      put_tag(pb, "    ");

    put_be32(pb, 0); /* Reserved */
    put_be16(pb, 0); /* Reserved */
    put_be16(pb, 1); /* Data-reference index, XXX  == 1 */
    /* SoundDescription */
    put_be16(pb, 0); /* Version */
    put_be16(pb, 0); /* Revision level */
    put_be32(pb, 0); /* Reserved */

    put_be16(pb, track->enc->channels); /* Number of channels */
    /* TODO: Currently hard-coded to 16-bit, there doesn't seem
                 to be a good way to get number of bits of audio */
    put_be16(pb, 0x10); /* Reserved */
    put_be16(pb, 0); /* compression ID (= 0) */
    put_be16(pb, 0); /* packet size (= 0) */
    put_be16(pb, track->timescale); /* Time scale */
    put_be16(pb, 0); /* Reserved */

    if(track->enc->codec_id == CODEC_ID_AAC)
        mov_write_esds_tag(pb, track);
    if(track->enc->codec_id == CODEC_ID_AMR_NB)
        mov_write_damr_tag(pb);
    return updateSize (pb, pos);
}

static int mov_write_d263_tag(ByteIOContext *pb)
{
    put_be32(pb, 0xf); /* size */
    put_tag(pb, "d263");
    put_tag(pb, "FFMP");
    put_be16(pb, 0x0a);
    put_byte(pb, 0);
    return 0xf;
}

/* TODO: No idea about these values */
static int mov_write_svq3_tag(ByteIOContext *pb)
{
    put_be32(pb, 0x15);
    put_tag(pb, "SMI ");
    put_tag(pb, "SEQH");
    put_be32(pb, 0x5);
    put_be32(pb, 0xe2c0211d);
    put_be32(pb, 0xc0000000);
    put_byte(pb, 0);   
    return 0x15;
}

static unsigned int descrLength(unsigned int len)
{
    if (len < 0x00000080)
        return 2 + len;
    else if (len < 0x00004000)
        return 3 + len;
    else if(len < 0x00200000)
        return 4 + len;
    else
        return 5 + len;
}

static void putDescr(ByteIOContext *pb, int tag, int size)
{
    uint32_t len;
    uint8_t  vals[4];

    len = size;
    vals[3] = (uint8_t)(len & 0x7f);
    len >>= 7;
    vals[2] = (uint8_t)((len & 0x7f) | 0x80); 
    len >>= 7;
    vals[1] = (uint8_t)((len & 0x7f) | 0x80); 
    len >>= 7;
    vals[0] = (uint8_t)((len & 0x7f) | 0x80);

    put_byte(pb, tag); // DescriptorTag

    if (size < 0x00000080)
    {
        put_byte(pb, vals[3]);
    }
    else if (size < 0x00004000)
    {
        put_byte(pb, vals[2]);
        put_byte(pb, vals[3]);
    }
    else if (size < 0x00200000)
    {
        put_byte(pb, vals[1]);
        put_byte(pb, vals[2]);
        put_byte(pb, vals[3]);
    }
    else if (size < 0x10000000)
    {
        put_byte(pb, vals[0]);
        put_byte(pb, vals[1]);
        put_byte(pb, vals[2]);
        put_byte(pb, vals[3]);
    }
}

static int mov_write_esds_tag(ByteIOContext *pb, MOVTrack* track) // Basic
{
    int decoderSpecificInfoLen = track->vosLen ? descrLength(track->vosLen):0;
    int pos = url_ftell(pb);

    put_be32(pb, 0);               // size
    put_tag(pb, "esds");
    put_be32(pb, 0);               // Version

    // ES descriptor
    putDescr(pb, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) +
             descrLength(1));
    put_be16(pb, 0x0001);          // ID (= 1)
    put_byte(pb, 0x00);            // flags (= no flags)

    // DecoderConfig descriptor
    putDescr(pb, 0x04, 13 + decoderSpecificInfoLen);

    if(track->enc->codec_id == CODEC_ID_AAC)
        put_byte(pb, 0x40);        // Object type indication
    else if(track->enc->codec_id == CODEC_ID_MPEG4)
        put_byte(pb, 0x20);        // Object type indication (Visual 14496-2)

    if(track->enc->codec_type == CODEC_TYPE_AUDIO)
        put_byte(pb, 0x15);            // flags (= Audiostream)
    else
        put_byte(pb, 0x11);            // flags (= Visualstream)

    put_byte(pb, 0x0);             // Buffersize DB (24 bits)
    put_be16(pb, 0x0dd2);          // Buffersize DB

    // TODO: find real values for these
    put_be32(pb, 0x0002e918);     // maxbitrate
    put_be32(pb, 0x00017e6b);     // avg bitrate

    if (track->vosLen)
    {
        // DecoderSpecific info descriptor
        putDescr(pb, 0x05, track->vosLen);
        put_buffer(pb, track->vosData, track->vosLen);
    }

    // SL descriptor
    putDescr(pb, 0x06, descrLength(1));
    put_byte(pb, 0x02);
    return updateSize (pb, pos);
}

static int mov_write_video_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    if(track->enc->codec_id == CODEC_ID_SVQ1)
      put_tag(pb, "SVQ1");
    else if(track->enc->codec_id == CODEC_ID_SVQ3)
      put_tag(pb, "SVQ3");
    else if(track->enc->codec_id == CODEC_ID_MPEG4)
      put_tag(pb, "mp4v");
    else if(track->enc->codec_id == CODEC_ID_H263)
      put_tag(pb, "s263");
    else
      put_tag(pb, "    "); /* Unknown tag */

    put_be32(pb, 0); /* Reserved */
    put_be16(pb, 0); /* Reserved */
    put_be16(pb, 1); /* Data-reference index */

    put_be32(pb, 0); /* Reserved (= 02000c) */
    put_be32(pb, 0); /* Reserved ("SVis")*/
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved (400)*/
    put_be16(pb, track->enc->width); /* Video width */
    put_be16(pb, track->enc->height); /* Video height */
    put_be32(pb, 0x00480000); /* Reserved */
    put_be32(pb, 0x00480000); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */

    put_be16(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be32(pb, 0); /* Reserved */
    put_be16(pb, 0x18); /* Reserved */
    put_be16(pb, 0xffff); /* Reserved */
    if(track->enc->codec_id == CODEC_ID_MPEG4)
        mov_write_esds_tag(pb, track);
    else if(track->enc->codec_id == CODEC_ID_H263)
        mov_write_d263_tag(pb);
    else if(track->enc->codec_id == CODEC_ID_SVQ3)
        mov_write_svq3_tag(pb);    

    return updateSize (pb, pos);
}

static int mov_write_stsd_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "stsd");
    put_be32(pb, 0); /* version & flags */
    put_be32(pb, 1); /* entry count */
    if (track->enc->codec_type == CODEC_TYPE_VIDEO)
        mov_write_video_tag(pb, track);
    else if (track->enc->codec_type == CODEC_TYPE_AUDIO)
        mov_write_audio_tag(pb, track);
    return updateSize(pb, pos);
}

/* TODO?: Currently all samples/frames seem to have same duration */
/* Time to sample atom */
static int mov_write_stts_tag(ByteIOContext *pb, MOVTrack* track)
{
    put_be32(pb, 0x18); /* size */
    put_tag(pb, "stts");
    put_be32(pb, 0); /* version & flags */
    put_be32(pb, 1); /* entry count */

    put_be32(pb, track->sampleCount); /* sample count */
    put_be32(pb, track->sampleDuration); /* sample duration */
    return 0x18;
}

static int mov_write_dref_tag(ByteIOContext *pb)
{
    put_be32(pb, 28); /* size */
    put_tag(pb, "dref");
    put_be32(pb, 0); /* version & flags */
    put_be32(pb, 1); /* entry count */

    put_be32(pb, 0xc); /* size */
    put_tag(pb, "url ");
    put_be32(pb, 1); /* version & flags */

    return 28;
}

static int mov_write_stbl_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "stbl");
    mov_write_stsd_tag(pb, track);
    mov_write_stts_tag(pb, track);
    if (track->enc->codec_type == CODEC_TYPE_VIDEO &&
        track->hasKeyframes)
        mov_write_stss_tag(pb, track);
    mov_write_stsc_tag(pb, track);
    mov_write_stsz_tag(pb, track);
    mov_write_stco_tag(pb, track);
    return updateSize(pb, pos);
}

static int mov_write_dinf_tag(ByteIOContext *pb)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "dinf");
    mov_write_dref_tag(pb);
    return updateSize(pb, pos);
}

static int mov_write_smhd_tag(ByteIOContext *pb)
{
    put_be32(pb, 16); /* size */
    put_tag(pb, "smhd");
    put_be32(pb, 0); /* version & flags */
    put_be16(pb, 0); /* reserved (balance, normally = 0) */
    put_be16(pb, 0); /* reserved */
    return 16;
}

static int mov_write_vmhd_tag(ByteIOContext *pb)
{
    put_be32(pb, 0x14); /* size (always 0x14) */
    put_tag(pb, "vmhd");
    put_be32(pb, 0x01); /* version & flags */
    put_be64(pb, 0); /* reserved (graphics mode = copy) */
    return 0x14;
}

static int mov_write_minf_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "minf");
    if(track->enc->codec_type == CODEC_TYPE_VIDEO)
        mov_write_vmhd_tag(pb);
    else
        mov_write_smhd_tag(pb);
    mov_write_dinf_tag(pb);
    mov_write_stbl_tag(pb, track);
    return updateSize(pb, pos);
}

static int mov_write_hdlr_tag(ByteIOContext *pb, MOVTrack* track)
{
    char *str;
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "hdlr");
    put_be32(pb, 0); /* Version & flags */
    put_be32(pb, 0); /* reserved */
    if(track->enc->codec_type == CODEC_TYPE_VIDEO)
        put_tag(pb, "vide"); /* handler type */
    else
        put_tag(pb, "soun"); /* handler type */
    put_be32(pb ,0); /* reserved */
    put_be32(pb ,0); /* reserved */
    put_be32(pb ,0); /* reserved */
    if(track->enc->codec_type == CODEC_TYPE_VIDEO)
        str = "VideoHandler";
    else
        str = "SoundHandler";
    put_byte(pb, strlen(str)); /* string counter */
    put_buffer(pb, str, strlen(str));
    return updateSize(pb, pos);
}

static int mov_write_mdhd_tag(ByteIOContext *pb, MOVTrack* track)
{
    put_be32(pb, 32); /* size */
    put_tag(pb, "mdhd");
    put_be32(pb, 0); /* Version & flags */
    put_be32(pb, track->time); /* creation time */
    put_be32(pb, track->time); /* modification time */
    put_be32(pb, track->timescale); /* time scale (sample rate for audio) */ 
    put_be32(pb, track->trackDuration); /* duration */
    put_be16(pb, 0); /* language, 0 = english */
    put_be16(pb, 0); /* reserved (quality) */
    return 32;
}

static int mov_write_mdia_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "mdia");
    mov_write_mdhd_tag(pb, track);
    mov_write_hdlr_tag(pb, track);
    mov_write_minf_tag(pb, track);
    return updateSize(pb, pos);
}

static int mov_write_tkhd_tag(ByteIOContext *pb, MOVTrack* track)
{
    int64_t maxTrackLenTemp;
    put_be32(pb, 0x5c); /* size (always 0x5c) */
    put_tag(pb, "tkhd");
    put_be32(pb, 0xf); /* version & flags (track enabled) */
    put_be32(pb, track->time); /* creation time */
    put_be32(pb, track->time); /* modification time */
    put_be32(pb, track->trackID); /* track-id */
    put_be32(pb, 0); /* reserved */
    maxTrackLenTemp = ((int64_t)globalTimescale*(int64_t)track->trackDuration)/(int64_t)track->timescale;
    put_be32(pb, (long)maxTrackLenTemp); /* duration */

    put_be32(pb, 0); /* reserved */
    put_be32(pb, 0); /* reserved */
    put_be32(pb, 0x0); /* reserved (Layer & Alternate group) */
    /* Volume, only for audio */
    if(track->enc->codec_type == CODEC_TYPE_AUDIO)
        put_be16(pb, 0x0100);
    else
        put_be16(pb, 0);
    put_be16(pb, 0); /* reserved */

    /* Matrix structure */
    put_be32(pb, 0x00010000); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x00010000); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x40000000); /* reserved */

    /* Track width and height, for visual only */
    if(track->enc->codec_type == CODEC_TYPE_VIDEO) {
        put_be32(pb, track->enc->width*0x10000);
        put_be32(pb, track->enc->height*0x10000);
    }
    else {
        put_be32(pb, 0);
        put_be32(pb, 0);
    }
    return 0x5c;
}

static int mov_write_trak_tag(ByteIOContext *pb, MOVTrack* track)
{
    int pos = url_ftell(pb);
    put_be32(pb, 0); /* size */
    put_tag(pb, "trak");
    mov_write_tkhd_tag(pb, track);
    mov_write_mdia_tag(pb, track);
    return updateSize(pb, pos);
}

/* TODO: Not sorted out, but not necessary either */
static int mov_write_iods_tag(ByteIOContext *pb, MOVContext *mov)
{
    put_be32(pb, 0x15); /* size */
    put_tag(pb, "iods");
    put_be32(pb, 0);    /* version & flags */
    put_be16(pb, 0x1007);
    put_byte(pb, 0);
    put_be16(pb, 0x4fff);
    put_be16(pb, 0xfffe);
    put_be16(pb, 0x01ff);
    return 0x15;
}

static int mov_write_mvhd_tag(ByteIOContext *pb, MOVContext *mov)
{
    int maxTrackID = 1, maxTrackLen = 0, i;
    int64_t maxTrackLenTemp;

    put_be32(pb, 0x6c); /* size (always 0x6c) */
    put_tag(pb, "mvhd");
    put_be32(pb, 0); /* version & flags */
    put_be32(pb, mov->time); /* creation time */
    put_be32(pb, mov->time); /* modification time */
    put_be32(pb, mov->timescale); /* timescale */
    for (i=0; i<MAX_STREAMS; i++) {
        if(mov->tracks[i].entry > 0) {
            maxTrackLenTemp = ((int64_t)globalTimescale*(int64_t)mov->tracks[i].trackDuration)/(int64_t)mov->tracks[i].timescale;
            if(maxTrackLen < maxTrackLenTemp)
                maxTrackLen = maxTrackLenTemp;
            if(maxTrackID < mov->tracks[i].trackID)
                maxTrackID = mov->tracks[i].trackID;
        }
    }
    put_be32(pb, maxTrackLen); /* duration of longest track */

    put_be32(pb, 0x00010000); /* reserved (preferred rate) 1.0 = normal */
    put_be16(pb, 0x0100); /* reserved (preferred volume) 1.0 = normal */
    put_be16(pb, 0); /* reserved */
    put_be32(pb, 0); /* reserved */
    put_be32(pb, 0); /* reserved */

    /* Matrix structure */
    put_be32(pb, 0x00010000); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x00010000); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x0); /* reserved */
    put_be32(pb, 0x40000000); /* reserved */

    put_be32(pb, 0); /* reserved (preview time) */
    put_be32(pb, 0); /* reserved (preview duration) */
    put_be32(pb, 0); /* reserved (poster time) */
    put_be32(pb, 0); /* reserved (selection time) */
    put_be32(pb, 0); /* reserved (selection duration) */
    put_be32(pb, 0); /* reserved (current time) */
    put_be32(pb, maxTrackID+1); /* Next track id */
    return 0x6c;
}

static int mov_write_moov_tag(ByteIOContext *pb, MOVContext *mov)
{
    int pos, i;
    pos = url_ftell(pb);
    put_be32(pb, 0); /* size placeholder*/
    put_tag(pb, "moov");
    mov->timescale = globalTimescale;

    for (i=0; i<MAX_STREAMS; i++) {
        if(mov->tracks[i].entry <= 0) continue;

        if(mov->tracks[i].enc->codec_type == CODEC_TYPE_VIDEO) {
            mov->tracks[i].timescale = mov->tracks[i].enc->frame_rate;
            mov->tracks[i].sampleDuration = mov->tracks[i].enc->frame_rate_base;
        }
        else if(mov->tracks[i].enc->codec_type == CODEC_TYPE_AUDIO) {
            /* If AMR, track timescale = 8000, AMR_WB = 16000 */
            if(mov->tracks[i].enc->codec_id == CODEC_ID_AMR_NB) {
                mov->tracks[i].sampleDuration = 160;  // Bytes per chunk
                mov->tracks[i].timescale = 8000;
            }
            else {
                mov->tracks[i].timescale = mov->tracks[i].enc->sample_rate;
                mov->tracks[i].sampleDuration = mov->tracks[i].enc->frame_size;
            }
        }

        mov->tracks[i].trackDuration = 
            mov->tracks[i].sampleCount * mov->tracks[i].sampleDuration;
        mov->tracks[i].time = mov->time;
        mov->tracks[i].trackID = i+1;
    }

    mov_write_mvhd_tag(pb, mov);
    //mov_write_iods_tag(pb, mov);
    for (i=0; i<MAX_STREAMS; i++) {
        if(mov->tracks[i].entry > 0) {
            mov_write_trak_tag(pb, &(mov->tracks[i]));
        }
    }

    return updateSize(pb, pos);
}

int mov_write_mdat_tag(ByteIOContext *pb, MOVContext* mov)
{
    mov->mdat_pos = url_ftell(pb); 
    put_be32(pb, 0); /* size placeholder*/
    put_tag(pb, "mdat");
    return 0;
}

/* TODO: This needs to be more general */
int mov_write_ftyp_tag(ByteIOContext *pb, AVFormatContext *s)
{
    put_be32(pb, 0x14 ); /* size */
    put_tag(pb, "ftyp");

    if (!strcmp("3gp", s->oformat->name))
        put_tag(pb, "3gp4");
    else
        put_tag(pb, "isom");

    put_be32(pb, 0x200 );

    if (!strcmp("3gp", s->oformat->name))
        put_tag(pb, "3gp4");
    else
        put_tag(pb, "mp41");

    return 0x14;
}

static int mov_write_header(AVFormatContext *s)
{
    ByteIOContext *pb = &s->pb;

    if(s->oformat != NULL) {
    if(!strcmp("3gp", s->oformat->name) || !strcmp("mp4", s->oformat->name))
        mov_write_ftyp_tag(pb,s);
    }

    put_flush_packet(pb);

    return 0;
}

static int Timestamp() {
    return 1067949799U+(24107*86400); //its the modification time of this line :)
}

static int mov_write_packet(AVFormatContext *s, int stream_index,
                            const uint8_t *buf, int size, int64_t pts)
{
    MOVContext *mov = s->priv_data;
    ByteIOContext *pb = &s->pb;
    AVCodecContext *enc = &s->streams[stream_index]->codec;
    MOVTrack* trk = &mov->tracks[stream_index];
    int cl, id;
    unsigned int samplesInChunk = 0;

    if (url_is_streamed(&s->pb)) return 0; /* Can't handle that */
    if (!size) return 0; /* Discard 0 sized packets */

    if (enc->codec_type == CODEC_TYPE_VIDEO ) {
        samplesInChunk = 1;
    }
    else if (enc->codec_type == CODEC_TYPE_AUDIO ) {
        if( enc->codec_id == CODEC_ID_AMR_NB) {
            /* We must find out how many AMR blocks there are in one packet */
            static uint16_t packed_size[16] =
                {13, 14, 16, 18, 20, 21, 27, 32, 6, 0, 0, 0, 0, 0, 0, 0};
            int len = 0;

            while (len < size && samplesInChunk < 100) {
                len += packed_size[(buf[len] >> 3) & 0x0F];
                samplesInChunk++;
            }
        }
        else if(enc->codec_id == CODEC_ID_PCM_ALAW) {
            samplesInChunk = size/enc->channels;
        }
        else {
            samplesInChunk = 1;
        }
    }

    if ((enc->codec_id == CODEC_ID_MPEG4 || enc->codec_id == CODEC_ID_AAC)
        && trk->vosLen == 0) {
        assert(enc->extradata_size);

        trk->vosLen = enc->extradata_size;
        trk->vosData = av_malloc(trk->vosLen);
        memcpy(trk->vosData, enc->extradata, trk->vosLen);
    }

    cl = trk->entry / MOV_INDEX_CLUSTER_SIZE;
    id = trk->entry % MOV_INDEX_CLUSTER_SIZE;

    if (trk->ents_allocated <= trk->entry) {
        trk->cluster = av_realloc(trk->cluster, (cl+1)*sizeof(void*)); 
        if (!trk->cluster)
            return -1;
        trk->cluster[cl] = av_malloc(MOV_INDEX_CLUSTER_SIZE*sizeof(MOVIentry));
        if (!trk->cluster[cl])
            return -1;
        trk->ents_allocated += MOV_INDEX_CLUSTER_SIZE;
    }
    if (mov->mdat_written == 0) {
        mov_write_mdat_tag(pb, mov);
        mov->mdat_written = 1;
        mov->time = Timestamp();
    }

    trk->cluster[cl][id].pos = url_ftell(pb) - mov->movi_list;
    trk->cluster[cl][id].samplesInChunk = samplesInChunk;
    trk->cluster[cl][id].size = size;
    trk->cluster[cl][id].entries = samplesInChunk;
    if(enc->codec_type == CODEC_TYPE_VIDEO) {
        trk->cluster[cl][id].key_frame = enc->coded_frame->key_frame;
        if(enc->coded_frame->pict_type == FF_I_TYPE)
            trk->hasKeyframes = 1;
    }
    trk->enc = enc;
    trk->entry++;
    trk->sampleCount += samplesInChunk;
    trk->mdat_size += size;

    put_buffer(pb, buf, size);

    put_flush_packet(pb);
    return 0;
}

static int mov_write_trailer(AVFormatContext *s)
{
    MOVContext *mov = s->priv_data;
    ByteIOContext *pb = &s->pb;
    int res = 0;
    int i, j;
    offset_t file_size;

    file_size = url_ftell(pb);
    j = 0;

    /* Write size of mdat tag */
    for (i=0; i<MAX_STREAMS; i++) {
        if(mov->tracks[i].ents_allocated > 0) {
            j += mov->tracks[i].mdat_size;
        }
    }
    url_fseek(pb, mov->mdat_pos, SEEK_SET);
    put_be32(pb, j+8);
    url_fseek(pb, file_size, SEEK_SET);

    mov_write_moov_tag(pb, mov);

    for (i=0; i<MAX_STREAMS; i++) {
        for (j=0; j<mov->tracks[i].ents_allocated/MOV_INDEX_CLUSTER_SIZE; j++) {
            av_free(mov->tracks[i].cluster[j]);
        }
        av_free(mov->tracks[i].cluster);
        mov->tracks[i].cluster = NULL;
        mov->tracks[i].ents_allocated = mov->tracks[i].entry = 0;
    }
    put_flush_packet(pb);

    return res;
}

static AVOutputFormat mov_oformat = {
    "mov",
    "mov format",
    NULL,
    "mov",
    sizeof(MOVContext),
    CODEC_ID_PCM_ALAW,
    CODEC_ID_MPEG4,
    mov_write_header,
    mov_write_packet,
    mov_write_trailer,
};

static AVOutputFormat _3gp_oformat = {
    "3gp",
    "3gp format",
    NULL,
    "3gp",
    sizeof(MOVContext),
    CODEC_ID_AMR_NB,
    CODEC_ID_H263,
    mov_write_header,
    mov_write_packet,
    mov_write_trailer,
};

static AVOutputFormat mp4_oformat = {
    "mp4",
    "mp4 format",
    "application/mp4",
    "mp4,m4a",
    sizeof(MOVContext),
    CODEC_ID_AAC,
    CODEC_ID_MPEG4,
    mov_write_header,
    mov_write_packet,
    mov_write_trailer,
};

int movenc_init(void)
{
    av_register_output_format(&mov_oformat);
    av_register_output_format(&_3gp_oformat);
    av_register_output_format(&mp4_oformat);
    return 0;
}
