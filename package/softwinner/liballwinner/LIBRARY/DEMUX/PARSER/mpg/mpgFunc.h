#ifndef _MPG_FUNC_H_
#define _MPG_FUNC_H_

#include "CdxMpgParser.h"
#include "dvdTitleIfo.h"

//void   DVD_set_vmg_memory(CdxMpgParserT *MpgParser);
//void   SetVtsMemory(CdxMpgParserT *MpgParser);
//void   process_dvd_nv_pack( DvdIfoT  *dvdIfo, cdx_uint8 *curPtr);                          
//void parse_pgci_info(struct cdx_stream_info *file, CdxMpgParserT *MpgParser, struct PgcIfo *curPgc,cdx_uint32 curPgcSa,cdx_uint8 pgcType, cdx_uint8 pgcIndex);					 
//cdx_int16 ParsePgciTable(CdxMpgParserT *MpgParser, struct PGCIT *pgciTab, cdx_uint32 startOffset,cdx_uint8 pgcType);				  
//void parse_menu_pgcIfo(struct cdx_stream_info *file, CdxMpgParserT *MpgParser, struct MPGCIUT *menuPgcit,  cdx_uint32 startSector, cdx_uint8 pgcType);
//void CalculateBaseSCR(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 *baseSCR);
//cdx_uint32  BSWAP32(cdx_uint8 *dataPtr); 
//cdx_uint16  BSWAP16(cdx_uint8 *dataPtr); 
//cdx_uint32  DVD_check_file_time_length(CdxMpgParserT *MpgParser);
//cdx_uint32  CheckStreamMap(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 offset);
//cdx_uint8   DVD_parse_next_cmd(CdxMpgParserT *MpgParser);
//cdx_uint8   title_select_suit_pts(CdxMpgParserT *MpgParser);
//cdx_uint8   CheckFirstPack(CdxMpgParserT *MpgParser, cdx_uint8 *buf);
//cdx_uint8   CheckVideoIdNo1172(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 off, cdx_uint32 pes_len);
//cdx_uint8   CheckVideoId1172(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 off, cdx_uint32 pes_len);
//cdx_uint8   CheckVideoId1172(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 off, cdx_uint32 pes_len);
//cdx_uint8   CheckAudioType(CdxMpgParserT *MpgParser, cdx_uint8 *buf, cdx_uint32 offset);                           
//cdx_int16   CalculateFrameRate(CdxMpgParserT *MpgParser, cdx_uint32 *fstSysPos);
//cdx_int16   ParseAudioInfo(CdxMpgParserT *MpgParser, cdx_uint16 audioNum);
//cdx_int16   ParseSubPicInfo(CdxMpgParserT *MpgParser, cdx_uint16 subpicNum);     
//cdx_int16 DVD_parse_cmdIfo(CdxMpgParserT *MpgParser, struct  PgcCmdTable *cmdTabIfo, cdx_uint8 cmdType,cdx_uint8 pgcType, cdx_uint8 pgcIndex);				  	                         
//cdx_int16 process_mpg_forward_info(CdxMpgParserT *MpgParser);
//cdx_int16 process_mpg_backward_info(CdxMpgParserT *MpgParser);
//cdx_int16  MPG_reader_close_file(MpgParserContextT *mMpgParserCxt);
//cdx_int16 DVD_open_next_file(CdxMpgParserT *MpgParser, char *fn);
//cdx_int16 DVD_parse_vmg_info(CdxMpgParserT *MpgParser, cdx_int16 *file_name);


// function interfaces of statusChange.c
void     MpgStatusInitStatusTitleChange(CdxMpgParserT *MpgParser);
void     MpgStatusInitStatusChange(CdxMpgParserT *MpgParser);
void     MpgStatusInitParamsProcess(CdxMpgParserT *MpgParser);
cdx_uint8   MpgStatusSelectSuitPts(CdxMpgParserT *MpgParser);


