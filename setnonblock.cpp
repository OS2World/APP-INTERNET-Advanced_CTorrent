#include "./setnonblock.h"

#if defined( WINDOWS ) || ( defined( __OS2__ ) && defined( __WATCOMC__ ) )

#if defined( __OS2__ )
#include <sys/ioctl.h>
#endif

int setfd_nonblock(SOCKET socket)
{
  unsigned long val = 1;
  return ioctl(socket,FIONBIO,&val);
}

#else

#include <unistd.h>
#include <fcntl.h>

int setfd_nonblock(SOCKET socket)
{
  int f_old;
  f_old = fcntl(socket,F_GETFL,0);  
  if( f_old < 0 ) return -1;  
  f_old |= O_NONBLOCK;  
  return (fcntl(socket,F_SETFL,f_old));
}

#endif
