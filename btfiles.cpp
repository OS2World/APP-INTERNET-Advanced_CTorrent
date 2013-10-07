#define INCL_LONGLONG
#include <os2.h>


#include "btfiles.h"

#ifdef WINDOWS
#include <io.h>
#include <memory.h>
#include <direct.h>
#else

#include <unistd.h>
#if defined( __OS2__ ) && defined( __WATCOMC__ )
#include <direct.h>
#else
#include <dirent.h>
#endif

#include <sys/param.h>
#endif

#include <sys/stat.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>  // isprint

#ifdef ERFILES
#include <fcntl.h>
#include <share.h>
#include "erfile.h"
#endif

#include "bencode.h"
#include "btcontent.h"
#include "bitfield.h"
#include "console.h"
#include "bttime.h"

#ifndef HAVE_SNPRINTF
#include "compat.h"
#endif

//acw
#include "acw.h"

#ifdef ERFILES

char* fenc="ibm-12511";

static int tf_close( t_file **f )
{
    int r = 0;
    if ( *f != NULL )
    {
        r = (*f)->close();
        delete *f;
        *f = NULL;
    }
    return r;
}

#define MAX_OPEN_FILES 300
#else
#define MAX_OPEN_FILES 20
#endif



btFiles::btFiles()
{
  m_btfhead = (BTFILE*) 0;
  m_nfiles = 0;
  m_file = (BTFILE **)0;
  m_total_files_length = 0;
  m_total_opened = 0;
  m_flag_automanage = 1;
  m_directory = (char*)0;
}

btFiles::~btFiles()
{
  _btf_destroy();
  if( m_directory ) delete []m_directory;
}

BTFILE* btFiles::_new_bfnode()
{
  BTFILE *pnew = new BTFILE;
  pnew->bf_flag_opened = 0;
  pnew->bf_flag_readonly = 0;

  pnew->bf_filename = (char*) 0;
#ifdef ERFILES
  pnew->bf_fp = NULL;
#else
  pnew->bf_fp = (FILE*) 0;
#endif
  pnew->bf_length = 0;

  pnew->bf_buffer = (char *) 0;
  pnew->bf_last_timestamp = (time_t) 0;
  pnew->bf_next = (BTFILE*) 0;
  return pnew;
}

void btFiles::CloseFile(size_t nfile)
{
  if( nfile && nfile <= m_nfiles )
    _btf_close(m_file[nfile-1]);
}

int btFiles::_btf_close_oldest()
{
  BTFILE *pbf_n,*pbf_close;
  pbf_close = (BTFILE *) 0;
  for(pbf_n = m_btfhead; pbf_n; pbf_n = pbf_n->bf_next){
    if(!pbf_n->bf_flag_opened) continue; // file not been opened.
    if(!pbf_close || pbf_n->bf_last_timestamp < pbf_close->bf_last_timestamp)
      pbf_close = pbf_n;
  }
  if(!pbf_close) return -1;
  return _btf_close(pbf_close);
}

int btFiles::_btf_close(BTFILE *pbf)
{
  if( !pbf->bf_flag_opened ) return 0;

//acw
#ifdef ERFILES
  if(tf_close( &pbf->bf_fp )) {
    pbf->bf_fp = 0;
    os2err(rc_api);
    CONSOLE.Warning(2, "%s warn, error closing file \"%s\":  %s",
      pErrDbgStr, pbf->bf_filename, pErrStr);
  }
#else
  if( fclose(pbf->bf_fp) == EOF ) {
    pbf->bf_fp = (FILE *)0;
    CONSOLE.Warning(2, " warn, error closing file \"%s\":  %s",
      pbf->bf_filename, strerror(errno));
  }
#endif
  pbf->bf_flag_opened = 0;

  if( pbf->bf_buffer ){
    delete []pbf->bf_buffer;
    pbf->bf_buffer = (char *)0;
  }

  m_total_opened--;
  return 0;
}

