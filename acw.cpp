// defines & includes /*fold00*/

#define INCL_VIO
#define INCL_DOS
#define INCL_WIN
#define INCL_PM
#define INCL_LONGLONG
#define INCL_KBD

#include <os2.h>
#include <stdio.h>
#include <process.h>
#include <io.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/time.h>
#include <conio.h>

#include "bencode.h"
#include "btconfig.h"
#include "btcontent.h"
#include "peerlist.h"
#include "tracker.h"
#include "console.h"
#include "btfiles.h"
#include "bttime.h"
#include "my_iconv.h"
#include "ctcs.h"
#include "parser.h"


#define bcolor0 "\x1B[1;30;40m"
#define bcolor1 "\x1B[1;31;40m"
#define bcolor2 "\x1B[1;32;40m"
#define bcolor3 "\x1B[1;33;40m"
#define bcolor4 "\x1B[1;34;40m"
#define bcolor5 "\x1B[1;35;40m"
#define bcolor6 "\x1B[1;36;40m"
#define bcolor7 "\x1B[1;37;40m"

#define color0 "\x1B[0;30;40m"
#define color1 "\x1B[0;31;40m"
#define color2 "\x1B[0;32;40m"
#define color3 "\x1B[0;33;40m"
#define color4 "\x1B[0;34;40m"
#define color5 "\x1B[0;35;40m"
#define color6 "\x1B[0;36;40m"
#define color7 "\x1B[0;37;40m"


// extern /*FOLD00*/

ULONG  rc_api;
btFiles  a_btfiles;

char  pErrDbgStr[100] = {'\0'};
char  pErrStr[MAXMSGLEN] = {'\0'};

HMTX  hmtxStLine = 0;
HMTX  hmtxConsole = 0;
HSWITCH		hSw;
SWCNTRL		SwData;
RECTL		WinRect;
VIOCURSORINFO	CurData;

HAB		hab = 0;
HMQ		hmq = 0;
HPOINTER  hIcon_act = 0;
HPOINTER	hIcon0 = 0;
HPOINTER	hIcon1 = 0;
TIB		*tib;
PIB		*pib;

float		fFileLen = 0;

ULONG		p_id;
ULONG		pid;
ULONG		tid;
char		pIconName[MAXNAMEL];
char		pTitle[MAXNAMEL];
char		*pTitleDef = "ACT: ";
char		pTitleFileName[1024];
ULONG		IndIcons[2]	// mode:          0 - leecher, 1 - seeder.
                        [3]	// down:          0 - none, 1 - peer, 2 - douwn.
		        [3]	// up:            0 - none, 1 - peer, 2 - up.
		        [2]	// transfer down: 0 - none, 1 - was.
		        [2];	// transfer up:   0 - none, 1 - was.

UCHAR		f_IndM = 0;
UCHAR		f_IndD = 0;
UCHAR		f_IndU = 0;
UCHAR		f_IndTD = 0;
UCHAR		f_IndTU = 0;

ULONG		Icon_now = 0;
ULONG		Icon_new = 0;

//USHORT		usWinRow = 3; //an64 ->acw.h
USHORT		usWinCol = 90;

SWP		WinPos;
VIOMODEINFO	VioMode;

long		lWinPosX = 0;
long		lWinPosY = 0;

UCHAR  f_clIcon = 0;
UCHAR  f_clTitle = 0;
UCHAR  f_clStyle = 2;
UCHAR  f_clWin = 0;
UCHAR  f_clOrg = 0;
UCHAR  f_clInd = 0;
UCHAR  f_clPtg = 0;
UCHAR  f_clRate = 0;
UCHAR  f_clLog = 0;
UCHAR  f_clFile = 0;
UCHAR  f_clDetach = 0;
UCHAR  f_clTime = 0;
UCHAR  f_clDbg = 0;
UCHAR  f_clUTF = 0;
UCHAR  f_clenc = 0;//an64

UCHAR  f_IconOk = 0;
UCHAR  f_WinTitleInit = 0;
UCHAR  f_suspend;
UCHAR  f_Created = 0;

UCHAR PM_init = 0;
int  VioCol = 0;

char  pLogName[CCHMAXPATH] = {'\0'};
int  hFileLog;
float  fPtgNow = -1;
int  Port = 0;
UCHAR  f_LogPtg = 0;
UCHAR  f_LogTime = 0;
float  fPtgDelta = 0;
ULONG  ulTimeDelta = 0;
ULONG  ulTimeNow = 0;
UCHAR  *pFileNums = NULL;
ULONG  ulChunkLen = 1;
UCHAR  *pFileCreat = NULL;
UINT  uFilesMax = 0;
ULONG  ulChunksMax = 0;
UCHAR  *pChunkChk = NULL;
ULONG  ulChunkChk = 0;
UCHAR  *pFileExist = NULL;
LONGLONG  *pFileLen = NULL;
char  *pDir = NULL;
char  *pCfgName = NULL;
char  *pCfg = NULL;

BTFILE  *pFiles = NULL;
char  *pArgFiles = NULL;

const UCHAR  ucLiveChar[3] = {'∙', '+','*'};
UCHAR  ucLiveIdx = 0;

int  PeerSeed;
int  PeerLeech;
int  PeerTot;
int  ChunkHave;
int  ChunkAvl;
int  ChunkTot;
float  fDwn;
float  fUp;
float  fRateDwn, fRateDwnPrev = -1;
float  fRateUp, fRateUpPrev = -1;
int  TrkRef;
int  TrkOk;
float  fDwnPtg = -1, fDwnPtgPrev = -2;

double  dnow;
double  fFakeUL = 1.0;
double  fFakeDL = 1.0;

UCHAR  f_console = 0;
UCHAR  f_max_bandwidth_down = 0;
UCHAR  f_max_bandwidth_up = 0;
UCHAR  f_seed_hours = 0;
UCHAR  f_seed_ratio = 0;
UCHAR  f_min_peers = 0;
UCHAR  f_max_peers = 0;
UCHAR  f_cache_size = 0;
UCHAR  f_files = 0;
UCHAR  f_CTCS = 0;
UCHAR  f_exit = 0;
UCHAR  f_Exit = 0;
UCHAR  f_Mutex = 0;
UCHAR  f_CTCSRq = 0;
UCHAR  f_Detach = 0;
UCHAR  f_Seed = 0;

int  max_bandwidth_down;
int  max_bandwidth_up;
UINT  seed_hours;
double  seed_ratio;
UINT  min_peers;
UINT  max_peers;
UINT  cache_size;

int  PartTot = 0, PartHave = 0;
char pWinTimeLeft[20] = {'\0'};


int InitPM(void);
int ParsWinParam(void);
void SetIcon(HPOINTER Icon);
int BaseName(char *, char *);
void SetWinList(void);
//void IndUpd(void);
void dtime(void);

int InitPM(void) /*fold00*/
{

  APIRET  rc;

   VioGetCurType(&CurData, 0);
   CurData.attr = -1;
   VioSetCurType(&CurData, 0);

   VioSetAnsi(1, 0);
   VioMode.cb = sizeof(VioMode);
   rc = VioGetMode(&VioMode, 0);
   VioCol = VioMode.col;

   p_id = getpid();
   DosGetInfoBlocks (&tib, &pib);

   if (pib->pib_ultype == 2)
      pib->pib_ultype = 3;  // морфинг в PM
   else
      return 1;

   hab = WinInitialize(0);
   if(!hab) return 1;

   hmq = WinCreateMsgQueue(hab, 0);
   if(!hmq) {
      WinTerminate(hab);
      return 1;
   }

   hSw = WinQuerySwitchHandle(0, p_id);
   if(!hSw) {
      WinTerminate(hab);
      WinDestroyMsgQueue(hmq);
      return 1;
   }

   WinQuerySwitchEntry(hSw, &SwData);
   WinQueryWindowRect(SwData.hwnd, &WinRect);

   WinRect.xRight += 30;
   WinRect.yBottom -= 30;

   WinInvalidateRect(SwData.hwnd, &WinRect, 1);
   BaseName(arg_metainfo_file, pTitleFileName);

   PM_init = 1;
   return 0;
}

void SetWinList(void) /*fold00*/
{

  char  pWinStr[MAXNAMEL + 4];
  char  *ptr;
  UCHAR  f_Set = 0;

  if(!f_WinTitleInit || f_clPtg || f_clRate || f_clTime) {
    if(!f_WinTitleInit) {
      f_WinTitleInit = 1;
      f_Set = 1;
    }
    ptr = pWinStr + sprintf(pWinStr, "%s", pTitleDef);
    if(f_clPtg) {
      if(fDwnPtg < 0) ptr += sprintf(ptr, "0%% ");
      else if(fDwnPtg != 100) ptr += sprintf(ptr, "%.1f%% ", fDwnPtg);
      else ptr += sprintf(ptr, "100%%");
      if(fDwnPtg != fDwnPtgPrev) {
        fDwnPtgPrev = fDwnPtg;
	f_Set = 1;
      }
    }
    if(f_clRate) {
      if(fRateDwn) ptr += sprintf(ptr, "%.1f", fRateDwn);
      else ptr += sprintf(ptr, "0");
      if(fRateDwn != fRateDwnPrev) {
	fRateDwnPrev = fRateDwn;
	f_Set = 1;
      }
      if(fRateUp) ptr += sprintf(ptr, "%.1f ", fRateUp);
      else ptr += sprintf(ptr, "0 ");
      if(fRateUp != fRateUpPrev) {
	fRateUpPrev = fRateUp;
	f_Set = 1;
      }
    }
//    if(f_clTime && (f_clStyle == 2 || f_clStyle == 3)) {
    if(f_clTime) {
      ptr += sprintf(ptr, "%s ", pWinTimeLeft);
      f_Set = 1;
    }

    if(f_Set) {
      if(f_clTitle)  // задан заголовок.
        snprintf(ptr,
	  (strlen(pTitle) + 1 >= sizeof(pWinStr) - (ptr - pWinStr)) ?
	  sizeof(pWinStr) - (ptr - pWinStr) : strlen(pTitle) + 1,
	  "%s", pTitle);
      else
        snprintf(ptr,
	  (strlen(pTitleFileName) + 1 >= sizeof(pWinStr) - (ptr - pWinStr)) ?
	  sizeof(pWinStr) - (ptr - pWinStr) : strlen(pTitleFileName) + 1,
	  "%s", pTitleFileName);

    strcpy(SwData.szSwtitle, pWinStr);
    WinChangeSwitchEntry(hSw, &SwData);
    }
  }
}

