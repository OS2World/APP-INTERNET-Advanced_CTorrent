#include "./def.h"
#include <sys/types.h>

#ifdef WINDOWS
#include <windows.h>
#else
#include <unistd.h>
#include <signal.h>
#endif

#include <sys/time.h>
#include <time.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

//acw
#ifdef __OS2__
#define INCL_DOS
#include <os2.h>
#include <process.h>
#endif

//#include "ctorrent.h"
#include "btconfig.h"
#include "btcontent.h"
#include "downloader.h"
#include "peerlist.h"
#include "tracker.h"
#include "ctcs.h"
#include "console.h"

//acw
#include "acw.h"
#include "Parser.h"

#include "./config.h"

#ifndef HAVE_RANDOM
#include "compat.h"
#endif

#ifndef WINDOWS
#include "sigint.h"
#endif


void usage();

#ifdef WINDOWS

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrzevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
}

#else

void Random_init()
{
  unsigned long seed;
#ifdef HAVE_GETTIMEOFDAY
  struct timeval tv;
  gettimeofday(&tv,(struct timezone*) 0);
  seed = tv.tv_usec + tv.tv_sec + getpid();
#else
  seed = (unsigned long)time((time_t *)0);
#endif
  srandom(seed);
}

int main(int argc, char **argv)
{

  char *s;
//acw
  int  irc;

//acw
#ifdef __OS2__
  LONG fh = 0;
  ULONG curfh = 0;
  DosSetRelMaxFH( &fh, &curfh );
  DosSetMaxFH( curfh + 500 );
#endif

  Random_init();
  arg_user_agent = new char[MAX_PF_LEN+1];
  strcpy(arg_user_agent,PEER_PFX);

  cfg_user_agent = new char[strlen(PACKAGE_NAME)+strlen(PACKAGE_VERSION)+2];
  sprintf(cfg_user_agent, "%s/%s", PACKAGE_NAME, PACKAGE_VERSION);
  while(s = strchr(cfg_user_agent, ' ')) *s = '-';

//acw
  GetParam(argc, argv, 'c');  // искать -cfg.
  irc = ParsCfg(argv, pCfgName);  // проход по конфигу.
  if(irc == -1 ) exit(-1);
  irc = GetParam(argc, argv, 0);  // парсить командную строку.
  if(irc == -1 ) exit(-1);
  if(irc == -2 ) { usage(); exit(-1); }

  if( cfg_min_peers >= cfg_max_peers ) cfg_min_peers = cfg_max_peers - 1;
  if(!arg_metainfo_file) {  // файл не задан.
    if(arg_flg_make_torrent)
      CONSOLE.Warning(1, "Must specify torrent contents (one file or directory)");
    else CONSOLE.Warning(1, "Must specify one torrent file. Use -h or -? for help/usage.");
    exit(-1);
  }

  if(!irc && !arg_bitfield_file) {
    arg_bitfield_file = new char[strlen(arg_metainfo_file) + 4];
    strcpy(arg_bitfield_file, arg_metainfo_file);
    strcat(arg_bitfield_file, ".bf");
  }

  if(f_clDetach) arg_daemon = 0;
  if(arg_daemon) { f_Detach = 1; arg_daemon = 0; }

  if( arg_flg_make_torrent ){
    if( !arg_announce ){
      CONSOLE.Warning(1, " please use -u to specify a announce URL!");
      exit(1);
    }
    if( !arg_save_as ){
      CONSOLE.Warning(1, " please use -s to specify a metainfo file name!");
      exit(1);
    }
    if( BTCONTENT.InitialFromFS(arg_metainfo_file, arg_announce,
                                arg_piece_length) < 0 ||
        BTCONTENT.CreateMetainfoFile(arg_save_as) < 0 ){
      CONSOLE.Warning(1, " create metainfo failed.");
      exit(1);
    }
    CONSOLE.Print("create metainfo file %s successful.", arg_save_as);
    exit(0);
  }

//acw  if( arg_daemon ) CONSOLE.Daemonize();

  if(f_Detach && !arg_flg_exam_only && !arg_flg_check_only)
    StartDetach(argc, argv);

  if( !arg_flg_exam_only && (!arg_flg_check_only || arg_flg_force_seed_mode) )
    if( arg_ctcs ) CTCS.Initial();

//acw
  SetTitleIcon();
  SetDimPos();
  LoadIcons();
  if(f_clLog) OpenLog();

  if( BTCONTENT.InitialFromMI(arg_metainfo_file, arg_save_as) < 0){
//acw
//    CONSOLE.Warning(1, "error, initial meta info failed.");
    CONSOLE.Warning(1, " error, initial meta info failed in '%s'.", arg_metainfo_file);

//acw
   RstWinSet();

    exit(1);
  }

  if( !arg_flg_exam_only && (!arg_flg_check_only || arg_flg_force_seed_mode) ){
    if(WORLD.Initial_ListenPort() < 0){
      CONSOLE.Warning(2, " warn, you can't accept connections.");
    }
    if( Tracker.Initial() < 0 ){
      CONSOLE.Warning(1, " error, tracker setup failed.");
      exit(1);
    }


//acw
#ifndef __OS2__
    sig_setup();  // setup signal handling
#else
    signal(SIGINT,sig_catch);
    signal(SIGTERM,sig_catch);
#endif

    CONSOLE.Interact(
      "Press 'h' or '?' for help (display/control client options)." );
    Downloader();
    if( cfg_cache_size ) BTCONTENT.FlushCache();
  }
  if( !arg_flg_exam_only ) BTCONTENT.SaveBitfield();
  WORLD.CloseAll();

  if(arg_verbose) CONSOLE.cpu();

//acw
  RstWinSet();

  exit(0);
}