int btFiles::_btf_open(BTFILE *pbf, const int iotype)
{
  char fn[MAXPATHLEN];

  if( pbf->bf_flag_opened ){
    if( pbf->bf_flag_readonly && iotype ) _btf_close(pbf);
    else return 0;  // already open in a usable mode
  }

  if(m_flag_automanage && (m_total_opened >= MAX_OPEN_FILES)){  // close a file
    if( _btf_close_oldest() < 0 ) return -1;
  }

  if( m_directory ){
    if( MAXPATHLEN <= snprintf(fn, MAXPATHLEN, "%s%c%s", m_directory, PATH_SP,
                               pbf->bf_filename) ){
      errno = ENAMETOOLONG;
      return -1;
    }
  }else{
    strcpy(fn, pbf->bf_filename);
  }

  pbf->bf_last_timestamp = now + 1;

//acw
#ifdef ERFILES
  pbf->bf_fp = new t_file;
  int err = 0;
  int mode = m_flag_readonly ? O_RDONLY : O_RDWR;  //nickk
  if ( t_file::exist( fn ) ) {
//nickk    err = pbf->bf_fp->open( fn, O_RDWR, SH_DENYWR );
    err = pbf->bf_fp->open( fn, mode, SH_DENYWR );
  } else {
//nickk    err = pbf->bf_fp->open( fn, O_CREAT|O_RDWR, SH_DENYWR );
    err = pbf->bf_fp->open( fn, O_CREAT|mode, SH_DENYWR );
  }
  if( err < 0 ) {
      delete pbf->bf_fp;
      pbf->bf_fp = NULL;
      return -1;
  }
#else
  if( !(pbf->bf_fp = fopen(fn, iotype ? "r+b" : "rb")) ){
    if( EMFILE == errno || ENFILE == errno ){
      if( _btf_close_oldest() < 0 ||
          !(pbf->bf_fp = fopen(fn, iotype ? "r+b" : "rb")) )
        return -1;  // caller prints error
    }else return -1;  // caller prints error
  }
  pbf->bf_buffer = new char[DEFAULT_SLICE_SIZE];
  if(pbf->bf_buffer)
    setvbuf(pbf->bf_fp, pbf->bf_buffer, _IOFBF, DEFAULT_SLICE_SIZE);

#endif

  pbf->bf_flag_opened = 1;
  pbf->bf_flag_readonly = iotype ? 0 : 1;
  m_total_opened++;
  return 0;
}

