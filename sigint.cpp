#ifndef WINDOWS

#include "sigint.h"  // def.h

#include <sys/types.h>
#include <signal.h>

#include "btcontent.h"
#include "tracker.h"
#include "peerlist.h"
#include "btconfig.h"
#include "console.h"

extern "C" {

//static RETSIGTYPE sig_catch2(int sig_no);
RETSIGTYPE sig_catch2(int sig_no);

RETSIGTYPE sig_catch(int sig_no)
{
  if(sig_no == SIGBREAK || sig_no == SIGINT || sig_no == SIGTERM) {
//    Tracker.ClearRestart();
//    Tracker.SetStoped();

//printf("\nsig_catch %u\n", f_Exit); fflush(stdout);

    signal(sig_no, sig_catch2);
    if(!f_Exit) {
      f_exit = 1;
      f_console = 1;
    }
//    else raise(sig_no);
  }
}

//static RETSIGTYPE sig_catch2(int sig_no)
RETSIGTYPE sig_catch2(int sig_no)
{
  if(sig_no == SIGBREAK || sig_no == SIGINT || sig_no == SIGTERM) {

//printf("\nsig_catch2\n"); fflush(stdout);

    if(cfg_cache_size) BTCONTENT.FlushCache();
    BTCONTENT.SaveBitfield();
    WORLD.CloseAll();
//    signal(sig_no, SIG_DFL);
//    raise(sig_no);
    exit(0);
  }
  signal(sig_no, sig_catch2);

}

} // extern "C"

#endif

