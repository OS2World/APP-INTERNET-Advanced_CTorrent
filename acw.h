#ifndef ACW_H
#define ACW_H

#define INCL_ERRORS
#define INCL_DOSSEMAPHORES

#define MAXMSGLEN 2 * 1024

#include <os2.h>
#include <bseerr.h>
#include "btfiles.h"
#include "btcontent.h"

extern ULONG  rc_api;

extern HMTX  hmtxStLine;
extern HMTX  hmtxConsole;

extern char  pErrStr[MAXMSGLEN];
extern char  pErrDbgStr[100];
extern char  pIconName[MAXNAMEL];
extern char  pTitle[MAXNAMEL];
extern char  pTitleFileName[1024];
extern float  fFileLen;
extern int  Port;
extern UINT  uFilesMax;
extern UCHAR  *pFileNums;
extern UCHAR  *pFileCreat;
extern ULONG  ulChunkLen;
extern ULONG  ulChunksMax;
extern UCHAR  *pChunkChk;
extern ULONG  ulChunkChk;
extern UCHAR  *pFileExist;
extern LONGLONG  *pFileLen;
extern char  *pDir;
extern char  *pCfgName;
extern char  *pCfg;

extern BTFILE  *pFiles;
extern char  *pArgFiles;

extern int  PeerSeed;
extern int  PeerLeech;
extern int  PeerTot;
extern int  ChunkHave;
extern int  ChunkAvl;
extern int  ChunkTot;
extern float  fDwn;
extern float  fUp;
extern float  fRateDwn;
extern float  fRateUp;
extern int  TrkRef;
extern int  TrkOk;
extern float  fDwnPtg;

extern UCHAR  f_clIcon;
extern UCHAR  f_clTitle;
extern UCHAR  f_clStyle;
extern UCHAR  f_clWin;
extern UCHAR  f_clOrg;
extern UCHAR  f_clInd;
extern UCHAR  f_clPtg;
extern UCHAR  f_clRate;
extern UCHAR  f_clLog;
extern UCHAR  f_clFile;
extern UCHAR  f_clDetach;
extern UCHAR  f_clTime;
extern UCHAR  f_clDbg;
extern UCHAR  f_clUTF;

extern UCHAR  f_suspend;
extern UCHAR  f_Created;

extern double  dnow;
extern double  fFakeUL;
extern double  fFakeDL;

extern UCHAR  f_console;
extern UCHAR  f_max_bandwidth_down;
extern UCHAR  f_max_bandwidth_up;
extern UCHAR  f_seed_hours;
extern UCHAR  f_seed_ratio;
extern UCHAR  f_min_peers;
extern UCHAR  f_max_peers;
extern UCHAR  f_cache_size;
extern UCHAR  f_files;
extern UCHAR  f_CTCS;
extern UCHAR  f_exit;
extern UCHAR  f_Exit;
extern UCHAR  f_Mutex;
extern UCHAR  f_CTCSRq;
extern UCHAR  f_Detach;
extern UCHAR  f_Seed;

extern int  max_bandwidth_down;
extern int  max_bandwidth_up;
extern UINT  seed_hours;
extern double  seed_ratio;
extern UINT  min_peers;
extern UINT  max_peers;
extern UINT  cache_size;

extern char  pWinTimeLeft[20];

void SetTitleIcon(void);
void SetDimPos(void);
void LoadIcons(void);
void RstWinSet(void);

void ParsColRow(char *pColRow);
void ParsOrg(char *pColRow);
void ParsLog(char *pTP);
void OpenLog(void);
void WrLog(char *pStLine);

int ParsFileNums(char *pFlt, char *pList);
void ChkFileNums(BTFILE *p);
void ChkChunks(UCHAR *pFileList);
void TitleVIO();
void IndUpd(void);

//size_t bencode_str_cnv(const char *str, FILE *fp);
void StLine2(char buffer[]);
void StLine3(char buffer[]);
void StLineOut(FILE *stream, char *buf);
time_t ttime(time_t *pTime);
void dtime(void);
int CSC(char *pBufOut, char *pBufIn, size_t BufOutLen, size_t BufInLen, char cFrom);
int CSC1(char *pBufOut, char *pBufIn, size_t BufOutLen, size_t BufInLen, char cFrom, char* fenc);
void FAR Kbd(void*);
void ChkConsole(void);
void StartDetach(int, char **argv);
void FAR Kbd(void*);
int ParsCfg(char **argv, char* pCfgFile);
int ParsFake(char *pFake);
void os2err(ULONG errnum);

extern UCHAR  f_clenc;//an64
extern char  *fenc;//an64

//extern FILE *fl;
//void ErrLog(int err, char *Str);

//extern UCHAR  f_dbg;
//extern FILE  *fdbg;
//extern UCHAR  f_Set;
//extern UCHAR  f_trc;
//extern UCHAR  f_rerr;


USHORT		usWinRow=24;
#endif

