//
// Created by VincentZhang on 4/21/2022.
//

#include "StreamedBinaryWrite.h"

CachedWriter& StreamedBinaryWrite::Init(const CachedWriter& cachedWriter, TransferInstructionFlags flags) //, BuildTargetSelection target,
                                        // const BuildUsageTag& buildUsageTag, const GlobalBuildData& globalBuildData)
{
    m_Flags = flags;
    // m_Target = target;
    m_Cache = cachedWriter;
    m_UserData = NULL;

#if UNITY_EDITOR
    m_BuildUsageTag = &buildUsageTag;
    m_GlobalBuildData = &globalBuildData;
#endif

#if UNITY_EDITOR && CHECK_SERIALIZE_ALIGNMENT
    m_Cache.m_CheckSerializeAlignment = true;
#endif
    return m_Cache;
}

CachedWriter& StreamedBinaryWrite::Init(TransferInstructionFlags flags)//, BuildTargetSelection target, void * manageReferenceToReuse)
{
    m_Flags = flags;
    m_UserData = NULL;
//    m_Target = target;
//    m_ReferenceFromIDCache = manageReferenceToReuse;

#if UNITY_EDITOR && CHECK_SERIALIZE_ALIGNMENT
    m_Cache.m_CheckSerializeAlignment = true;
#endif
    return m_Cache;
}