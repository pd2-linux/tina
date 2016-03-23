#include "CdxMp4Muxer.h"
#include <errno.h>

#define CACHE_ID_VIDEO 1 //same as track ID don't change
#define CACHE_ID_AUDIO 2 //same as track ID don't change
#define WRITE_FREE_TAG 0

#define ByteIOContext CdxStreamT

cdx_int32 put_buffer_cache(ByteIOContext *s, cdx_int8 *buf, cdx_int32 size);
static __inline void put_byte_cache(ByteIOContext *s, cdx_int32 b);
void put_le32_cache(ByteIOContext *s, cdx_uint32 val);
void put_be32_cache(ByteIOContext *s, cdx_uint32 val);
void put_be16_cache(ByteIOContext *s, cdx_uint32 val);
void put_be24_cache(ByteIOContext *s, cdx_uint32 val);
void put_tag_cache(ByteIOContext *s, const char *tag);

static cdx_uint32 movGetStcoTagSize(MOVTrack *track)
{
    cdx_uint32 stcoTagSize = (track->stco_size+4)*4;
    return stcoTagSize;
}

/* Chunk offset atom */
static cdx_int32 mov_write_stco_tag(ByteIOContext *pb, MOVTrack *track)
{
    cdx_uint32 i,j;
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 stcoTagSize = movGetStcoTagSize(track);
    put_be32_cache(pb, stcoTagSize); /* size */
    put_tag_cache(pb, "stco");
    put_be32_cache(pb, 0); /* version & flags */
    put_be32_cache(pb, track->stco_size); /* entry count */

    if(track->stco_tiny_pages == 0)
    {
        for (i=0; i<track->stco_size; i++)
        {
            put_be32_cache(pb,*mov->cache_read_ptr[STCO_ID][track->stream_type]++);
        }
    }
	else
	{
        CdxStreamT *pStcoStream = ((CdxStreamT*)mov->fd_stco[track->stream_type]);
        CdxStreamSeek(pStcoStream, 0, SEEK_SET);
		for (i = 0; i < track->stco_tiny_pages;i++)
		{
            CdxStreamRead(pStcoStream, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
			for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
			{
				put_be32_cache(pb, mov->cache_tiny_page_ptr[j]);
			}
		}

        for (i = track->stco_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i<track->stco_size; i++)
        {
            put_be32_cache(pb, *mov->cache_read_ptr[STCO_ID][track->stream_type]++);
			if(mov->cache_read_ptr[STCO_ID][track->stream_type] > mov->cache_end_ptr[STCO_ID][track->stream_type])
			{
				mov->cache_read_ptr[STCO_ID][track->stream_type] = mov->cache_start_ptr[STCO_ID][track->stream_type];
			}
        }
    }
    
    return stcoTagSize;//updateSize(pb, pos); 
}

static cdx_uint32 movGetStszTagSize(MOVTrack *track)
{
    cdx_uint32 stszTagSize;
    if (track->enc->codec_id == CDX1_CODEC_ID_PCM)
    {
        stszTagSize = 5 * 4;
    }
    else
    {
        stszTagSize = (track->stsz_size  +5) * 4;
    }
    return stszTagSize;
    
}

/* Sample size atom */
static cdx_int32 mov_write_stsz_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 stszTagSize = movGetStszTagSize(track);
    //offset_t pos = url_ftell(pb);
    cdx_uint32 i,j;
    put_be32_cache(pb, stszTagSize); /* size */
    put_tag_cache(pb, "stsz");
    put_be32_cache(pb, 0); /* version & flags */
    if (track->enc->codec_id == CDX1_CODEC_ID_PCM)
    {
        put_be32_cache(pb, track->enc->channels*(track->enc->bits_per_sample>>3)); // sample size
    }
    else
    {
        put_be32_cache(pb, 0); // sample size
    }
    put_be32_cache(pb, track->stsz_size); // sample count
    if (track->enc->codec_id == CDX1_CODEC_ID_PCM)
    {
        return stszTagSize;
    }

	if (track->stsz_tiny_pages == 0)
	{
        for (i=0; i<track->stsz_size; i++)
        {
            put_be32_cache(pb, *mov->cache_read_ptr[STSZ_ID][track->stream_type]++);
        }
    }
	else
	{
        CdxStreamT *pStszStream = ((CdxStreamT*)mov->fd_stsz[track->stream_type]);
        CdxStreamSeek(pStszStream, 0, SEEK_SET);
		for (i = 0; i < track->stsz_tiny_pages; i++)
		{
		    CdxStreamRead(pStszStream, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
			for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
			{
				put_be32_cache(pb,mov->cache_tiny_page_ptr[j]);
			}
		}
		
        for (i = track->stsz_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i < track->stsz_size; i++)
        {
            put_be32_cache(pb, *mov->cache_read_ptr[STSZ_ID][track->stream_type]++);
			if(mov->cache_read_ptr[STSZ_ID][track->stream_type] > mov->cache_end_ptr[STSZ_ID][track->stream_type])
			{
				mov->cache_read_ptr[STSZ_ID][track->stream_type] = mov->cache_start_ptr[STSZ_ID][track->stream_type];
			}
        }
    }
	
    return stszTagSize; //updateSize(pb, pos);
}

static cdx_uint32 movGetStscTagSize(MOVTrack *track)
{
    cdx_uint32 stscTagSize = (track->stsc_size*3 + 4)*4;
    return stscTagSize;
}

/* Sample to chunk atom */
static cdx_int32 mov_write_stsc_tag(ByteIOContext *pb, MOVTrack *track)
{
	cdx_uint32 i;
	CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 stscTagSize = movGetStscTagSize(track);
    //offset_t pos = url_ftell(pb);
    put_be32_cache(pb, stscTagSize); /* size */
    put_tag_cache(pb, "stsc");
    put_be32_cache(pb, 0); // version & flags
    put_be32_cache(pb, track->stsc_size); // entry count

	if (track->stsc_tiny_pages == 0)
	{
        for (i=0; i<track->stsc_size; i++)
        {
			put_be32_cache(pb, i+1); // first chunk
            put_be32_cache(pb,*mov->cache_read_ptr[STSC_ID][track->stream_type]++);
			put_be32_cache(pb, 0x1); // sample description index
        }
    }
	else
	{
		CdxStreamT *pStscStream = ((CdxStreamT*)mov->fd_stsc[track->stream_type]);
        CdxStreamSeek(pStscStream, 0, SEEK_SET);
		for (i = 0; i < track->stsc_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i++)
		{
			put_be32_cache(pb, i+1); // first chunk
			CdxStreamRead(pStscStream, mov->cache_tiny_page_ptr, 4);
			put_be32_cache(pb,*mov->cache_tiny_page_ptr);
			put_be32_cache(pb, 0x1); // sample description index
		}
		
        for (i=track->stsc_tiny_pages*MOV_CACHE_TINY_PAGE_SIZE; i<track->stsc_size; i++)
        {
			put_be32_cache(pb, i+1); // first chunk
            put_be32_cache(pb,*mov->cache_read_ptr[STSC_ID][track->stream_type]++);
			put_be32_cache(pb, 0x1); // sample description index
			if(mov->cache_read_ptr[STSC_ID][track->stream_type] > mov->cache_end_ptr[STSC_ID][track->stream_type])
			{
				mov->cache_read_ptr[STSC_ID][track->stream_type] = mov->cache_start_ptr[STSC_ID][track->stream_type];
			}
        }
    }

    return stscTagSize; //updateSize(pb, pos);
}

static cdx_uint32 movGetStssTagSize(MOVTrack *track)
{
    cdx_int32 keyframes = track->keyFrame_num;
    cdx_uint32 stssTagSize = (keyframes+4)*4;
    return stssTagSize;
}

/* Sync sample atom */
static cdx_int32 mov_write_stss_tag(ByteIOContext *pb, MOVTrack *track)
{
	CedarX1_MOVContext *mov = track->mov;
    cdx_int32 i, keyframes,key_index = 1;
    cdx_uint32 stssTagSize = movGetStssTagSize(track);
	keyframes = track->keyFrame_num;
    put_be32_cache(pb, stssTagSize); // size
    put_tag_cache(pb, "stss");
    put_be32_cache(pb, 0); // version & flags
    put_be32_cache(pb, keyframes); // entry count
    for (i = 0; i < keyframes; i++)
    {
        put_be32_cache(pb, mov->cache_keyframe_ptr[i]);
    }

    return stssTagSize;//updateSize(pb, pos);
}

static cdx_uint32 descrLength(cdx_uint32 len)
{
    cdx_int32 i;
    for(i=1; len >> (7 * i); i++);
    return (len + 1 + i);
}

static void putDescr_cache(ByteIOContext *pb, cdx_int32 tag, cdx_uint32 size)
{
    cdx_int32 i= descrLength(size) - size - 2;
    put_byte_cache(pb, tag);
    for(; i > 0; i--)
    {
        put_byte_cache(pb, (size >> (7*i)) | 0x80);
     }
    put_byte_cache(pb, size & 0x7F);
}

static cdx_uint32 movGetEsdsTagSize(MOVTrack *track)
{
    cdx_uint32 esdsTagSize = 12;
    cdx_uint32 DecoderSpecificInfoSize = track->vosLen ? descrLength(track->vosLen):0;
    cdx_uint32 DecoderConfigDescriptorSize = descrLength(13 + DecoderSpecificInfoSize);
    cdx_uint32 SLConfigDescriptorSize = descrLength(1);
    cdx_uint32 ES_DescriptorSize = descrLength(3 + DecoderConfigDescriptorSize + SLConfigDescriptorSize);
    esdsTagSize += ES_DescriptorSize;
    return esdsTagSize;
}

static cdx_int32 mov_write_esds_tag(ByteIOContext *pb, MOVTrack *track) // Basic
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 esdsTagSize = movGetEsdsTagSize(track);
    cdx_int32 decoderSpecificInfoLen = track->vosLen ? descrLength(track->vosLen):0;
	
    put_be32_cache(pb, esdsTagSize); /* size */
    put_tag_cache(pb, "esds");
    put_be32_cache(pb, 0); // Version
	
    // ES descriptor
    putDescr_cache(pb, 0x03, 3 + descrLength(13 + decoderSpecificInfoLen) +
		descrLength(1));
    put_be16_cache(pb, track->trackID);
    put_byte_cache(pb, 0x00); // flags (= no flags)
	
    // DecoderConfig descriptor
    putDescr_cache(pb, 0x04, 13 + decoderSpecificInfoLen);
	
    
	// Object type indication
    put_byte_cache(pb, track->enc->codec_id);
    	
    // the following fields is made of 6 bits to identify the streamtype (4 for video, 5 for audio)
    // plus 1 bit to indicate upstream and 1 bit set to 1 (reserved)
	if(track->enc->codec_type == CODEC_TYPE_AUDIO)
	{
	    put_byte_cache(pb, 0x15); // flags (= Audiostream)
	}
	else
	{
    	put_byte_cache(pb, 0x11); // flags (= Visualstream)
    }
	
    put_byte_cache(pb,  0);    // Buffersize DB (24 bits)
    put_be16_cache(pb,  0); // Buffersize DB
	
    put_be32_cache(pb, 0); // maxbitrate (FIXME should be max rate in any 1 sec window)
    put_be32_cache(pb, 0); // vbr
	
    if (track->vosLen)
    {
        // DecoderSpecific info descriptor
        putDescr_cache(pb, 0x05, track->vosLen);
        put_buffer_cache(pb, track->vosData, track->vosLen);
    }
	
    // SL descriptor
    putDescr_cache(pb, 0x06, 1);
    put_byte_cache(pb, 0x02);
    //return updateSize(pb, pos);
    return esdsTagSize;
}

