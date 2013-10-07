#ifndef __T_FILE_H
#define __T_FILE_H

class t_file
{
    protected:
        int handle;
        int lasterr;
 	char fname[261];  // nick
    public:
        t_file();
        ~t_file();

        int open( const char *name, int mode, int sharing );
        int reopen(int mode, int sharing );  // nick
        int close();
        int read( void *buf, int len );
        int write( void *buf, int len );
        long long seek( long long offset, int method );
        long long size();
        int setSize( long long newsize );
        int flush();
        int error() { return lasterr; }

//acw
//        static int SetFileSize(const char *FileName, ULONG Size);

        static long long size( const char *filename );
        static bool exist( const char *fn );
        static int move( const char *namOld, const char *namNew );
        static int copy( const char *namOld, const char *namNew );
        static int erase( const char *fn );

        static bool canUseLargeFiles();
};

#endif //__T_FILE_H
