#include "CdxMp4Muxer.h"

int initMov(Mp4MuxContext *impl)
{
    CedarX1_MOVContextT *mov = NULL;
    if ((mov = (CedarX1_MOVContextT*)malloc(sizeof(CedarX1_MOVContextT))) == NULL)
    {
        logv("Mp4MuxContext->priv_data malloc failed\n");
        return -1;
    }
    memset(mov, 0, sizeof(CedarX1_MOVContextT));
    
    if ((mov->cache_keyframe_ptr = (cdx_uint32*)malloc(KEYFRAME_CACHE_SIZE)) == NULL)
    {
        logv("mov->cache_keyframe_ptr malloc failed\n");
        return -2;
    }

    if ((impl->mov_inf_cache = (cdx_int8*)malloc(TOTAL_CACHE_SIZE * 4)) == NULL)
    {
        logv("Mp4MuxContext->mov_inf_cache malloc failed\n");
        return -3;
    }
      
	mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] = mov->cache_read_ptr[STCO_ID][CODEC_TYPE_VIDEO] 
		= mov->cache_write_ptr[STCO_ID][CODEC_TYPE_VIDEO] = (cdx_uint32*)impl->mov_inf_cache;
	mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_read_ptr[STCO_ID][CODEC_TYPE_AUDIO] 
		= mov->cache_write_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + STCO_CACHE_SIZE;
	
	mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_VIDEO] 
		= mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + STCO_CACHE_SIZE;
	mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_read_ptr[STSZ_ID][CODEC_TYPE_AUDIO] 
		= mov->cache_write_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + STSZ_CACHE_SIZE;
	
	mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_read_ptr[STSC_ID][CODEC_TYPE_VIDEO] 
		= mov->cache_write_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + STSZ_CACHE_SIZE;
	mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_read_ptr[STSC_ID][CODEC_TYPE_AUDIO] 
		= mov->cache_write_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + STSC_CACHE_SIZE;
	mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_read_ptr[STTS_ID][CODEC_TYPE_VIDEO] 
		= mov->cache_write_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + STSC_CACHE_SIZE;	
	mov->cache_tiny_page_ptr = mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + STTS_CACHE_SIZE;
	
	mov->cache_end_ptr[STCO_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_VIDEO] + (STCO_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_VIDEO] + (STSZ_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSC_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_VIDEO] + (STSC_CACHE_SIZE - 1);
	mov->cache_end_ptr[STTS_ID][CODEC_TYPE_VIDEO] = mov->cache_start_ptr[STTS_ID][CODEC_TYPE_VIDEO] + (STTS_CACHE_SIZE - 1);
	
	mov->cache_end_ptr[STCO_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STCO_ID][CODEC_TYPE_AUDIO] + (STCO_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSZ_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSZ_ID][CODEC_TYPE_AUDIO] + (STSZ_CACHE_SIZE - 1);
	mov->cache_end_ptr[STSC_ID][CODEC_TYPE_AUDIO] = mov->cache_start_ptr[STSC_ID][CODEC_TYPE_AUDIO] + (STSC_CACHE_SIZE - 1);

    mov->last_stream_index = -1;
    
    impl->priv_data = (void*)mov;

    return 0;
}

void freeMov(Mp4MuxContext* impl)
{
    CedarX1_MOVContextT *mov = impl->priv_data;
    int i;

    for(i = 0; i < 2; i++)
    {
        if(mov->fd_stts[i])
        {
            CdxStreamClose((CdxStreamT*)mov->fd_stts[i]);
            mov->fd_stts[i] = 0;
			remove(mov->FilePath_stts[i]);
            logd("(f:%s, l:%d) remove fd_stts[%d]name[%s]", __FUNCTION__, __LINE__, i, mov->FilePath_stts[i]);
        }
        if(mov->fd_stsz[i])
        {
            CdxStreamClose((CdxStreamT*)mov->fd_stsz[i]);
            mov->fd_stsz[i] = 0;
			remove(mov->FilePath_stts[i]);
            logd("(f:%s, l:%d) remove fd_stsz[%d]name[%s]", __FUNCTION__, __LINE__, i, mov->FilePath_stsz[i]);
        }
        if(mov->fd_stco[i])
        {
            CdxStreamClose((CdxStreamT*)mov->fd_stco[i]);
            mov->fd_stco[i] = 0;
			remove(mov->FilePath_stts[i]);
            logd("(f:%s, l:%d) remove fd_stco[%d]name[%s]", __FUNCTION__, __LINE__, i, mov->FilePath_stco[i]);
        }
        if(mov->fd_stsc[i])
        {
            CdxStreamClose((CdxStreamT*)mov->fd_stsc[i]);
            mov->fd_stsc[i] = 0;
			remove(mov->FilePath_stts[i]);
            logd("(f:%s, l:%d) remove fd_stsc[%d]name[%s]", __FUNCTION__, __LINE__, i, mov->FilePath_stsc[i]);
        }
    }
    
	for(i = 0; i < MAX_STREAMS_IN_FILE; i++)
	{
        if(mov->tracks[i].vosData)
    	{
    		free(mov->tracks[i].vosData);
            mov->tracks[i].vosData = NULL;
    	}
		if(impl->streams[i])
		{
			free(impl->streams[i]);
            impl->streams[i] = NULL;
		}
	}

    if(mov->cache_keyframe_ptr)
    {
        free(mov->cache_keyframe_ptr);
        mov->cache_keyframe_ptr = NULL;
    }

    free(impl->priv_data);
    impl->priv_data = NULL;
}