//add for h264encoder
#define AV_RB32(x)  ((((const cdx_uint8*)(x))[0] << 24) | \
                     (((const cdx_uint8*)(x))[1] << 16) | \
                     (((const cdx_uint8*)(x))[2] <<  8) | \
                      ((const cdx_uint8*)(x))[3])

static __inline int BSWAP32(unsigned int val)
{
    val= ((val<<8)&0xFF00FF00) | ((val>>8)&0x00FF00FF);
    val= (val>>16) | (val<<16);
	return val;
}

cdx_uint8 *ff_avc_find_startcode(cdx_uint8 *p, cdx_uint8 *end)
{
    cdx_uint8 *a = p + 4 - ((long)p & 3);

    for (end -= 3; p < a && p < end; p++)
    {
        if(p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
            return p;
        }
    }

    for( end -= 3; p < end; p += 4 )
    {
        cdx_uint32 x = *(const cdx_uint32*)p;
        if((x - 0x01010101) & (~x) & 0x80808080)
        { // generic
            if(p[1] == 0)
            {
                if(p[0] == 0 && p[2] == 1)
                {
                    return (p - 1);
                 }
                if(p[2] == 0 && p[3] == 1)
                {
                    return p;
                }
            }
            if( p[3] == 0 )
            {
                if( p[2] == 0 && p[4] == 1 )
                {
                    return (p + 1);
                }
                if( p[4] == 0 && p[5] == 1 )
                {
                    return (p + 2);
                }
            }
        }
    }

    for(end += 3; p < end; p++)
    {
        if(p[0] == 0 && p[1] == 0 && p[2] == 1)
        {
            return p;
        }
    }

    return (end + 3);
}

int ff_avc_parse_nal_units(cdx_uint8 *buf_in, cdx_uint8 **buf, int *size)
{
    cdx_uint8 *p = buf_in,*ptr_t;
    cdx_uint8 *end = p + *size;
    cdx_uint8 *nal_start, *nal_end;
    unsigned int nal_size,nal_size_b;

    ptr_t = *buf = malloc(*size + 256);
    nal_start = ff_avc_find_startcode(p, end);
    while (nal_start < end)
    {
        while (!*(nal_start++));
        nal_end = ff_avc_find_startcode(nal_start, end);
        nal_size = nal_end - nal_start;
        nal_size_b = BSWAP32(nal_size);
        memcpy(ptr_t, &nal_size_b, 4);
        ptr_t += 4;
        memcpy(ptr_t, nal_start, nal_size);
        ptr_t += nal_size;
        nal_start = nal_end;
    }

    *size = ptr_t - *buf;
    return 0;
}

static cdx_uint32 FFIsomGetAvccSize(cdx_uint8 *data, cdx_int32 len)
{
    cdx_uint32 avccSize = 0;
    if (len > 6)
    {
        /* check for h264 start code */
        if (AV_RB32(data) == 0x00000001) 
        {
            cdx_uint8 *buf=NULL, *end, *start;
            cdx_uint32 sps_size=0, pps_size=0;
            cdx_uint8 *sps=0, *pps=0;

            int ret = ff_avc_parse_nal_units(data, &buf, &len);
            if (ret < 0)
            {
                logw("(f:%s, l:%d) fatal error! ret[%d] of ff_avc_parse_nal_units() < 0", __FUNCTION__, __LINE__, ret);
                return 0;
            }
            start = buf;
            end = buf + len;

            /* look for sps and pps */
            while (buf < end) 
            {
                unsigned int size;
                cdx_uint8 nal_type;
                size = AV_RB32(buf);
                nal_type = buf[4] & 0x1f;
                if (nal_type == 7) 
                { /* SPS */
                    sps = buf + 4;
                    sps_size = size;
                } 
                else if (nal_type == 8) 
                { /* PPS */
                    pps = buf + 4;
                    pps_size = size;
                }
                buf += size + 4;
            }
            avccSize = (6 + 2 + sps_size + 1 + 2 + pps_size);
            free(start);
        } 
        else 
        {
            avccSize = len;
        }
    }
    return avccSize;
}

int ff_isom_write_avcc(ByteIOContext *pb, cdx_uint8 *data, int len, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    if (len > 6) {
        /* check for h264 start code */
        if (AV_RB32(data) == 0x00000001)
        {
            cdx_uint8 *buf=NULL, *end, *start;
            cdx_uint32 sps_size=0, pps_size=0;
            cdx_uint8 *sps=0, *pps=0;

            int ret = ff_avc_parse_nal_units(data, &buf, &len);
            if (ret < 0)
            {
                return ret;
            }
            start = buf;
            end = buf + len;

            /* look for sps and pps */
            while (buf < end)
            {
                unsigned int size;
                cdx_uint8 nal_type;
                size = AV_RB32(buf);
                nal_type = buf[4] & 0x1f;
                if (nal_type == 7) 
                { /* SPS */
                    sps = buf + 4;
                    sps_size = size;
                } else if (nal_type == 8) 
                { /* PPS */
                    pps = buf + 4;
                    pps_size = size;
                }
                buf += size + 4;
            }

            put_byte_cache(pb, 1); /* version */
            put_byte_cache(pb, sps[1]); /* profile */
            put_byte_cache(pb, sps[2]); /* profile compat */
            put_byte_cache(pb, sps[3]); /* level */
            put_byte_cache(pb, 0xff); /* 6 bits reserved (111111) + 2 bits nal size length - 1 (11) */
            put_byte_cache(pb, 0xe1); /* 3 bits reserved (111) + 5 bits number of sps (00001) */

            put_be16_cache(pb, sps_size);
            put_buffer_cache(pb, (cdx_int8*)sps, sps_size);
            put_byte_cache(pb, 1); /* number of pps */
            put_be16_cache(pb, pps_size);
            put_buffer_cache(pb, (cdx_int8*)pps, pps_size);
            free(start);
        } 
        else
        {
            put_buffer_cache(pb, (cdx_int8*)data, len);
        }
    }
    return 0;
}