void SetIcon(HPOINTER hIcon) /*fold00*/
{
  if (hmq && hIcon) {
    WinSendMsg(SwData.hwnd, WM_SETICON, (MPARAM)hIcon, 0);
    WinInvalidateRect(SwData.hwnd, &WinRect, 1);
  }
}


void RstWinSet(void) /*fold00*/
{

  char  im, id, iu, itd, itu;

  WinFreeFileIcon(hIcon0);
  WinFreeFileIcon(hIcon1);

  WinFreeFileIcon(hIcon_act);

  for(im = 0; im < 2; im++)
    for(id = 0; id < 3; id++)
      for(iu = 0; iu < 3; iu++)
        for(itd = 0; itd < 2; itd++)
          for(itu = 0; itu < 2; itu++)
            WinFreeFileIcon(IndIcons[im][id][iu][itd][itu]);

  WinDestroyMsgQueue(hmq);
  WinTerminate(hab);
}


void SetTitleIcon(void) /*fold00*/
{

  if(!InitPM()) {
    HPOINTER hIcon = 0;
    while(1) {
      if(f_clIcon) {
	hIcon1 = WinLoadFileIcon(pIconName, 0);
	if(hIcon1) {
	  hIcon = hIcon1;
	  break;
	}
      }
      hIcon0 = WinLoadFileIcon("act.ico", 0);
      if(hIcon0) {
        hIcon = hIcon0;
	break;
      }
      hIcon_act = WinLoadPointer(HWND_DESKTOP, 0, 3L);
      hIcon = hIcon_act;
      break;
    }
    SetIcon(hIcon);
    SetWinList();
  }
}


void SetDimPos(void) /*fold00*/
{

  long  lTemp1, lTemp2;
  APIRET  rc;

  if(PM_init) {
    if(f_clWin) {
      VioMode.col = usWinCol;
      VioMode.row = usWinRow;
      VioSetMode(&VioMode, 0);
      VioGetMode(&VioMode, 0);
      VioCol = VioMode.col;
      VioScrollUp(0, 0, (USHORT)(VioMode.row - 1), USHORT(VioMode.col - 1),
      (USHORT)(VioMode.row + 1), "\0\0", 0);
    }
    if(f_clOrg) {
      lTemp1 = WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN);
      lTemp2 = WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN);
      rc = WinQueryWindowPos(SwData.hwnd, &WinPos);
      if(rc && (lWinPosX < lTemp1) && (lWinPosY < lTemp2))
        WinSetWindowPos(SwData.hwnd, HWND_TOP, lWinPosX, lWinPosY,
                        WinPos.cx, WinPos.cy, WinPos.fl);
    }
  }
}

int ParsWinParam(char *pStr, long *lX, long *lY) /*fold00*/
{

//8192  row = 8192/col  col = 10...255

  ULONG  Temp, Len;
  char  *pCharSet = "-0123456789";
  char  *ptr1, *ptr2;


  Len = strlen(pStr);
  if(!Len) return -1;
  ptr1 = strpbrk(pStr, pCharSet);  // ищем начало цифр.
  if(!ptr1 || pStr + Len < ptr1 + 3) return -1;  // в хвосте должно быть минимум 'x,y'.
  Temp = strspn(ptr1, pCharSet);   // длина цифр.
  ptr2 = ptr1 + Temp;
  *(ptr2++) = '\0';
  Len -= Temp;

  ptr2 = strpbrk(ptr2, pCharSet);  // ищем начало цифр.
  if(!ptr2) return -1;
  Temp = strspn(ptr1, pCharSet);    // длина цифр.
  *(ptr2 + Temp) = '\0';

  *lX = atol(ptr1);
  *lY = atol(ptr2);

  return 0;

}

void ParsColRow(char *pColRow) /*fold00*/
{

  long  lTemp1, lTemp2;

  if(ParsWinParam(pColRow, &lTemp1, &lTemp2)) {
    printf("! '-win' argument is incorrect.\n");
    return;
  }

  if(lTemp1 < 10 || lTemp1 > 255 || lTemp2 < 1 || lTemp1 * lTemp2 > 8192) {
    printf("! incorrect numbers of columns or rows\n.");
    return;
  }

  usWinCol = (USHORT)lTemp1;
  usWinRow = (USHORT)lTemp2;
  f_clWin = 1;

}

void ParsOrg(char *pOrg) /*fold00*/
{

  if(ParsWinParam(pOrg, &lWinPosX, &lWinPosY)) {
    printf("! '-org' argument is incorrect.\n");
    return;
  }

  if(lWinPosX > 30000) lWinPosX = 30000;
  if(lWinPosY > 30000) lWinPosY = 30000;
  if(lWinPosX < -30000) lWinPosX = -30000;
  if(lWinPosY < -30000) lWinPosY = -30000;
  f_clOrg = 1;

}

int ParsFake(char *pStr) /*fold00*/
{

  ULONG  ulLen;
  char  *pCharSet = ".0123456789";
  char  *ptr1, *pEnd;


  ulLen = strlen(pStr);
  if(!ulLen) return -1;
  pEnd = pStr + ulLen;  // позиция \0.
  if(*pStr == ',' || *pStr == ';') {
    ++pStr;
    fFakeDL = 1.0;
  }
  else {
    pStr = strpbrk(pStr, pCharSet);  // ищем начало цифр.
    if(!pStr) return -1;
    ptr1 = strspnp(pStr, pCharSet);  // ищем конец цифр.
    if(ptr1) {
      *ptr1 = '\0';
      fFakeDL = atof(pStr);
      if(ptr1 == pEnd) { fFakeUL = 1.0;	return 0; }
      pStr = ptr1 + 1;
    }
    else {
      fFakeDL = atof(pStr);
      fFakeUL = 1.0;
      return 0;
    }
  }

  pStr = strpbrk(pStr, pCharSet);  // ищем начало цифр.
  if(!pStr) { fFakeUL = 1.0; return 0; }
  ptr1 = strspnp(pStr, pCharSet);  // ищем конец цифр.
  if(ptr1) *ptr1 = '\0';
  fFakeUL = atof(pStr);
  return 0;

}

void ParsLog(char *pTP) /*fold00*/
{

  char  *ptr;
  ULONG  Len;
  long  lTemp;

  f_clLog = 0;
  f_LogPtg = 0;
  f_LogTime = 0;
  fPtgDelta = 0;


  Len = strlen(pTP);
  if(!Len) return;
  fPtgDelta = strtod(pTP, &ptr);
  if(fPtgDelta > 0.0) f_LogPtg = 1;
  if(ptr >= pTP + Len - 1) return;
  lTemp = strtol(ptr + 1, NULL, 10);
  if(lTemp > 0) {
    ulTimeDelta = (ULONG)lTemp;
    f_LogTime = 1;
    f_clLog = 1;
  }
  else if(f_LogPtg) f_clLog = 1;

}