ssize_t btFiles::IO(char *buf, uint64_t off, size_t len, const int iotype)
{
//acw
//  uint64_t n = 0;
//  off_t pos,nio;
//  BTFILE *pbf = m_btfhead;

  uint64_t n = 0;
#ifdef ERFILES
  uint64_t pos;
  off_t nio;
#else
  off_t pos,nio;
#endif
  BTFILE *pbf = m_btfhead;


  if( (off + (uint64_t)len) > m_total_files_length ){
    CONSOLE.Warning(1, " error, data offset %llu length %lu out of range",
      (unsigned long long)off, (unsigned long)len);
    return -1;
  }

  for(; pbf; pbf = pbf->bf_next){
    n += (uint64_t) pbf->bf_length;
    if(n > off) break;
  }

  if( !pbf ){
    CONSOLE.Warning(1, " error, failed to find file for offset %llu",
      (unsigned long long)off);
    return -1;
  }

  pos = off - (n - pbf->bf_length);

  for(; len ;){
    if( (!pbf->bf_flag_opened || (iotype && pbf->bf_flag_readonly)) &&
        _btf_open(pbf, iotype) < 0 ){
      CONSOLE.Warning(1, " error, failed to open file \"%s\":  %s",
        pbf->bf_filename, strerror(errno));
      return -1;
    }

    pbf->bf_last_timestamp = now;

//acw
#ifdef ERFILES
    if( pbf->bf_fp->seek(pos,SEEK_SET) < 0){
      os2err(rc_api);
      CONSOLE.Warning(1, "%s error, failed to seek to %llu on file \"%s\":  %s",
        pErrDbgStr, (unsigned long long)pos, pbf->bf_filename, pErrStr);
#else

#ifdef HAVE_FSEEKO
    if( fseeko(pbf->bf_fp,pos,SEEK_SET) < 0){
#else
    if( fseek(pbf->bf_fp,(long) pos,SEEK_SET) < 0){
#endif
      CONSOLE.Warning(1, " error, failed to seek to %llu on file \"%s\":  %s",
        (unsigned long long)pos, pbf->bf_filename, strerror(errno));
#endif
      return -1;
    }

//acw    nio = (len < pbf->bf_length - pos) ? len : (pbf->bf_length - pos);
    nio = (len < pbf->bf_length - pos) ? len : (off_t)(pbf->bf_length - pos);

    if(0 == iotype){
      errno = 0;
//acw
#ifdef ERFILES
      if( pbf->bf_fp->read(buf,(UINT)nio) != nio ) {
	os2err(rc_api);
	CONSOLE.Warning(1, "%s error, read failed at %llu on file \"%s\":  %s",
          pErrDbgStr, (unsigned long long)pos, pbf->bf_filename, pErrStr);
#else
      if( 1 != fread(buf,nio,1,pbf->bf_fp) && ferror(pbf->bf_fp) ){
        CONSOLE.Warning(1, " error, read failed at %llu on file \"%s\":  %s",
          (unsigned long long)pos, pbf->bf_filename, strerror(errno));
#endif
        return -1;
      }
    }else{
      errno = 0;
#ifdef ERFILES
      if( pbf->bf_fp->write(buf,(UINT)nio)!=nio ) {
	os2err(rc_api);
	CONSOLE.Warning(1, "%s error, write failed at %llu on file \"%s\":  %s",
          pErrDbgStr, (unsigned long long)pos, pbf->bf_filename, pErrStr);
#else
      if( 1 != fwrite(buf,nio,1,pbf->bf_fp) ){
        CONSOLE.Warning(1, " error, write failed at %llu on file \"%s\":  %s",
          (unsigned long long)pos, pbf->bf_filename, strerror(errno));
#endif
        return -1;
      }
//#ifdef ERFILES
//      if(pbf->bf_fp->flush()) {
//	os2err(rc_api);
//	CONSOLE.Warning(1, "%s error, flush failed at %llu on file \"%s\":  %s",
  //        pErrDbgStr, (unsigned long long)pos, pbf->bf_filename, pErrStr);
//#else
//      if( fflush(pbf->bf_fp) == EOF ){
//        CONSOLE.Warning(1, " error, flush failed at %llu on file \"%s\":  %s",
//          (unsigned long long)pos, pbf->bf_filename, strerror(errno));
//#endif
//        return -1;
//      }
    }

    len -= nio;
    buf += nio;

    if( len ){
      do{
        pbf = pbf->bf_next;
        if( !pbf ){
          CONSOLE.Warning(1, " error, data left over with no more files to write");
          return -1;
        }
      }while( 0==pbf->bf_length );
      pos = 0;
    }
  } // end for
  return 0;
}

int btFiles::_btf_destroy()
{
  BTFILE *pbf,*pbf_next;
  for(pbf = m_btfhead; pbf;){
    pbf_next = pbf->bf_next;
//acw
#ifdef ERFILES
    if( pbf->bf_fp && pbf->bf_flag_opened ) tf_close( &pbf->bf_fp );
#else
    if( pbf->bf_fp && pbf->bf_flag_opened ) fclose( pbf->bf_fp );
#endif
    if( pbf->bf_filename ) delete []pbf->bf_filename;
    if( pbf->bf_buffer ) delete []pbf->bf_buffer;
    delete pbf;
    pbf = pbf_next;
  }
  m_btfhead = (BTFILE*) 0;
  m_total_files_length = (uint64_t) 0;
  m_total_opened = 0;
  return 0;
}

#ifndef ERFILES
int btFiles::_btf_ftruncate(int fd,int64_t length)
{
  if( arg_allocate ){
    char *c = new char[256*1024];
    if( !c ){ errno = ENOMEM; return -1; }
    memset(c, 0, 256*1024);
    int r, wlen;
    int64_t len = 0;
    for( int i=0; len < length; i++ ){
      if( len + 256*1024 > length ) wlen = (int)(length - len);
      else wlen = 256*1024;
      if( 0 == i % 100 ) CONSOLE.Interact_n(".");
      if( (r = write(fd, c, wlen)) < 0 ) return r;
      len += wlen;
    }
    delete []c;
    return r;
  }
#if defined( WINDOWS ) || ( defined( __OS2__ ) && defined( __WATCOMC__ ) )
  char c = (char)0;
  if( lseek(fd,length - 1, SEEK_SET) < 0 ) return -1;
  return write(fd, &c, 1);
#else
  // ftruncate() not allowed on [v]fat under linux
  int retval = ftruncate(fd,length);
  if( retval < 0 ) {
    char c = (char)0;
    if( lseek(fd,length - 1, SEEK_SET) < 0 ) return -1;
    return write(fd, &c, 1);
  }
  else return retval;
#endif
}
#endif

int btFiles::_btf_recurses_directory(const char *cur_path, BTFILE* *plastnode)
{
  char full_cur[MAXPATHLEN];
  char fn[MAXPATHLEN];
  struct stat sb;
  struct dirent *dirp;
  DIR *dp;
  BTFILE *pbf;

  if( !getcwd(full_cur,MAXPATHLEN) ) return -1;

  if( cur_path ){
    strcpy(fn, full_cur);
    if( MAXPATHLEN <= snprintf(full_cur, MAXPATHLEN, "%s%c%s", fn, PATH_SP,
                               cur_path) ){
      errno = ENAMETOOLONG;
      return -1;
    }
  }

  if( (DIR*) 0 == (dp = opendir(full_cur)) ){
    CONSOLE.Warning(1, " error, open directory \"%s\" failed:  %s",
      cur_path, strerror(errno));
    return -1;
  }

  while( (struct dirent*) 0 != (dirp = readdir(dp)) ){

    if( 0 == strcmp(dirp->d_name, ".") ||
        0 == strcmp(dirp->d_name, "..") ) continue;

    if( cur_path ){
      if(MAXPATHLEN < snprintf(fn, MAXPATHLEN, "%s%c%s", cur_path, PATH_SP,
                               dirp->d_name)){
        CONSOLE.Warning(1, " error, pathname too long");
        errno = ENAMETOOLONG;
        return -1;
      }
    }else{
      strcpy(fn, dirp->d_name);
    }

    if( stat(fn, &sb) < 0 ){
      CONSOLE.Warning(1, " error, stat \"%s\" failed:  %s",fn,strerror(errno));
      return -1;
    }

    if( S_IFREG & sb.st_mode ){

      pbf = _new_bfnode();
      pbf->bf_filename = new char[strlen(fn) + 1];
      strcpy(pbf->bf_filename, fn);

//acw
#ifdef ERFILES
      long long st_size = t_file::size( fn );
      pbf->bf_length = st_size;
      m_total_files_length += st_size;
#else
      pbf->bf_length = sb.st_size;
      m_total_files_length += sb.st_size;
#endif

      if( *plastnode ) (*plastnode)->bf_next = pbf; else m_btfhead = pbf;

      *plastnode = pbf;

    }else if( S_IFDIR & sb.st_mode ){
      if(_btf_recurses_directory(fn, plastnode) < 0){closedir(dp); return -1;}
    }else{
      CONSOLE.Warning(1, " error, \"%s\" is not a directory or regular file.",
        fn);
      closedir(dp);
      return -1;
    }
  } // end while
  closedir(dp);
  return 0;
}

#if defined( __GNUC__ ) && defined( __OS2__ )
#define mkdir(x) mkdir(x,0755)
#endif

int btFiles::_btf_creat_by_path(const char *pathname, int64_t file_length)
{
  struct stat sb;
//acw  int fd;
//acw  char *p,*pnext,last = 0;
  char  *p;
  char  *pnext;
  char  last = 0;
  char sp[MAXPATHLEN];

  strcpy(sp,pathname);

  pnext = sp;
  if(PATH_SP == *pnext) pnext++;

  for(; !last; ){
    for(p = pnext; *p && PATH_SP != *p; p++) ;
    if( !*p ) last = 1;
    if(last && PATH_SP == *p){ last = 0; break;}
    *p = '\0';
    if(stat(sp,&sb) < 0){
      if( ENOENT == errno ){
        if( !last ){
#if defined( WINDOWS ) || defined( __OS2__ )
//          if(mkdir(sp) < 0) break;
          if ( !( ( strlen(sp) == 2 ) && ( sp[1]==':' ) ) ) { // ER it's drive letter
            if(mkdir(sp) < 0) break;
          }
#else
          if(mkdir(sp,0755) < 0) break;
#endif
        }else{
//acw
#ifdef ERFILES
          if ( sp[ strlen(sp)-1 ] != '\\' ) {  // if sp ends with '\' - skip
            t_file tf;
            if( tf.open(sp,O_CREAT|O_TRUNC|O_WRONLY,SH_DENYRW) < 0 )  { last = 0; break; }
            if(file_length && tf.setSize(file_length) < 0) { last = 0; break;}
          }
#else
          if((fd = creat(sp,0644)) < 0){ last = 0; break; }
          if(file_length && _btf_ftruncate(fd, file_length) < 0){
            close(fd); last = 0; break;
          }
          close(fd);
#endif
        }
      }else{last = 0; break;}
    }
    if( !last ){ *p = PATH_SP; pnext = p + 1; }
  }
  return last;
}

int btFiles::BuildFromFS(const char *pathname)
{
  struct stat sb;
  BTFILE *pbf = (BTFILE*) 0;
  BTFILE *lastnode = (BTFILE*) 0;

  if( stat(pathname, &sb) < 0 ){
    CONSOLE.Warning(1, " error, stat file \"%s\" failed:  %s",
      pathname, strerror(errno));
    return -1;
  }

  if( S_IFREG & sb.st_mode ){
    pbf = _new_bfnode();
#ifdef ERFILES
    long long st_size = t_file::size( pathname );
    pbf->bf_length = m_total_files_length = st_size;
#else
    pbf->bf_length = m_total_files_length = sb.st_size;
#endif
    pbf->bf_filename = new char[strlen(pathname) + 1];
    strcpy(pbf->bf_filename, pathname);
    m_btfhead = pbf;
  }else if( S_IFDIR & sb.st_mode ){
    char wd[MAXPATHLEN];
    if( !getcwd(wd,MAXPATHLEN) ) return -1;
    m_directory = new char[strlen(pathname) + 1];
    strcpy(m_directory, pathname);

    if(chdir(m_directory) < 0){
      CONSOLE.Warning(1, " error, change work directory to \"%s\" failed:  %s",
        m_directory, strerror(errno));
      return -1;
    }

    if(_btf_recurses_directory((const char*)0, &lastnode) < 0) return -1;
    if( chdir(wd) < 0) return -1;
  }else{
    CONSOLE.Warning(1, " error, \"%s\" is not a directory or regular file.",
      pathname);
    return -1;
  }
  return 0;
}

int btFiles::BuildFromMI(const char *metabuf, const size_t metabuf_len, const char *saveas)
{
  char path[MAXPATHLEN];
  const char *s, *p;
  size_t r,q,n;
  int64_t t;
  int f_warned = 0;

  if(!decode_query(metabuf, metabuf_len, "info|name.utf-8", &s, &q, (int64_t*)0, QUERY_STR) ||
    MAXPATHLEN <= q) {
    if(!decode_query(metabuf, metabuf_len, "info|name", &s, &q, (int64_t*)0, QUERY_STR) ||
      MAXPATHLEN <= q)
      return -1;
  }

  memcpy(path, s, q);
  path[q] = '\0';

  r = decode_query(metabuf, metabuf_len, "info|files", (const char**)0, &q,
                   (int64_t*)0, QUERY_POS);

  if( r ){  // info|files => с каталогом.
    BTFILE *pbf_last = (BTFILE*) 0;
    BTFILE *pbf = (BTFILE*) 0;
    size_t dl;
    if( decode_query(metabuf,metabuf_len,"info|length",
                    (const char**) 0,(size_t*) 0,(int64_t*) 0,QUERY_LONG) )
      return -1;

    if( saveas ){
      m_directory = new char[strlen(saveas) + 1];
      strcpy(m_directory,saveas);
    }else{
      char *tmpfn = new char[strlen(path)*2+5];
//acw
      if(!f_clUTF) {
	if(!CSC(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u'))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
      }
//an64
else
{
if(f_clenc){
if(!CSC1(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u', fenc))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
}};//
      m_directory = new char[strlen(path) + 1];
      strcpy(m_directory, path);
      delete []tmpfn;
    }

    /* now r saved the pos of files list. q saved list length */
    p = metabuf + r + 1;
    q--;
    for(; q && 'e' != *p; p += dl, q -= dl){
      dl = decode_dict(p, q, (const char*) 0);
      if(!dl) return -1;
      if( !decode_query(p, dl, "length", (const char**) 0,
                       (size_t*) 0,&t,QUERY_LONG) ) return -1;
      pbf = _new_bfnode();
      pbf->bf_length = t;
      m_total_files_length += t;
      r = decode_query(p, dl, "path.utf-8", (const char **)0, &n, (int64_t*)0,
                       QUERY_POS);
      if(!r) {
        r = decode_query(p, dl, "path", (const char **)0, &n, (int64_t*)0,
                         QUERY_POS);
	if(!r) return -1;
      }

      if(!decode_list2path(p + r, n, path)) return -1;

      char *tmpfn = new char[strlen(path)*2+5];
//acw
      if(!f_clUTF) {
        if(!CSC(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u'))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
      }
//an64
else
{
if(f_clenc){
if(!CSC1(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u', fenc))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
}};//

      pbf->bf_filename = new char[strlen(path) + 1];
      strcpy(pbf->bf_filename, path);
      delete []tmpfn;
      if(pbf_last) pbf_last->bf_next = pbf; else m_btfhead = pbf;
      pbf_last = pbf;
    }
  }else{  // ! info|files => 1 файл без каталога.
    if( !decode_query(metabuf,metabuf_len,"info|length",
                     (const char**) 0,(size_t*) 0,&t,QUERY_LONG) )
      return -1;
    m_btfhead = _new_bfnode();
    m_btfhead->bf_length = m_total_files_length = t;
    if( saveas ){
      m_btfhead->bf_filename = new char[strlen(saveas) + 1];
      strcpy(m_btfhead->bf_filename, saveas);
    }else if( arg_flg_convert_filenames ){
      char *tmpfn = new char[strlen(path)*2+5];
//acw
      if(!f_clUTF) {
        if(!CSC(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u'))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
      }
//an64
else
{
if(f_clenc){
if(!CSC1(tmpfn, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u', fenc))
	  strcpy(path, tmpfn);  // перекодировалось успешно.
}};//

      strcpy(m_btfhead->bf_filename, path);
      delete []tmpfn;
    }else{
      m_btfhead->bf_filename = new char[strlen(path) + 1];
//acw
      if(!f_clUTF) {
	if(CSC(m_btfhead->bf_filename, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u'))
	  strcpy(m_btfhead->bf_filename, path);  // перекодировалось неуспешно.
      }
//an64
else
{
if(f_clenc){
if(CSC1(m_btfhead->bf_filename, path, (size_t)strlen(path) * 2 + 5, (size_t)strlen(path), 'u' , fenc))
strcpy(m_btfhead->bf_filename, path);  // перекодировалось неуспешно.
}};//

    }
  }

//acw
  if(!pFiles) {
    BTFILE *ps = m_btfhead, *pd;
    pd = new BTFILE;
    pFiles = pd;
    for(; ps; ps = ps->bf_next) {
      *pd = *ps;
      pd = ps->bf_next;
      pd = new BTFILE;
      ++uFilesMax;
    }
    if(m_directory) {
      pDir = new char[strlen(m_directory) + 1];
      strcpy(pDir, m_directory);
    }
  }

  return 0;
}

int btFiles::CreateFiles()
{
  int check_exist = 0;
  char fn[MAXPATHLEN];
//acw
//  BTFILE *pbt = m_btfhead;
  BTFILE *pbt = pFiles;
  struct stat sb;
  int i = 0;

//acw
  ULONG  ulFileNum = 0;


  if(!pFileExist) pFileExist = new UCHAR[uFilesMax + 1];
  for(int ii = 0; ii <= uFilesMax; ii++) pFileExist[ii] = 0;

  for(; pbt; pbt = pbt->bf_next){
//acw
//    m_nfiles++;
    if(!f_Created) m_nfiles++;
    ++ulFileNum;

//    if( m_directory ){
    if(pDir){
      if( MAXPATHLEN <= snprintf(fn, MAXPATHLEN, "%s%c%s",
//          m_directory, PATH_SP, pbt->bf_filename) )
	  pDir, PATH_SP, pbt->bf_filename) ) {
        errno = ENAMETOOLONG;
	return -1;
      }
    }else{
      strcpy(fn, pbt->bf_filename);
    }

    if(stat(fn, &sb) < 0){  // файл не существует.
//acw
      if(f_clFile && pFileCreat[ulFileNum] != 1) continue;  // текущий файл не заявлен.
      pFileExist[ulFileNum] = 1;

      if(ENOENT == errno){
//        if( arg_allocate ){
        CONSOLE.Interact_n("");
//          CONSOLE.Interact_n("Creating %s", fn);
        CONSOLE.Interact_n("Creating file \"%s\"", fn);
//        }
        if( !_btf_creat_by_path(fn,pbt->bf_length)){
          CONSOLE.Warning(1, " error, create file \"%s\" failed:  %s", fn, strerror(errno));
	  return -1;
        }
      }else{
        CONSOLE.Warning(1, " error, couldn't create file \"%s\":  %s", fn,
          strerror(errno));
        return -1;
      }
//acw
      pFileExist[ulFileNum] = 1;

    }else{  // файл существует.
      if( !check_exist) check_exist = 1;
      if( !(S_IFREG & sb.st_mode) ){
        if(f_clFile && pFileCreat[ulFileNum] != 1) continue;  // текущий файл не заявлен.
        CONSOLE.Warning(1, " error, file \"%s\" is not a regular file.", fn);
        return -1;
      }
#ifdef ERFILES
      LONGLONG  llLen;
      llLen = t_file::size(fn);
      if(llLen != pbt->bf_length) {
//      if(t_file::size(fn) != pbt->bf_length){
        if(f_clFile && pFileCreat[ulFileNum] != 1) continue;  // текущий файл не заявлен.
#else
      if(sb.st_size != pbt->bf_length){
#endif
	CONSOLE.Warning(1," error, file \"%s\" size doesn't match; must be %llu",
                fn, (unsigned long long)(pbt->bf_length));
//ff
        CONSOLE.Warning(1," file will be released and recreated");
        unlink(fn);
        if( !_btf_creat_by_path(fn,pbt->bf_length)){
          CONSOLE.Warning(1, " error, create file \"%s\" failed.",fn);
          return -1;
        }
      }
    pFileExist[ulFileNum] = 1;
    }
  } //end for

//acw
if(!f_Created) {
  m_file = new BTFILE *[m_nfiles];
  if( !m_file ){
    CONSOLE.Warning(1, " error, failed to allocate memory for files list");
    return -1;
  }
  for( pbt = m_btfhead; pbt; pbt = pbt->bf_next ){
    m_file[i++] = pbt;
  }
}

  f_Created = 1;
  return check_exist;

}

void btFiles::PrintOut()
{

  BTFILE *p = m_btfhead;
  size_t id = 0;
  CONSOLE.Print("");
  CONSOLE.Print("FILES INFO");
  BitField tmpBitField, tmpFilter;
  if(m_directory) CONSOLE.Print("Directory: %s", m_directory);
  for( ; p ; p = p->bf_next ){
    ++id;
//acw
    if(f_clFile && pFileNums && !pFileNums[id]) continue;

    CONSOLE.Print_n("");
    CONSOLE.Print_n("<%d> %s%s [%llu]", (int)id, m_directory ? " " : "",
      p->bf_filename, (unsigned long long)(p->bf_length));
    if( !arg_flg_exam_only ){
      BTCONTENT.SetTmpFilter(id, &tmpFilter);
      tmpBitField = *BTCONTENT.pBF;
      tmpBitField.Except(tmpFilter);
      CONSOLE.Print_n(" %d/%d (%d%%)",
        (int)(tmpBitField.Count()), (int)(GetFilePieces(id)),
        GetFilePieces(id) ?
          (100 * tmpBitField.Count() / GetFilePieces(id)) : 100);
    }
  }
  CONSOLE.Print("Total: %lu MB",
    (unsigned long)(m_total_files_length/1024/1024));

//acw
  fFileLen = m_total_files_length / 1024.0 /1024.0;
  if(arg_flg_exam_only) {  // первый вызов.
    TitleVIO();
  }

}

size_t btFiles::FillMetaInfo(FILE* fp)
{
  BTFILE *p;
  if( m_directory ){
    // multi files
    if( bencode_str("files", fp) != 1 ) return 0;

    if( bencode_begin_list(fp) != 1) return 0;

    for( p = m_btfhead; p; p = p->bf_next){
      if( bencode_begin_dict(fp) != 1) return 0;

      if( bencode_str("length", fp) != 1 ) return 0;
      if( bencode_int(p->bf_length, fp) != 1) return 0;

      if( bencode_str("path", fp) != 1) return 0;
      if( bencode_path2list(p->bf_filename, fp) != 1 ) return 0;

      if( bencode_end_dict_list(fp) != 1) return 0;
    }

    if(bencode_end_dict_list(fp) != 1 ) return 0;

    if(bencode_str("name", fp) != 1) return 0;
//acw
//    return bencode_str(m_directory, fp);
    size_t  rc;
    char *pBuf = new char[strlen(m_directory) * 6 + 1];
    if(!(CSC(pBuf, (char*)m_directory, strlen(m_directory) * 6 + 1, strlen(m_directory), '\0')))
      rc = bencode_str(pBuf, fp);
    else rc = bencode_str(m_directory, fp);
    delete []pBuf;
    return rc;

  }else{
    if( bencode_str("length", fp) != 1 ) return 0;
    if( bencode_int(m_btfhead->bf_length, fp) != 1) return 0;

    if( bencode_str("name", fp) != 1 ) return 0;
//acw
//    return bencode_str(m_btfhead->bf_filename, fp);
    size_t  rc;
    char *pBuf = new char[strlen(m_btfhead->bf_filename) * 6 + 1];
    if(!(CSC(pBuf, (char*)m_btfhead->bf_filename, strlen(m_btfhead->bf_filename) * 6 + 1,
       strlen(m_btfhead->bf_filename), '\0')))
      rc = bencode_str(pBuf, fp);
    else rc = bencode_str(m_btfhead->bf_filename, fp);
    delete []pBuf;
    return rc;
  }
//acw  return 1;
}


void btFiles::SetFilter(int nfile, BitField *pFilter, size_t pieceLength)
{
  BTFILE *p = m_btfhead;
  size_t id = 0;
  uint64_t sizeBuffer=0;
  size_t index;

  if( nfile==0 || nfile>m_nfiles ){
    pFilter->Clear();
    return;
  }

  pFilter->SetAll();
  for( ; p ; p = p->bf_next ){
    if(++id == nfile){
      if( 0 == p->bf_length ){
        p->bf_npieces = 0;
        return;
      }
      size_t start, stop;
      start = (size_t)(sizeBuffer / pieceLength);
      stop  = (size_t)((sizeBuffer + p->bf_length) / pieceLength);
      // calculation is off if file ends on a piece boundary
      if(stop > start && 0 == (sizeBuffer + p->bf_length) % pieceLength)
        --stop;
      p->bf_npieces = stop - start + 1;
      for(index = start; index <= stop; index++) {
        pFilter->UnSet(index);
      }
      break;
    }
    sizeBuffer += p->bf_length;
  }
}

char *btFiles::GetFileName(size_t nfile) const
{
  if( nfile && nfile <= m_nfiles )
    return m_file[nfile-1]->bf_filename;
  return (char *)0;
}

uint64_t btFiles::GetFileSize(size_t nfile) const
{
  if( nfile && nfile <= m_nfiles )
    return m_file[nfile-1]->bf_length;
  return 0;
}

size_t btFiles::GetFilePieces(size_t nfile) const
{
  //returns the number of pieces in the file
  if( nfile && nfile <= m_nfiles )
    return m_file[nfile-1]->bf_npieces;
  return 0;
}

/*
int btFiles::ConvertFilename(char *dst, const char *src, int size)
{
  int retval=0, i, j, f_print=0, f_punct=0;

  for(i=j=0; src[i] != '\0' && j < size-2; i++){
    if( isprint(src[i]) ){
      if( ispunct(src[i]) ) f_punct = 1;
      else f_punct = 0;
      if(j && !f_print && !f_punct){ sprintf(dst+j, "_"); j++; }
      dst[j++] = src[i];
      f_print = 1;
    }else{
      if(f_print && !f_punct){ sprintf(dst+j, "_"); j++; }
      snprintf(dst+j, 3, "%.2X", (unsigned char)(src[i]));
      j += 2;
      f_print = f_punct = 0;
      if( !retval ) retval = 1;
    }
  }
  dst[j] = '\0';
  return retval;
}
*/

char *btFiles::GetDataName() const
{
  return m_directory ? m_directory : m_btfhead->bf_filename;
}

void btFiles::setReadOnly()  //nickk
{
	BTFILE *p;
	m_flag_readonly = 1;
	for (p = m_btfhead; p; p = p->bf_next)
	{
		if (!p->bf_fp) continue;
		p->bf_fp->reopen(O_RDONLY, SH_DENYNO);
	}
}