static cdx_uint32 movGetAvccTagSize(MOVTrack *track)
{
    cdx_uint32 avccTagSize = 8;
    avccTagSize += FFIsomGetAvccSize((cdx_uint8*)track->vosData, track->vosLen);
    return avccTagSize;
}
static int mov_write_avcc_tag(ByteIOContext *pb, MOVTrack *track){    
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 avccTagSize = movGetAvccTagSize(track);
    put_be32_cache(pb, avccTagSize);   /* size */
    put_tag_cache(pb, "avcC");    
    ff_isom_write_avcc(pb, (cdx_uint8*)track->vosData, track->vosLen, track);    
    return avccTagSize;
}

static cdx_uint32 movGetAudioTagSize(MOVTrack *track)
{
    cdx_int32 version = 0;
    cdx_uint32 audioTagSize = 16 + 8 + 12;
    if(track->enc->codec_id == CDX1_CODEC_ID_ADPCM)
    {
        version = 1;
    }
    if(version == 1) 
    { /* SoundDescription V1 extended info */
        audioTagSize += 16;
    }
    if(track->tag == MKTAG('m','p','4','a'))
    {
        audioTagSize += movGetEsdsTagSize(track);
    }

    return audioTagSize;
}

static cdx_int32 mov_write_audio_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 audioTagSize = movGetAudioTagSize(track);
    //offset_t pos = url_ftell(pb);
    cdx_int32 version = 0;

    if(track->enc->codec_id == CDX1_CODEC_ID_ADPCM)
    {
        version = 1;
    }
	
    put_be32_cache(pb, audioTagSize); /* size */
    put_le32_cache(pb, track->tag); // store it byteswapped
    put_be32_cache(pb, 0); /* Reserved */
    put_be16_cache(pb, 0); /* Reserved */
    put_be16_cache(pb, 1); /* Data-reference index, XXX  == 1 */
	
    /* SoundDescription */
    put_be16_cache(pb, version); /* Version */
    put_be16_cache(pb, 0); /* Revision level */
    put_be32_cache(pb, 0); /* vendor */
	
    { /* reserved for mp4/3gp */
        put_be16_cache(pb, track->enc->channels); //channel
        put_be16_cache(pb, track->enc->bits_per_sample);//bits per sample
        put_be16_cache(pb, 0); /* compression id = 0*/
    }
	
    put_be16_cache(pb, 0); /* packet size (= 0) */
    put_be16_cache(pb, track->enc->sample_rate); /* Time scale !!!??? */
    put_be16_cache(pb, 0); /* Reserved */
	
    if(version == 1) 
    { /* SoundDescription V1 extended info */
        if(track->enc->codec_id == CDX1_CODEC_ID_ADPCM)
        {
            put_be32_cache(pb, track->enc->frame_size); /* Samples per packet */
            put_be32_cache(pb, track->enc->frame_size*(track->enc->bits_per_sample>>3)); /* Bytes per packet */
            put_be32_cache(pb, track->enc->frame_size*(track->enc->bits_per_sample>>3)*track->enc->channels); /* Bytes per frame */
            put_be32_cache(pb, 2); /* Bytes per sample */
        }
        else
        {
            put_be32_cache(pb, track->enc->frame_size); /* Samples per packet */
            put_be32_cache(pb, track->sampleSize / track->enc->channels); /* Bytes per packet */
            put_be32_cache(pb, track->sampleSize); /* Bytes per frame */
            put_be32_cache(pb, 2); /* Bytes per sample */
        }
    }
	
    if(track->tag == MKTAG('m','p','4','a'))
    {
        mov_write_esds_tag(pb, track);
    }
	
    //return updateSize(pb, pos);
    return audioTagSize;
}

static cdx_int32 mov_find_codec_tag(MOVTrack *track)
{
    cdx_int32 tag = track->enc->codec_tag;
	
	switch(track->enc->codec_id)
	{
	case CDX1_CODEC_ID_H264:
		tag = MKTAG('a','v','c','1');
		break;
	case CDX1_CODEC_ID_MPEG4:
		tag = MKTAG('m','p','4','v');
		break;
	case CDX1_CODEC_ID_AAC:	
		tag = MKTAG('m','p','4','a');
		break;
    case CDX1_CODEC_ID_PCM:
	    tag = MKTAG('s','o','w','t');
		break;
	case CDX1_CODEC_ID_MP3:
	    tag = MKTAG('.','m','p','3');
		break;
	case CDX1_CODEC_ID_ADPCM:
	    tag = MKTAG('m','s',0x00,0x11);
		break;
	case CDX1_CODEC_ID_MJPEG:  /* gushiming compressed source */
	    tag = MKTAG('m','j','p','a');  /* Motion-JPEG (format A) */
		break;
	default:
		break;
	}
	
    return tag;
}

static cdx_uint32 movGetVideoTagSize(MOVTrack *track)
{
    cdx_uint32 videoTagSize = 16 + 4 + 12 + 18 + 32 + 4;
    if (track->tag == MKTAG('a','v','c','1'))
    {
        videoTagSize += movGetAvccTagSize(track);
    }
    else if (track->tag == MKTAG('m','p','4','v'))
    {
        videoTagSize += movGetEsdsTagSize(track);
    }
    return videoTagSize;
}

static cdx_int32 mov_write_video_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 videoTagSize = movGetVideoTagSize(track);
    //offset_t pos = url_ftell(pb);
    cdx_int8 compressor_name[32];
	
    put_be32_cache(pb, videoTagSize); /* size */
    put_le32_cache(pb, track->tag); // store it byteswapped
    put_be32_cache(pb, 0); /* Reserved */
    put_be16_cache(pb, 0); /* Reserved */
    put_be16_cache(pb, 1); /* Data-reference index */
	
    put_be16_cache(pb, 0); /* Codec stream_writer version */
    put_be16_cache(pb, 0); /* Codec stream_writer revision (=0) */
    {
        put_be32_cache(pb, 0); /* Reserved */
        put_be32_cache(pb, 0); /* Reserved */
        put_be32_cache(pb, 0); /* Reserved */
    }
    put_be16_cache(pb, track->enc->width); /* Video width */
    put_be16_cache(pb, track->enc->height); /* Video height */
    put_be32_cache(pb, 0x00480000); /* Horizontal resolution 72dpi */
    put_be32_cache(pb, 0x00480000); /* Vertical resolution 72dpi */
    put_be32_cache(pb, 0); /* Data size (= 0) */
    put_be16_cache(pb, 1); /* Frame count (= 1) */
	
    memset(compressor_name,0,32);
    put_byte_cache(pb, strlen((const char *)compressor_name));
    put_buffer_cache(pb, compressor_name, 31);
	
    put_be16_cache(pb, 0x18); /* Reserved */
    put_be16_cache(pb, 0xffff); /* Reserved */
    if(track->tag == MKTAG('a','v','c','1'))
    {
        mov_write_avcc_tag(pb, track);
    }
    else if(track->tag == MKTAG('m','p','4','v'))
    {
        mov_write_esds_tag(pb, track);
    }
	
    //return updateSize(pb, pos);
    return videoTagSize;
}

static cdx_uint32 movGetStsdTagSize(MOVTrack *track)
{
    cdx_uint32 stsdTagSize = 16;
    if (track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        stsdTagSize += movGetVideoTagSize(track);
    }
    else if (track->enc->codec_type == CODEC_TYPE_AUDIO)
    {
        stsdTagSize += movGetAudioTagSize(track);
    }
    return stsdTagSize;
}

static cdx_int32 mov_write_stsd_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 stsdTagSize = movGetStsdTagSize(track);
    //offset_t pos = url_ftell(pb);
    put_be32_cache(pb, stsdTagSize); /* size */
    put_tag_cache(pb, "stsd");
    put_be32_cache(pb, 0); /* version & flags */
    put_be32_cache(pb, 1); /* entry count */
    if (track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        mov_write_video_tag(pb, track);
    }
    else if (track->enc->codec_type == CODEC_TYPE_AUDIO)
    {
        mov_write_audio_tag(pb, track);
    }
    
    return stsdTagSize;
}

static cdx_uint32 movGetSttsTagSize(MOVTrack *track)
{
    MOV_stts_t stts_entries[1];
    cdx_uint32 entries = 0;
    cdx_uint32 atom_size;
    if (track->enc->codec_type == CODEC_TYPE_AUDIO) 
    {
        entries = 1;
        atom_size = 16 + (entries * 8);
    } 
    else 
    {
        entries = track->stts_size;
        atom_size = 16 + (entries * 8);
    }
    return atom_size;
}