void LoadIcons(void) /*fold00*/
{

   ULONG ulTemp;

   if(PM_init && f_clInd && ((f_clStyle == 1) || (f_clStyle == 2))) {

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 10L);
      if(!ulTemp) return;
      IndIcons[0][0][0][0][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 11L);
      if(!ulTemp) return;
      IndIcons[0][0][0][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 12L);
      if(!ulTemp) return;
      IndIcons[0][0][0][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 13L);
      if(!ulTemp) return;
      IndIcons[0][0][0][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 14L);
      if(!ulTemp) return;
      IndIcons[0][1][0][0][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 15L);
      if(!ulTemp) return;
      IndIcons[0][1][0][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 16L);
      if(!ulTemp) return;
      IndIcons[0][1][0][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 17L);
      if(!ulTemp) return;
      IndIcons[0][1][0][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 18L);
      if(!ulTemp) return;
      IndIcons[0][2][0][0][0] = ulTemp;
      IndIcons[0][2][0][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 19L);
      if(!ulTemp) return;
      IndIcons[0][2][0][0][1] = ulTemp;
      IndIcons[0][2][0][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 20L);
      if(!ulTemp) return;
      IndIcons[0][0][1][0][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 21L);
      if(!ulTemp) return;
      IndIcons[0][0][1][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 22L);
      if(!ulTemp) return;
      IndIcons[0][0][1][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 23L);
      if(!ulTemp) return;
      IndIcons[0][0][1][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 24L);
      if(!ulTemp) return;
      IndIcons[0][1][1][0][0] = ulTemp;
      IndIcons[1][1][1][0][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 25L);
      if(!ulTemp) return;
      IndIcons[0][1][1][1][0] = ulTemp;
      IndIcons[1][1][1][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 26L);
      if(!ulTemp) return;
      IndIcons[0][1][1][0][1] = ulTemp;
      IndIcons[1][1][1][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 27L);
      if(!ulTemp) return;
      IndIcons[0][1][1][1][1] = ulTemp;
      IndIcons[1][1][1][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 28L);
      if(!ulTemp) return;
      IndIcons[0][1][2][0][0] = ulTemp;
      IndIcons[0][1][2][0][1] = ulTemp;
      IndIcons[1][1][2][0][0] = ulTemp;
      IndIcons[1][1][2][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 29L);
      if(!ulTemp) return;
      IndIcons[0][1][2][1][0] = ulTemp;
      IndIcons[0][1][2][1][1] = ulTemp;
      IndIcons[1][1][2][1][0] = ulTemp;
      IndIcons[1][1][2][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 30L);
      if(!ulTemp) return;
      IndIcons[0][2][1][0][0] = ulTemp;
      IndIcons[0][2][1][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 31L);
      if(!ulTemp) return;
      IndIcons[0][2][1][0][1] = ulTemp;
      IndIcons[0][2][1][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 32L);
      if(!ulTemp) return;
      IndIcons[0][2][2][0][0] = ulTemp;
      IndIcons[0][2][2][0][1] = ulTemp;
      IndIcons[0][2][2][1][0] = ulTemp;
      IndIcons[0][2][2][1][1] = ulTemp;
      IndIcons[1][2][2][0][0] = ulTemp;
      IndIcons[1][2][2][0][1] = ulTemp;
      IndIcons[1][2][2][1][0] = ulTemp;
      IndIcons[1][2][2][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 33L);
      if(!ulTemp) return;
      IndIcons[0][0][2][0][0] = ulTemp;
      IndIcons[0][0][2][0][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 34L);
      if(!ulTemp) return;
      IndIcons[0][0][2][1][0] = ulTemp;
      IndIcons[0][0][2][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 35L);
      if(!ulTemp) return;
      IndIcons[1][0][0][0][0] = ulTemp;
      IndIcons[1][0][0][1][0] = ulTemp;
      IndIcons[1][1][0][0][0] = ulTemp;
      IndIcons[1][1][0][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 36L);
      if(!ulTemp) return;
      IndIcons[1][0][0][0][1] = ulTemp;
      IndIcons[1][0][0][1][1] = ulTemp;
      IndIcons[1][1][0][0][1] = ulTemp;
      IndIcons[1][1][0][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 37L);
      if(!ulTemp) return;
      IndIcons[1][0][1][0][0] = ulTemp;
      IndIcons[1][0][1][1][0] = ulTemp;
      IndIcons[1][1][1][0][0] = ulTemp;
      IndIcons[1][1][1][1][0] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 38L);
      if(!ulTemp) return;
      IndIcons[1][0][1][0][1] = ulTemp;
      IndIcons[1][0][1][1][1] = ulTemp;
      IndIcons[1][1][1][0][1] = ulTemp;
      IndIcons[1][1][1][1][1] = ulTemp;

      ulTemp = WinLoadPointer(HWND_DESKTOP, 0, 39L);
      if(!ulTemp) return;
      IndIcons[1][0][2][0][0] = ulTemp;
      IndIcons[1][0][2][0][1] = ulTemp;
      IndIcons[1][0][2][1][0] = ulTemp;
      IndIcons[1][0][2][1][1] = ulTemp;
      IndIcons[1][1][2][0][0] = ulTemp;
      IndIcons[1][1][2][0][1] = ulTemp;
      IndIcons[1][1][2][1][0] = ulTemp;
      IndIcons[1][1][2][1][1] = ulTemp;

      f_IconOk = 1;

   }
}


int BaseName(char *StrIn, char *StrOut) /*fold00*/
{

   char		Ch, f_Dot = 0, f_Text = 0, Idx = 0, IdxDot = 0;
   int		i, len;

   if((len = strlen(StrIn)) == 0 || len > PATH_MAX) return -1;

   for(i = 0; i <= len; i++) {
      Ch = *StrIn++;
      switch(Ch) {
         case '\0':
            if(f_Dot) {
               StrOut[IdxDot] = Ch;
               return IdxDot;		// длина строки.
            }
            else {
               StrOut[Idx] = Ch;
               return Idx;		// длина строки.
            }
//            break;
         case '\\':
         case '/':
         case ':':
            Idx = 0;
            IdxDot = 0;
            f_Dot = 0;
            f_Text = 0;
            break;
         case '.':
            IdxDot = Idx;
            StrOut[Idx++] = Ch;
            f_Dot = f_Text;
            break;
         default:
            StrOut[Idx++] = Ch;
            f_Text = 1;
            break;
      }
   }
   return -1;
}


void OpenLog(void) /*fold00*/
{

  int  PathLen = 0;


  PathLen = strlen(arg_metainfo_file);
  if(!PathLen) {
    f_clLog = 0;
    return;
  }
  if(stricmp(arg_metainfo_file + (PathLen - 8), ".torrent")) {
    f_clLog = 0;
    return;
  }
  strcpy(pLogName, arg_metainfo_file);
  memcpy(pLogName + (PathLen - 8), ".log", 5);

  if(!access(pLogName, F_OK | W_OK)) {  //  существует и доступен на запись.
    if((hFileLog = open(pLogName, O_WRONLY | O_BINARY | O_APPEND, S_IREAD)) == -1)
       f_clLog = 0;
  }
  else if(!access(pLogName, F_OK))  //  существует и недоступен на запись.
    f_clLog = 0;
  else  // не существует
    if((hFileLog = open(pLogName, O_CREAT | O_BINARY | O_WRONLY,
                                  S_IRWXU, S_IRWXO)) == -1)
      f_clLog = 0;
}


void WrLog(char *pStLine) /*fold00*/
{

  char  *ptr;
  UCHAR  f_Log = 0;
  time_t  ltime;
  struct tm  tmbuf;

  char *pLogLine = new char[1024];

  ltime = time(NULL);
  if(f_LogTime) {
    if(((ltime - ulTimeNow) / 60) >= ulTimeDelta) {
      ulTimeNow = ltime;
      f_Log = 1;
    }
  }
  if(f_LogPtg && (fDwnPtg - fPtgNow >= fPtgDelta || fDwnPtg == 100.0)) {
    fPtgNow = fDwnPtg;
    f_Log = 1;
  }
  if(f_Log) {
    f_Log = 0;

// 2007/08/24 09:20:26 [+ччмм"]
// [+ччмм] - опционное смещение от UTC :)

    *pLogLine = '\n';
    ptr = pLogLine + 1;

    _localtime(&ltime, &tmbuf);
    sprintf(ptr, "%4.4d/%2.2d/%2.2d %2.2d:%2.2d:%2.2d",
            tmbuf.tm_year + 1900, tmbuf.tm_mon + 1, tmbuf.tm_mday,
            tmbuf.tm_hour, tmbuf.tm_min, tmbuf.tm_sec);
    ptr += 5 + 3 + 2 + 1 + 3 + 3 + 2;

    sprintf(ptr, " port:%5.d  ", Port);
    ptr += 13;
    strcat(pLogLine, pStLine);

    if(write(hFileLog, pLogLine, strlen(pLogLine)) != strlen(pLogLine)) {
      f_clLog = 0;
      close(hFileLog);
    }
    if(f_LogPtg && fDwnPtg == 100.0) {
      f_LogPtg = 0;
      if(!f_LogTime) {
        f_clLog = 0;
        close(hFileLog);
      }
    }
  }

  delete []pLogLine;
//  delete []pStLine;

}

int ParsFileNums(char *pFlt, char *pList) /*fold00*/
{

// первый проход при uFilesMax == 0 на этапе парсинга командлайна на предмет явных ошибок.

  char  *pDigSet = "0123456789";
  char  *pStr, *pS1, *pS2, *pT;
  long  lTemp, ulLen;
  UCHAR  f_Range = 0, f_All = 0, f_L2A = 0, f_err = 0;
  char  cChr;
  long  lPrevVal = 0;


  lTemp = strlen(pList);
  if(!lTemp) return -1;

  pStr = new char[lTemp + 2];
  strcpy(pStr, pList);
  *(pStr + lTemp) = ',';
  *(pStr + lTemp + 1) = '\0';
  *pFlt = '\0';
  pT = pFlt;

  if(uFilesMax) {
    if(!pFileNums) pFileNums = new UCHAR[uFilesMax + 2];
    for(int i = 0; i <= uFilesMax; i++) pFileNums[i] = 0;
  }

  pS1 = strtok(pStr, ";,");

  do {
    lPrevVal = 0;
    f_Range = 0;
    pS2 = pS1;

    while(1) {
      f_L2A = 0;
      pS1 = strpbrk(pS1, pDigSet);  // ищем начало цифр.
      if(!pS1) {
        if(strstr(pS2, "...") || strchr(pS2, '*')) { f_All = 1; *(pT++) = '*'; *pT = '\0'; }
	break;
      }

      pS2 = strspnp(pS1, pDigSet);  // конец цифр + 1.
      if(pS2) { cChr = *pS2; *(pS2++) = '\0'; }  // за цифрами что-то есть.

      lTemp = atoi(pS1);
      if(!lTemp) { lTemp = 1; sprintf(pS1, "%lu", lTemp); }  // замена '0' на '1'.

      if(uFilesMax) {
	if(lTemp > uFilesMax) { lTemp = uFilesMax; f_L2A = 1; }
        pFileNums[lTemp] = 1;
      }

      if(f_Range && lPrevVal == lTemp) {
        f_Range = 0;
        *(pT - 1) = ',';
        if(pS2) { pS1 = pS2; continue; }
        *pT = '\0';
        break;
      }

      if(f_Range) {
	if(lPrevVal > lTemp) { f_Range = 0; *(pT - 1) = ','; }
	else if(uFilesMax) for(int i = lPrevVal; i <= lTemp; i++) pFileNums[i] = 1;
      }

      if(!f_L2A) strcpy(pT, pS1);
      else (ulLen = sprintf(pT, "%lu", lTemp));
      if(!pS2) {  // конец токена.
	if(!f_L2A) pT += strlen(pS1);
	else pT += ulLen;
	*(pT++) = ','; *pT = '\0'; break;
      }
      if(!f_L2A) pT += pS2 - pS1 - 1;
      else pT += ulLen;
      pS1 = pS2;
      lPrevVal = lTemp;

      if(cChr == '+') { f_Range = 0; *(pT++) = '+'; continue; }
      if(cChr == '-') {
        if(f_Range) { f_err = 1; break; }
        else { f_Range = 1; *(pT++) = '-'; continue; }
      }
      f_Range = 0;
      *(pT++) = ',';
    }
    if(f_All || f_err) break;
    if(pT != pFlt && *(pT - 1) != ',') { *(pT - 1) = ','; *pT = '\0'; }
    pS1 = strtok(NULL, ";,");
  } while(pS1);
  delete []pStr;
  lTemp = strlen(pFlt);
  if(!lTemp || f_err) { *pFlt = '\0'; return -1; }
  if(*(pFlt + lTemp - 1) == ',') *(pFlt + lTemp - 1) = '\0';
  if(f_All && uFilesMax) for(int i = 1; i <= uFilesMax; i++) pFileNums[i] = 1;

/*
if(!uFilesMax) return 0;
for(int i = 0; i <= uFilesMax; i++) {
if(pFileNums[i]) printf("[%d]", i);
}
exit(0);
*/

  return 0;
}

