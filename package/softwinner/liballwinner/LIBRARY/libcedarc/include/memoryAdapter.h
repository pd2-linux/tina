
#ifndef MEMORY_ADAPTER_H
#define MEMORY_ADAPTER_H

#ifdef __cplusplus
extern "C" {
#endif

//* get current ddr frequency, if it is too slow, we will cut some spec off.
int MemAdapterGetDramFreq();

struct ScMemOpsS* MemAdapterGetOpsS();
struct ScMemOpsS* SecureMemAdapterGetOpsS();

#ifdef __cplusplus
}
#endif

#endif /* MEMORY_ADAPTER_H */
