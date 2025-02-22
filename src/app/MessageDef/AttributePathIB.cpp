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

#include "AttributePathIB.h"
#include "MessageDefHelper.h"

#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>

#include <app/AppBuildConfig.h>

namespace chip {
namespace app {
#if CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK
CHIP_ERROR AttributePathIB::Parser::CheckSchemaValidity() const
{
    CHIP_ERROR err      = CHIP_NO_ERROR;
    int TagPresenceMask = 0;
    TLV::TLVReader reader;

    PRETTY_PRINT("AttributePathIB =");
    PRETTY_PRINT("{");

    // make a copy of the Path reader
    reader.Init(mReader);

    while (CHIP_NO_ERROR == (err = reader.Next()))
    {
        VerifyOrReturnError(TLV::IsContextTag(reader.GetTag()), CHIP_ERROR_INVALID_TLV_TAG);
        uint32_t tagNum = TLV::TagNumFromTag(reader.GetTag());
        switch (tagNum)
        {
        case to_underlying(Tag::kEnableTagCompression):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEnableTagCompression))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEnableTagCompression));
#if CHIP_DETAIL_LOGGING
            {
                bool enableTagCompression;
                ReturnErrorOnFailure(reader.Get(enableTagCompression));
                PRETTY_PRINT("\tenableTagCompression = %s, ", enableTagCompression ? "true" : "false");
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kNode):
            // check if this tag has appeared before

            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kNode))), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kNode));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                NodeId node;
                reader.Get(node);
                PRETTY_PRINT("\tNode = 0x%" PRIx64 ",", node);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kEndpoint):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kEndpoint))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kEndpoint));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                EndpointId endpoint;
                reader.Get(endpoint);
                PRETTY_PRINT("\tEndpoint = 0x%" PRIx16 ",", endpoint);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kCluster):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kCluster))), err = CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kCluster));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), err = CHIP_ERROR_WRONG_TLV_TYPE);

#if CHIP_DETAIL_LOGGING
            {
                ClusterId cluster;
                ReturnErrorOnFailure(reader.Get(cluster));
                PRETTY_PRINT("\tCluster = 0x%" PRIx32 ",", cluster);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kAttribute):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kAttribute))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kAttribute));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            {
                AttributeId attribute;
                ReturnErrorOnFailure(reader.Get(attribute));
                PRETTY_PRINT("\tAttribute = " ChipLogFormatMEI ",", ChipLogValueMEI(attribute));
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        case to_underlying(Tag::kListIndex):
            // check if this tag has appeared before
            VerifyOrReturnError(!(TagPresenceMask & (1 << to_underlying(Tag::kListIndex))), CHIP_ERROR_INVALID_TLV_TAG);
            TagPresenceMask |= (1 << to_underlying(Tag::kListIndex));
            VerifyOrReturnError(TLV::kTLVType_UnsignedInteger == reader.GetType(), CHIP_ERROR_WRONG_TLV_TYPE);
#if CHIP_DETAIL_LOGGING
            if (TLV::kTLVType_UnsignedInteger == reader.GetType())
            {
                uint16_t listIndex;
                ReturnErrorOnFailure(reader.Get(listIndex));
                PRETTY_PRINT("\tListIndex = 0x%" PRIx16 ",", listIndex);
            }
#endif // CHIP_DETAIL_LOGGING
            break;
        default:
            PRETTY_PRINT("Unknown tag num %" PRIu32, tagNum);
            break;
        }
    }

    PRETTY_PRINT("}");
    PRETTY_PRINT("\t");
    // if we have exhausted this container
    if (CHIP_END_OF_TLV == err)
    {
        if ((TagPresenceMask & (1 << to_underlying(Tag::kAttribute))) == 0 &&
            (TagPresenceMask & (1 << to_underlying(Tag::kListIndex))) != 0)
        {
            err = CHIP_ERROR_IM_MALFORMED_ATTRIBUTE_PATH;
        }
        else
        {
            err = CHIP_NO_ERROR;
        }
    }

    ReturnErrorOnFailure(err);
    ReturnErrorOnFailure(reader.ExitContainer(mOuterContainerType));
    return CHIP_NO_ERROR;
}
#endif // CHIP_CONFIG_IM_ENABLE_SCHEMA_CHECK

CHIP_ERROR AttributePathIB::Parser::GetEnableTagCompression(bool * const apEnableTagCompression) const
{
    return GetSimpleValue(to_underlying(Tag::kEnableTagCompression), TLV::kTLVType_Boolean, apEnableTagCompression);
}

CHIP_ERROR AttributePathIB::Parser::GetNode(NodeId * const apNode) const
{
    return GetUnsignedInteger(to_underlying(Tag::kNode), apNode);
}

CHIP_ERROR AttributePathIB::Parser::GetEndpoint(EndpointId * const apEndpoint) const
{
    return GetUnsignedInteger(to_underlying(Tag::kEndpoint), apEndpoint);
}

CHIP_ERROR AttributePathIB::Parser::GetCluster(ClusterId * const apCluster) const
{
    return GetUnsignedInteger(to_underlying(Tag::kCluster), apCluster);
}

CHIP_ERROR AttributePathIB::Parser::GetAttribute(AttributeId * const apAttribute) const
{
    return GetUnsignedInteger(to_underlying(Tag::kAttribute), apAttribute);
}

CHIP_ERROR AttributePathIB::Parser::GetListIndex(ListIndex * const apListIndex) const
{
    return GetUnsignedInteger(to_underlying(Tag::kListIndex), apListIndex);
}

AttributePathIB::Builder & AttributePathIB::Builder::EnableTagCompression(const bool aEnableTagCompression)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->PutBoolean(TLV::ContextTag(to_underlying(Tag::kEnableTagCompression)), aEnableTagCompression);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Node(const NodeId aNode)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kNode)), aNode);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Endpoint(const EndpointId aEndpoint)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kEndpoint)), aEndpoint);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Cluster(const ClusterId aCluster)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kCluster)), aCluster);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::Attribute(const AttributeId aAttribute)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kAttribute)), aAttribute);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::ListIndex(const chip::ListIndex aListIndex)
{
    // skip if error has already been set
    if (mError == CHIP_NO_ERROR)
    {
        mError = mpWriter->Put(TLV::ContextTag(to_underlying(Tag::kListIndex)), aListIndex);
    }
    return *this;
}

AttributePathIB::Builder & AttributePathIB::Builder::EndOfAttributePathIB()
{
    EndOfContainer();
    return *this;
}
}; // namespace app
}; // namespace chip