// function interfaces of mpgTime.c
cdx_int16  MpgTimeMpeg1ReadPackSCR(cdx_uint8 *buf, cdx_uint32 *scr_low,cdx_uint32 *scr_high);
cdx_int16  MpgTimeMpeg2ReadPackSCR(cdx_uint8 *buf, cdx_uint32 *scr_base_low,cdx_uint32 *scr_base_high,cdx_uint32 *scr_ext);
cdx_uint32  MpgTimePts2Ms(cdx_uint32 PTS,cdx_uint32 nBaseSCR,cdx_uint32 nBaseTime);
cdx_int16   MpgTimeCheckPsPts(CdxMpgParserT *MpgParser);
cdx_int16 MpgTimeCheckPsNetWorkStreamPts(CdxMpgParserT *MpgParser, u32 startPos);

cdx_int64  VobCheckStartPts(CdxMpgParserT *MpgParser, cdx_int64 curFpPos);
cdx_int16   VobCheckPsPts(CdxMpgParserT *MpgParser,cdx_int64 fstSysPos);
cdx_int16  VobCheckUseInfo(CdxMpgParserT *MpgParser, cdx_uint8 *buf);

// function interfaces of mpgOpen.c
cdx_uint8  *MpgOpenFindPackStartReverse(cdx_uint8 *buf_end, cdx_uint8 *buf_begin);
cdx_uint8  *MpgOpenFindPackStart(cdx_uint8 *buf, cdx_uint8 *buf_end);
cdx_uint8  *MpgOpenFindNextStartCode(CdxMpgParserT *MpgParser,cdx_uint8 *buf, cdx_uint8 *buf_end);
cdx_uint8  *MpgOpenJudgePacketType(CdxMpgParserT *MpgParser, cdx_uint8 *cur_ptr, cdx_int16 *nRet);
cdx_uint8  *MpgOpenSearchStartCode(cdx_uint8 *startPtr,cdx_uint32 dataLen, cdx_uint32 *startCode);
cdx_int16   MpgOpenReaderOpenFile(CdxMpgParserT *MpgParser, CdxStreamT  *stream);
cdx_uint32  MpgOpenShowDword(cdx_uint8 *buf);

// function interfaces of mpgRead.c
cdx_uint8   MpgReadJudgeNextDataType(CdxMpgParserT *MpgParser);
cdx_uint8   MpgReadParserReadData(CdxMpgParserT *MpgParser);
void        MpgReadNotFindPackStart(CdxMpgParserT *MpgParser);
cdx_int16   MpgReadNotEnoughData(CdxMpgParserT *MpgParser); 
cdx_uint8  *MpgReadProcessAudioPacket(CdxMpgParserT *MpgParser, cdx_uint32 cur_id, cdx_uint8 *cur_packet_ptr,  cdx_int64 *packetLen);
cdx_uint8  *MpgReadFindStartCode(CdxMpgParserT *MpgParser, cdx_int16 *result);
cdx_uint8  *MpgReadJudgePacket(CdxMpgParserT *MpgParser, cdx_uint8 *cur_ptr, cdx_uint32 code, cdx_int16 *result);
cdx_uint8  *MpgReadProcessIsISO11172(CdxMpgParserT *MpgParser, cdx_uint8 *curPtr, cdx_int16 *result, cdx_int64 *packetLen, cdx_uint32 *ptsLow, cdx_uint32 *ptsHigh);
cdx_uint8  *MpgReadProcessNonISO11172(CdxMpgParserT *MpgParser, cdx_uint8 *curPtr, cdx_int16 *result, cdx_int64 *packetLen, cdx_uint32 *ptsLow, cdx_uint32 *ptsHigh);
cdx_int16   MpgReadAddPacketIntoArray(CdxMpgParserT *MpgParser, cdx_uint32 cur_id, cdx_uint8 sub_stream_id,cdx_uint8 *curPtr,cdx_int64 packetLen,cdx_int64 orgPacketLen, cdx_bool bHasPtsFlag); 
void        MpgReadCheckScrPts(CdxMpgParserT *MpgParser, cdx_uint32 cur_id, cdx_uint32 scrLow, cdx_uint32 scrHigh,cdx_uint32 ptsLow, cdx_uint32 ptsHigh);
cdx_int16   MpgReadProcessVideoPacket(CdxMpgParserT *MpgParser, cdx_uint8* curPtr,cdx_uint32 packetLen, cdx_uint8 hasPts, cdx_uint32 nPtsValue);

#endif /* _MPG_FUNC_H_ */
