#ifndef CDX_MP4_MUXER_H
#define CDX_MP4_MUXER_H

#include "log.h"
#include "CdxMuxer.h"

typedef void *				__hdle;
#define offset_t cdx_int64
#define MAX_STREAMS 2

typedef enum  CedarX1_CodecID{
    CDX1_CODEC_ID_MPEG4 = 32,
    CDX1_CODEC_ID_H264 = 33,
    CDX1_CODEC_ID_MJPEG = 108,
    CDX1_CODEC_ID_AAC = 64,
    CDX1_CODEC_ID_MP3 = 105,
    CDX1_CODEC_ID_PCM,   // 16-bit little-endian, twos-complement
    CDX1_CODEC_ID_ADPCM, // DVI/Intel IMAADPCM-ACM code 17
}CedarX1_CodecID;

enum CodecType 
{
    CODEC_TYPE_UNKNOWN = -1,
    CODEC_TYPE_VIDEO,
    CODEC_TYPE_AUDIO,
    CODEC_TYPE_SUBTITLE,
    CODEC_TYPE_DATA
};

typedef enum{
	UNKOWNCMD = 0,
    SETAVPARA,
	SETTOTALTIME,
	SETFALLOCATELEN,    //set fallocate size for fd.
	SETCACHEFD,
	SETCACHEFD2,
	SETOUTURL,
	REGISTER_WRITE_CALLBACK,
	SETSDCARDSTATE,
	SETCACHEMEM,
	SET_FS_WRITE_MODE,
	SET_FS_SIMPLE_CACHE_SIZE,
	SET_STREAM_CALLBACK,
	SETMOV
} RECODERWRITECOMMANDS;

typedef struct CedarX1_AVCodecContext {
    cdx_int32             width;
    cdx_int32             height;
    enum CodecType      codec_type; /* see CODEC_TYPE_xxx */
    cdx_int32             rotate_degree;
    cdx_uint32          codec_tag;
    cdx_uint32          codec_id;

    cdx_int32             channels;
    cdx_int32             frame_rate;
    cdx_int32             frame_size;   //for audio, it means sample count a audioFrame contain; 
	                                   //in fact, its value is assigned by pcmDataSize(MAXDECODESAMPLE=1024) which is transport by previous AudioSourceComponent.
	                                   //aac encoder will encode a frame from MAXDECODESAMPLE samples; but for pcm, one frame == one sample according to movSpec.
    cdx_int32             bits_per_sample;
    cdx_int32             sample_rate;
}CedarX1_AVCodecContext;

typedef struct CedarX1_AVStream {
    cdx_int32                   index;    /**< stream index in AVFormatContext */
    CedarX1_AVCodecContext      codec; /**< codec context */
    void                        *priv_data;
    float                       quality;
    cdx_int64                   duration;
} CedarX1_AVStream;

typedef struct Mp4MuxContext 
{
    CdxMuxerT            muxInfo;
    CdxStreamT          *stream_writer;
    void                *priv_data; //MOVContext
    cdx_int32           nb_streams;
    CedarX1_AVStream    *streams[MAX_STREAMS];
    cdx_int8            *mov_inf_cache;
    cdx_uint32          mbSdcardDisappear; // 1:sdcard disapear; 0:sdcard is normal
} Mp4MuxContext;

typedef struct
{
    cdx_uint32 extra_data_len;
    cdx_uint8 extra_data[32];
} __extra_data_t;

#define MAX_STREAMS_IN_FILE 2
#define MKTAG(a,b,c,d) (a | (b << 8) | (c << 16) | (d << 24))
#define AV_ROUND_UP 3
#define MOV_BUF_NAME_LEN	(128)
#define MOV_CACHE_TINY_PAGE_SIZE (1024 * 2)//set it to 2K !!attention it must set to below MOV_RESERVED_CACHE_ENTRIES
#define MOV_CACHE_TINY_PAGE_SIZE_IN_BYTE (MOV_CACHE_TINY_PAGE_SIZE*4)
#define globalTimescale 1000
#define MODE_MP4  0x01

typedef struct CedarX1_MOVContext CedarX1_MOVContext;
typedef struct MOVIndex {
    cdx_int32         timescale;
	cdx_int32         sampleDuration;
    cdx_int32         time;
	cdx_int64         last_pts;
    cdx_int64         trackDuration;
    cdx_int32         sampleSize;
    cdx_int32         language;
    cdx_int32         trackID;
	cdx_int32		   stream_type;
    CedarX1_MOVContext      *mov;//used for reverse access
    cdx_int32         tag; ///< stsd fourcc, e,g, 'sowt', 'mp4a'
    CedarX1_AVCodecContext  *enc;

    cdx_int32         vosLen;
    cdx_int8          *vosData;

    cdx_uint32         stsz_size;
    cdx_uint32         stco_size;
    cdx_uint32         stsc_size;
    cdx_uint32         stts_size;
    cdx_uint32         stss_size;

	cdx_uint32         stsz_tiny_pages;
    cdx_uint32         stco_tiny_pages;
    cdx_uint32         stsc_tiny_pages;
    cdx_uint32         stts_tiny_pages;

    cdx_uint32         keyFrame_num;
} MOVTrack;

