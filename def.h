#ifndef DEF_H
#define DEF_H

#include "./config.h"

//acw
#ifdef __OS2__

#include <types.h>

#ifndef SOCKET
typedef int SOCKET;
#endif

//typedef int ssize_t;
typedef int socklen_t;
//typedef unsigned long long u_int64_t;
//typedef unsigned char u_int8_t;

#define PATH_SP '\\'
#define MAXPATHLEN 1024
#define MAXHOSTNAMELEN 256

#define strncasecmp strnicmp  //
#define strcasecmp stricmp    //

#ifdef __WATCOMC__
#define random rand
#define srandom srand

#define RECV(fd,buf,len) recv((fd),(buf),(len),0)
#define SEND(fd,buf,len) send((fd),(buf),(len),0)
#define EWOULDBLOCK 10035
#define EINPROGRESS 10036
#define ECONNREFUSED 10061
#define CLOSE_SOCKET(sk) soclose(sk)
#define __TCPIP_HEADERS

#else
#define CLOSE_SOCKET(sk) close((sk))
#define RECV(fd,buf,len) read((fd),(buf),(len))
#define SEND(fd,buf,len) write((fd),(buf),(len))
#endif

#define INVALID_SOCKET -1

#else

#ifdef WINDOWS	//if Windows ********************
typedef int ssize_t;
typedef int socklen_t;

#define PATH_SP '\\'
#define MAXPATHLEN 1024
#define MAXHOSTNAMELEN 256

//#define mkdir(path,mode) _mkdir((path))

#define snprintf _snprintf
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define ioctl ioctlsocket

#define RECV(fd,buf,len) recv((fd),(buf),(len),0)
#define SEND(fd,buf,len) send((fd),(buf),(len),0)
#define EWOULDBLOCK	WSAEWOULDBLOCK
#define EINPROGRESS	WSAEINPROGRESS
#define CLOSE_SOCKET(sk) closesocket((sk))

#else		// if *Nix *****************************

#define CLOSE_SOCKET(sk) close((sk))

#ifndef SOCKET
typedef int SOCKET;
#endif

#define INVALID_SOCKET -1

#define PATH_SP '/'
#define RECV(fd,buf,len) read((fd),(buf),(len))
#define SEND(fd,buf,len) write((fd),(buf),(len))

#endif

#endif

//acw
#endif // OS2