//size_t bencode_str_cnv(const char *str, FILE *fp) /*FOLD00*/
/*{

  size_t  rc;
  int  Len;
  char  *pBufOut = 0;

  Len = strlen(str);
  pBufOut = new char[Len * 6 + 1];
  rc = CSC(pBufOut, (char*)str, Len * 6 + 1, Len, '\0');

  if(!rc) rc = bencode_buf(pBufOut, strlen(pBufOut), fp);
  else rc = bencode_buf(str, strlen(str), fp);

  delete []pBufOut;

  return rc;
}
*/


void StLine2(char buffer[]) /*fold00*/
{

  PartTot = 0; PartHave = 0;
  char partial[30] = "";

  if( BTCONTENT.GetFilter() && !BTCONTENT.GetFilter()->IsEmpty() ){
    BitField tmpBitField = *BTCONTENT.pBF;
    tmpBitField.Except(BTCONTENT.GetFilter());
    PartTot = (int)(BTCONTENT.GetNPieces() - BTCONTENT.GetFilter()->Count());
    PartHave = (int)(tmpBitField.Count());
    sprintf(partial, "P:%d/%d ", PartHave, PartTot);
  }
  else { PartTot = 0, PartHave = 0; }

  char checked[14] = "";
  if( BTCONTENT.CheckedPieces() < BTCONTENT.GetNPieces() ){
    sprintf( checked, "Checking: %d%%",
      100 * BTCONTENT.CheckedPieces() / BTCONTENT.GetNPieces() );
  }

  PeerSeed = (int)WORLD.GetSeedsCount();
  PeerLeech = (int)(WORLD.GetPeersCount() - WORLD.GetSeedsCount());
  PeerTot = (int)Tracker.GetPeersCount();

  ChunkHave = (int)BTCONTENT.pBF->Count();
  ChunkTot = (int)BTCONTENT.GetNPieces();
  ChunkAvl = (int)WORLD.Pieces_I_Can_Get();

  fDwn = (float)Self.TotalDL() / 1024.0 / 1024.0;
  fUp = (float)Self.TotalUL() / 1024.0 / 1024.0;

  fRateDwn = (float)Self.RateDL() / 1024.0;
  fRateUp = (float)Self.RateUL() / 1024.0;

  TrkRef = (int)Tracker.GetRefuseClick();
  TrkOk = (int)Tracker.GetOkClick();

  fDwnPtg = (float)100.0 * ChunkHave / ChunkTot;

  long remain = -1;
  char timeleft[100];
  size_t rate;
  if( !BTCONTENT.Seeding() || BTCONTENT.FlushFailed() ){  // downloading
    f_Seed = 0;
    rate = Self.RateDL();
    if(rate) {
      if(!PartTot) {
        if( BTCONTENT.GetLeftBytes() < (uint64_t)rate << 22 )
          remain = (long)(BTCONTENT.GetLeftBytes() / rate / 60);
	else remain = 99999;
      }
      else {  // выборочное скачивание
	LONGLONG  llLeft = (PartTot - PartHave) * ulChunkLen;
        if(llLeft < (uint64_t)rate << 22 )
          remain = (long)(llLeft / rate / 60);
	else remain = 99999;
      }
    }
  }else{  //seeding
    f_Seed = 1;
    if( cfg_seed_hours )
      remain = cfg_seed_hours * 60 - (now - BTCONTENT.GetSeedTime()) / 60;
    else {
      rate = Self.RateUL();
      if(rate){
      // don't overflow remain
        if( cfg_seed_ratio *
            (Self.TotalDL() ? Self.TotalDL() : BTCONTENT.GetTotalFilesLength()) -
            Self.TotalUL() < (uint64_t)rate << 22 )
          remain = (long)( cfg_seed_ratio *
            (Self.TotalDL() ? Self.TotalDL() : BTCONTENT.GetTotalFilesLength()) -
            Self.TotalUL() ) / rate / 60;
	else remain = 99999;
      }
    }
  }
  if( remain >= 0 ){
    if( remain < 60000 ) {  // 1000 hours
      snprintf(timeleft, sizeof(timeleft), "%s%d%s:%s%2.2d%s",
	(f_Seed) ? bcolor1 : bcolor2, (int)(remain / 60), color7,
	(f_Seed) ? bcolor1 : bcolor2, (int)(remain % 60), color7);
      sprintf(pWinTimeLeft, "%d:%2.2d", (int)(remain / 60), (int)(remain % 60));
    }
    else {
      sprintf(timeleft, "%s>999h%s", (f_Seed) ? bcolor1 : bcolor2, color7);
      strcpy(pWinTimeLeft, ">999h");
    }
  }else {
    sprintf(timeleft, "%sN/A%s", (f_Seed) ? bcolor1 : bcolor2, color7);
    strcpy(pWinTimeLeft, "N/A");
  }


// *1:2:17 1.60.0k/s 0.030.00M 699:795:795 87.9% 0:1 17:18 Checking: 97%
  sprintf(buffer,

"%s%c\
%s%d%s:%s%d%s:%s%d\
 %s%.1f%s%s%.1f%s%sk/s\
 %s%.2f%s%s%.2f%s%sM\
 %s%d%s:%s%d%s:%s%d\
 %s%.1f%s%%\
 %s%d%s:%s%d%s\
 %s%s\
 %s",

/*   %s%c   */
    bcolor5, ucLiveChar[ucLiveIdx++],
/*   %s%d%s: %s%d%s: %s%d  0:0:0   */
    (PeerSeed) ? bcolor2 : bcolor0, PeerSeed, color7,
    (PeerLeech) ? bcolor1 : bcolor0, PeerLeech, color7,
    (PeerTot) ? bcolor3 : bcolor0, PeerTot,
/*   %s%.1f%s %s%.1f%s%sk/s  0.00.0k/s   */
    (fRateDwn >= 0.1) ? bcolor2 : bcolor0, fRateDwn, color2,
    (fRateUp >= 0.1) ? bcolor1 : bcolor0, fRateUp, color1, color7,
/* %s%.2f%s %s%.2f%s%sM  0.000.00M   */
    (fDwn) ? bcolor2 : bcolor0, fDwn, color2,
    (fUp) ? bcolor1 : bcolor0, fUp, color1, color7,
/*   %s%d%s: %s%d%s: %s%d  642:642:642   */
    (fDwnPtg != 100) ? bcolor2 : color2, ChunkHave, color7,
    (ChunkAvl > ChunkHave) ? bcolor3 : color3, ChunkAvl, color7,
    (fDwnPtg != 100 && ChunkAvl < ChunkTot) ? bcolor2 : color2, ChunkTot,
/*   %s%.1f%s%%  100.0%   */
    (fDwnPtg != 100) ? bcolor6 : color6, fDwnPtg, color7,
/*   %s%d%s: %s%d%s  0:0   */
    (TrkRef) ? bcolor3 : bcolor0, TrkRef, color7,
    (TrkOk) ? bcolor3 : bcolor0, TrkOk, color7,

    partial, timeleft,

    (Tracker.GetStatus()==T_CONNECTING) ? "Connecting" :
      ( (Tracker.GetStatus()==T_READY) ? "Connected" :
          (Tracker.IsRestarting() ? "Restarting" :
            (Tracker.IsQuitting() ? "Quitting" :
              (WORLD.IsPaused() ? "Paused" : checked))) )
  );


  if(ucLiveIdx >= 3) ucLiveIdx = 0;


  if(f_clLog) {
//    ∙0:0:0 0.00.0k/s 0.000.00M 642:642:642 100.0% 0:0
    char *pStLine = new char[1024];

    sprintf(pStLine, "%d:%d:%d %.1f%.1fk/s %.2f%.2fM %d:%d:%d %.1f%% %d:%d %s",
                      PeerSeed, PeerLeech, PeerTot,
                      fRateDwn, fRateUp,
                      fDwn, fUp,
                      ChunkHave, ChunkAvl, ChunkTot,
                      fDwnPtg,
                      TrkRef, TrkOk,
                      partial);
    WrLog(pStLine);
    delete []pStLine;
  }
  IndUpd();
}

