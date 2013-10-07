//acw
#include <sys/types.h>

#include "connect_nonb.h"  // def.h

//acw
//#include <errno.h>
#ifndef __WATCOMC__
#include <errno.h>
#endif

#if defined( __OS2__ ) && defined( __WATCOMC__ )
#undef errno
#define errno sock_errno()
#define strerror sock_strerror
#endif


// ����ֵ 
// >0 �����ѳɹ�
// -1 ������ʧ��
// -2 �������ڽ���
int connect_nonb(SOCKET sk,struct sockaddr* psa)
{
  int r;
  r = connect(sk,psa,sizeof(struct sockaddr));
  if(r < 0 && errno == EINPROGRESS) r = -2;
  return r;
}
