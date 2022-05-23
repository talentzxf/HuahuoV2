/*!
 *
 * Umbra
 * -----------------------------------------
 *
 * (C) 2011 Umbra Software Ltd.
 * All Rights Reserved.
 *
 * This file consists of unpublished, proprietary source code of
 * Umbra Software Ltd., and is considered Confidential Information for
 * purposes of non-disclosure agreement. Disclosure outside the terms
 * outlined in signed agreement may result in irrepairable harm to
 * Umbra Software Ltd. and legal action against the party in breach.
 *
 * \file
 * \brief   Umbra Runtime Info getter wrappers.
 *
 */

#include "umbraPrivateVersion.hpp"

namespace Umbra
{

const char* getRuntimeInfoString(InfoString info)
{
    return getAPIInfoString(info);
}

int getRuntimeInfoValue(InfoValue info)
{
    return getAPIInfoValue(info);
}

} // namespace Umbra
