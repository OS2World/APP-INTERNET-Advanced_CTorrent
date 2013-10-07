/**
 * Firecell heap implementation for debugging memory problems
 */


#include <stdarg.h>
//#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#define INCL_DOSERRORS
//#define INCL_DOSFILEMGR
//#define INCL_DOSMEMMGR

//acw
#define INCL_DOSPROCESS

#include "dbgheap.h"
#include <os2.h>

#define DHEAP_USE_HMA
#define DHEAP_AUTOCHECK_FULL
//#define DHEAP_HEAP_SIZE (1024 * 1024 * 1024)
#define DHEAP_HEAP_SIZE (100 * 1024 * 1024)
#define DHEAP_PAGE_SIZE 0x1000
#define DHEAP_FILL_CHAR '\0xE5'
#define DHEAP_MAGIC 0x562fecb4U


struct HeapObj_s;
typedef struct HeapObj_s
{
  unsigned magic;
  size_t dataLength;
  size_t gapLength; // number of free (uncommited) bytes after data. Must be at least one page
  struct HeapObj_s* prevObj;
  struct HeapObj_s* nextObj;
} HeapObj;

static HeapObj* s_heapStart;

static RamMutex s_heapMutex;

class AutoRamMutex
{
public:
  AutoRamMutex(RamMutex& mtx): m_mtx(mtx) {m_mtx.Enter();}
  ~AutoRamMutex()                         {m_mtx.Leave();}
private:
  RamMutex& m_mtx;
};

static void dlog(const char* fmt, ...)
{
  char buf[1024];
  size_t maxDataSize = sizeof(buf) - 3; // \r\n\0

  va_list args;
  va_start(args, fmt);
  int formatedSize = vsnprintf(buf, maxDataSize, fmt, args);
  va_end(args);

  if (formatedSize < 0)
  {
    strcpy(buf, "!!!dbgheap!!!: snprintf error");
    formatedSize = strlen(buf);
  }
  else if (formatedSize > maxDataSize)
  {
    formatedSize = maxDataSize;
  }

  buf[formatedSize] = '\r';
  buf[formatedSize+1] = '\n';
  buf[formatedSize+2] = '\0';

  ULONG actual;
  DosWrite(1, buf, formatedSize + 3, &actual);
}

#define PTR_ADD(ptrType, ptr, ptrInc) ((ptrType*)(((byte*)(ptr)) + ptrInc))
#define PTR_DIFF(ptr1, ptr2) (((byte*)(ptr1)) - ((byte*)(ptr2)))

#define SUPPLEMENT_SIZE(len, page) (((len) + (page) - 1) & ~((page) - 1))
#define SUPPLEMENT_SIZE_OBJ(len) (SUPPLEMENT_SIZE((len) + sizeof(HeapObj) + sizeof(void*), DHEAP_PAGE_SIZE))
#define DATA_TO_OBJPTR(dataPtr) ((HeapObj**)(((byte*)dataPtr) - sizeof(void*)))
#define DATA_TO_OBJ(dataPtr) (*DATA_TO_OBJPTR(dataPtr))
#define OBJ_TO_DATA(objPtr, objSize, dataSize) PTR_ADD(void, (objPtr), (objSize) - (dataSize))

#define CHECK_MAGIC(place1, place2, heapObjPtr)\
  if ((heapObjPtr)->magic != DHEAP_MAGIC)\
  {\
    dlog("!!!dbgheap!!!: CHECK_MAGIC in %s%s failed at %08lX", place1, place2, (unsigned long)heapObjPtr);\
    abort();\
  }

