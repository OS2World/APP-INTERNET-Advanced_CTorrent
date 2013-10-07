#include <os2.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>

#include "getopts.h"
#include "btconfig.h"
#include "console.h"
#include "acw.h"


int GetParam(int argc, char **argv, char cMode) /*FOLD00*/
//int GetParam(int argc, char **argv, UCHAR f_Cfg, UCHAR f_ChkCfg)
{

  int    Temp = 0;
  int  CmdErr = 0; // статус команд и аргументов.
  UCHAR  f_Help = 0;   // -h || -H option.
  int  rc;

  char *Opts[] = {  // команды

    "-a", "^",          //
    "-A", "^", ":",     // HTTP user-agent header string
    "-b", "^", ":",     //
    "-c", "^",          // Check exist only
    "-C", "^", ":",     // Max cache size
    "-d", "^",          // daemon mode (fork to background)
    "-D", "^", ":",     // download bandwidth limit
    "-e", "^", ":",     // Exit while complete
    "-E", "^", ":",     // target seed ratio
    "-f", "^",          // force seed mode, skip sha1 check when startup.
    "-h", "^",          // help
    "-H", "^",          // help
    "-i", "^", ":",     // listen on ip XXXX
    "-I", "^", ":",     // public/external IP address
    "-l", "^", ":",     // piece Length (default 262144)
    "-M", "^", ":",     // Max peers
    "-m", "^", ":",     // Min peers
    "-n", "^", ":",     // Which file download
    "-P", "^", ":",     // peer ID prefix
    "-p", "^", ":",     // listen on Port XXXX
    "-s", "^", ":",     // Save as FILE/DIR NAME
    "-S", "^", ":",     // CTCS server
    "-t", "^",          // make Torrent
    "-T", "^",          // convert foreign filenames to printable text
    "-u", "^", ":",     // Announce url
    "-U", "^", ":",     //
    "-v", "^",          // verbose output
    "-x", "^",          // print torrent information only
    "-X", "^", ":",     // "user exit" on download completion
    "-z", "^", ":",     // slice size

    "-icon", ":",       // f_clIcon (filename)
    "-title", ":",      // f_clTitle (string)
    "-style", ":",      // f_clStyle (number)
    "-win", ":",        // f_clWin (string)
    "-org", ":",        // f_clOrg (string)
    "-ind",             // f_clInd
    "-rate",            // f_clRate
    "-pdown",           // f_clPtg
    "-log", ":",        // f_clLog
    "-file", ":",       // f_clFile
    "-detach",          // f_clDetach
    "-time",            // f_clTime
    "-com", ":",        // comment
    "-pvt",             // private
    "-cfg", ":",        // config filename
    "-utf",             // filenames not in utf-8
    "-fenc", ":",        // filenames not in utf-8 , recode from <codepage>

    "-fake", ":",       // fFakeDL,fFakeUL

    "-dbg",             // f_dbg
//    "-trc",             // funks tracer

    ""
  };

/*
if(f_Cfg) {
  printf("\n[%d]", argc); fflush(stdout);
  for(int i = 0; i < argc; i++) {
    printf(" '%s'", argv[i]);
    fflush(stdout);
  }
  printf("\n"); fflush(stdout);
  exit(0);
}
*/

//  if(cMode != 'c' && argc < 2) return -2;  // разбор командной строки.
  if(argc < 2) return 0;

  if(cMode == 'c') {
    while((rc = getopts(argc, argv, Opts)) != -1) {  // ищем -cfg.
      if(rc != '?' && rc != '-' && rc != ':') {
        if(!strcmp(Opts[OptNum], "-cfg")) {  // config filename
	  pCfgName = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(pCfgName, argv[ParamNum]);
	  break;
	}
      }
    }
    return 0;
  }

  if(!strcmp(argv[1], "?") || !stricmp(argv[1], "h") ||
    !strcmp(argv[1], "-?") || !stricmp(argv[1], "-h"))
    return -2;

  ArgIdx = 0;
  while((rc = getopts(argc, argv, Opts)) != -1) {
    if(CmdErr >= 10) {
      printf("! too many errors, aborted.\n");
      exit(-1);
    }
    if(rc == '?') {
      if((strlen(argv[OptNum]) > 1) && (*argv[OptNum] == '-')) {
	printf("! unknown option '%s'. Use -h for help/usage.\n", ++argv[OptNum]);
	++CmdErr;
      }
      else if(*argv[OptNum] != '-') {   // парметр без команды.
	if(arg_metainfo_file) {
	  printf("! argument '%s' requires an option.\n", argv[OptNum]);
          ++CmdErr;
	}
	else {
	  arg_metainfo_file = new char[strlen(argv[OptNum]) + 1];
	  strcpy(arg_metainfo_file, argv[OptNum]);
	}
      }
    }
    else if(rc == ':') {
      printf("! option '-%s' requires an argument. Use -h for help/usage.\n", ++Opts[OptNum]);
      ++CmdErr;
    }

    else {

/*
      if(!strcmp(Opts[OptNum], "-dbg")) {
        if(!f_dbg) {
	  fdbg = fopen( "dbg.log", "a+b" );
	  fprintf(fdbg, "\n");
        }
        f_dbg = 1;
      }

    else if(!strcmp(Opts[OptNum], "-trc")) {
      if(!f_dbg && !f_trc) fdbg = fopen( "dbg.log", "wb" );
      f_trc = 1;
f_Set = 1;
    }
*/


      if(!strcmp(Opts[OptNum], "-a"))
        arg_allocate = 1;

      else if(!strcmp(Opts[OptNum], "-b")) {
        arg_bitfield_file = new char[strlen(argv[ParamNum]) + 1];
        strcpy(arg_bitfield_file, argv[ParamNum]);
      }

      else if(!strcmp(Opts[OptNum], "-i"))  // listen on ip XXXX
        cfg_listen_ip = inet_addr(argv[ParamNum]);

      else if(!strcmp(Opts[OptNum], "-I")) {  // set public ip XXXX
        cfg_public_ip = new char[strlen(argv[ParamNum]) + 1];
        strcpy(cfg_public_ip, argv[ParamNum]);
      }

      else if(!strcmp(Opts[OptNum], "-p")) {  // listen on Port XXXX
//        if( arg_flg_make_torrent ) arg_flg_private = 1;
//        else cfg_listen_port = atoi(argv[ParamNum]);
        cfg_listen_port = atoi(argv[ParamNum]);
      }

      else if(!strcmp(Opts[OptNum], "-s")) { // Save as FILE/DIR NAME
        if( arg_save_as ) ++CmdErr;  // specified twice
        else {
	  arg_save_as = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(arg_save_as, argv[ParamNum]);
        }
      }

      else if(!strcmp(Opts[OptNum], "-e"))  // Exit while complete
        cfg_seed_hours = (time_t)strtoul(argv[ParamNum], NULL, 10);

      else if(!strcmp(Opts[OptNum], "-E"))  // target seed ratio
        cfg_seed_ratio = atof(argv[ParamNum]);

      else if(!strcmp(Opts[OptNum], "-c")) {  // Check exist only
//        if(arg_flg_make_torrent) {
//          arg_comment = new char[strlen(argv[ParamNum]) + 1];
//          strcpy(arg_comment, argv[ParamNum]);
//	}
//	else arg_flg_check_only = 1;
	arg_flg_check_only = 1;
      }

      else if(!strcmp(Opts[OptNum], "-C"))  // Max cache size
        cfg_cache_size = atoi(argv[ParamNum]);

      else if(!strcmp(Opts[OptNum], "-M")) {  // Max peers
        cfg_max_peers = atoi(argv[ParamNum]);
        if( cfg_max_peers > 1000 || cfg_max_peers < 2 ){
          CONSOLE.Warning(1, "'-M' argument must be between 2 and 1000");
          ++CmdErr;
        }
      }

      else if(!strcmp(Opts[OptNum], "-m")) {  // Min peers
        cfg_min_peers = atoi(argv[ParamNum]);
        if( cfg_min_peers > 1000 || cfg_min_peers < 1 ){
          CONSOLE.Warning(1, "'-m' argument must be between 1 and 1000");
	  ++CmdErr;
        }
      }

      else if(!strcmp(Opts[OptNum], "-z")) {  // slice size
        cfg_req_slice_size = atoi(argv[ParamNum]) * 1024;
        if(cfg_req_slice_size < 1024 || cfg_req_slice_size > cfg_max_slice_size){
          CONSOLE.Warning(1, "'-z' argument must be between 1 and %d", cfg_max_slice_size / 1024);
          ++CmdErr;
        }
      }

      else if(!strcmp(Opts[OptNum], "-n")) {  // Which file download
	if(arg_file_to_download || f_clFile) {
	  ++CmdErr;  // specified twice
	  printf("! '-file' or '-n' option specified more than once.\n");
        }
        else {
	  arg_file_to_download = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(arg_file_to_download, argv[ParamNum]);
	}
      }

      else if(!strcmp(Opts[OptNum], "-f"))  // force seed mode, skip sha1 check when startup.
        arg_flg_force_seed_mode = 1;

      else if(!strcmp(Opts[OptNum], "-D"))  // download bandwidth limit
        cfg_max_bandwidth_down = (int)(strtod(argv[ParamNum], NULL) * 1024);

      else if(!strcmp(Opts[OptNum], "-U"))  // upload bandwidth limit
        cfg_max_bandwidth_up = (int)(strtod(argv[ParamNum], NULL) * 1024);

      else if(!strcmp(Opts[OptNum], "-P")) {  // peer ID prefix
        int  l = strlen(argv[ParamNum]);
        if (l > MAX_PF_LEN) {
          CONSOLE.Warning(1, "'-P' arg must be %d or less characters", MAX_PF_LEN);
          ++CmdErr;
        }
        else {
	  if (l == 1 && *argv[ParamNum] == '-') *arg_user_agent = (char) 0;
	  else strcpy(arg_user_agent, argv[ParamNum]);
        }
      }

      else if(!strcmp(Opts[OptNum], "-A")) {  // HTTP user-agent header string
        if( cfg_user_agent ) delete []cfg_user_agent;
        cfg_user_agent = new char[strlen(argv[ParamNum]) + 1];
        strcpy(cfg_user_agent, argv[ParamNum]);
      }

      else if(!strcmp(Opts[OptNum], "-T"))  // convert foreign filenames to printable text
      //        arg_flg_convert_filenames = 1;
       {}  // disabled

      /* BELOW OPTIONS USED FOR CREATE TORRENT */

      else if(!strcmp(Opts[OptNum], "-u")) {  // Announce url
	if( arg_announce ) {
//	  ++CmdErr;  // specified twice
//	  printf("! '-u' option specified more than once.\n");
	  delete []arg_announce;
//	  arg_announce = NULL;
        }
//        else {
	  arg_announce = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(arg_announce, argv[ParamNum]);
//        }
      }

      else if(!strcmp(Opts[OptNum], "-t")) {  // make Torrent
        arg_flg_make_torrent = 1;
	CONSOLE.ChangeChannel(O_INPUT, "off", 0);
      }

      else if(!strcmp(Opts[OptNum], "-l")) {  // piece Length (default 262144)
        arg_piece_length = atoi(argv[ParamNum]);
        if( arg_piece_length < 65536 || arg_piece_length > 4096*1024 ) {
          CONSOLE.Warning(1, "'-l' argument must be between 65536 and %d", 4096*1024);
          ++CmdErr;
        }
      }

      /* ABOVE OPTIONS USED FOR CREATE TORRENT */

      else if(!strcmp(Opts[OptNum], "-x")) {  // print torrent information only
        arg_flg_exam_only = 1;
        CONSOLE.ChangeChannel(O_INPUT, "off", 0);
      }

      else if(!strcmp(Opts[OptNum], "-S")) {  // CTCS server
	if( arg_ctcs ) {
//	  ++CmdErr;  // specified twice
//	  printf("! '-S' option specified more than once.\n");
	  delete []arg_ctcs;
//	  arg_ctcs = NULL;
        }
//        else {
	  arg_ctcs = new char[strlen(argv[ParamNum]) + 1];
          if( !strchr(argv[ParamNum], ':') ){
            CONSOLE.Warning(1, "'-S' argument requires a port number");
	    ++CmdErr;
	    delete []arg_ctcs;
	    arg_ctcs = NULL;
	  }
	  else strcpy(arg_ctcs, argv[ParamNum]);
//        }
      }

      else if(!strcmp(Opts[OptNum], "-X")) {  // "user exit" on download completion
        if( arg_completion_exit ) ++CmdErr;  // specified twice
        else {
	  arg_completion_exit = new char[strlen(argv[ParamNum]) + 1];
#ifndef HAVE_SYSTEM
          CONSOLE.Warning(1, "'-X' is not supported on your system");
          ++CmdErr;
#endif
#ifndef HAVE_WORKING_FORK
          CONSOLE.Warning(2, "No working fork function; be sure the -X command is brief!");
          ++CmdErr;
#endif
	  if(!CmdErr) strcpy(arg_completion_exit, argv[ParamNum]);
        }
      }

      else if(!strcmp(Opts[OptNum], "-v"))  // verbose output
        arg_verbose = 1;

      else if(!strcmp(Opts[OptNum], "-d"))  // daemon mode (fork to background)
        arg_daemon = 1;

      else if(!stricmp(Opts[OptNum], "-h") || !stricmp(Opts[OptNum], "-?")) {  // help
        f_Help = 1;
        break;
      }

      else if(!strcmp(Opts[OptNum], "-icon")) {
        strcpy(pIconName, argv[ParamNum]);
        f_clIcon = 1;
      }

      else if(!strcmp(Opts[OptNum], "-title")) {
        strncpy(pTitle, argv[ParamNum],
               (strlen(argv[ParamNum]) >= MAXNAMEL - 1) ? MAXNAMEL - 1 : strlen(argv[ParamNum]));
        f_clTitle = 1;
      }

      else if(!strcmp(Opts[OptNum], "-style")) {
        f_clStyle = (UCHAR)atoi(argv[ParamNum]);
        if(f_clStyle > 3) f_clStyle = 3;
      }

      else if(!strcmp(Opts[OptNum], "-win")) {
        char  *pStr = new char[strlen(argv[ParamNum]) + 1];
        strcpy(pStr, argv[ParamNum]);
        ParsColRow(pStr);
        delete []pStr;
      }

      else if(!strcmp(Opts[OptNum], "-org")) {
        char  *pStr = new char[strlen(argv[ParamNum]) + 1];
        strcpy(pStr, argv[ParamNum]);
        ParsOrg(pStr);
        delete []pStr;
      }

      else if(!strcmp(Opts[OptNum], "-ind"))
	f_clInd = 1;

      else if(!strcmp(Opts[OptNum], "-pdown"))
	f_clPtg = 1;

      else if(!strcmp(Opts[OptNum], "-rate"))
	f_clRate = 1;

      else if(!strcmp(Opts[OptNum], "-log")) {
//	if(f_clLog) {
//	  printf("! '-log' option specified more than once.\n");
//	}
//	else {
	  char  *pStr = new char[strlen(argv[ParamNum]) + 1];
          strcpy(pStr, argv[ParamNum]);
          ParsLog(pStr);
	  delete []pStr;
//	}
      }

      else if(!strcmp(Opts[OptNum], "-file")) {  // Which file download
	if(arg_file_to_download || f_clFile) {
	  ++CmdErr;  // specified twice
	  printf("! '-file' or '-n' option specified more than once.\n");
	}
        else {
	  arg_file_to_download = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(arg_file_to_download, argv[ParamNum]);
	  f_clFile = 1;
	  pArgFiles = new char[strlen(arg_file_to_download) + 2];
	  if(ParsFileNums(pArgFiles, arg_file_to_download)) {
	    printf("! '-file' option has incorrect argument.\n");
	    ++CmdErr;
	  }
        }
      }

      else if(!strcmp(Opts[OptNum], "-detach"))
	f_clDetach = 1;

      else if(!strcmp(Opts[OptNum], "-time"))
	f_clTime = 1;

      else if(!strcmp(Opts[OptNum], "-com")) {  // Add comment
	if(arg_comment) delete []arg_comment;
        arg_comment = new char[strlen(argv[ParamNum]) * 6 + 1];
        if(CSC(arg_comment, (char*)argv[ParamNum], strlen(argv[ParamNum]) * 6 + 1,
           strlen(argv[ParamNum]), '\0'))
          strcpy(arg_comment, argv[ParamNum]);  // ошибка перекодирования.
      }

      else if(!strcmp(Opts[OptNum], "-pvt"))
	arg_flg_private = 1;

      else if(!strcmp(Opts[OptNum], "-cfg")) {  // config filename
/*	if(f_Cfg) {
	    printf("! '-cfg' option is not allowed in configuration file.\n");
	    ++CmdErr;
	}
	else if(pCfgName) {
	  ++CmdErr;  // specified twice
	  printf("! '-cfg' option specified more than once.\n");
	}
	else {
	  pCfgName = new char[strlen(argv[ParamNum]) + 1];
	  strcpy(pCfgName, argv[ParamNum]);
	} */
      }

      else if(!strcmp(Opts[OptNum], "-fake")) {
        int  irc;
        char  *pStr = new char[strlen(argv[ParamNum]) + 1];
        strcpy(pStr, argv[ParamNum]);
        irc = ParsFake(pStr);
        delete []pStr;
	if(irc) { fFakeDL = 1.0; fFakeUL = 1.0; }
	else {
	  if(fFakeDL > 1.0 || fFakeDL == 0.) fFakeDL = 1.0;
	  if(fFakeUL < 1.0 || fFakeUL == 0.) fFakeUL = 1.0;
	}
      }

      else if(!strcmp(Opts[OptNum], "-dbg"))
	f_clDbg = 1;

      else if(!strcmp(Opts[OptNum], "-utf"))
	f_clUTF = 1;

//an64
      else if(!strcmp(Opts[OptNum], "-fenc")) {
	f_clUTF = 1;
	f_clenc = 1;
	fenc = new char[strlen(argv[ParamNum]) + 1];
	strcpy(fenc, argv[ParamNum]);
      }


    }
  }

  if(f_Help) return -2;
  if(CmdErr) return -1;
  return 0;

}


