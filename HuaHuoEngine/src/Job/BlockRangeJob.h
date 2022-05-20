//
// Created by VincentZhang on 5/18/2022.
//

#ifndef HUAHUOENGINE_BLOCKRANGEJOB_H
#define HUAHUOENGINE_BLOCKRANGEJOB_H

// Any function that receives a BlockRange* for storing outputs is expecting an array of size kMaximumBlockRangeCount
enum { kMaximumBlockRangeCount = 16 };
struct BlockRange
{
    // The index where we should start processing
    size_t                startIndex;

    // The amount of objects that should be processed by this job.
    // Sometimes jobs also write into this as an output,
    // which is then used by the combine job to combine the outputs.
    size_t                rangeSize;

    // The total amount of jobs scheduled
    size_t                rangesTotal;

    BlockRange() {}
    BlockRange(size_t _startIndex, size_t _rangeSize) : startIndex(_startIndex), rangeSize(_rangeSize) {}
};

int ConfigureBlockRanges(BlockRange* blockRanges, int arrayLength, int blockRangeCount);

#endif //HUAHUOENGINE_BLOCKRANGEJOB_H