static void initHeap()
{
  ULONG allocFlags = PAG_READ | PAG_WRITE;
#if defined(DHEAP_USE_HMA)
  allocFlags |= OBJ_ANY;
#endif

  APIRET rc = DosAllocMem((void**)&s_heapStart, DHEAP_HEAP_SIZE, allocFlags);
  if (rc != NO_ERROR)
  {
    dlog("!!!dbgheap!!!: DosAllocMem rc=%lu", rc);
    return;
  }

  DosSetMem(s_heapStart, DHEAP_PAGE_SIZE, PAG_COMMIT|PAG_DEFAULT);
  s_heapStart->magic = DHEAP_MAGIC;
  s_heapStart->dataLength = 0;
  s_heapStart->gapLength = DHEAP_HEAP_SIZE - DHEAP_PAGE_SIZE;
  s_heapStart->prevObj = NULL;
  s_heapStart->nextObj = NULL;
  *DATA_TO_OBJPTR(OBJ_TO_DATA(s_heapStart, DHEAP_PAGE_SIZE, s_heapStart->dataLength)) = s_heapStart;
}

#define CHECK_INIT\
  {\
    if (!s_heapStart) {initHeap();}\
    if (!s_heapStart) {return NULL;}\
  }

// TODO: check filling of space with DHEAP_FILL_CHAR
static void fullHeapCheck(const char* place)
{
  HeapObj* prevObj = NULL;
  HeapObj* curObj = s_heapStart;
  do
  {
    CHECK_MAGIC("fullHeapCheck on ", place, curObj);

    if (curObj<s_heapStart || curObj>=PTR_ADD(HeapObj, s_heapStart, DHEAP_HEAP_SIZE))
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX is outside heap", place, (unsigned long)curObj);
      abort();
    }

    if ((unsigned long)curObj != ((unsigned long)curObj & ~(DHEAP_PAGE_SIZE - 1)))
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX has invalid address", place, (unsigned long)curObj);
      abort();
    }

    if (curObj->prevObj != prevObj)
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX has invalid prevObj", place, (unsigned long)curObj);
      abort();
    }

    size_t curGapLength = curObj->gapLength;
    if (curGapLength != (curGapLength & ~(DHEAP_PAGE_SIZE - 1)) || curGapLength < DHEAP_PAGE_SIZE)
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX has invalid gapLength", place, (unsigned long)curObj);
      abort();
    }

    HeapObj* nextObj = curObj->nextObj;
    HeapObj* nextObjAddr = (nextObj) ? nextObj : PTR_ADD(HeapObj, s_heapStart, DHEAP_HEAP_SIZE);

    if (PTR_DIFF(nextObjAddr, curObj) != SUPPLEMENT_SIZE_OBJ(curObj->dataLength) + curGapLength)
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX has nextObj inconsistent with gapLength", place, (unsigned long)curObj);
      abort();
    }

    if (DATA_TO_OBJ(OBJ_TO_DATA(curObj, SUPPLEMENT_SIZE_OBJ(curObj->dataLength), curObj->dataLength)) != curObj)
    {
      dlog("!!!dbgheap!!!: fullHeapCheck on %s. Object 0x%08lX has objPtr inconsistent with object beginning", place, (unsigned long)curObj);
      abort();
    }

    prevObj = curObj;
    curObj = nextObj;
  } while (curObj);
}