static cdx_int32 mov_write_stts_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    MOV_stts_t stts_entries[1];
    cdx_uint32 entries = 0;
    cdx_uint32 atom_size;
    cdx_uint32 i,j;
    cdx_uint32 pos;
	
    if (track->enc->codec_type == CODEC_TYPE_AUDIO)
    {
        stts_entries[0].count = track->stsz_size;
        if(track->enc->codec_id == CDX1_CODEC_ID_PCM)    //for uncompressed audio, one frame == one sample.
        {
            stts_entries[0].duration = 1;
        }
        else
        {
            stts_entries[0].duration = track->enc->frame_size;
        }
        entries = 1;
        atom_size = 16 + (entries * 8);
        put_be32_cache(pb, atom_size); /* size */
        put_tag_cache(pb, "stts");
        put_be32_cache(pb, 0); /* version & flags */
        put_be32_cache(pb, entries); /* entry count */
        for (i=0; i<entries; i++)
        {
            put_be32_cache(pb, stts_entries[i].count);
            put_be32_cache(pb, stts_entries[i].duration);
        }
    } 
    else
    {
        cdx_int32 skip_first_frame = 1;
        
        entries = track->stts_size;
        atom_size = 16 + (entries * 8);
        put_be32_cache(pb, atom_size); /* size */
        put_tag_cache(pb, "stts");
        put_be32_cache(pb, 0); /* version & flags */
        put_be32_cache(pb, entries); /* entry count */

        if(track->stts_tiny_pages == 0)
        {
            for (i=0; i<track->stts_size; i++)
            {
                if(skip_first_frame)
                {
                    skip_first_frame = 0;
                    mov->cache_read_ptr[STTS_ID][track->stream_type]++;
                    continue;
                }
                put_be32_cache(pb, 1);//count
                put_be32_cache(pb,*mov->cache_read_ptr[STTS_ID][track->stream_type]++);
            }
        }
    	else
    	{
            CdxStreamT *pSttsStream = ((CdxStreamT*)mov->fd_stts[track->stream_type]);
            CdxStreamSeek(pSttsStream, 0, SEEK_SET);
            pos = CdxStreamTell(pSttsStream);
            logv("the position is :%d\n",pos);
    		for (i = 0; i < track->stts_tiny_pages; i++)
    		{
    		    CdxStreamRead(pSttsStream, mov->cache_tiny_page_ptr, MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
    			for (j = 0;j < MOV_CACHE_TINY_PAGE_SIZE; j++)
    			{
    			    if(skip_first_frame)
                    {
                        skip_first_frame = 0;
                        continue;
                    }
    			    put_be32_cache(pb, 1);//count
    				put_be32_cache(pb,mov->cache_tiny_page_ptr[j]);
    			}
    		}
    		
            for (i = track->stts_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE; i < track->stts_size; i++) {
                put_be32_cache(pb, 1);//count
                put_be32_cache(pb,*mov->cache_read_ptr[STTS_ID][track->stream_type]++);
    			if(mov->cache_read_ptr[STTS_ID][track->stream_type] > mov->cache_end_ptr[STTS_ID][track->stream_type])
    			{
    				mov->cache_read_ptr[STTS_ID][track->stream_type] = mov->cache_start_ptr[STTS_ID][track->stream_type];
    			}
            }
        }

        //write last packet duration, set it to 0??
        put_be32_cache(pb, 1);//count
        put_be32_cache(pb, 0);
    }

    return atom_size;
}

static cdx_uint32 movGetDrefTagSize()
{
    return 28;
}

static cdx_int32 mov_write_dref_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 drefTagSize = movGetDrefTagSize();
    put_be32_cache(pb, drefTagSize); /* size */
    put_tag_cache(pb, "dref");
    put_be32_cache(pb, 0); /* version & flags */
    put_be32_cache(pb, 1); /* entry count */
	
    put_be32_cache(pb, 0xc); /* size */
    put_tag_cache(pb, "url ");
    put_be32_cache(pb, 1); /* version & flags */
	
    return drefTagSize;
}

static cdx_uint32 movGetStblTagSize(MOVTrack *track)
{
    cdx_uint32 stblTagSize = 8;
    stblTagSize += movGetStsdTagSize(track);
    stblTagSize += movGetSttsTagSize(track);
    if (track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        stblTagSize += movGetStssTagSize(track);
    }
    stblTagSize += movGetStscTagSize(track);
    stblTagSize += movGetStszTagSize(track);
    stblTagSize += movGetStcoTagSize(track);
    return stblTagSize;
}

static cdx_int32 mov_write_stbl_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 stblTagSize = movGetStblTagSize(track);
    //offset_t pos = url_ftell(pb);
    put_be32_cache(pb, stblTagSize); /* size */
    put_tag_cache(pb, "stbl");
    mov_write_stsd_tag(pb, track);
    mov_write_stts_tag(pb, track);
    if (track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        mov_write_stss_tag(pb, track);
    }
    mov_write_stsc_tag(pb, track);
    mov_write_stsz_tag(pb, track);
    mov_write_stco_tag(pb, track);

    return stblTagSize;
}

static cdx_uint32 movGetDinfTagSize()
{
    cdx_uint32 dinfTagSize = 8;
    dinfTagSize += movGetDrefTagSize();
    return dinfTagSize;
}

static cdx_int32 mov_write_dinf_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 dinfTagSize = movGetDinfTagSize();
    put_be32_cache(pb, dinfTagSize); /* size */
    put_tag_cache(pb, "dinf");
    mov_write_dref_tag(pb, track);
    return dinfTagSize;
}

static cdx_uint32 movGetSmhdTagSize()
{
    return 16;
}
static cdx_int32 mov_write_smhd_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 smhdTagSize = movGetSmhdTagSize();
    put_be32_cache(pb, smhdTagSize); /* size */
    put_tag_cache(pb, "smhd");
    put_be32_cache(pb, 0); /* version & flags */
    put_be16_cache(pb, 0); /* reserved (balance, normally = 0) */
    put_be16_cache(pb, 0); /* reserved */
    return smhdTagSize;
}

static cdx_uint32 movGetVmhdTagSize()
{
    return 0x14;
}

static cdx_int32 mov_write_vmhd_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 vmhdTagSize = movGetVmhdTagSize();
    put_be32_cache(pb, vmhdTagSize); /* size (always 0x14) */
    put_tag_cache(pb, "vmhd");
    put_be32_cache(pb, 0x01); /* version & flags */
	put_be32_cache(pb, 0x0);
	put_be32_cache(pb, 0x0);
    return vmhdTagSize;
}

static cdx_uint32 movGetHdlrTagSize(MOVTrack *track)
{
    cdx_uint32 hdlrTagSize = 32 + 1;
	
    if (!track) 
    { /* no media --> data handler */
        hdlrTagSize += strlen("DataHandler");
    } 
    else 
    {
        if (track->enc->codec_type == CODEC_TYPE_VIDEO) 
        {
            hdlrTagSize += strlen("VideoHandler");
        } 
        else 
        {
            hdlrTagSize += strlen("SoundHandler");
        }
    }
    return hdlrTagSize;
}

static cdx_int32 mov_write_hdlr_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    char *descr, *hdlr, *hdlr_type;
    //offset_t pos = url_ftell(pb);
	
    if (!track)
    { /* no media --> data handler */
        hdlr = "dhlr";
        hdlr_type = "url ";
        descr = "DataHandler";
    }
    else
    {
        hdlr = "\0\0\0\0";
        if (track->enc->codec_type == CODEC_TYPE_VIDEO)
        {
            hdlr_type = "vide";
            descr = "VideoHandler";
        }
        else
        {
            hdlr_type = "soun";
            descr = "SoundHandler";
        }
    }
	cdx_uint32 hdlrTagSize = movGetHdlrTagSize(track);
    put_be32_cache(pb, hdlrTagSize); /* size */
    put_tag_cache(pb, "hdlr");
    put_be32_cache(pb, 0); /* Version & flags */
    put_buffer_cache(pb, (cdx_int8*)hdlr, 4); /* handler */
    put_tag_cache(pb, hdlr_type); /* handler type */
    put_be32_cache(pb ,0); /* reserved */
    put_be32_cache(pb ,0); /* reserved */
    put_be32_cache(pb ,0); /* reserved */
    put_byte_cache(pb, strlen((const char *)descr)); /* string counter */
    put_buffer_cache(pb, (cdx_int8*)descr, strlen((const char *)descr)); /* handler description */

    return hdlrTagSize;
}