#endif

void usage()
{

  CONSOLE.ChangeChannel(O_INPUT, "off", 0);

  fprintf(stdout,"%s %s\n",PACKAGE_STRING, ACTVERSION);
  fprintf(stdout,"Original code Copyright: YuHong(992126018601033)\n");
  fprintf(stdout,"WARNING: THERE IS NO WARRANTY FOR CTorrent. USE AT YOUR OWN RISK!!!\n");
  fprintf(stdout,"\nGeneral Options:\n");
  fprintf(stdout, "%-15s %s\n", "-h/-H", "Show this message");
  fprintf(stdout, "%-15s %s\n", "-x",
    "Decode metainfo (torrent) file only, don't download");
  fprintf(stdout, "%-15s %s\n", "-c", "Check pieces only, don't download");
  fprintf(stdout, "%-15s %s\n", "-v", "Verbose output (for debugging)");

  fprintf(stdout,"\nDownload Options:\n");
  fprintf(stdout, "%-15s %s\n", "-e int",
    "Exit while seed <int> hours later (default 72 hours)");
  fprintf(stdout, "%-15s %s\n", "-E num",
    "Exit after seeding to <num> ratio (UL:DL)");
  fprintf(stdout, "%-15s %s\n", "-i ip",
    "Listen for connections on specific IP address (default all/any)");
  fprintf(stdout, "%-15s %s\n", "-p port",
    "Listen port (default 2706 -> 2106)");
  fprintf(stdout, "%-15s %s\n", "-I ip",
    "Specify public/external IP address for peer connections");
  fprintf(stdout, "%-15s %s\n", "-u num or URL",
    "Use an alternate announce (tracker) URL");
  fprintf(stdout, "%-15s %s\n", "-s filename",
    "Download (\"save as\") to a different file or directory");
  fprintf(stdout, "%-15s %s\n", "-C cache_size",
    "Cache size, unit MB (default 16MB)");
  fprintf(stdout, "%-15s %s\n", "-f",
    "Force saved bitfield or seed mode (skip initial hash check)");
  fprintf(stdout, "%-15s %s\n", "-b filename",
    "Specify bitfield save file (default is torrent+\".bf\")");
  fprintf(stdout, "%-15s %s\n", "-M max_peers",
    "Max peers count (default 100)");
  fprintf(stdout, "%-15s %s\n", "-m min_peers", "Min peers count (default 1)");
  fprintf(stdout, "%-15s %s\n", "-z slice_size",
    "Download slice/block size, unit KB (default 16, max 128)");
  fprintf(stdout, "%-15s %s\n", "-n file_list",
    "Specify file number(s) to download");
  fprintf(stdout, "%-15s %s\n", "-D rate", "Max bandwidth down (unit KB/s)");
  fprintf(stdout, "%-15s %s\n", "-U rate", "Max bandwidth up (unit KB/s)");
  fprintf(stdout, "%-15s %s%s\")\n", "-P peer_id",
    "Set Peer ID prefix (default \"", PEER_PFX);
  fprintf(stdout, "%-15s %s%s\")\n", "-A user_agent",
    "Set User-Agent header (default \"", cfg_user_agent);
  fprintf(stdout, "%-15s %s\n", "-S host:port",
    "Use CTCS server at host:port");
  fprintf(stdout, "%-15s %s\n", "-a", "Preallocate files on disk");
//  fprintf(stdout, "%-15s %s\n", "-T",
//    "Convert foreign filenames to printable text");
  fprintf(stdout, "%-15s %s\n", "-X command",
    "Run command upon download completion (\"user exit\")");
  fprintf(stdout, "%-15s %s\n", "-d", "Daemon mode (fork to background)");
//  fprintf(stdout, "%-15s %s\n", "-dd", "Daemon mode with I/O redirection");

  fprintf(stdout,"\nMake metainfo (torrent) file options:\n");
  fprintf(stdout, "%-15s %s\n", "-t", "Create a new torrent file");
  fprintf(stdout, "%-15s %s\n", "-u URL", "Tracker's URL");
  fprintf(stdout, "%-15s %s\n", "-l piece_len",
    "Piece length (default 262144)");
  fprintf(stdout, "%-15s %s\n", "-s filename", "Specify metainfo file name");
  fprintf(stdout, "%-15s %s\n", "-pvt", "Private (disable peer exchange)");
  fprintf(stdout, "%-15s %s\n", "-com comment", "Include a comment/description");

  fprintf(stdout,"\nExample:\n");
  fprintf(stdout,"ctorrent -s new_filename -e 12 -C 32 -p 6881 example.torrent\n");
  fprintf(stdout,"\nhome page: http://ctorrent.sourceforge.net/\n");
  fprintf(stdout,"see also: http://www.rahul.net/dholmes/ctorrent/\n");
  fprintf(stdout,"bug report: %s\n",PACKAGE_BUGREPORT);
  fprintf(stdout,"original author: bsdi@sina.com\n\n");

//acw

  fprintf(stdout, "--- end of original\n");
  fprintf(stdout, "\nAdvanced options (case insensitive):\n");
  fprintf(stdout, "-icon filename\tSet icon as... (default 'act.ico')\n");
  fprintf(stdout, "-title string\tSet title as... (default torrent filename)\n");
  fprintf(stdout, "-style number\tStatus line style 0 - 3 (default 2)\n");
  fprintf(stdout, "-win dimensions\tSet window dimensions as 'column,row'\n");
  fprintf(stdout, "-org position\tSet window corner position (left, bottom) as 'x,y'\n");
  fprintf(stdout, "-ind\t\tUse icon as process indicator.\n");
  fprintf(stdout, "-rate\t\tShow current Down/Up rate in Window List string.\n");
  fprintf(stdout, "-pdown\t\tShow current download %% in Window List string.\n");
  fprintf(stdout, "-time\t\tShow time left in Window List string.\n");
  fprintf(stdout, "-log ptg,time\tSaves status line in 'torrent_name.log' each 'ptg' percent of\n");
  fprintf(stdout, "\t\tdownload and 'time' minutes. Use zero value to disable logging.\n");
  fprintf(stdout, "-file file_list\tSame as -n, but will creat selected file(s) only.\n");
  fprintf(stdout, "-cfg filename\tConfiguration file to be used (default 'act.cfg').\n");
  fprintf(stdout, "-utf\t\tDon't convert from utf-8 to locale name of file (directory) when creating.\n");
  fprintf(stdout, "-fenc ibm-xxxx\tConvert from codepage <ibm-xxxx> to locale name of file (directory) when creating.\n");//an64
}