void setVideoInfo(Mp4MuxContext *impl, MuxerVideoStreamInfoT *pVInfo)
{
    CedarX1_AVCodecContext *pVideo = &(impl->streams[0]->codec);
    CedarX1_MOVContextT *mov = impl->priv_data;
    
    pVideo->height = pVInfo->nHeight;
    pVideo->width = pVInfo->nWidth;
    pVideo->frame_rate = pVInfo->nFrameRate;
    pVideo->rotate_degree = pVInfo->nRotateDegree;
    pVideo->codec_id = (pVInfo->eCodeType == VENC_CODEC_JPEG) ? CDX1_CODEC_ID_MJPEG : CDX1_CODEC_ID_H264;  

    mov->create_time = pVInfo->nCreatTime;
    mov->tracks[0].timescale = 1000;

    impl->nb_streams++;   
}

void setAudioInfo(Mp4MuxContext *impl, MuxerAudioStreamInfoT *pAInfo)
{
    CedarX1_AVCodecContext *pAudio = &(impl->streams[1]->codec);
    CedarX1_MOVContextT *mov = impl->priv_data;

    pAudio->channels = pAInfo->nChannelNum;
    pAudio->bits_per_sample = pAInfo->nBitsPerSample;
    pAudio->frame_size = pAInfo->nSampleCntPerFrame;
    pAudio->sample_rate = pAInfo->nSampleRate;

	if(pAInfo->eCodecFormat == AUDIO_ENCODER_PCM_TYPE)
		pAudio->codec_id = CDX1_CODEC_ID_PCM;
	else if(pAInfo->eCodecFormat == AUDIO_ENCODER_AAC_TYPE)
		pAudio->codec_id = CDX1_CODEC_ID_AAC;
	else if(pAInfo->eCodecFormat == AUDIO_ENCODER_MP3_TYPE)
		pAudio->codec_id = CDX1_CODEC_ID_MP3;
	else
	{
		loge("unlown codectype(%d)", pAudio->codec_id );
	}
	
    mov->tracks[1].timescale = pAudio->sample_rate;
    mov->tracks[1].sampleDuration = 1;
}

static int __Mp4MuxerWriteExtraData(CdxMuxerT *mux, unsigned char *vosData, int vosLen, int idx)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
    CedarX1_MOVContextT *mov = impl->priv_data;
    MOVTrack *trk = &mov->tracks[idx];
    if (vosLen)
    {
        trk->vosData = (cdx_int8*)malloc(vosLen);
    	trk->vosLen  = vosLen;
        memcpy(trk->vosData, vosData, vosLen);
	}
    else
    {
        trk->vosData = NULL;
        trk->vosLen  = 0;
    }
    return 0;
}

static int __Mp4MuxerWriteHeader(CdxMuxerT *mux)
{
	Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	return mov_write_header(impl);
}

static int __Mp4MuxerWritePacket(CdxMuxerT *mux, CdxMuxerPacketT *packet)
{
	Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	return mov_write_packet(impl, packet);
}