static cdx_uint32 movGetMinfTagSize(MOVTrack *track)
{
    cdx_uint32 minfTagSize = 8;
    if(track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        minfTagSize += movGetVmhdTagSize();
    }
    else
    {
        minfTagSize += movGetSmhdTagSize();
    }

    minfTagSize += movGetDinfTagSize();
    minfTagSize += movGetStblTagSize(track);
    return minfTagSize;
}

static cdx_int32 mov_write_minf_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 minfTagSize = movGetMinfTagSize(track);
    put_be32_cache(pb, minfTagSize); /* size */
    put_tag_cache(pb, "minf");
    if(track->enc->codec_type == CODEC_TYPE_VIDEO)
    {
        mov_write_vmhd_tag(pb, track);
    }
    else
    {
        mov_write_smhd_tag(pb, track);
    }
    mov_write_dinf_tag(pb, track);
    mov_write_stbl_tag(pb, track);

    return minfTagSize;
}

static cdx_uint32 movGetMdhdTagSize()
{
    return 0x20;
}

static cdx_int32 mov_write_mdhd_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
	cdx_uint32 mdhdTagSize = movGetMdhdTagSize();
    put_be32_cache(pb, mdhdTagSize); /* size */
    put_tag_cache(pb, "mdhd");
    put_byte_cache(pb, 0);
    put_be24_cache(pb, 0); /* flags */
    
    put_be32_cache(pb, track->time); /* creation time */
    put_be32_cache(pb, track->time); /* modification time */
    put_be32_cache(pb, track->timescale); /* time scale (sample rate for audio) */
    put_be32_cache(pb, track->trackDuration); /* duration */
    put_be16_cache(pb, /*track->language*/0); /* language */
    put_be16_cache(pb, 0); /* reserved (quality) */
	
    return mdhdTagSize;
}

static cdx_uint32 movGetMdiaTagSize(MOVTrack *track)
{
    cdx_uint32 mdiaTagSize = 8;
    mdiaTagSize += movGetMdhdTagSize();
    mdiaTagSize += movGetHdlrTagSize(track);
    mdiaTagSize += movGetMinfTagSize(track);
    return mdiaTagSize;
}
static cdx_int32 mov_write_mdia_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 mdiaTagSize = movGetMdiaTagSize(track);

    put_be32_cache(pb, mdiaTagSize); /* size */
    put_tag_cache(pb, "mdia");
    mov_write_mdhd_tag(pb, track);
    mov_write_hdlr_tag(pb, track);
    mov_write_minf_tag(pb, track);

    return mdiaTagSize;
}

static cdx_int32 av_rescale_rnd(cdx_int64 a, cdx_int64 b, cdx_int64 c)
{
    return (a * b + c - 1) / c;
}

static cdx_uint32 movGetTkhdTagSize()
{
    return 0x5c;
}

static cdx_int32 mov_write_tkhd_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_int64 duration = av_rescale_rnd(track->trackDuration, globalTimescale, track->timescale);
    cdx_int32 version = 0;
	cdx_uint32 tkhdTagSize = movGetTkhdTagSize();
    put_be32_cache(pb, tkhdTagSize); /* size */
    put_tag_cache(pb, "tkhd");
    put_byte_cache(pb, version);
    put_be24_cache(pb, 0xf); /* flags (track enabled) */
    
    put_be32_cache(pb, track->time); /* creation time */
    put_be32_cache(pb, track->time); /* modification time */
    
    put_be32_cache(pb, track->trackID); /* track-id */
    put_be32_cache(pb, 0); /* reserved */
    put_be32_cache(pb, duration);
	
    put_be32_cache(pb, 0); /* reserved */
    put_be32_cache(pb, 0); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved (Layer & Alternate group) */
    /* Volume, only for audio */
    if(track->enc->codec_type == CODEC_TYPE_AUDIO)
    {
        put_be16_cache(pb, 0x0100);
    }
    else
    {
        put_be16_cache(pb, 0);
    }
    put_be16_cache(pb, 0); /* reserved */
	
    {
    	int degrees = track->enc->rotate_degree;
        cdx_uint32 a = 0x00010000;
        cdx_uint32 b = 0;
        cdx_uint32 c = 0;
        cdx_uint32 d = 0x00010000;
        switch (degrees)
        {
            case 0:
                break;
            case 90:
                a = 0;
                b = 0x00010000;
                c = 0xFFFF0000;
                d = 0;
                break;
            case 180:
                a = 0xFFFF0000;
                d = 0xFFFF0000;
                break;
            case 270:
                a = 0;
                b = 0xFFFF0000;
                c = 0x00010000;
                d = 0;
                break;
            default:
                loge("Should never reach this unknown rotation");
                break;
        }

        put_be32_cache(pb, a);           // a
        put_be32_cache(pb, b);           // b
        put_be32_cache(pb, 0);           // u
        put_be32_cache(pb, c);           // c
        put_be32_cache(pb, d);           // d
        put_be32_cache(pb, 0);           // v
        put_be32_cache(pb, 0);           // x
        put_be32_cache(pb, 0);           // y
        put_be32_cache(pb, 0x40000000);  // w
    }

    /* Track width and height, for visual only */
    if(track->enc->codec_type == CODEC_TYPE_VIDEO) 
    {
		put_be32_cache(pb, track->enc->width*0x10000);
        put_be32_cache(pb, track->enc->height*0x10000);
    }
    else {
        put_be32_cache(pb, 0);
        put_be32_cache(pb, 0);
    }
    return tkhdTagSize;
}

static cdx_uint32 movGetTrakTagSize(MOVTrack *track)
{
    cdx_uint32 trakTagSize = 8;
    trakTagSize += movGetTkhdTagSize();
    trakTagSize += movGetMdiaTagSize(track);
    return trakTagSize;
}

static cdx_int32 mov_write_trak_tag(ByteIOContext *pb, MOVTrack *track)
{
    CedarX1_MOVContext *mov = track->mov;
    cdx_uint32 trakTagSize = movGetTrakTagSize(track);

    put_be32_cache(pb, trakTagSize); /* size */
    put_tag_cache(pb, "trak");
    mov_write_tkhd_tag(pb, track);
	
    mov_write_mdia_tag(pb, track);

    return trakTagSize;
}

static cdx_uint32 movGetMvhdTagSize()
{
    return 108;
}

static cdx_int32 mov_write_mvhd_tag(ByteIOContext *pb, CedarX1_MOVContext *mov)
{
    cdx_int32 maxTrackID = 1,i;

	for (i=0; i<2/*mov->nb_streams*/; i++)
	{
        if(maxTrackID < mov->tracks[i].trackID)
        {
            maxTrackID = mov->tracks[i].trackID;
        }
    }
    cdx_uint32 mvhdTagSize = movGetMvhdTagSize();
	put_be32_cache(pb, mvhdTagSize); /* size */
    put_tag_cache(pb, "mvhd");
    put_byte_cache(pb, 0);
    put_be24_cache(pb, 0); /* flags */
	
    put_be32_cache(pb, mov->create_time); /* creation time */
    put_be32_cache(pb, mov->create_time); /* modification time */
    put_be32_cache(pb, mov->timescale); /* timescale */
    put_be32_cache(pb, mov->tracks[0].trackDuration); /* duration of longest track */
	
    put_be32_cache(pb, 0x00010000); /* reserved (preferred rate) 1.0 = normal */
    put_be16_cache(pb, 0x0100); /* reserved (preferred volume) 1.0 = normal */
    put_be16_cache(pb, 0); /* reserved */
    put_be32_cache(pb, 0); /* reserved */
    put_be32_cache(pb, 0); /* reserved */
	
    /* Matrix structure */
    put_be32_cache(pb, 0x00010000); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x00010000); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x0); /* reserved */
    put_be32_cache(pb, 0x40000000); /* reserved */
	
    put_be32_cache(pb, 0); /* reserved (preview time) */
    put_be32_cache(pb, 0); /* reserved (preview duration) */
    put_be32_cache(pb, 0); /* reserved (poster time) */
    put_be32_cache(pb, 0); /* reserved (selection time) */
    put_be32_cache(pb, 0); /* reserved (selection duration) */
    put_be32_cache(pb, 0); /* reserved (current time) */
    put_be32_cache(pb, maxTrackID + 1); /* Next track id */
    return mvhdTagSize;
}

static void writeLatitude(ByteIOContext *pb, int degreex10000) {
    int isNegative = (degreex10000 < 0);
    char sign = isNegative? '-': '+';

    // Handle the whole part
    char str[9];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0)
    {
        snprintf(str, 5, "%c%.2d.", sign, wholePart);
    }
    else
    {
        snprintf(str, 5, "%+.2d.", wholePart);
    }

    // Handle the fractional part
    int fractionalPart = degreex10000 - (wholePart * 10000);
    if (fractionalPart < 0)
    {
        fractionalPart = -fractionalPart;
    }
    snprintf(&str[4], 5, "%.4d", fractionalPart);

    // Do not write the null terminator
	put_buffer_cache(pb, (cdx_int8 *)str, 8);
}

