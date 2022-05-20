//
// Created by VincentZhang on 5/19/2022.
//

#include "BlockRangeJob.h"
#include "Logging/LogAssert.h"

int ConfigureBlockRanges(BlockRange* blockRanges, int totalIndices, int blockRangeCount)
{
    Assert(blockRangeCount != 0);
    Assert(totalIndices != 0);

    //  Ensure remainder in final job is equal to or less than preceding jobs
    size_t indicesPerJob = (totalIndices + (blockRangeCount - 1)) / blockRangeCount;
    //  And adjust job count, if required, following remainder management
    size_t revisedNumJobs = (totalIndices + (indicesPerJob - 1)) / indicesPerJob;
    Assert(revisedNumJobs <= blockRangeCount);

    for (size_t i = 0; i < revisedNumJobs; i++)
    {
        size_t startIndex = indicesPerJob * i;
        blockRanges[i].startIndex = startIndex;

        // Normal jobs use indicesPerJob
        if (i != revisedNumJobs - 1)
        {
            blockRanges[i].rangeSize = indicesPerJob;
        }
            // Last job uses the rest
        else
        {
            size_t rangeSize = totalIndices - startIndex;
            blockRanges[i].rangeSize = rangeSize;
            //  Ensure final job picks up a true remainder to avoid over-indexing
            //  Final job is probably/usually last to start executing (when in a queue) so this matters
            Assert(rangeSize <= indicesPerJob);
        }
        blockRanges[i].rangesTotal = revisedNumJobs;
    }

    return revisedNumJobs;
}