static void* unserialized_malloc(size_t reqSize)
{
  size_t newObjSize = SUPPLEMENT_SIZE_OBJ(reqSize);
  // each object must have at least one uncommited page before and after itself
  size_t newObjSizeFull = newObjSize + 2*DHEAP_PAGE_SIZE;

  HeapObj* prevObj = s_heapStart;
  while (true)
  {
    CHECK_MAGIC("malloc()", "", prevObj);
    if (prevObj->gapLength >= newObjSizeFull)
    {
      break;
    }

    prevObj = prevObj->nextObj;
    if (!prevObj)
    {
      return NULL; // no object with enough space
    }
  }

  HeapObj* newObj = PTR_ADD(HeapObj, prevObj, SUPPLEMENT_SIZE_OBJ(prevObj->dataLength) + DHEAP_PAGE_SIZE);
  APIRET rc = DosSetMem(newObj, newObjSize, PAG_COMMIT|PAG_DEFAULT);
  if (rc != NO_ERROR)
  {
    dlog("!!!dbgheap!!!: malloc() DosSetMem(COMMIT) rc=%lu", rc);
    abort();
  }

  HeapObj* nextObj = prevObj->nextObj;

  memset(newObj, DHEAP_FILL_CHAR, newObjSize);
  newObj->magic = DHEAP_MAGIC;
  newObj->dataLength = reqSize;
  newObj->gapLength = prevObj->gapLength - newObjSize - DHEAP_PAGE_SIZE;
  newObj->prevObj = prevObj;
  newObj->nextObj = nextObj;

  void* newData = OBJ_TO_DATA(newObj, newObjSize, reqSize);
  *DATA_TO_OBJPTR(newData) = newObj;

  prevObj->nextObj = newObj;
  prevObj->gapLength = DHEAP_PAGE_SIZE; // new object is always one page after previous
  if (nextObj)
  {
    nextObj->prevObj = newObj;
  }

  return newData;
}

CRT_EXPORT void* std::malloc(size_t reqSize)
{
  if (!reqSize)
  {
    return NULL;
  }

  AutoRamMutex amx(s_heapMutex);
  CHECK_INIT;

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("before malloc()");
#endif

  void* res = unserialized_malloc(reqSize);

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("after malloc()");
#endif

  return res;
}

CRT_EXPORT void* std::calloc(size_t numElems, size_t elemSize)
{
  // no alignment for now
  size_t reqSize = numElems * elemSize;
  void* ptr = malloc(reqSize);
  bzero(ptr, reqSize);
  return ptr;
}

static void unserialized_free(void* ptr, HeapObj* curObj)
{
  size_t curDataLength = curObj->dataLength;
  size_t curObjSize = SUPPLEMENT_SIZE_OBJ(curDataLength);
  if (ptr != OBJ_TO_DATA(curObj, curObjSize, curDataLength))
  {
    dlog("!!!dbgheap!!!: free(0x%08lX) invalid data address at present object", (unsigned long)ptr);
    abort();
  }

  HeapObj* prevObj = curObj->prevObj;
  HeapObj* nextObj = curObj->nextObj;
  HeapObj* nextObjAddr = (nextObj) ? nextObj : PTR_ADD(HeapObj, s_heapStart, DHEAP_HEAP_SIZE);

  size_t newGapLength = curObjSize + curObj->gapLength + prevObj->gapLength;
  if (PTR_DIFF(nextObjAddr, prevObj) != SUPPLEMENT_SIZE_OBJ(prevObj->dataLength) + newGapLength)
  {
    dlog("!!!dbgheap!!!: free(0x%08lX) new gap length is invalid", (unsigned long)ptr);
    abort();
  }

  prevObj->nextObj = nextObj;
  prevObj->gapLength = newGapLength;
  if (nextObj)
  {
    nextObj->prevObj = prevObj;
  }

  APIRET rc = DosSetMem(curObj, curObjSize, PAG_DECOMMIT);
  if (rc != NO_ERROR)
  {
    dlog("!!!dbgheap!!!: free() DosSetMem(DECOMMIT) rc=%lu", rc);
    abort();
  }
}

CRT_EXPORT void std::free(void* ptr)
{
  if (!s_heapStart)
  {
    dlog("!!!dbgheap!!!: free(0x%08lX) on not initalized heap", (unsigned long)ptr);
    abort();
  }

  if (!ptr)
  {
    return;
  }

  /* FIXME: openwatcom attempts to free some of its internal file data without
   * std:malloc them. This behaviour needs to be researched and the following
   * code should be removed */
#if defined(DHEAP_USE_HMA)
  if ((unsigned long)ptr < 512UL * 1024 * 1024)
  {
    return;
  }
#endif

  AutoRamMutex amx(s_heapMutex);

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("before free()");
#endif

  HeapObj* curObj = DATA_TO_OBJ(ptr);
  CHECK_MAGIC("free()", "", curObj);

  if (curObj<=s_heapStart || curObj>=PTR_ADD(HeapObj, s_heapStart, DHEAP_HEAP_SIZE))
  {
    dlog("!!!dbgheap!!!: free(0x%08lX) is outside heap", (unsigned long)ptr);
    abort();
  }

  unserialized_free(ptr, curObj);

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("after free()");
#endif
}