// Written in +/- DDD.DDDD format
static void writeLongitude(ByteIOContext *pb, int degreex10000) {
    int isNegative = (degreex10000 < 0);
    char sign = isNegative? '-': '+';

    // Handle the whole part
    char str[10];
    int wholePart = degreex10000 / 10000;
    if (wholePart == 0)
    {
        snprintf(str, 6, "%c%.3d.", sign, wholePart);
    }
    else
    {
        snprintf(str, 6, "%+.3d.", wholePart);
    }

    // Handle the fractional part
    int fractionalPart = degreex10000 - (wholePart * 10000);
    if (fractionalPart < 0)
    {
        fractionalPart = -fractionalPart;
    }
    snprintf(&str[5], 5, "%.4d", fractionalPart);

    // Do not write the null terminator
	put_buffer_cache(pb, (cdx_int8 *)str, 9);
}

static cdx_uint32 movGetUdtaTagSize()
{
    cdx_uint32 size = 4+4+4+4  //(be32_cache, tag_cache, be32_cache, tag_cache)
                +4+8+9+1;      //(0x001215c7, latitude, longitude, 0x2F)
    return size;
}

static cdx_int32 mov_write_udta_tag(ByteIOContext *pb, CedarX1_MOVContext *mov)
{
    cdx_uint32 udtaTagSize = movGetUdtaTagSize();
	
    put_be32_cache(pb, udtaTagSize); /* size */

    put_tag_cache(pb, "udta");

	put_be32_cache(pb, udtaTagSize-8);
	put_tag_cache(pb, "\xA9xyz");

	/*
	 * For historical reasons, any user data start
	 * with "\0xA9", must be followed by its assoicated
	 * language code.
	 * 0x0012: text string length
	 * 0x15c7: lang (locale) code: en
	 */
	put_be32_cache(pb, 0x001215c7);
	
	writeLatitude(pb, mov->latitudex10000);
	writeLongitude(pb, mov->longitudex10000);
	put_byte_cache(pb, 0x2F);

    return udtaTagSize;
}

static cdx_uint32 movGetMoovTagSize(CedarX1_MOVContext *mov)
{
    cdx_int32 i;
    cdx_uint32 size = 0;
    size += 8;  //size, "moov" tag,
    if(mov->geo_available)
    {
        size += movGetUdtaTagSize();
    }
    size += movGetMvhdTagSize();
    for (i=0; i<2/*mov->nb_streams*/; i++) 
    {
        if(mov->tracks[i].stsz_size> 0) 
        {
            size += movGetTrakTagSize(&(mov->tracks[i]));
        }
    }
    return size;
}

static cdx_int32 mov_write_free_tag(ByteIOContext *pb, cdx_int32 size)
{
    put_be32_cache(pb, size);
    put_tag_cache(pb, "free");
    return size;
}

static cdx_int32 mov_write_moov_tag(ByteIOContext *pb, CedarX1_MOVContext *mov)
{
    cdx_int32 i;
    cdx_int32 moov_size;

    moov_size = movGetMoovTagSize(mov);
#if WRITE_FREE_TAG
    if (moov_size + 28 + 8 <= MOV_HEADER_RESERVE_SIZE)
    {
        CdxStreamSeek(pb, mov->free_pos, SEEK_SET);
    }
#endif
    put_be32_cache(pb, moov_size); /* size placeholder*/
    put_tag_cache(pb, "moov");
    mov->timescale = globalTimescale;

	if(mov->geo_available)
	{
		mov_write_udta_tag(pb, mov);
	}
	
    for (i = 0; i < 2/*mov->nb_streams*/; i++)
    {
        mov->tracks[i].time = mov->create_time;
        mov->tracks[i].trackID = i+1;
        mov->tracks[i].mov = mov;
		mov->tracks[i].stream_type = i;
    }

    mov_write_mvhd_tag(pb, mov);
    for (i = 0; i < 2/*mov->nb_streams*/; i++)
    {
        if(mov->tracks[i].stsz_size> 0)
        {
            mov_write_trak_tag(pb, &(mov->tracks[i]));
        }
    }
#if WRITE_FREE_TAG
    if (moov_size + 28 + 8 <= MOV_HEADER_RESERVE_SIZE)
    {
        mov_write_free_tag(pb, MOV_HEADER_RESERVE_SIZE - moov_size - 28);
    }
#endif	
    return moov_size;
}

static __inline void put_byte_cache(ByteIOContext *s, cdx_int32 b)
{
    put_buffer_cache(s, (cdx_int8*)(&b), 1);
}

cdx_int32 put_buffer_cache(ByteIOContext *s, cdx_int8 *buf, cdx_int32 size)
{
    return CdxStreamWrite(s, (cdx_uint8*)buf, size);
}

void put_le32_cache(ByteIOContext *s, cdx_uint32 val)
{
	put_buffer_cache(s, (cdx_int8*)&val, 4);
}

void put_be32_cache(ByteIOContext *s, cdx_uint32 val)
{
	val= ((val<<8)&0xFF00FF00) | ((val>>8)&0x00FF00FF);
    val= (val>>16) | (val<<16);
    put_buffer_cache(s, (cdx_int8*)&val, 4);
}

void put_be16_cache(ByteIOContext *s, cdx_uint32 val)
{
    put_byte_cache(s, val >> 8);
    put_byte_cache(s, val);
}

void put_be24_cache(ByteIOContext *s, cdx_uint32 val)
{
    put_be16_cache(s, val >> 8);
    put_byte_cache(s, val);
}

void put_tag_cache(ByteIOContext *s, const char *tag)
{
    while (*tag)
    {
        put_byte_cache(s, *tag++);
    }
}

static cdx_int32 mov_write_mdat_tag(ByteIOContext *pb, CedarX1_MOVContext *mov)
{
#if WRITE_FREE_TAG
    mov->mdat_pos = MOV_HEADER_RESERVE_SIZE; //url_ftell(pb);
	CdxStreamSeek(pb, mov->mdat_pos, SEEK_SET);
#else
    mov->mdat_pos = 28;
#endif
	mov->mdat_start_pos = mov->mdat_pos + 8;
	   
    
    put_be32_cache(pb, 0); /* size placeholder */
    put_tag_cache(pb, "mdat");
    return 0;
}

static cdx_int32 mov_write_ftyp_tag(ByteIOContext *pb, Mp4MuxContext *s)
{
    cdx_int32 minor = 0x200;
	CedarX1_MOVContext *mov = s->priv_data;

    put_be32_cache(pb, 28); /* size */
    put_tag_cache(pb, "ftyp");
    put_tag_cache(pb, "isom");
    put_be32_cache(pb, minor);
	
    put_tag_cache(pb, "isom");
    put_tag_cache(pb, "iso2");
	
    put_tag_cache(pb, "mp41");

    return 0;//updateSize(pb, pos);
}

cdx_int32 mov_write_header(Mp4MuxContext* s)
{
    ByteIOContext *pb = s->stream_writer;
    CedarX1_MOVContext *mov = s->priv_data;
    cdx_int32 i;
	
    mov_write_ftyp_tag(pb,s);
	
    for(i=0; i<2/*s->nb_streams*/; i++){
        CedarX1_AVStream *st= s->streams[i];
        MOVTrack *track= &mov->tracks[i];
		
        track->enc = &st->codec;
        track->tag = mov_find_codec_tag(track);
    }
#if WRITE_FREE_TAG
    mov_write_free_tag(pb, MOV_HEADER_RESERVE_SIZE - 28);
#endif
    mov_write_mdat_tag(pb, mov);

    mov->nb_streams = s->nb_streams;
    mov->free_pos = 28;

    return 0;
}

static void printCacheSize(CedarX1_MOVContext *mov)
{
    cdx_int32 i;
    for (i = 0;i < 2; i++)
    {
        logd("mov->stsc_cache_size[%d] = [%d]kB", i, mov->stsc_cache_size[i] / 1024);
        logd("mov->stco_cache_size[%d] = [%d]kB", i, mov->stco_cache_size[i] / 1024);
        logd("mov->stsz_cache_size[%d] = [%d]kB", i, mov->stsz_cache_size[i] / 1024);
        logd("mov->stts_cache_size[%d] = [%d]kB", i, mov->stts_cache_size[i] / 1024);
    }
}