void StLine3(char buffer[]) /*fold00*/
{

  PartTot = 0; PartHave = 0;

  char partial[30] = "";
  if( BTCONTENT.GetFilter() && !BTCONTENT.GetFilter()->IsEmpty() ){
    BitField tmpBitField = *BTCONTENT.pBF;
    tmpBitField.Except(BTCONTENT.GetFilter());
    PartTot = (int)(BTCONTENT.GetNPieces() - BTCONTENT.GetFilter()->Count());
    PartHave = (int)(tmpBitField.Count());
    sprintf(partial, "P:%d/%d ", PartHave, PartTot);
  }
  else { PartTot = 0, PartHave = 0; }

  char checked[14] = "";
  if( BTCONTENT.CheckedPieces() < BTCONTENT.GetNPieces() ){
    sprintf( checked, "Checking: %d%%",
      100 * BTCONTENT.CheckedPieces() / BTCONTENT.GetNPieces() );
  }

  PeerSeed = (int)(WORLD.GetSeedsCount());
  PeerLeech = (int)(WORLD.GetPeersCount()) - WORLD.GetSeedsCount();
  PeerTot = (int)(Tracker.GetPeersCount());

  ChunkHave = (int)(BTCONTENT.pBF->Count());
  ChunkTot = (int)(BTCONTENT.GetNPieces());
  ChunkAvl = (int)(WORLD.Pieces_I_Can_Get());

  fDwn = (float)((Self.TotalDL() / 1024.0) / 1024.0);
  fUp = (float)((Self.TotalUL() / 1024.0) / 1024.0);

  fRateDwn = (float)(Self.RateDL() / 1024.0);
  fRateUp = (float)(Self.RateUL() / 1024.0);

  TrkRef = (int)(Tracker.GetRefuseClick());
  TrkOk = (int)(Tracker.GetOkClick());

  fDwnPtg = (float)100.0 * ChunkHave / ChunkTot;

  long remain = -1;
  char timeleft[100];
  size_t rate;
  if( !BTCONTENT.Seeding() || BTCONTENT.FlushFailed() ){  // downloading
    f_Seed = 0;
    rate = Self.RateDL();
    if(rate) {
      if(!PartTot) {
        if( BTCONTENT.GetLeftBytes() < (uint64_t)rate << 22 )
          remain = (long)(BTCONTENT.GetLeftBytes() / rate / 60);
	else remain = 99999;
      }
      else {  // выборочное скачивание
	LONGLONG  llLeft = (PartTot - PartHave) * ulChunkLen;
        if(llLeft < (uint64_t)rate << 22 )
          remain = (long)(llLeft / rate / 60);
	else remain = 99999;
      }
    }
  }else{  //seeding
    f_Seed = 1;
    if( cfg_seed_hours )
      remain = cfg_seed_hours * 60 - (now - BTCONTENT.GetSeedTime()) / 60;
    else {
      rate = Self.RateUL();
      if(rate){
      // don't overflow remain
        if( cfg_seed_ratio *
            (Self.TotalDL() ? Self.TotalDL() : BTCONTENT.GetTotalFilesLength()) -
            Self.TotalUL() < (uint64_t)rate << 22 )
          remain = (long)( cfg_seed_ratio *
            (Self.TotalDL() ? Self.TotalDL() : BTCONTENT.GetTotalFilesLength()) -
            Self.TotalUL() ) / rate / 60;
	else remain = 99999;
      }
    }
  }

  if( remain >= 0 ){
    if( remain < 60000 ) {  // 1000 hours
      snprintf(timeleft, sizeof(timeleft), "%s%d%s:%s%2.2d%s",
	(f_Seed) ? bcolor1 : bcolor2, (int)(remain / 60), color7,
	(f_Seed) ? bcolor1 : bcolor2, (int)(remain % 60), color7);
      sprintf(pWinTimeLeft, "%d:%2.2d", (int)(remain / 60), (int)(remain % 60));
    }
    else {
      sprintf(timeleft, "%s>999h%s", (f_Seed) ? bcolor1 : bcolor2, color7);
      strcpy(pWinTimeLeft, ">999h");
    }
  }else {
    sprintf(timeleft, "%sN/A%s", (f_Seed) ? bcolor1 : bcolor2, color7);
    strcpy(pWinTimeLeft, "N/A");
  }

  sprintf(buffer,
// * 0/1/130 [564/1155/1179] 48.6% D[25.38M 1.6k/s] U[1.0M 0.6k/s] E[0/1]

"%s%c\
 %s%d%s/%s%d%s/%s%d\
 %s[%s%d%s/%s%d%s/%s%d%s]\
 %s%.1f%s%%\
 %sD%s[%s%.2f%sM %s%.1f%sk/s]\
 %sU%s[%s%.2f%sM %s%.1f%sk/s]\
 %sE%s%[%s%d%s/%s%d%s]\
 %s%s\
 %s",

/*   %s%c   */
    bcolor5, ucLiveChar[ucLiveIdx++],
/*   %s%d%s/ %s%d%s/ %s%d  0:0:0   */
    bcolor2, PeerSeed, color7,
    bcolor1, PeerLeech, color7,
    color3, PeerTot,
/*   %s[%s%d%s/%s%d%s/%s%d%s]   */
    color7, bcolor3, ChunkHave, color7,
    bcolor3, ChunkAvl, color7,
    bcolor3, ChunkTot, color7,
/*   %s%.1f%s%%  100.0%   */
//    (fDwnPtg != 100) ? bcolor6 : color6, fDwnPtg, color7,
    bcolor6, fDwnPtg, color7,
/*   %sD%s[%s25.38%sM %s1.6%sk/s]   */
    bcolor2, color7, bcolor2, fDwn, color7, bcolor2, fRateDwn, color7,
/*   %sU%s[%s1.0%sM %s0.6%sk/s]   */
    bcolor1, color7, bcolor1, fUp, color7, bcolor1, fRateUp, color7,
/*   %sE%s%[%s%d%s/%s%d%s]   */
    bcolor3, color7, bcolor3, TrkRef, color7, bcolor3, TrkOk, color7,

    partial, timeleft,

    (Tracker.GetStatus()==T_CONNECTING) ? "Connecting" :
      ( (Tracker.GetStatus()==T_READY) ? "Connected" :
          (Tracker.IsRestarting() ? "Restarting" :
            (Tracker.IsQuitting() ? "Quitting" :
              (WORLD.IsPaused() ? "Paused" : checked))) )
  );

  if(ucLiveIdx == 3) ucLiveIdx = 0;

  if(f_clLog) {
// * 0/1/130 [564/1155/1179] 48.6% D[25.38M 1.6k/s] U[1.0M 0.6k/s] E[0/1]
    char *pStLine = new char[1024];

    sprintf(pStLine, "%d/%d/%d [%d/%d/%d] %.1f%% D[%.2fM %.1fk/s] U[%.2fM %.1fk/s] E[%d/%d] %s",
                      PeerSeed, PeerLeech, PeerTot,
                      ChunkHave, ChunkAvl, ChunkTot,
                      fDwnPtg,
                      fDwn, fRateDwn,
                      fUp, fRateUp,
                      TrkRef, TrkOk,
                      partial);
    WrLog(pStLine);
    delete []pStLine;
  }

  IndUpd();
}

void StLineOut(FILE *stream, char *buf) /*fold00*/
{

  USHORT  Col, Row;

  fprintf(stream, "\r%s", buf);
  fflush(stream);
  if(!VioGetCurPos(&Row, &Col, 0) && VioCol) {
    for(int i = Col; i < VioCol - 1; i++)
      fprintf(stream, " ");
    fflush(stream);
  }

}

void IndUpd(void) /*fold00*/
{

  if(f_IconOk) {
    if(fDwnPtg == 100.0) f_IndM = 1;
    else f_IndM = 0;

    if(fRateDwn >= 0.1) { f_IndD = 2; f_IndTD = 1; }
    else {
      if(PeerSeed) f_IndD = 1;
      else f_IndD = 0;
    }

    if(fRateUp >= 0.1) { f_IndU = 2; f_IndTU = 1; }
    else {
      if(PeerLeech) f_IndU = 1;
      else f_IndU = 0;
    }

    Icon_new = IndIcons[f_IndM][f_IndD][f_IndU][f_IndTD][f_IndTU];

    if(Icon_new != Icon_now) {
      Icon_now = Icon_new;
      SetIcon(Icon_now);
    }
  }
  if(PM_init) SetWinList();

}


void TitleVIO() /*fold00*/
{

  if(PM_init) {  // успешный морфинг.
    char  pTitleVIO[1024 + 20];
    if(!f_clTitle) {  // заголовок не задан
      strcpy(pTitleVIO, pTitleFileName);
      sprintf(pTitleVIO + strlen(pTitleVIO), " [%.3fM]", fFileLen);
    }
    else
      strcpy(pTitleVIO, pTitle);
    WinSetWindowText(SwData.hwnd, pTitleVIO);
  }
}

void ChkFileNums(BTFILE *p) /*fold00*/
{

// мутный алгоритм вычисления требуемых файлов

  UINT  uFileNum;
  LONGLONG  llTotLenStr = 0, llTotLenEnd = 0;
  LONGLONG  llLeft;
  UINT  uIdx;


  if(!pFileLen) {
    pFileLen = new LONGLONG[uFilesMax + 1];
    uFileNum = 1;
    for(; p; p = p->bf_next) pFileLen[uFileNum++] = p->bf_length;  // массив длин файлов.
  }

  if(!pFileCreat) pFileCreat = new UCHAR[uFilesMax + 1];
  for(uIdx = 0; uIdx <= uFilesMax; uIdx++) pFileCreat[uIdx] = 0;

  uFileNum = 0;
  while(++uFileNum <= uFilesMax) {

    if(pFileNums[uFileNum] != 1) {  // текущий файл не заявлен.
      llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
      continue;
    }

    // файл заявлен.
    pFileCreat[uFileNum] = 1;

    // проверка на чанк назад.
    llLeft = (LONGLONG)llTotLenStr % ulChunkLen;  // остаток чанка в предыдущем файле.

    if(llLeft) {  // чанк не по границе начала файла.
      uIdx = uFileNum;

      while(llLeft > 0 && uIdx > 1) {  // создавать предыдущие файлы до заполнения чанка.
	if(pFileCreat[--uIdx] == 1) break;  // предыдущий файл уже создан.
	pFileCreat[uIdx] = 1;
	llLeft -= pFileLen[uIdx];
      }
    }

    // проверка на чанк вперед.
    llTotLenEnd = llTotLenStr + pFileLen[uFileNum];  // длина на конец файла.
    llLeft = llTotLenEnd % ulChunkLen;  // остаток чанка.

    if(llLeft) {  // чанк не по границе конца файла.
      if(uFileNum == uFilesMax) break;  // последний файл.
      if(pFileNums[uFileNum + 1] == 1) {  // следующий файл заказан.
	llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
	continue;
      }
      if(llTotLenEnd > ulChunkLen) {  // llLeft - остаток чанка в текущем файле.
        llLeft = ulChunkLen - llLeft;  // не хватает части чанка.
      }
      else llLeft = ulChunkLen - llTotLenEnd;  // не хватает части чанка.
      llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
      while(llLeft > 0 && uFileNum < uFilesMax) {  // создавать следующие файлы до заполнения чанка.
	pFileCreat[++uFileNum] = 1;
	llLeft -= pFileLen[uFileNum];
	llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
      }
    }
    else llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
  }


/*
printf("\n FileCreat ");
for(int i = 0; i <= uFilesMax; i++) {
  if(pFileCreat[i]) printf("[%d]", i);
}
printf("\n");
//exit(0);
*/

}