CRT_EXPORT void* std::realloc(void* oldPtr, size_t newSize)
{
  if (!oldPtr)
  {
    return malloc(newSize);
  }

  if (!newSize)
  {
    free(oldPtr);
    return NULL;
  }

  AutoRamMutex amx(s_heapMutex);
  CHECK_INIT;

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("before realloc()");
#endif

  HeapObj* oldObj = DATA_TO_OBJ(oldPtr);
  CHECK_MAGIC("realloc()", "", oldObj);

  if (oldObj<=s_heapStart || oldObj>=PTR_ADD(HeapObj, s_heapStart, DHEAP_HEAP_SIZE))
  {
    dlog("!!!dbgheap!!!: realloc(0x%08lX) is outside heap", (unsigned long)oldPtr);
    abort();
  }

  size_t oldSize = oldObj->dataLength;
  size_t oldObjSize = SUPPLEMENT_SIZE_OBJ(oldSize);
  if (oldPtr != OBJ_TO_DATA(oldObj, oldObjSize, oldSize))
  {
    dlog("!!!dbgheap!!!: realloc(0x%08lX) invalid data address at present object", (unsigned long)oldPtr);
    abort();
  }

  size_t newObjSize = SUPPLEMENT_SIZE_OBJ(newSize);
  void* newPtr;
  if (newObjSize <= oldObjSize)
  {
    newPtr = OBJ_TO_DATA(oldObj, newObjSize, newSize);
    if (newSize > oldSize)
    {
      memmove(newPtr, oldPtr, oldSize);
      memset(PTR_ADD(void, newPtr, oldSize), DHEAP_FILL_CHAR, newSize - oldSize);
    }
    else
    {
      memmove(newPtr, oldPtr, newSize);
      memset(DATA_TO_OBJPTR(oldPtr), DHEAP_FILL_CHAR, oldSize - newSize);
    }

    oldObj->dataLength = newSize;
    *DATA_TO_OBJPTR(newPtr) = oldObj;

    if (newObjSize < oldObjSize)
    {
      size_t sizeDiff = oldObjSize - newObjSize;
      APIRET rc = DosSetMem(oldObj + newObjSize, sizeDiff, PAG_DECOMMIT);
      if (rc != NO_ERROR)
      {
        dlog("!!!dbgheap!!!: realloc() DosSetMem(DECOMMIT) rc=%lu", rc);
        abort();
      }
      oldObj->gapLength += sizeDiff;
    }
  }
  else
  {
    // Well, it is possible to make more checks for expanding into the left or into the right gap
    // but there is a minimal sense to do this. It is debug heap after all :)
    newPtr = unserialized_malloc(newSize);
    memcpy(newPtr, oldPtr, oldSize);
    unserialized_free(oldPtr, oldObj);
  }

#if defined (DHEAP_AUTOCHECK_FULL)
  fullHeapCheck("after realloc()");
#endif

  return newPtr;
}


//acw

static inline byte atomic_xchgb(byte volatile * mem, byte v)
{
    _asm
    {
        mov ebx, mem
        mov al, v
        xchg byte ptr[ebx], al
        mov v, al
    }
    return v;
}

void RamMutex::Enter()
{
    while (true)
    {
        byte status = atomic_xchgb(&m_busy, 1);
        if (!status)
        {
            break;
        }
        Sleep(1);
    }
}

void RamMutex::Leave()
{
    m_busy = 0;
}

void Sleep(dword dMsec)
{
    DosSleep(dMsec);
}