typedef struct CedarX1_MOVContext {
    cdx_int64       create_time;
    cdx_int32       nb_streams;
    
    offset_t        mdat_pos;
	offset_t        mdat_start_pos;//raw bitstream start pos
    cdx_int64       mdat_size;
    
    cdx_int32       timescale;

    // for user infomation
	cdx_int32       geo_available;
	cdx_int32       latitudex10000;
	cdx_int32       longitudex10000;
	
	cdx_int32	    rotate_degree;
	
    MOVTrack        tracks[MAX_STREAMS];

    __hdle          fd_cache;
    __hdle          fd_stsz[2];
    __hdle          fd_stco[2];
    __hdle          fd_stsc[2];
    __hdle          fd_stts[2];

	cdx_uint32      *cache_start_ptr[4][2]; //[3]stco,stsz,stsc,stts [2]video audio
    cdx_uint32      *cache_end_ptr[4][2];
    cdx_uint32      *cache_write_ptr[4][2];
    cdx_uint32      *cache_read_ptr[4][2];
	cdx_uint32      *cache_tiny_page_ptr;
    cdx_uint32      *cache_keyframe_ptr;

	offset_t        stsz_cache_offset_in_file[2];
	offset_t        stco_cache_offset_in_file[2];
	offset_t        stsc_cache_offset_in_file[2];
	offset_t        stts_cache_offset_in_file[2];

	cdx_int32       stsz_cache_size[2];
	cdx_int32       stco_cache_size[2];
	cdx_int32       stsc_cache_size[2];
	cdx_int32       stts_cache_size[2];

    cdx_int32       stsc_cnt;//sample in one chunk count
    cdx_int32       last_stream_index;
    cdx_int64       last_video_packet_pts;

    char            FilePath_stsz[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char            FilePath_stts[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char            FilePath_stco[MAX_STREAMS][MOV_BUF_NAME_LEN];
    char            FilePath_stsc[MAX_STREAMS][MOV_BUF_NAME_LEN];

    cdx_int32       keyframe_num;

    offset_t        free_pos;
} CedarX1_MOVContextT;

typedef enum CHUNKID
{
	STCO_ID = 0,//don't change all
	STSZ_ID = 1,
	STSC_ID = 2,
	STTS_ID = 3
}CHUNKID;

//1280*720. 30fps, 17.5 minute
#define STCO_CACHE_SIZE  (8 * 1024)          //about 1 hours !must times of tiny_page_size
#define STSZ_CACHE_SIZE  (8 * 1024 * 4) //(8*1024*16)       //about 1 hours !must times of tiny_page_size
#define STTS_CACHE_SIZE  (STSZ_CACHE_SIZE)      //about 1 hours !must times of tiny_page_size
#define STSC_CACHE_SIZE  (STCO_CACHE_SIZE)      //about 1 hours !must times of tiny_page_size

#define STSZ_CACHE_OFFSET_INFILE_VIDEO  (0 * 1024 * 1024) 
#define STSZ_CACHE_OFFSET_INFILE_AUDIO  (2 * 1024 * 1024) 
#define STTS_CACHE_OFFSET_INFILE_VIDEO  (4 * 1024 * 1024)
#define STCO_CACHE_OFFSET_INFILE_VIDEO  (6 * 1024 * 1024) 
#define STCO_CACHE_OFFSET_INFILE_AUDIO  (6 * 1024 * 1024 + 512 * 1024) 
#define STSC_CACHE_OFFSET_INFILE_VIDEO  (7 * 1024 * 1024) 
#define STSC_CACHE_OFFSET_INFILE_AUDIO  (7 * 1024 * 1024 + 512 * 1024) 

#define TOTAL_CACHE_SIZE (STCO_CACHE_SIZE * 2 + STSZ_CACHE_SIZE * 2 + STSC_CACHE_SIZE * 2 + STTS_CACHE_SIZE + MOV_CACHE_TINY_PAGE_SIZE) //32bit

#define KEYFRAME_CACHE_SIZE (8 * 1024 * 16)

#define MOV_HEADER_RESERVE_SIZE     (1024 * 1024)

typedef struct {
    cdx_int32 count;
    cdx_int32 duration;
} MOV_stts_t;

#define PKT_FLAG_KEY 1

cdx_int32 mov_write_header(Mp4MuxContext*s);
cdx_int32 mov_write_packet(Mp4MuxContext *s, CdxMuxerPacketT *pkt);
cdx_int32 mov_write_trailer(Mp4MuxContext *s);
cdx_int32 movCreateTmpFile(CedarX1_MOVContext *mov);
#endif /* CDX_MP4_MUXER_H */