cdx_int32 mov_write_packet(Mp4MuxContext *s, CdxMuxerPacketT *pkt)
{
    CedarX1_MOVContext *mov = s->priv_data;
    ByteIOContext *pb = s->stream_writer;
    MOVTrack *trk = &mov->tracks[pkt->streamIndex];
    cdx_int32 size = pkt->buflen;
    cdx_int8  *pkt_buf = (cdx_int8*)(pkt->buf);
    cdx_uint8 tmpStrmByte;
    cdx_uint8 tmpMjepgTrailer[2];
	long long last_time, now_time, write_time;
   
	if (NULL == mov || NULL == s->stream_writer)
	{
		logw("in param is null\n");
		return -1;
	}
	
	if (pkt->streamIndex == -1)//last packet
	{
		*mov->cache_write_ptr[STSC_ID][mov->last_stream_index]++ = mov->stsc_cnt;
		mov->tracks[mov->last_stream_index].stsc_size++;
		//!!!! add protection
		return 0;
	}

	if (s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_MJPEG)
	{  /* gushiming compressed source */
		size += 2;
	}
	
	//writeCallbackPacket(s, pkt);
    if(pkt->streamIndex != mov->last_stream_index)
    {
		*mov->cache_write_ptr[STCO_ID][pkt->streamIndex]++ = mov->mdat_size + mov->mdat_start_pos;
		trk->stco_size++;
		mov->stco_cache_size[pkt->streamIndex]++;

		if(mov->last_stream_index >= 0)
		{
			*mov->cache_write_ptr[STSC_ID][mov->last_stream_index]++ = mov->stsc_cnt;
			mov->tracks[mov->last_stream_index].stsc_size++;
			mov->stsc_cache_size[mov->last_stream_index]++;

			if(mov->cache_write_ptr[STSC_ID][mov->last_stream_index] > mov->cache_end_ptr[STSC_ID][mov->last_stream_index])
			{
				mov->cache_write_ptr[STSC_ID][mov->last_stream_index] = mov->cache_start_ptr[STSC_ID][mov->last_stream_index];
			}

			if(mov->stsc_cache_size[mov->last_stream_index] >= STSC_CACHE_SIZE)
			{
				cdx_int32 ret;
				CdxStreamT *pStscStream = NULL;
				
                if(!mov->fd_stsz[0])
                {
                    logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!", __FUNCTION__, __LINE__, mov->last_stream_index);
                    //not create mp4 tmp file, create now.
                    if(0!=movCreateTmpFile(mov))
                    {
                        loge("(f:%s, l:%d) fatal error! movCreateTmpFile() fail!", __FUNCTION__, __LINE__);
						return -1;
                    }                    
                }
				
                pStscStream = (CdxStreamT*)mov->fd_stsc[mov->last_stream_index];
				CdxStreamSeek(pStscStream, (trk->stsc_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
                ret = CdxStreamWrite(pStscStream, mov->cache_read_ptr[STSC_ID][mov->last_stream_index], MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
				if(ret < 0)
				{
                    loge("(f:%s, l:%d) write STSC failed\n", __FUNCTION__, __LINE__);
					return -1;
			    }
                
				trk->stsc_tiny_pages++;
				mov->stsc_cache_size[mov->last_stream_index] -= MOV_CACHE_TINY_PAGE_SIZE;
				mov->cache_read_ptr[STSC_ID][mov->last_stream_index] += MOV_CACHE_TINY_PAGE_SIZE;
				if(mov->cache_read_ptr[STSC_ID][mov->last_stream_index] > mov->cache_end_ptr[STSC_ID][mov->last_stream_index])
				{
					mov->cache_read_ptr[STSC_ID][mov->last_stream_index] = mov->cache_start_ptr[STSC_ID][mov->last_stream_index];
				}
			}
		}
		mov->stsc_cnt = 0;

        if(mov->cache_write_ptr[STCO_ID][pkt->streamIndex] > mov->cache_end_ptr[STCO_ID][pkt->streamIndex])
        {
			mov->cache_write_ptr[STCO_ID][pkt->streamIndex] = mov->cache_start_ptr[STCO_ID][pkt->streamIndex];
		}
		
		if(mov->stco_cache_size[pkt->streamIndex] >= STCO_CACHE_SIZE)
		{
            cdx_int32 ret;
            CdxStreamT *pStcoStream = NULL;
            
			if(!mov->fd_stsz[0])
            {
                logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!", __FUNCTION__, __LINE__, pkt->streamIndex);                
                //not create mp4 tmp file, create now.
                if(0 != movCreateTmpFile(mov))
                {
                    loge("(f:%s, l:%d) fatal error! movCreateTmpFile() fail!", __FUNCTION__, __LINE__);
					return -1;
                }              
            }
			
            pStcoStream = (CdxStreamT*)mov->fd_stco[pkt->streamIndex];
            CdxStreamSeek(pStcoStream, (trk->stco_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
            ret = CdxStreamWrite(pStcoStream, mov->cache_read_ptr[STCO_ID][pkt->streamIndex], MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
			if(ret < 0)
			{
                loge("(f:%s, l:%d) write STCO failed\n", __FUNCTION__, __LINE__);
				return -1;
			}
			
			trk->stco_tiny_pages++;
			mov->stco_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
			mov->cache_read_ptr[STCO_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
            if(mov->cache_read_ptr[STCO_ID][pkt->streamIndex] > mov->cache_end_ptr[STCO_ID][pkt->streamIndex])
            {
                mov->cache_read_ptr[STCO_ID][pkt->streamIndex] = mov->cache_start_ptr[STCO_ID][pkt->streamIndex];
            }
        }     
    }
	
    mov->last_stream_index = pkt->streamIndex;
    if(s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_PCM)   //uncompressed audio need not write stsz field, because sample size is fixed.
    {
        trk->stsz_size+=size/(trk->enc->channels*(trk->enc->bits_per_sample>>3)); //av stsz size
        mov->stsc_cnt+=size/(trk->enc->channels*(trk->enc->bits_per_sample>>3));
    }
    else
    {
        *mov->cache_write_ptr[STSZ_ID][pkt->streamIndex]++ = size;
        trk->stsz_size++; //av stsz size
        mov->stsz_cache_size[pkt->streamIndex]++;
        mov->stsc_cnt++;
    }

	if(mov->cache_write_ptr[STSZ_ID][pkt->streamIndex] > mov->cache_end_ptr[STSZ_ID][pkt->streamIndex])
	{
		mov->cache_write_ptr[STSZ_ID][pkt->streamIndex] = mov->cache_start_ptr[STSZ_ID][pkt->streamIndex];
	}


	if(mov->stsz_cache_size[pkt->streamIndex] >= STSZ_CACHE_SIZE)
	{
		cdx_int32 ret;
		CdxStreamT *pStszStream = NULL;
		
		if(!mov->fd_stsz[0])
        {
            logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!", __FUNCTION__, __LINE__, pkt->streamIndex);            
            //not create mp4 tmp file, create now.
            if (0 != movCreateTmpFile(mov))
            {
                loge("(f:%s, l:%d) fatal error! movCreateTmpFile() fail!", __FUNCTION__, __LINE__);
				return -1;
            }
            
        }
		
        pStszStream = (CdxStreamT*)mov->fd_stsz[pkt->streamIndex];
        CdxStreamSeek(pStszStream, (trk->stsz_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
        ret = CdxStreamWrite(pStszStream, mov->cache_read_ptr[STSZ_ID][pkt->streamIndex], MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
		if (ret < 0)
		{
            loge("(f:%s, l:%d) write STSZ failed\n", __FUNCTION__, __LINE__);
			return -1;
	    }
        
		trk->stsz_tiny_pages++;
		mov->stsz_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
		mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
		if(mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] > mov->cache_end_ptr[STSZ_ID][pkt->streamIndex])
		{
			mov->cache_read_ptr[STSZ_ID][pkt->streamIndex] = mov->cache_start_ptr[STSZ_ID][pkt->streamIndex];
		}
	}
	
    if(pkt->streamIndex == 0)//video
    {
        //*mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ = pkt->duration;
        if (trk->last_pts == 0)
        {
            *mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ = pkt->duration;
        }
		else
        {
            *mov->cache_write_ptr[STTS_ID][pkt->streamIndex]++ = (pkt->pts - trk->last_pts);
        }
		
        trk->stts_size++; 
        mov->stts_cache_size[pkt->streamIndex]++;
				
        if(mov->cache_write_ptr[STTS_ID][pkt->streamIndex] > mov->cache_end_ptr[STTS_ID][pkt->streamIndex])
    	{
    		mov->cache_write_ptr[STTS_ID][pkt->streamIndex] = mov->cache_start_ptr[STTS_ID][pkt->streamIndex];
    	}
    	
    	if(mov->stts_cache_size[pkt->streamIndex] >= STTS_CACHE_SIZE)
    	{
    		cdx_int32 ret;
    		CdxStreamT *pSttsStream = NULL;
    		if(!mov->fd_stsz[0])
            {
                logd("(f:%s, l:%d) strm[%d] not create mp4 tmp file, create now!", __FUNCTION__, __LINE__, pkt->streamIndex);
                
                //not create mp4 tmp file, create now.
                if(0 != movCreateTmpFile(mov))
                {
                    loge("(f:%s, l:%d) fatal error! movCreateTmpFile() fail!", __FUNCTION__, __LINE__);
					return -1;
                }
                
            }
    		
            pSttsStream = (CdxStreamT*)mov->fd_stts[pkt->streamIndex];
            CdxStreamSeek(pSttsStream, (trk->stts_tiny_pages * MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE), SEEK_SET);
            ret = CdxStreamWrite(pSttsStream, mov->cache_read_ptr[STTS_ID][pkt->streamIndex], MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE);
    		if(ret < 0)
    		{
    		    loge("(f:%s, l:%d) write STTS failed\n", __FUNCTION__, __LINE__);
    			return -1;
    		}
            
    		trk->stts_tiny_pages++;
    		mov->stts_cache_size[pkt->streamIndex] -= MOV_CACHE_TINY_PAGE_SIZE;
    		mov->cache_read_ptr[STTS_ID][pkt->streamIndex] += MOV_CACHE_TINY_PAGE_SIZE;
    		if(mov->cache_read_ptr[STTS_ID][pkt->streamIndex] > mov->cache_end_ptr[STTS_ID][pkt->streamIndex])
    		{
    			mov->cache_read_ptr[STTS_ID][pkt->streamIndex] = mov->cache_start_ptr[STTS_ID][pkt->streamIndex];
    		}
    	}      
        tmpStrmByte = pkt_buf[4];
        if (s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_H264)
        {
            if((tmpStrmByte&0x1f) == 5)
            {
                mov->cache_keyframe_ptr[trk->keyFrame_num] = trk->stts_size;
                trk->keyFrame_num++;
            }

        }
        else if (s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_MPEG4)
        {
            if((tmpStrmByte>>6) == 0)
            {
                mov->cache_keyframe_ptr[trk->keyFrame_num] = trk->stts_size;
                trk->keyFrame_num++;
            }
        }
		else if (s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_MJPEG)
        {  /* gushiming compressed source */
        	if(mov->keyframe_num == 0) {
			    mov->cache_keyframe_ptr[trk->keyFrame_num] = trk->stts_size;
                trk->keyFrame_num++;	
        	}
			mov->keyframe_num ++;
			if(mov->keyframe_num >= 25) {
				mov->keyframe_num = 0;
			}
        }
        else
        {
            loge("err codec type!\n");
            return -1;
        }

        if(s->streams[0]->codec.codec_id == CDX1_CODEC_ID_H264)
        {   
            int pkt_size = BSWAP32(size - 4);
            memcpy(pkt_buf, &pkt_size, 4);
        }
    }
    
    mov->mdat_size += size;
   
	if(pkt->buflen)
	{
		int ret = put_buffer_cache(pb, pkt_buf, pkt->buflen);
		if (ret > 0)
		{
		    logv("write %d bytes to mp4file\n", ret);
		}
		else if (ret < 0)
		{
		    loge("write mp4file failed\n");
		    return -1;
		}
	}

	if(s->streams[pkt->streamIndex]->codec.codec_id == CDX1_CODEC_ID_MJPEG)
	{  /* gushiming compressed source */
		tmpMjepgTrailer[0] = 0xff;
		tmpMjepgTrailer[1] = 0xd9;
		put_buffer_cache(pb, (cdx_int8*)tmpMjepgTrailer, 2);	
	}

    if (pkt->streamIndex == 0)
    {
        trk->trackDuration += *(mov->cache_write_ptr[STTS_ID][pkt->streamIndex] - 1);
    }
	else
    {
        if (trk->last_pts == 0)
        {
           trk->trackDuration += pkt->duration;
        }
        else
        {
            trk->trackDuration += pkt->pts - trk->last_pts;
        }
    }
    trk->last_pts = pkt->pts;
    return 0;
}

cdx_int32 mov_write_trailer(Mp4MuxContext *s)
{
    CedarX1_MOVContext *mov = s->priv_data;
    ByteIOContext *pb = s->stream_writer;
    offset_t filesize = 0;//moov_pos = url_ftell(pb);
	CdxMuxerPacketT pkt;

	pkt.buf = NULL;
	pkt.buflen = 0;
	pkt.streamIndex = -1;
	
	mov_write_packet(s, &pkt);//update last packet

    mov_write_moov_tag(pb, mov);
    
    CdxStreamSeek(pb, mov->mdat_pos, SEEK_SET);
    put_be32_cache(pb, mov->mdat_size + 8);
    //flush_payload_cache(mov, pb);
    return filesize;
}

#define MOV_TMPFILE_DIR "/tmp/mp4/"
#define MOV_TMPFILE_EXTEND_NAME ".buf"
static void makeMovTmpFullFilePath(char *pPath, cdx_int32 nPathSize, char *filetype, cdx_int32 nStreamIndex, void* nFileId)
{
    // /mnt/extsd/mov_stsz_muxer_0[0xaaaabbbb].buf
    if(!strcmp(filetype, "stsz"))
    {
        sprintf(pPath, "write://"MOV_TMPFILE_DIR"mov_stsz_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,nStreamIndex, nFileId);
    }
    else if(!strcmp(filetype, "stts"))
    {
        sprintf(pPath, "write://"MOV_TMPFILE_DIR"mov_stts_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,nStreamIndex, nFileId);
    }
    else if(!strcmp(filetype, "stco"))
    {
        sprintf(pPath, "write://"MOV_TMPFILE_DIR"mov_stco_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,nStreamIndex, nFileId);
    }
    else if(!strcmp(filetype, "stsc"))
    {
        sprintf(pPath, "write://"MOV_TMPFILE_DIR"mov_stsc_muxer_%d[0x%p]"MOV_TMPFILE_EXTEND_NAME ,nStreamIndex, nFileId);
    }
    else
    {
        loge("(f:%s, l:%d) fatal error, mov tmp filetype[%s] is not support!", __FUNCTION__, __LINE__, filetype);
    }
    return;
}

static __hdle openMovTmpFile(char *pPath)
{
    int fd;
    int ret;
    CdxStreamT *pStream = NULL;

    CdxDataSourceT datasourceDesc;
    memset(&datasourceDesc, 0, sizeof(CdxDataSourceT));
    datasourceDesc.uri = strdup(pPath);
    
    ret = CdxStreamOpen(&datasourceDesc, NULL, NULL, &pStream, NULL);
    if(ret < 0)
    {
        loge("(f:%s, l:%d) fatal error! create mov tmp file fail.", __FUNCTION__, __LINE__);
        return NULL;
    }
    return (__hdle)pStream;
}

cdx_int32 movCreateTmpFile(CedarX1_MOVContext *mov)
{
    cdx_int32 i;

    mkdir(MOV_TMPFILE_DIR, 0777);
    for(i = 0;i < 2;i++)
    {
        if(!mov->fd_stsz[i])
        {
            makeMovTmpFullFilePath(mov->FilePath_stsz[i], MOV_BUF_NAME_LEN, "stsz", i, (void*)mov);
            mov->fd_stsz[i] = openMovTmpFile(mov->FilePath_stsz[i]);
            if(!mov->fd_stsz[i])
            {
                loge("error = %d\n",-errno);
                return -1;
            }
        }

        if(!mov->fd_stts[i])
        {
            makeMovTmpFullFilePath(mov->FilePath_stts[i], MOV_BUF_NAME_LEN, "stts", i, (void*)mov);
            mov->fd_stts[i] = openMovTmpFile(mov->FilePath_stts[i]);
            if(!mov->fd_stts[i])
            {
                logw("can not open %d mov_stts_muxer.buf\n",i);
                return -1;
            }
        }
        
        if(!mov->fd_stco[i])
        {
            makeMovTmpFullFilePath(mov->FilePath_stco[i], MOV_BUF_NAME_LEN, "stco", i, (void*)mov);
            mov->fd_stco[i] = openMovTmpFile(mov->FilePath_stco[i]);
            if(!mov->fd_stco[i])
            {
                logw("can not open %d mov_stco_muxer.buf\n",i);
                return -1;
            }
        }

        if(!mov->fd_stsc[i])
        {
            makeMovTmpFullFilePath(mov->FilePath_stsc[i], MOV_BUF_NAME_LEN, "stsc", i, (void*)mov);
            mov->fd_stsc[i] = openMovTmpFile(mov->FilePath_stsc[i]);
            if(!mov->fd_stsc[i])
            {
                logw("can not open %d mov_stsc_muxer.buf\n",i);
                return -1;
            }
        }
    }
    return 0;
}