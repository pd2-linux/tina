#include "CdxTsMuxer.h"

int setTsVideoInfo(TsMuxContext *impl, MuxerVideoStreamInfoT *pVInfo)
{
    if (pVInfo->eCodeType == VENC_CODEC_H264)
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_H264;
    }
    else
    {
        loge("unlown codectype(%d)\n", impl->streams[impl->nb_streams]->codec.codec_id );
		return -1;
    }
    impl->streams[impl->nb_streams]->codec.height = pVInfo->nHeight;
    impl->streams[impl->nb_streams]->codec.width  = pVInfo->nWidth;
    impl->streams[impl->nb_streams]->codec.frame_rate = pVInfo->nFrameRate;	
    
    impl->nb_streams++;
    return 0;
}

int setTsAudioInfo(TsMuxContext *impl, MuxerAudioStreamInfoT *pAInfo)
{
    if(pAInfo->eCodecFormat == AUDIO_ENCODER_PCM_TYPE)
	{
		impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_PCM;
	}
    if(pAInfo->eCodecFormat == AUDIO_ENCODER_AAC_TYPE) //for aac 
    {
        impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_AAC;
    }
    else if(pAInfo->eCodecFormat == AUDIO_ENCODER_MP3_TYPE)
	{
		impl->streams[impl->nb_streams]->codec.codec_id = MUX_CODEC_ID_MP3;
	}
	else
	{
		loge("unlown codectype(%d)\n", impl->streams[impl->nb_streams]->codec.codec_id );
		return -1;
	}
    impl->streams[impl->nb_streams]->codec.channels = pAInfo->nChannelNum;
    impl->streams[impl->nb_streams]->codec.bits_per_sample = pAInfo->nBitsPerSample;
    impl->streams[impl->nb_streams]->codec.frame_size  = pAInfo->nSampleCntPerFrame;
    impl->streams[impl->nb_streams]->codec.sample_rate = pAInfo->nSampleRate;   
    impl->nb_streams++;
    logd("pAInfo->nSampleRate: %d\n", pAInfo->nSampleRate);
    return 0;
}

static int __TsMuxerSetMediaInfo(CdxMuxerT *mux, CdxMuxerMediaInfoT *pMediaInfo)
{
	TsMuxContext *impl = (TsMuxContext*)mux;
	TsWriter *ts = NULL;
	int i;  

    if ((impl->cache_in_ts_stream = (cdx_uint8*)malloc(TS_PACKET_SIZE * 1024)) == NULL)
    {
        loge("impl->cache_in_ts_stream malloc failed\n");
		return -1;
    }

    if ((ts = (TsWriter*)malloc(sizeof(TsWriter))) == NULL)
    {
       loge("impl->priv_data malloc failed\n");
       return -1;
    }
    memset(ts, 0, sizeof(TsWriter));
    impl->priv_data = (void*)ts;

    ts->cur_pcr = 0;
    ts->nb_services = 1;
    ts->tsid = DEFAULT_TSID;
    ts->onid = DEFAULT_ONID;

    ts->sdt.pid = SDT_PID;
    ts->sdt.cc = 15; // Initialize at 15 so that it wraps and be equal to 0 for the first packet we write
    ts->sdt.write_packet = TsSectionWritePacket;
    ts->sdt.opaque = impl;

    ts->pat.pid = PAT_PID;
    ts->pat.cc = 15;
    ts->pat.write_packet = TsSectionWritePacket;
    ts->pat.opaque = impl;

    ts->ts_cache_start = impl->cache_in_ts_stream;
    ts->ts_cache_end = impl->cache_in_ts_stream + TS_PACKET_SIZE * 1024 - 1;

    ts->ts_write_ptr = ts->ts_read_ptr = ts->ts_cache_start;
    ts->cache_size = 0;
    ts->cache_page_num = 0;
    ts->cache_size_total = 0;

    ts->sdt_packet_period = 200;
    ts->pat_packet_period = 3;
    ts->pat_packet_count = ts->pat_packet_period;
   
    ts->services = NULL;
    if ((ts->services = (MpegTSService**)malloc(MAX_SERVERVICES_IN_FILE*sizeof(MpegTSService*))) == NULL)
    {
        loge("ts->services malloc failed\n");
        return -1;
    }

    if ((impl->pes_buffer = (cdx_uint8*)malloc(512 * 1024)) == NULL)
    {
        loge("impl->pes_buffer malloc failed\n");
        return -1;    
    }

	impl->pes_bufsize = 512 * 1024;
    impl->max_delay = 1;
	impl->audio_frame_num = 0;
	impl->output_buffer_mode = OUTPUT_TS_FILE;
	impl->nb_streams = 0;
	impl->pat_pmt_counter = 0;
	impl->pat_pmt_flag = 1;
	impl->first_video_pts = 1;
	impl->base_video_pts = 0;
	impl->pcr_counter = 0;

    for(i = 0;i < MAX_SERVERVICES_IN_FILE; i++)
    {
        MpegTSService *service = NULL;
        if ((service = (MpegTSService*)malloc(sizeof(MpegTSService))) == NULL)
        {
            loge("ts->services[%d] malloc failed\n", i);
            return -1;
        }		
        memset(service, 0, sizeof(MpegTSService));
        service->pmt.write_packet = TsSectionWritePacket;
        service->pmt.opaque = impl;
        service->pmt.cc = 15;
        service->pmt.pid = 0x0100;
        service->pcr_pid = 0x1000;
        service->sid = 0x00000001;
		
        ts->services[i] = service;
    }

    for(i=0;i<MAX_STREAMS_IN_TS_FILE;i++)
    {
        AVStream *st = NULL;
        MpegTSWriteStream *ts_st = NULL;
        if ((st = (AVStream *)malloc(sizeof(AVStream))) == NULL)
        {
            loge("impl->streams[%d] failed\n", i);
            return -1;
        }
        memset(st, 0, sizeof(AVStream));
        impl->streams[i] = st;
		impl->streams[i]->firstframeflag = 1;
		
        if ((ts_st = (MpegTSWriteStream*)malloc(sizeof(MpegTSWriteStream))) == NULL)
        {
            loge("st->priv_data malloc failed\n");
            return -1;
        }
        memset(ts_st, 0, sizeof(MpegTSWriteStream));
        st->priv_data = ts_st;
        ts_st->service = ts->services[0];
        ts_st->pid = DEFAULT_START_PID + i;

		logv("ts_st->pid: %x", ts_st->pid);
        ts_st->payload_pts = -1;
        ts_st->payload_dts = -1;
        ts_st->first_pts_check = 1;
        ts_st->cc = 15;
        st->codec.codec_type = (i==0) ? CODEC_TYPE_VIDEO : CODEC_TYPE_AUDIO;

		if( st->codec.codec_type == CODEC_TYPE_VIDEO)
		{
			ts_st->pid = 0x1011;
		}
		else
		{
			ts_st->pid = 0x1100;
		}
    }  
    
    if (pMediaInfo->videoNum)
    {
        if (setTsVideoInfo(impl, &pMediaInfo->video))
        {
            return  -1;
        }
    }

    if (pMediaInfo->audioNum)
    {
        if (setTsAudioInfo(impl, &pMediaInfo->audio))
        {
            return -1;
        }
    }

	return 0;
}

