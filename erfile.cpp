#define INCL_DOS
#define INCL_LONGLONG
#include <os2.h>

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <share.h>
#include <time.h>
#include <sys/utime.h>

#include "erfile.h"
#include "acw.h"

#ifndef ORD_DOSOPENL
#define ORD_DOSOPENL        981
#endif
#ifndef ORD_DOSSETFILEPTRL
#define ORD_DOSSETFILEPTRL  988
#endif
#ifndef ORD_DOSSETFILESIZEL
#define ORD_DOSSETFILESIZEL  989
#endif


APIRET APIENTRY (*pDosOpenL)( PCSZ pszFileName, PHFILE pHf, PULONG pulAction, LONGLONG cbFile, ULONG ulAttribute, ULONG fsOpenFlags, ULONG fsOpenMode, PEAOP2 peaop2 );
APIRET APIENTRY (*pDosSetFilePtrL)( HFILE hFile, LONGLONG ib, ULONG method, PLONGLONG ibActual );
APIRET APIENTRY (*pDosSetFileSizeL)( HFILE hFile, LONGLONG cbSize );

static bool loadLLFuncs();

static HMODULE dcHndl = NULLHANDLE;
static bool funcsLoadedLL = loadLLFuncs();

static void freeDoscalls()
{
    if ( dcHndl != NULLHANDLE ) {
    	DosFreeModule( dcHndl );
    }
}

static bool loadLLFuncs()
{
    bool res = false;
    do
    {
        if ( DosLoadModule( NULL, 0, "DOSCALLS", &dcHndl ) != 0 )
            break;
        if ( DosQueryProcAddr( dcHndl, ORD_DOSOPENL, NULL, (PFN *)&pDosOpenL ) != 0 )
            break;
        if ( DosQueryProcAddr( dcHndl, ORD_DOSSETFILEPTRL, NULL, (PFN *)&pDosSetFilePtrL ) != 0 )
            break;
        if ( DosQueryProcAddr( dcHndl, ORD_DOSSETFILESIZEL, NULL, (PFN *)&pDosSetFileSizeL ) != 0 )
            break;

        res = true;
    } while (0);

    atexit( freeDoscalls );

    return res;
}


// static method
bool t_file::canUseLargeFiles()
{
    return (bool)funcsLoadedLL;
}

t_file::t_file()
{
    handle  = -1;
    lasterr = 0;
    memset(fname, 0, sizeof(fname));  // nick
}

t_file::~t_file()
{
    close();
}

int t_file::close()
{
    if ( handle != -1 ) {
        lasterr = DosClose( handle );
        handle = -1;
        if ( lasterr != 0 ) {
	  rc_api = lasterr;  //acw
	  if(f_clDbg) sprintf(pErrDbgStr, " DosClose:%lu ", rc_api);  //acw
	  return -1;
        }
    }
    return 0;
}

int t_file::flush()
{
    lasterr = DosResetBuffer( handle );
    if ( lasterr != 0 ) {
        rc_api = lasterr;  //acw
	if(f_clDbg) sprintf(pErrDbgStr, " DosResetBuffer:%lu ", rc_api);  //acw
        return -1;
    }
    return 0;
}

int t_file::reopen(int mode, int sharing)  // nick
{
	if (handle < 0 || !*fname)
	{
		printf("reopen: file not opened\n");
		return -1;
	}
	long long pos = seek(0, SEEK_CUR);
	int rc = open(fname, mode,  sharing);
	if (rc < 0)
	{
		return -1;
	}
	if (pos >= 0)
	{
		seek(pos, SEEK_SET);
	}
	return 0;
}

// possible modes:
// O_CREAT, O_TRUNC
// O_RDONLY, O_WRONLY, O_RDWR
// O_NOINHERIT
// sharing: SH_DENYRW, SH_DENYWR, SH_DENYRD, SH_DENYNO
int t_file::open( const char *name, int mode, int sharing )
{
    // map modes
    ULONG openFlags = 0;
    if ( mode & O_CREAT ) {
        if( mode & O_TRUNC ) {
            openFlags |= ( OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_REPLACE_IF_EXISTS );
        } else {
            openFlags |= ( OPEN_ACTION_CREATE_IF_NEW | OPEN_ACTION_FAIL_IF_EXISTS );
        }
    } else if ( mode & O_TRUNC ) {
        openFlags |= OPEN_ACTION_REPLACE_IF_EXISTS;
    } else {
        openFlags |= ( OPEN_ACTION_FAIL_IF_NEW | OPEN_ACTION_OPEN_IF_EXISTS );
    }

    ULONG openMode = 0;
    int accessMode = mode & 0x0f;
    if ( accessMode == O_RDONLY ) {
        openMode |= OPEN_ACCESS_READONLY;
    } else if ( accessMode == O_WRONLY ) {
        openMode |= OPEN_ACCESS_WRITEONLY;
    } else if ( accessMode == O_RDWR ) {
        openMode |= OPEN_ACCESS_READWRITE;
    }
    if ( mode & O_NOINHERIT ) {
        openMode |= OPEN_FLAGS_NOINHERIT;
    }
    if ( sharing & SH_DENYRW ) {
//nick        openMode |= OPEN_SHARE_DENYREADWRITE;
        openMode |= OPEN_SHARE_DENYWRITE;
    } else if ( sharing & SH_DENYWR ) {
        openMode |= OPEN_SHARE_DENYWRITE;
    } else if ( sharing & SH_DENYRD ) {
//nick        openMode |= OPEN_SHARE_DENYREAD;
        openMode |= OPEN_SHARE_DENYWRITE;
    } else if ( sharing & SH_DENYNO ) {
//nick        openMode |= OPEN_SHARE_DENYNONE;
        openMode |= OPEN_SHARE_DENYWRITE;
    }

    ULONG action = 0;
    if ( handle != -1 ) {
      flush();  // nick
      close();
    }
    if (name != fname) strncpy(fname, name, CCHMAXPATH);  // nick

    if ( funcsLoadedLL ) {
        LONGLONG llns = 0;
        lasterr = pDosOpenL( name, (HFILE *)&handle, &action, llns, FILE_NORMAL,
                             openFlags, openMode, NULL );
    }
    else {
        lasterr = DosOpen( name, (HFILE *)&handle, &action, 0, FILE_NORMAL,
                           openFlags, openMode, NULL );
    }
    if ( lasterr != 0 ) {
 	printf("\nCan't open file (%s) : %d\n", fname, lasterr);  // nick
        handle = -1;
//	rc_api = lasterr;  //acw
        return -1;
    }
    return 0;
}