static int __Mp4MuxerSetMediaInfo(CdxMuxerT *mux, CdxMuxerMediaInfoT *pMediaInfo)
{
	Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	CedarX1_MOVContextT *mov; 
	CedarX1_AVStream *avs = NULL;	
	int cnt;  

    if (pMediaInfo == NULL)
    {
        logv("pMediaInfo is NULL\n");
        return -1;
    }
    
    if (impl->priv_data == NULL)
    {
        if (initMov(impl) < 0)
        {
            logv("initMov(impl) failed\n");
            return -1;
        }
    }

    for (cnt = 0; cnt < MAX_STREAMS_IN_FILE; cnt++)
    {
    	avs = (CedarX1_AVStream*)malloc(sizeof(CedarX1_AVStream));
        if (avs == NULL)
        {
            logv("Mp4MuxContext->streams[%d] malloc failed\n", cnt);
            return -1;
        }
        logd("avs(%p), sizeof(CedarX1_AVStream): %d", avs, sizeof(CedarX1_AVStream));
        memset(avs, 0x00, sizeof(CedarX1_AVStream));
        
        avs->codec.codec_type = (cnt == 0) ? CODEC_TYPE_VIDEO : CODEC_TYPE_AUDIO;
        impl->streams[cnt] = avs;
    }
    
    if (pMediaInfo->videoNum)
    {
        setVideoInfo(impl, &(pMediaInfo->video));
    }

    if (pMediaInfo->audioNum)
    {
        setAudioInfo(impl, &(pMediaInfo->audio));
    }

	mov = (CedarX1_MOVContextT*)impl->priv_data;
    mov->geo_available = pMediaInfo->geo_available;
    mov->latitudex10000 = pMediaInfo->latitudex;
    mov->longitudex10000 = pMediaInfo->longitudex;
    
	return 0;
}

static int __Mp4MuxerWriteTrailer(CdxMuxerT *mux)
{
    Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	return mov_write_trailer(impl);
}

static int __Mp4MuxerControl(CdxMuxerT *mux, int uCmd, void *pParam)
{
	Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	CedarX1_MOVContextT *mov = (CedarX1_MOVContextT*)impl->priv_data;

	switch (uCmd)
	{
	    case SETTOTALTIME:
	    {   // ms
	        cdx_uint64 time = *((cdx_uint64*)pParam);
	        mov->tracks[0].trackDuration = (cdx_uint32)((time * mov->tracks[0].timescale) / 1000);
		    mov->tracks[1].trackDuration = (cdx_uint32)((time * mov->tracks[1].timescale) / 1000);
		    break;
	    }
        case SETSDCARDSTATE:
        {
            impl->mbSdcardDisappear = *((cdx_uint32*)pParam);
            logd("(f:%s, l:%d) SETSDCARDSTATE, impl->mbSdcardDisappear[%d]", __FUNCTION__, __LINE__, impl->mbSdcardDisappear);
            break;
        }
        default:
            break;       
	}
	return 0;
}

static int __Mp4MuxerClose(CdxMuxerT *mux)
{
	Mp4MuxContext *impl = (Mp4MuxContext*)mux;
	CedarX1_MOVContextT *mov = impl->priv_data;
	int cnt;
	if(impl == NULL)
	{
	    logv("Mp4MuxContext* is NULL\n");
	    return -1;
	}

	if (impl->stream_writer)
	{
	    CdxStreamClose(impl->stream_writer);
	    impl->stream_writer = NULL;
	}

	if (impl->mov_inf_cache)
	{
	    free(impl->mov_inf_cache);
	    impl->mov_inf_cache = NULL;
	}

	if (impl->priv_data)
	{
	   freeMov(impl);
	}
	
    free(impl);
	return 0;
}

static struct CdxMuxerOpsS mp4MuxerOps = 
{
    .writeExtraData  = __Mp4MuxerWriteExtraData,
	.writeHeader     = __Mp4MuxerWriteHeader,	
	.writePacket     = __Mp4MuxerWritePacket,
	.writeTrailer    = __Mp4MuxerWriteTrailer,
	.control         = __Mp4MuxerControl,
	.setMediaInfo    = __Mp4MuxerSetMediaInfo,
	.close           = __Mp4MuxerClose
};


CdxMuxerT* __CdxMp4MuxerOpen(CdxStreamT *stream_writer)
{
	Mp4MuxContext *mp4Mux;
	logd("__CdxMp4MuxerOpen");

	mp4Mux = malloc(sizeof(Mp4MuxContext));
	if(!mp4Mux)
	{
		return NULL;
	}
	memset(mp4Mux, 0x00, sizeof(Mp4MuxContext));

	mp4Mux->stream_writer = stream_writer;
	mp4Mux->muxInfo.ops = &mp4MuxerOps;

	return &mp4Mux->muxInfo;
}


CdxMuxerCreatorT mp4MuxerCtor = 
{
	.create = __CdxMp4MuxerOpen
};