static int __TsMuxerWriteHeader(CdxMuxerT *mux)
{
	TsMuxContext *impl = (TsMuxContext*)mux;
	return 0;
}

static int __TsMuxerWriteExtraData(CdxMuxerT *mux, unsigned char *vosData, int vosLen, int idx)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
    AVStream *trk = impl->streams[idx];
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

static int __TsMuxerWritePacket(CdxMuxerT *mux, CdxMuxerPacketT *packet)
{
	TsMuxContext *impl = (TsMuxContext*)mux;
	return TsWritePacket(impl, packet);
}

static int __TsMuxerWriteTrailer(CdxMuxerT *mux)
{
    TsMuxContext *impl = (TsMuxContext*)mux;
	return TsWriteTrailer(impl);
}

static int __TsMuxerControl(CdxMuxerT *mux, int uCmd, void *pParam)
{
	TsMuxContext *impl = (TsMuxContext*)mux;

	switch (uCmd)
	{
        default:
            break;       
	}
	return 0;
}

static int __TsMuxerClose(CdxMuxerT *mux)
{
	TsMuxContext *impl = (TsMuxContext*)mux;
	TsWriter *ts = (TsWriter*)impl->priv_data;
	AVStream *st = NULL;
	cdx_int32 i;
	
	for(i=0;i<MAX_STREAMS_IN_TS_FILE;i++)
    {
        st = impl->streams[i];
        if(st)
        {
        	if(st->priv_data)
    		{
    			free(st->priv_data);
				st->priv_data = NULL;
    		}
            free(st);
            st = NULL;
        }
    }

    if(*ts->services)
    {
        free(*ts->services);
        *ts->services = NULL;

		free(ts->services);
		ts->services = NULL;
    }

    if(impl->stream_writer)
    {
    	CdxStreamClose(impl->stream_writer);
    	impl->stream_writer = NULL;
    }

    if(impl->cache_in_ts_stream)
    {
        free(impl->cache_in_ts_stream);
        impl->cache_in_ts_stream = NULL;
    }
    if(impl->priv_data)
    {
        free(impl->priv_data);
        impl->priv_data = NULL;
    }
    
	if(impl->pes_buffer)
	{
		free(impl->pes_buffer);
		impl->pes_buffer = NULL;
	}

    if(impl)
    {
        free(impl);
        impl = NULL;
    }

	return 0;
}

static struct CdxMuxerOpsS tsMuxerOps = 
{
    .writeExtraData  = __TsMuxerWriteExtraData,
	.writeHeader     = __TsMuxerWriteHeader,	
	.writePacket     = __TsMuxerWritePacket,
	.writeTrailer    = __TsMuxerWriteTrailer,
	.control         = __TsMuxerControl,
	.setMediaInfo    = __TsMuxerSetMediaInfo,
	.close           = __TsMuxerClose
};


CdxMuxerT* __CdxTsMuxerOpen(CdxStreamT *stream_writer)
{
	TsMuxContext *mp4Mux;
	logd("__CdxTsMuxerOpen");

	mp4Mux = malloc(sizeof(TsMuxContext));
	if(!mp4Mux)
	{
		return NULL;
	}
	memset(mp4Mux, 0x00, sizeof(TsMuxContext));

	mp4Mux->stream_writer = stream_writer;
	mp4Mux->muxInfo.ops = &tsMuxerOps;

	return &mp4Mux->muxInfo;
}


CdxMuxerCreatorT tsMuxerCtor = 
{
	.create = __CdxTsMuxerOpen
};

