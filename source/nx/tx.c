#include "tx.h"

static Service g_txSrv;
static u64 g_refCnt;

Result txInitialize(void) {
    atomicIncrement64(&g_refCnt);

    if (serviceIsActive(&g_txSrv))
        return 0;

    Result rc = smGetService(&g_txSrv, "tx");
    if (R_FAILED(rc))
        return rc;

    if (R_FAILED(rc))
        txExit();

    return rc;
}

void txExit(void) {
    if (atomicDecrement64(&g_refCnt) == 0)
    {
        serviceClose(&g_txSrv);
    }
}

