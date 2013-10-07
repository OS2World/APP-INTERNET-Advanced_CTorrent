#include "csc.h"
#include "my_iconv.h"

int CSC(char *pBufOut, char *pBufIn, size_t BufOutLen, size_t BufInLen, char cFrom) {

  iconv_t  cd;
  size_t  rc;
  char  *pOut, *pCPF, *pCPT;
  const char  *pIn;

  pIn = pBufIn;
  pOut = pBufOut;

  if(cFrom == 'u') {pCPF = "utf-8"; pCPT = "";}
  else {pCPF = ""; pCPT = "utf-8";}

  if((cd = iconv_open(pCPT, pCPF)) == (iconv_t)-1) return 1;
  rc = iconv(cd, &pIn, &BufInLen, &pOut, &BufOutLen);
  iconv_close(cd);
  *pOut = '\0';
  if(rc) return 1;
  return 0;

}