void ChkChunks(UCHAR *pFileList) /*fold00*/
{

// мутный алгоритм вычисления требуемых чанков

  UINT  uFileNum;
  LONGLONG  llTotLenStr = 0, llFileLen;
  LONGLONG  llLeft, llLeftPrev = 0;
  ULONG  ulIdxPrev = 0, ulChunkIdx, ulChunkIdxStr, ulChunkIdxEnd;

/*
printf("\n FileExist ");
for(int i = 0; i <= uFilesMax; i++) {
  if(pFileExist[i]) printf("(%d)", i);
}
printf("\n");
*/

  if(!pChunkChk) {
    pChunkChk = new UCHAR[ulChunksMax + 1];
    for(ulChunkIdx = 0; ulChunkIdx <= ulChunksMax; ulChunkIdx++) pChunkChk[ulChunkIdx] = 0;
  }

  uFileNum = 0;
  while(++uFileNum <= uFilesMax) {
    if(pFileList[uFileNum] != 1) {  // текущий файл не заявлен.
      llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
      continue;
    }

    // файл заявлен.
    llFileLen = pFileLen[uFileNum];
    ulChunkIdxStr = (ULONG)(llTotLenStr / ulChunkLen + 1);  // индекс первого чанка в файле.
    llLeft = llTotLenStr % ulChunkLen;  // остаток чанка в предыдущем файле.

    if(llLeft) {                        // чанк не по границе начала файла.
      if(ulIdxPrev == ulChunkIdxStr) {
	llLeftPrev += (llFileLen >= (ulChunkLen - llLeft)) ? (ulChunkLen - llLeft) : llFileLen;
	if(llLeftPrev == ulChunkLen) {
	  pChunkChk[ulChunkIdxStr] = 1;
	  llLeftPrev = 0;
	  ulIdxPrev = 0;
	}
      }
      else llLeftPrev = 0;
      llFileLen -= ulChunkLen - llLeft;
      if(llFileLen > 0) ++ulChunkIdxStr;
    }

    ulChunkIdxEnd = ulChunkIdxStr;
    if(llFileLen >= ulChunkLen) {  // в файле (остатке) есть полные чанки.
      ulChunkIdxEnd += (ULONG)(llFileLen / ulChunkLen - 1);  // полных чанков.
      for(ulChunkIdx = ulChunkIdxStr; ulChunkIdx <= ulChunkIdxEnd; ulChunkIdx++) {
	pChunkChk[ulChunkIdx] = 1;
	llFileLen -= ulChunkLen;
      }
      ++ulChunkIdxEnd;
    }
    if(llFileLen > 0) {  // есть остаток чанка в конце файла.
      ulIdxPrev = ulChunkIdxEnd;
      llLeftPrev = llFileLen;
    }
    if(uFileNum == uFilesMax) pChunkChk[ulChunksMax] = 1;  // всегда дополнять последний файл.
    llTotLenStr += pFileLen[uFileNum];  // длина до начала следующего файла.
  }

  ulChunkChk = 0;
  for(ulChunkIdx = 1; ulChunkIdx <= ulChunksMax; ulChunkIdx++)
    if(pChunkChk[ulChunkIdx]) ++ulChunkChk;

/*
printf("\n Chunks ");
for(ulChunkIdx = 0; ulChunkIdx <= ulChunksMax; ulChunkIdx++)
  if(pChunkChk[ulChunkIdx]) { ++ulChunkChk; printf(" %d", ulChunkIdx); }
printf("\n");
exit(0);
*/

}

time_t ttime(time_t *pTime) /*fold00*/
{

// вот такая замена time(), которое отстает от gettimeofday() на 1 час в зимний период.

  ULONG  ulTemp;
  struct  timeval tv;

  gettimeofday(&tv, (struct timezone *)0);
  ulTemp = (ULONG)tv.tv_sec;
  if(pTime != NULL) *pTime = ulTemp;
  return ulTemp;

}

void dtime(void) /*fold00*/
{
// тоже некоторая замена, но с double

  struct  timeval tv;

  gettimeofday(&tv, (struct timezone *)0);
  dnow = tv.tv_sec + (double)(tv.tv_usec / 1000000.0);
  now = (ULONG)dnow;

}

//an64
int CSC1(char *pBufOut, char *pBufIn, size_t BufOutLen, size_t BufInLen, char cFrom , char* fenc) /*FOLD00*/
{

  iconv_t  cd;
  size_t  rc = 0;
  char  *pOut, *pCPF, *pCPT;
  const char  *pIn;

  pIn = pBufIn;
  pOut = pBufOut;

  if(cFrom == 'u') {pCPF = fenc; pCPT = "";}
  else {pCPF = ""; pCPT = fenc;}

  if((cd = iconv_open(pCPT, pCPF)) == (iconv_t)-1) return 1;
  rc = iconv(cd, &pIn, &BufInLen, &pOut, &BufOutLen);
  iconv_close(cd);
  *pOut = '\0';
  if(rc == (size_t)-1) return 1;
  return 0;
}

int CSC(char *pBufOut, char *pBufIn, size_t BufOutLen, size_t BufInLen, char cFrom) /*FOLD00*/
{

  iconv_t  cd;
  size_t  rc = 0;
  char  *pOut, *pCPF, *pCPT;
  const char  *pIn;

  pIn = pBufIn;
  pOut = pBufOut;

  if(cFrom == 'u') {pCPF = "utf-8"; pCPT = "";}
  else {pCPF = ""; pCPT = "utf-8";}

  if((cd = iconv_open(pCPT, pCPF)) == (iconv_t)-1) return 1;
  rc = iconv(cd, &pIn, &BufInLen, &pOut, &BufOutLen);
  iconv_close(cd);
  *pOut = '\0';
  if(rc == (size_t)-1) return 1;
  return 0;


/*
  if((cd = iconv_open(pCPT, pCPF)) == (iconv_t)-1) {
    printf("\ncsc iconv_open()");
    return 1;
  }
  rc = iconv(cd, &pIn, &BufInLen, &pOut, &BufOutLen);
  iconv_close(cd);
  *pOut = '\0';
  if(rc == (size_t)-1) {
    printf("\ncsc iconv()");
    switch(errno) {
    case EILSEQ:
      printf(" EILSEQ");
      break;
    case E2BIG:
      printf(" E2BIG");
      break;
    case EINVAL:
      printf(" EINVAL");
      break;
    case EBADF:
      printf(" EBADF");
      break;
    default:
      printf(" ???");
      break;
    }
    return 1;
  }
  return 0;
*/

}

void FAR Kbd(void*) /*fold00*/
{

// пришлось вынести обработку клавиатуры в отдельный тред из select() по понятным причинам.

  int  irc;


  while(1) {
    while(!kbhit()) DosSleep(50);
    DosRequestMutexSem(hmtxStLine, SEM_INDEFINITE_WAIT);
    irc = CONSOLE.UserMenu();
    DosReleaseMutexSem(hmtxStLine);
    if(irc) break;
  }
  return;
}

void ChkConsole(void) /*fold00*/
{

// действия по консольным командам вынесены во избежание коллизий.

  APIRET  rc;

  if(f_console && !f_Exit) {
    rc = DosRequestMutexSem(hmtxConsole, SEM_IMMEDIATE_RETURN);
    if(rc == NO_ERROR) {
      f_console = 0;
      if(f_max_bandwidth_down) {
	cfg_max_bandwidth_down = max_bandwidth_down;
	f_max_bandwidth_down = 0;
      }
      if(f_max_bandwidth_up) {
	cfg_max_bandwidth_up = max_bandwidth_up;
	f_max_bandwidth_up = 0;
      }
      if(f_seed_hours) {
	cfg_seed_hours = seed_hours;
	f_seed_hours = 0;
      }
      if(f_seed_ratio) {
	cfg_seed_ratio = seed_ratio;
	f_seed_ratio = 0;
      }
      if(f_min_peers) {
	cfg_min_peers = min_peers;
	f_min_peers = 0;
      }
      if(f_max_peers) {
	cfg_max_peers = max_peers;
	f_max_peers = 0;
      }
      if(f_cache_size) {
	cfg_cache_size = cache_size;
        BTCONTENT.CacheConfigure();
	f_cache_size = 0;
      }
      if(f_files) {
	if(f_clFile && f_files == 2) {
          ChkFileNums(pFiles);
          if(a_btfiles.CreateFiles() == -1) exit(-1);
	  ChkChunks(pFileExist);
	}
	BTCONTENT.SetFilter();
	f_files = 0;
      }
      if(f_CTCS) {
	if(f_CTCS == 1) CTCS.Reset(1);
	else { CTCS.Initial(); CTCS.Reset(1); }
	f_CTCS = 0;
      }
      if(f_CTCSRq) {
	if(f_CTCSRq == 1) CTCS.Send_bw();
	else CTCS.Send_Config();
	f_CTCSRq = 0;
      }
      if(f_exit) {
	f_Exit = 1;
        Tracker.ClearRestart();
	Tracker.SetStoped();
	f_exit = 0;
      }
      DosReleaseMutexSem(hmtxConsole);
    }
  }
}

