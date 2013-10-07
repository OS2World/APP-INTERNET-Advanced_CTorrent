#define INCL_DOSSEMAPHORES
#define INCL_DOSPROCESS
#include <os2.h>
#include <process.h>

#include "def.h"

#include <sys/types.h>

#include <sys/time.h>
#include <time.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "peerlist.h"
#include "tracker.h"
#include "btcontent.h"
#include "ctcs.h"
#include "btconfig.h"
#include "bttime.h"
#include "console.h"


//acw
#include "acw.h"
#include <conio.h>

#undef FD_SET
#define FD_SET(fd, set) do { \
        if(!FD_ISSET(fd, set)) \
          if (((fd_set *)(set))->fd_count < FD_SETSIZE) \
          ((fd_set *)(set))->fd_array[((fd_set *)(set))->fd_count++]=fd;\
} while(0)


#define MAX_SLEEP 1


time_t now;

void Downloader()
{
  int nfds = 0, maxfd, r;
  struct timeval timeout;
  fd_set rfd, rfdnext;
  fd_set wfd, wfdnext;
  int stopped = 0, f_idleused = 0, f_poll = 0;
//  struct timespec nowspec;
  double maxsleep;
  time_t then, concheck = (time_t)0;


  DosCreateMutexSem(0, &hmtxConsole, 0L, FALSE);
  DosCreateMutexSem(0, &hmtxStLine, 0L, FALSE);
  _beginthread(Kbd, NULL, 10 * 1024, (void*)0);

  max_bandwidth_down = cfg_max_bandwidth_down;
  max_bandwidth_up = cfg_max_bandwidth_up;
  seed_hours = cfg_seed_hours;
  seed_ratio = cfg_seed_ratio;
  min_peers = cfg_min_peers;
  max_peers = cfg_max_peers;
  cache_size = cfg_cache_size;

//  dtime();

  FD_ZERO(&rfdnext); FD_ZERO(&wfdnext);

//acw t
//  time(&now);
//  dtime();

  do{
     dtime();
     if( !stopped ){
      if( !Tracker.IsQuitting() && BTCONTENT.SeedTimeout() )
        Tracker.SetStoped();
      if( Tracker.IsQuitting() && !Tracker.IsRestarting() ){
        stopped = 1;
        WORLD.Pause();
        if( arg_ctcs ) CTCS.Send_Status();
      }
    }

    maxfd = -1;
    maxsleep = -1;
    rfd = rfdnext;
    wfd = wfdnext;

    if( f_poll ){
      FD_ZERO(&rfd); FD_ZERO(&wfd);  // remove non-peers from sets
      maxsleep = 0;  // waited for bandwidth--poll now
    }else{
      WORLD.DontWaitBW();
      if( WORLD.IsIdle() ){
	f_idleused = 0;
	if( BTCONTENT.CheckedPieces() < BTCONTENT.GetNPieces() &&
            !BTCONTENT.NeedFlush() ){
          if( BTCONTENT.CheckNextPiece() < 0 ){
            CONSOLE.Warning(1, " Error while checking piece %d of %d",
              (int)(BTCONTENT.CheckedPieces()), (int)(BTCONTENT.GetNPieces()));
            Tracker.SetStoped();
            maxsleep = 2;
          }else maxsleep = 0;
          f_idleused = 1;
          dtime();
        }
	r = Tracker.IntervalCheck(&rfd, &wfd);
        if( r > maxfd ) maxfd = r;
        if( arg_ctcs ){
          r = CTCS.IntervalCheck(&rfd, &wfd);
          if( r > maxfd ) maxfd = r;
        }
	if( !f_idleused || concheck <= now-2 || WORLD.IsIdle() ){
          concheck = now;
//acw
//          r = CONSOLE.IntervalCheck(&rfd, &wfd);
          r = CONSOLE.IntervalCheck();
          if( r > maxfd ) maxfd = r;
	}
      }
    }
    r = WORLD.IntervalCheck(&rfd, &wfd);
    if( r > maxfd ) maxfd = r;

    if( !f_poll ){
      dtime();
      while( BTCONTENT.NeedFlush() && WORLD.IsIdle() ){
        BTCONTENT.FlushQueue();
        maxsleep = 0;
        dtime();
      }
    }

    rfdnext = rfd;
    wfdnext = wfd;

    if( maxsleep < 0 ){  //not yet set
      maxsleep = WORLD.WaitBW();  // must do after intervalchecks!
//      if( maxsleep <= -100 ) maxsleep = 0;
      if(maxsleep <= 0 ) maxsleep = 0.01;
      else if( maxsleep <= 0 || maxsleep > MAX_SLEEP ) maxsleep = MAX_SLEEP;
    }

    timeout.tv_sec = (long)maxsleep;
    timeout.tv_usec = (long)( (maxsleep-(long)maxsleep) * 1000000 );

    WORLD.UnLate();

//acw
//    nfds = select(maxfd + 1,&rfd,&wfd,(fd_set*) 0,&timeout);
    if(rfd.fd_count || wfd.fd_count) nfds = select(maxfd + 1,&rfd,&wfd,(fd_set*) 0,&timeout);
    else {
      nfds = 0;
      DosSleep(10);
    }

    if( nfds < 0 ){
      CONSOLE.Debug("Error from select:  %s", strerror(errno));
      FD_ZERO(&rfdnext); FD_ZERO(&wfdnext);
      nfds = 0;
    }

    if( f_poll ) f_poll = 0;
    else if( nfds > 0 ) WORLD.DontWaitBW();
    else if( maxsleep > 0 && maxsleep < MAX_SLEEP ) f_poll = 1;

    then = now;
//acw t
//    time(&now);
    dtime();
//acw bf
//    if( now == then-1 ) now = then;
    if( now < then ) now = then;

    if( !f_poll && nfds > 0 ){
      if(T_FREE != Tracker.GetStatus())
        Tracker.SocketReady(&rfd,&wfd,&nfds,&rfdnext,&wfdnext);
      if(nfds > 0 && T_FREE != CTCS.GetStatus())
        CTCS.SocketReady(&rfd,&wfd,&nfds,&rfdnext,&wfdnext);
    }
    ChkConsole();

    if(nfds > 0)
      WORLD.AnyPeerReady(&rfd,&wfd,&nfds,&rfdnext,&wfdnext);

  } while(Tracker.GetStatus() != T_FINISHED || Tracker.IsRestarting());

//acw
  DosCloseMutexSem(hmtxStLine);
  DosCloseMutexSem(hmtxConsole);
  printf("\n\nExit. '%s'", arg_metainfo_file);

}

