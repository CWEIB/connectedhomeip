/*
 *
 *    Copyright (c) 2021 Google LLC.
 *    All rights reserved.
 *
 *    Licensed under the Apache License, Version 2.0 (the "License");
 *    you may not use this file except in compliance with the License.
 *    You may obtain a copy of the License at
 *
 *        http://www.apache.org/licenses/LICENSE-2.0
 *
 *    Unless required by applicable law or agreed to in writing, software
 *    distributed under the License is distributed on an "AS IS" BASIS,
 *    WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *    See the License for the specific language governing permissions and
 *    limitations under the License.
 */

// ================================================================================
// Main Code
// ================================================================================

#include "openthread/platform/logging.h"
#include <mbedtls/platform.h>
#include <openthread-system.h>
#include <openthread/cli.h>
#include <openthread/error.h>
#include <openthread/heap.h>

#include <lib/core/CHIPError.h>
#include <lib/support/CHIPMem.h>
#include <lib/support/CHIPPlatformMemory.h>
#include <lib/support/logging/CHIPLogging.h>
#include <platform/CHIPDeviceLayer.h>
#include <platform/ThreadStackManager.h>

#include "FreeRtosHooks.h"
#include "app_config.h"

#include "radio.h"

using namespace ::chip;
using namespace ::chip::Inet;
using namespace ::chip::DeviceLayer;
using namespace ::chip::Logging;

#include <AppTask.h>

typedef void (*InitFunc)(void);
extern InitFunc __init_array_start;
extern InitFunc __init_array_end;

/* needed for FreeRtos Heap 4 */
uint8_t __attribute__((section(".heap"))) ucHeap[HEAP_SIZE];

extern "C" void main_task(void const * argument)
{
    /* Call C++ constructors */
    InitFunc * pFunc = &__init_array_start;
    for (; pFunc < &__init_array_end; ++pFunc)
    {
        (*pFunc)();
    }

    mbedtls_platform_set_calloc_free(CHIPPlatformMemoryCalloc, CHIPPlatformMemoryFree);

    /* Used for HW initializations */
    otSysInit(0, NULL);

    K32W_LOG("Welcome to NXP Lighting Demo App");

    /* Mbedtls Threading support is needed because both
     * Thread and Weave tasks are using it */
    freertos_mbedtls_mutex_init();

    // Init Chip memory management before the stack
    chip::Platform::MemoryInit();

    CHIP_ERROR ret = PlatformMgr().InitChipStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().InitWeaveStack()");
        goto exit;
    }

    ret = ThreadStackMgr().InitThreadStack();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgr().InitThreadStack()");
        goto exit;
    }

    ret = ConnectivityMgr().SetThreadDeviceType(ConnectivityManager::kThreadDeviceType_MinimalEndDevice);
    if (ret != CHIP_NO_ERROR)
    {
        goto exit;
    }

    ret = PlatformMgr().StartEventLoopTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during PlatformMgr().StartEventLoopTask();");
        goto exit;
    }

    // Start OpenThread task
    ret = ThreadStackMgrImpl().StartThreadTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during ThreadStackMgrImpl().StartThreadTask()");
        goto exit;
    }

    ret = GetAppTask().StartAppTask();
    if (ret != CHIP_NO_ERROR)
    {
        K32W_LOG("Error during GetAppTask().StartAppTask()");
        goto exit;
    }

    GetAppTask().AppTaskMain(NULL);

exit:
    return;
}

extern "C" void otSysEventSignalPending(void)
{
    {
        BaseType_t yieldRequired = ThreadStackMgrImpl().SignalThreadActivityPendingFromISR();
        portYIELD_FROM_ISR(yieldRequired);
    }
}