void StartDetach(int, char **argv) /*fold00*/
{

// переход в детач по опции -d в комманд-лайн.
// то есть запускаем детач-процесс и вываливаемся.

  APIRET  rc;
  char  LoadError[CCHMAXPATH + 20] = {'\0'};
  RESULTCODES  ChildRC = {'\0'};
  int  Temp;


  char  *Args;
  Temp = _bgetcmd(NULL, 0) + 1;
  Args = new char[strlen(argv[0]) + 1 + Temp + strlen(" -detach") + 2];
  strcpy(Args, argv[0]);
  _bgetcmd(Args + strlen(Args) + 1, Temp);
  sprintf(Args + strlen(Args) + Temp, " -detach%c", '\0');

  rc = DosExecPgm(LoadError,           // Object name buffer
                  sizeof(LoadError),   // Length of object name buffer
                  EXEC_BACKGROUND,
                  Args,                // Argument string
                  NULL,                // Environment string
                  &ChildRC,            // Termination codes
                  argv[0]);            // Program file name

  if(rc) {
    printf("! can't start detached process.\n");
    fflush(stdout);
    exit(1);
  }
  exit(0);

}

int ParsCfg(char **argv, char* pCfgFile) /*fold00*/
{

  int  irc = 0;
  FILE  *fpCfg;
  struct stat sb;
  char  pFileName[1024] = {'\0'};
  UCHAR  f_File = 0;


  if(pCfgFile) {
    f_File = 1;
    strcpy(pFileName, pCfgFile);
  }
  if(!f_File) {  // ищем act.cfg в текущем каталоге.
    strcpy(pFileName, "act.cfg");
    if(0 == access(pFileName, F_OK)) f_File = 1;
  }
  if(!f_File) {  // ищем act.cfg в каталоге старта.
    strcpy(pFileName, argv[0]);
    char  *ptr = pFileName + strlen(pFileName) - 1;  // последний символ в имени.
    while(*ptr != '\\') --ptr;  // удаляем имя файла.
    strcpy(ptr + 1, "act.cfg");
    if(0 == access(pFileName, F_OK)) f_File = 1;
  }
  if(!f_File) {  // ищем act.cfg в %home%.
    char  *env;
    env = getenv( "HOME");
    if(env) {
      strcpy(pFileName, env);
      char  *ptr = pFileName + strlen(pFileName) - 1;  // последний символ в пути.
      if(*ptr != '\\') *(++ptr) = '\\';
      ++ptr;  // \0
      strcpy(ptr, "act.cfg");
      if(0 == access(pFileName, F_OK)) f_File = 1;
    }
  }
  if(!f_File) {  // ищем act.cfg в %home%.
    char  *env;
    env = getenv( "ETC");
    if(env) {
      strcpy(pFileName, env);
      char  *ptr = pFileName + strlen(pFileName) - 1;  // последний символ в пути.
      if(*ptr != '\\') *(++ptr) = '\\';
      ++ptr;  // \0
      strcpy(ptr, "act.cfg");
      if(0 == access(pFileName, F_OK)) f_File = 1;
    }
  }

  if(!f_File) return 0;

  if(stat(pFileName, &sb) < 0) {  // файл не существует.
    printf("! not found '%s'.\n", pFileName);
    return -1;
  }

  if(!(S_IFREG & sb.st_mode)) {
    printf("! not a regular file '%s'.\n", pFileName);
    return -1;
  }

  if(sb.st_size >= 1024 * 1024) {
    printf("! too large file size '%s'.\n", pFileName);
    return -1;
  }

  if(sb.st_size == 0) return 0;

  fpCfg = fopen(pFileName, "rb");
  if(!fpCfg) {
    printf("! access error '%s': %s\n", pFileName, strerror(errno));
    return -1;
  }

  pCfg = new char[sb.st_size + 2];
  if(fread(pCfg, sb.st_size, 1, fpCfg) != 1) {
    printf("! read error '%s': %s\n", pFileName, strerror(errno));
    return -1;
  }

  fclose(fpCfg);

  pCfg[sb.st_size] = 0x0A;
  pCfg[sb.st_size + 1] = '\0';

  char  *ptr, *ptr1, *pStringEnd;
  int  ParamCnt = 0, ErrCnt = 0;

  typedef struct CfgParam {
    char  *pName;
    struct CfgParam  *next;
  } CFGPARAM;

//  int  ParamSize = sizeof(CFGPARAM);

  CFGPARAM  *ParamHead = NULL;
  CFGPARAM  *ParamPrev = NULL;
  CFGPARAM  *ParamNext = NULL;

//  ParamHead = new CFGPARAM[ParamSize];
  ParamHead = new CFGPARAM;
  ParamHead->next = NULL;
  ParamPrev = ParamHead;

  while(1) {
    pStringEnd = strpbrk(pCfg, "\x0D\x0A");  // конец очередной строки.
    if(!pStringEnd) {
      printf("! incorrect configuration file structure, aborted.\n");
      exit(-1);
    }
    *pStringEnd = '\0';
    while(1) {  // проход по строке.
      while(*pCfg == ' ' || *pCfg == '\t') ++pCfg;  // удаляем лидирующие пробелы и табы.
      if(!*pCfg || *pCfg == ';') break;

      // в строке что-то осталось

      if(*pCfg == '"') {  // параметр в кавычках.
        ptr = strchr(++pCfg, '"');  // ищем закрывающую кавычку.
        if(!ptr) {
          printf("! incorrect parameter '%s'.\n", pCfg - 1); fflush(stdout);
          if(++ErrCnt == 10) {
            printf("! too many errors, aborted.\n");
            exit(-1);
          }
          break;
        }
        *ptr = '\0';  // отсекаем '"' в конце.
        if(ptr == pCfg) {  // ""
          pCfg = ptr + 1;
          continue;
        }
        ptr1 = ptr - 1;
	while(ptr1 >= pCfg && (*ptr1 == ' ' || *ptr1 == '\t'))
	  *(ptr1--) = '\0'; // удаляем финальные пробелы и табы.
        if(ptr1 < pCfg) {  // "    "
          pCfg = ptr + 1;
          continue;
        }
      }
      else {
        ptr = strpbrk(pCfg, "\t ");  // ищем конец параметра.
        if(ptr) *ptr = '\0';  // конец параметра.
        ptr1 = strchr(pCfg, ';');
        if(ptr1) {
          *ptr1 = '\0';  // отсекаем комментарий.
          ptr = NULL;
        }
      }

      ++ParamCnt;
      ParamPrev->pName = pCfg;
//      ParamNext = new CFGPARAM[ParamSize];
      ParamNext = new CFGPARAM;
      ParamNext->next = NULL;
      ParamPrev->next = ParamNext;
      ParamPrev = ParamNext;
      if(!ptr) break;
      pCfg = ptr + 1;
    }  // проход по строке.

    pCfg = strspnp(pStringEnd + 1, "\x0D\x0A");  // начало следующей строки.
    if(!pCfg) break;
  }

  if(!ParamCnt) {
    delete []pCfg;
//    delete []ParamHead;
    delete ParamHead;
    return 0;
  }

  char **pCfgV = new char*[ParamCnt + 1];
  char **pptr;

  pptr = pCfgV;
  *pptr = argv[0];
  ParamPrev = ParamHead;

  for(int i = 1; i <= ParamCnt; i++) {
    *(++pptr) = ParamPrev->pName;
    ParamNext = ParamPrev->next;
//    delete []ParamPrev;
    delete ParamPrev;
    ParamPrev = ParamNext;
  }

  if(!ErrCnt) irc = GetParam(++ParamCnt, pCfgV, 0);
  delete []pCfg;
  delete []pCfgV;

  if(irc) return irc;
  if(ErrCnt) return -1;
  return 0;

}


