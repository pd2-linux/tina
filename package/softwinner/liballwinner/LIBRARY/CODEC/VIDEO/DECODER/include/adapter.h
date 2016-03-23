
#ifndef ADAPTER_H
#define ADAPTER_H

#include "ve.h"
#include "memoryAdapter.h"
#include "secureMemoryAdapter.h"

int   AdapterInitialize(int bIsSecureVideoFlag);

void  AdpaterRelease(int bIsSecureVideoFlag);

int   AdapterLockVideoEngine(int flag);

void  AdapterUnLockVideoEngine(int flag);

void* AdapterMemPalloc(int nSize);

void  AdapterMemPfree(void* pMem);

void  AdapterMemFlushCache(void* pMem, int nSize);

void* AdapterMemGetPhysicAddress(void* pVirtualAddress);

void* AdapterMemGetVirtualAddress(void* pPhysicAddress);

void  AdapterVeReset(void);

int   AdapterVeWaitInterrupt(void);

void* AdapterVeGetBaseAddress(void);

int   AdapterMemGetDramType(void);

void AdapterMemSet(void* pMem, int nValue, int nSize);

void AdapterMemCopy(void* pMemDst, void* pMemSrc, int nSize);

int AdapterMemRead(void* pMemSrc, void* pMemDst, int nSize);

int AdapterMemWrite(void* pMemSrc, void* pMemDst, int nSize);

void* AdapterSecureMemAdapterAlloc(int size);

void AdapterSecureMemAdapterFree(void* ptr);

void AdapterSecureMemAdapterCopy(void *dest, void *src, int n);

void AdapterSecureMemAdapterFlushCache(void *ptr, int size);

int AdapterSecureMemAdapterRead(void *src, void *dest, int n);

int AdapterSecureMemAdapterWrite(void *src, void *dest, int n);

void AdapterSecureMemAdapterSet(void *s, int c, int n);

void * AdapterSecureMemAdapterGetPhysicAddress(void *virt);

void * AdapterSecureMemAdapterGetVirtualAddress(void *phy);

void* AdapterSecureMemAdapterGetPhysicAddressCpu(void *virt);

void* AdapterSecureMemAdapterGetVirtualAddressCpu(void *phy);
void AdapterMemThresh(int nMemoryThreshold);

int AdapterSetupSecureHardware();

int AdapterShutdownSecureHardware();

#endif