int t_file::read( void *buf, int len )
{
    ULONG act = 0;
    lasterr = DosRead( handle, buf, len, &act );
    rc_api = lasterr;  //acw
    if(f_clDbg) sprintf(pErrDbgStr, " DosRead:%lu ", rc_api);  //acw
    return act;
}

int t_file::write( void *buf, int len )
{
    ULONG act = 0;
    lasterr = DosWrite( handle, buf, len, &act );
    rc_api = lasterr;  //acw
    if(f_clDbg) sprintf(pErrDbgStr, " DosWrite:%lu ", rc_api);  //acw
    return act;
}

// method - SEEK_SET, SEEK_CUR, SEEK_END
long long t_file::seek( long long offset, int method )
{
    ULONG mth = 0;
    switch ( method )
    {
        case SEEK_SET: mth = FILE_BEGIN;   break;
        case SEEK_CUR: mth = FILE_CURRENT; break;
        case SEEK_END: mth = FILE_END;     break;
    }

    long long act = 0;
    if ( funcsLoadedLL )
    {
        LONGLONG tmpact = 0;
        LONGLONG lloffs = offset;
        lasterr = pDosSetFilePtrL( handle, lloffs, mth, &tmpact );
        act = tmpact;
    }
    else
    {
        ULONG tmpact = 0;
        lasterr = DosSetFilePtr( handle, (LONG)offset, mth, &tmpact );
        act = tmpact;
    }
    if ( lasterr != 0 ) {
        rc_api = lasterr;  //acw
	if(f_clDbg) sprintf(pErrDbgStr, " DosSetFilePtr(L):%lu ", rc_api);  //acw
        return -1;
    }
    return act;
}

long long t_file::size()
{
    long long ret = 0;

    if ( funcsLoadedLL ) {
        FILESTATUS3L f;
        lasterr = DosQueryFileInfo( handle, FIL_STANDARDL, &f, sizeof( f ) );
        ret = f.cbFile;
    }
    else {
        FILESTATUS3 f;
        lasterr = DosQueryFileInfo( handle, FIL_STANDARD, &f, sizeof( f ) );
        ret = f.cbFile;
    }
    if ( lasterr != 0 ) {
        rc_api = lasterr;  //acw
	if(f_clDbg) sprintf(pErrDbgStr, " DosQueryFileInfo:%lu ", rc_api);  //acw
        return -1;
    }

    return ret;
}

int t_file::setSize( long long newsize )
{
    if ( funcsLoadedLL ) {
        LONGLONG ns = newsize;
        lasterr = pDosSetFileSizeL( handle, ns );
    } else {
        lasterr = DosSetFileSize( handle, (ULONG)newsize );
    }
    if ( lasterr != 0 ) {
        rc_api = lasterr;  //acw
	if(f_clDbg) sprintf(pErrDbgStr, " DosSetFileSize(L):%lu ", rc_api);  //acw
        return -1;
    }

    return 0;
}

// static method
long long t_file::size( const char *filename )
{
    APIRET rc = 0;
    long long ret = 0;

    if ( funcsLoadedLL ) {
        FILESTATUS3L f;
        rc = DosQueryPathInfo( filename, FIL_STANDARDL, &f, sizeof( f ) );
        ret = f.cbFile;
    }
    else {
        FILESTATUS3 f;
        rc = DosQueryPathInfo( filename, FIL_STANDARD, &f, sizeof( f ) );
        ret = f.cbFile;
    }
    if ( rc != 0 ) {
        rc_api = rc;  //acw
	if(f_clDbg) sprintf(pErrDbgStr, " DosQueryPathInfo:%lu ", rc_api);  //acw
        return -1;
    }

    return ret;
}


// static method
bool t_file::exist( const char *fn )
{
    return ( access( fn, F_OK ) == 0 );
}

// static method
int t_file::move( const char *namOld, const char *namNew )
{
    return DosMove( namOld, namNew );
}

// static method
int t_file::copy( const char *namOld, const char *namNew )
{
    return DosCopy( namOld, namNew, DCPY_EXISTING );
}

// static method
int t_file::erase( const char *fn )
{
    return DosForceDelete( fn );
}

/*
//acw
int t_file::SetFileSize(const char *FileName, LONGLONG Size) {

    if ( funcsLoadedLL ) {
        LONGLONG ns = Size;
        lasterr = pDosSetFileSizeL( handle, ns );
    }
    else {
      lasterr = DosSetFileSize( handle, newsize );
    }
    if ( lasterr != 0 ) {
        return -1;
    }

    return 0;
}
*/
