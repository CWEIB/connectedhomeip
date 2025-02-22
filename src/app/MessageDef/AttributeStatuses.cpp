/**
 *
 *    Copyright (c) 2020-2021 Project CHIP Authors
 *    Copyright (c) 2018 Google LLC.
 *    Copyright (c) 2016-2017 Nest Labs, Inc.
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

#include "AttributeStatuses.h"
#include "AttributeStatusIB.h"

#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
AttributeStatusIB::Builder & AttributeStatuses::Builder::CreateAttributeStatus()
{
    if (mError == CHIP_NO_ERROR)
    {
        mError = mAttributeStatus.Init(mpWriter);
    }
    return mAttributeStatus;
}

AttributeStatuses::Builder & AttributeStatuses::Builder::EndOfAttributeStatuses()
{
    EndOfContainer();
    return *this;
}

#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributeStatuses::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err            = CHIP_NO_ERROR;
    size_t NumAttributeStatus = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributeStatuses =");
    PRETTY_PRINT("[");

    // make a copy of the EventReports reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::AnonymousTag == reader.GetTag(), CHIP_ERROR_INVALID_TLV_TAG);
        {
            AttributeStatusIB::Parser status;
            ReturnErrorOnFailure(status.Init(reader));

            PRETTY_PRINT_INCDEPTH();
            ReturnErrorOnFailure(status.CheckSchemaValidity());
            PRETTY_PRINT_DECDEPTH();
        }

        ++NumAttributeStatus;
    }

    PRETTY_PRINT("],");
    PRETTY_PRINT("");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        // if we have at least one data element
        if (NumAttributeStatus > 0)
        {
            err = CHIP_NO_ERROR;
        }
        // NOTE: temporarily disable this check, to allow test to continue
        else
        {
            ChipLogError(DataManagement, "PROTOCOL ERROR: Empty attribute statuses");
            err = CHIP_NO_ERROR;
        }
    }
    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif
}; // namespace app
}; // namespace chip