void os2err(ULONG errnum)
{

  int  rc_max = 487;
  int  rc_tbl[] = {
    1,	        //ERROR_INVALID_FUNCTION 		Incorrect function.
    2,	        //ERROR_FILE_NOT_FOUND			The system cannot find the file specified.
    3,	        //ERROR_PATH_NOT_FOUND 			The system cannot find the path specified.
    5,	        //ERROR_ACCESS_DENIED 			Access is denied.
    6,	        //ERROR_INVALID_HANDLE			Incorrect internal file identifier.
    8,	        //ERROR_NOT_ENOUGH_MEMORY		There is not enough memory available to process this command. All available memory is in use.
    10,	        //ERROR_BAD_ENVIRONMENT			The environment is incorrect.
    11,	        //ERROR_BAD_FORMAT 			An attempt was made to load a program with an incorrect format.
    12,	        //ERROR_INVALID_ACCESS			The access code is invalid.
    13,	        //ERROR_INVALID_DATA			The data is invalid.
    15,	        //ERROR_INVALID_DRIVE 			The system cannot find the specified drive.
    16,	        //ERROR_CURRENT_DIRECTORY		The directory cannot be removed.
    17,	        //ERROR_NOT_SAME_DEVICE			The system cannot move the file to a different disk drive.
    18,	        //ERROR_NO_MORE_FILES			There are no more files.
    19,	        //ERROR_WRITE_PROTECT			The drive is currently write-protected.
    21,	        //ERROR_NOT_READY			The drive is not ready.
    24,	        //ERROR_BAD_LENGTH			The program issued a command but the command length is incorrect.
    26,	        //ERROR_NOT_DOS_DISK			The specified disk or diskette cannot be accessed.
    29,	        //ERROR_WRITE_FAULT			The system cannot write to the specified device.
    31,	        //ERROR_GEN_FAILURE			A device attached to the system is not functioning.
    32,	        //ERROR_SHARING_VIOLATION		The process cannot access the file because it is being used by another process.
    33,	        //ERROR_LOCK_VIOLATION			The process cannot access the file because another process has locked a portion of the file.
    36,	        //ERROR_SHARING_BUFFER_EXCEEDED		The system has detected an overflow in the sharing buffer.
    82,	        //ERROR_CANNOT_MAKE			The directory or file cannot be created.
    87,	        //ERROR_INVALID_PARAMETER		The parameter is incorrect.
    89,	        //ERROR_NO_PROC_SLOTS			The system cannot start another process at this time.
    99,	        //ERROR_DEVICE_IN_USE			The *** device is already in use by another application.
    100,	//ERROR_TOO_MANY_SEMAPHORES		Cannot create another system semaphore.
    103,	//ERROR_TOO_MANY_SEM_REQUESTS		The semaphore cannot be set again.
    105,	//ERROR_SEM_OWNER_DIED			The previous ownership of this semaphore has ended.
    108,	//ERROR_DRIVE_LOCKED			The disk is in use or locked by another process.
    109,	//ERROR_BROKEN_PIPE 			The pipe has been ended.
    110,	//ERROR_OPEN_FAILED			The system cannot open the device or file specified.
    112,	//ERROR_DISK_FULL			There is not enough space on the disk.
    114,	//ERROR_INVALID_TARGET_HANDLE		The target internal file identifier is incorrect.
    117,	//ERROR_INVALID_CATEGORY		The IOCTL call made by the application is incorrect.
    118,	//ERROR_INVALID_VERIFY_SWITCH		The verify-on-write parameter value is incorrect.
    121,	//ERROR_SEM_TIMEOUT			The semaphore timeout period has expired.
    122,	//ERROR_INSUFFICIENT_BUFFER		The data area passed to a system call is too small.
    123,	//ERROR_INVALID_NAME			A file name or volume label contains an incorrect character.
    124,	//ERROR_INVALID_LEVEL			The system call level is incorrect.
    125,	//ERROR_NO_VOLUME_LABEL			The disk has no volume label.
    127,	//ERROR_PROC_NOT_FOUND			The specified procedure could not be found.
    154,	//ERROR_LABEL_TOO_LONG			The volume label you entered exceeds the 11-character limit. ++
    156,	//ERROR_SIGNAL_REFUSED			The recipient process has refused the signal.
    157,	//ERROR_DISCARDED			The segment is already discarded and cannot be locked.
    164,	//ERROR_MAX_THRDS_REACHED		No more threads can be created in the system.
    170,	//ERROR_BUSY				The segment is in use by another process.
    180,	//ERROR_INVALID_SEGMENT_NUMBER		The system detected a segment number that was incorrect.
    187,	//ERROR_SEM_NOT_FOUND			The specified system semaphore name was not found.
    199,	//ERROR_AUTODATASEG_EXCEEDS_64K		The operating system cannot run this application program.
    201,	//ERROR_RELOCSRC_CHAIN_EXCEEDS_SEGLIMIT	The operating system cannot run ***.
    203,	//ERROR_ENVVAR_NOT_FOUND		The system could not find the environment option that was entered.
    205,	//ERROR_NO_SIGNAL_SENT			No process in the command subtree has a signal handler.
    206,	//ERROR_FILENAME_EXCED_RANGE		The file name or extension is too long.
    208,	//ERROR_META_EXPANSION_TOO_LONG		The global file name characters * or ? are entered incorrectly or too many global file name characters are specified.
    209,	//ERROR_INVALID_SIGNAL_NUMBER		The signal being posted is incorrect.
    212,	//ERROR_LOCKED				The segment is locked and cannot be reallocated.
    224,	//ERROR_SMG_NO_TARGET_WINDOW		The specified Presentation Manager (PM) session ID could not be selected.
    230,	//ERROR_BAD_PIPE			The system detected an invalid pipe operation.
    231,	//ERROR_PIPE_BUSY			The pipe is in use by another process.
    232,	//ERROR_NO_DATA				There is no data to be read.
    233,	//ERROR_PIPE_NOT_CONNECTED		The pipe is disconnected.
    234,	//ERROR_MORE_DATA			More data is available.
    250,	//ERROR_CIRCULARITY_REQUESTED		The move or rename operation is not allowed.
    251,	//ERROR_DIRECTORY_IN_CDS		The move or rename operation is not allowed.
    252,	//ERROR_INVALID_FSD_NAME		The specified file system name is incorrect.
    253,	//RROR_INVALID_PATH			The specified device name is incorrect.
    254,	//ERROR_INVALID_EA_NAME			An incorrect extended attribute name was used.
    255,	//ERROR_EA_LIST_INCONSISTENT		The extended-attribute list size is incorrect.
    259,	//ERROR_NO_MORE_ITEMS			There are no more attached devices or drives.
    267,	//ERROR_DIRECTORY			The directory cannot be copied.
    282,	//ERROR_EAS_NOT_SUPPORTED		The target file system cannot save extended attributes.
    285,	//ERROR_DUPLICATE_NAME			The semaphore name you specified has already been used.
    286,	//ERROR_EMPTY_MUXWAIT			You have an empty muxwait semaphore.
    287,	//ERROR_MUTEX_OWNED			You cannot wait on a muxwait semaphore that contains a mutex semaphore owned by the current thread.
    288,	//ERROR_NOT_OWNER			You cannot release a mutex semaphore that is not owned by the current thread.
    289,	//ERROR_PARAM_TOO_SMALL			The DosQueryMuxWaitSem parameter buffer is too small.
    290,	//ERROR_TOO_MANY_HANDLES		Too many semaphore handles have been requested.
    291,	//ERROR_TOO_MANY_OPENS			The semaphore has been opened too many times.
    292,	//ERROR_WRONG_TYPE			The wrong type of semaphore was specified.
    298,	//ERROR_TOO_MANY_POSTS			The maximum posts for the event semaphore has been reached.
    299,	//ERROR_ALREADY_POSTED			The event semaphore has already been posted.
    300,	//ERROR_ALREADY_RESET			The event semaphore has already been reset.
    301,	//ERROR_SEM_BUSY			The semaphore is in use.
    303,	//ERROR_INVALID_PROCID			The program specified a process ID that does not exist.
    316,	//ERROR_MR_MSG_TOO_LONG			The message passed to the message retriever was truncated.
    317,	//ERROR_MR_MID_NOT_FOUND		The system cannot find message *** in message file ***.
    318,	//ERROR_MR_UN_ACC_MSGF			Message file *** cannot be found for message ***.
    319,	//ERROR_MR_INV_MSGF_FORMAT		The system cannot read message file ***.
    320,	//ERROR_MR_INV_IVCOUNT			A programming error occurred when using the message retriever.
    321,	//ERROR_MR_UN_PERFORM			The system cannot display the message.
    330,	//ERROR_QUE_PROC_NOT_OWNED		The specified queue is not owned by the requesting process.
    332,	//ERROR_QUE_DUPLICATE			The specified queue name is already in use.
    333,	//ERROR_QUE_ELEMENT_NOT_EXIST		The specified queue element does not exist.
    334,	//ERROR_QUE_NO_MEMORY			Not enough memory is available to process a queue request.
    369,	//ERROR_SMG_INVALID_SESSION_ID		The specified session ID is incorrect.
    397,	//ERROR_NLS_OPEN_FAILED			OS/2 cannot open the COUNTRY.SYS file.
    398,	//ERROR_NLS_NO_CTRY_CODE		The system cannot find the country code or code page.
    399,	//ERROR_NLS_TABLE_TRUNCATED		The specified buffer length is too small, causing the returned NLS table to truncate.
    418,	//ERROR_SMG_INVALID_CALL		The call to DosSMRegisterDD is not allowed.
    455,	//ERROR_SMG_INVALID_BOND_OPTION		The specified parameter for the session bond option is incorrect.
    456,	//ERROR_SMG_INVALID_SELECT_OPT		The specified parameter for the session select option is incorrect.
    457,	//ERROR_SMG_START_IN_BACKGROUND		The Session Manager cannot start the process in the foreground.  It will be started in the background.
    458,	//ERROR_SMG_INVALID_STOP_OPTION		The specified parameter for the session end option is incorrect.
    459,	//ERROR_SMG_BAD_RESERVE			The reserved parameter specified is incorrect.
    460,	//ERROR_SMG_PROCESS_NOT_PARENT		The request is not valid from the process that issued it.
    461,	//ERROR_SMG_INVALID_DATA_LENGTH		The data buffer length is incorrect.
    463,	//ERROR_SMG_RETRY_SUB_ALLOC		Not enough memory is available to process this Session Manager function.
    487,	//ERROR_INVALID_ADDRESS			The specified address is incorrect.
    0
  };


  ULONG  bc;
  char   *pMsg, *ptr1, *ptr2;
  APIRET  rc;
  UCHAR  f_rc_err = 0;

  if(errnum > rc_max) f_rc_err = 1;
  else {
    int  i = 0, err;
    while(1) {
      err = rc_tbl[i++];
      if(err == errnum) break;
      if(!err || err > errnum) { f_rc_err = 1; break; }
    }
  }

  if(!f_rc_err) {
    pMsg = new char[MAXMSGLEN];
    rc = DosGetMessage(NULL, 0, pMsg, MAXMSGLEN - 1, errnum, "OSO001.MSG", &bc);
  }

  if(f_rc_err || rc) {
    sprintf(pErrStr, "rc=%lu", errnum);
    delete []pMsg;
    return;
  }

  pMsg[bc] = '\0';
  ptr1 = pMsg + bc - 1;
  while(*ptr1 == 0x0A || *ptr1 == 0x0D) *(ptr1--) = '\0';

  ptr1 = ptr2 = pMsg;
  *pErrStr = '\0';
  while(1) {
    while(*ptr2 && *ptr2 != 0x0A && *ptr2 != 0x0D && *ptr2 != '%') ++ptr2;
    if(!*ptr2) { strcat(pErrStr, ptr1); break; }
    if(*ptr2 == 0x0D) *ptr2 = ' ';
    else {
      *ptr2 = '\0';
      strcat(pErrStr, ptr1);
      ptr1 = ptr2 + 1;
    }
  }

  delete []pMsg;
}



//eof /*FOLD00*/


