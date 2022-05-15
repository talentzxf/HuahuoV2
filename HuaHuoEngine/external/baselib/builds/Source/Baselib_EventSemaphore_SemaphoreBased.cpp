#include "Include/Baselib.h"

#if !PLATFORM_FUTEX_NATIVE_SUPPORT

#include "Include/C/Baselib_EventSemaphore.h"

static bool Detail_Baselib_EventSemaphore_SemaphoreBased_OnWakeup(Baselib_EventSemaphore* semaphore, bool successfulAcquire)
{
    int32_t decreasedNumWaitingForSet, decreasedNumWaitingForSetAndStateFlags;

    // Decrease number of waiting threads.
    {
        // This is called when woken up, so need to reload state.
        int32_t numWaitingForSetAndStateFlags = Baselib_atomic_load_32_relaxed(&semaphore->state.parts.numWaitingForSetAndStateFlags);

        do
        {
            decreasedNumWaitingForSet = Detail_Baselib_EventSemaphore_GetWaitingForSetCount(numWaitingForSetAndStateFlags) - 1;
            BaselibAssert(decreasedNumWaitingForSet >= 0, "There needs to be always a non-negative amount of threads waiting for Set");
            // Both the Set flag and the SetInProgress flag may have changed by now, so need to take them into account!
            decreasedNumWaitingForSetAndStateFlags = Detail_Baselib_EventSemaphore_SetWaitingForSetCount(numWaitingForSetAndStateFlags, decreasedNumWaitingForSet);
        }
        while (!Baselib_atomic_compare_exchange_weak_32_relaxed_relaxed(
            &semaphore->state.parts.numWaitingForSetAndStateFlags,
            &numWaitingForSetAndStateFlags,
            decreasedNumWaitingForSetAndStateFlags));
    }

    // Things that are known at this point:
    // * semaphore->setInProgressSemaphore is not signaled and will not if we don't progress here
    // * Any state except a pure Set state is possible
    BaselibAssert((decreasedNumWaitingForSetAndStateFlags & (~Detail_Baselib_EventSemaphore_NumWaitingForSetMask)) != Detail_Baselib_EventSemaphore_SetFlag,
        "After a wakeup, EventSemaphore should never be in a pure Set state (since either a Set is in progress or wakeup did not occur at all)");
    // For timeout this means:
    // * (Set | SetInProgress) or SetInProgress: setSemaphore was released
    //      -> We need to consume an extra token from setSemaphore!
    // * if 0: We came in in a reset state and neither Set nor ResetAndRelease was called
    //      -> setSemaphore was not released!

    // Are we racing a Set or ResetAndRelease?
    if (!successfulAcquire && Detail_Baselib_EventSemaphore_IsSetInProgress(decreasedNumWaitingForSetAndStateFlags))
    {
        // We *know* there is a token available (or Set is providing it shortly)
        // Needs to happen before Detail_Baselib_EventSemaphore_SetInProgressFlag is cleared!
        // (Conveniently, Baselib_SystemSemaphore_Acquire is also an acquire barrier so it can't be reordered after that)
        Baselib_SystemSemaphore_Acquire(semaphore->setSemaphore);
        successfulAcquire = true;
    }

    // If we're the last ones to do so, free all threads waiting on setInProgressSemaphore.
    if (decreasedNumWaitingForSet == 0)
    {
        Detail_Baselib_EventSemaphore_State previousState, clearedSetInProgressState;
        Baselib_atomic_load_64_relaxed_v(&semaphore->state, &previousState); // Reload full state since it's likely outdated.
        do
        {
            clearedSetInProgressState.parts.numWaitingForSetAndStateFlags = previousState.parts.numWaitingForSetAndStateFlags & (~Detail_Baselib_EventSemaphore_SetInProgressFlag);
            clearedSetInProgressState.parts.numWaitingForSetInProgress = 0;
        }
        while (!Baselib_atomic_compare_exchange_weak_64_relaxed_relaxed(&semaphore->state.stateInt64, &previousState.stateInt64, clearedSetInProgressState.stateInt64));

        if (previousState.parts.numWaitingForSetInProgress > 0) // Can be negative in rare cases when timed out in acquire.
        {
            Baselib_SystemSemaphore_Release(semaphore->setInProgressSemaphore, previousState.parts.numWaitingForSetInProgress);
        }
    }

    return successfulAcquire;
}

BASELIB_C_INTERFACE
{
    void Detail_Baselib_EventSemaphore_SemaphoreBased_AcquireNonSet(const int32_t initialNumWaitingForSetAndStateFlags, Baselib_EventSemaphore* semaphore)
    {
        BaselibAssert(!Detail_Baselib_EventSemaphore_IsSet(initialNumWaitingForSetAndStateFlags));

        Detail_Baselib_EventSemaphore_State state;
        state.parts.numWaitingForSetAndStateFlags = initialNumWaitingForSetAndStateFlags;
        state.parts.numWaitingForSetInProgress = 0; // Not needed most of the time. Assume 0, if it is wrong, then we'll reload it in cmpex!
        do
        {
            if (Detail_Baselib_EventSemaphore_IsSetInProgress(state.parts.numWaitingForSetAndStateFlags))
            {
                Detail_Baselib_EventSemaphore_State newState = state;
                ++newState.parts.numWaitingForSetInProgress;

                if (Baselib_atomic_compare_exchange_weak_64_relaxed_relaxed_v(&semaphore->state, &state, &newState))
                    Baselib_SystemSemaphore_Acquire(semaphore->setInProgressSemaphore);
            }
            else
            {
                // Try to add ourselves as waiters before proceeding to wait.
                const int32_t increasedNumWaitingForSet = Detail_Baselib_EventSemaphore_GetWaitingForSetCount(state.parts.numWaitingForSetAndStateFlags) + 1;
                const int32_t increasedNumWaitingForSetAndStateFlags = increasedNumWaitingForSet; // If state is still what we think it is, then both the Set and the SetInProgress flags are not set!
                if (Baselib_atomic_compare_exchange_weak_32_relaxed_relaxed(
                    &semaphore->state.parts.numWaitingForSetAndStateFlags,
                    &state.parts.numWaitingForSetAndStateFlags,
                    increasedNumWaitingForSetAndStateFlags))
                {
                    Baselib_SystemSemaphore_Acquire(semaphore->setSemaphore);
                    Detail_Baselib_EventSemaphore_SemaphoreBased_OnWakeup(semaphore, true);
                    return;
                }
            }
        }
        while (!Detail_Baselib_EventSemaphore_IsSet(state.parts.numWaitingForSetAndStateFlags));
    }

    COMPILER_WARN_UNUSED_RESULT
    bool Detail_Baselib_EventSemaphore_SemaphoreBased_TryTimedAcquireNonSet(const int32_t initialNumWaitingForSetAndStateFlags, Baselib_EventSemaphore* semaphore, const uint32_t timeoutInMilliseconds)
    {
        BaselibAssert(!Detail_Baselib_EventSemaphore_IsSet(initialNumWaitingForSetAndStateFlags));

        Detail_Baselib_EventSemaphore_State state;
        state.parts.numWaitingForSetAndStateFlags = initialNumWaitingForSetAndStateFlags;
        state.parts.numWaitingForSetInProgress = 0; // Not needed most of the time. Assume 0, if it is wrong, then we'll reload it in cmpex!

        do
        {
            if (Detail_Baselib_EventSemaphore_IsSetInProgress(state.parts.numWaitingForSetAndStateFlags))
            {
                Detail_Baselib_EventSemaphore_State newState = state;
                ++newState.parts.numWaitingForSetInProgress;

                if (Baselib_atomic_compare_exchange_weak_64_relaxed_relaxed_v(&semaphore->state, &state, &newState))
                {
                    if (!Baselib_SystemSemaphore_TryTimedAcquire(semaphore->setInProgressSemaphore, timeoutInMilliseconds))
                    {
                        // Leaving a token on semaphore->setInProgressSemaphore is not problematic but may result in extra spins.
                        // By subtracting, we ensure that eventually we stop the extra spins (it does not entirely prevent it though!)
                        Baselib_atomic_fetch_add_32_relaxed(&semaphore->state.parts.numWaitingForSetInProgress, -1);
                    }
                }
            }
            else
            {
                // Try to add ourselves as waiters before proceeding to wait.
                const int32_t increasedNumWaitingForSet = Detail_Baselib_EventSemaphore_GetWaitingForSetCount(state.parts.numWaitingForSetAndStateFlags) + 1;
                const int32_t increasedNumWaitingForSetAndStateFlags = increasedNumWaitingForSet; // If state is still what we think it is, then both the Set and the SetInProgress flags are not set!
                if (Baselib_atomic_compare_exchange_weak_32_relaxed_relaxed(
                    &semaphore->state.parts.numWaitingForSetAndStateFlags,
                    &state.parts.numWaitingForSetAndStateFlags,
                    increasedNumWaitingForSetAndStateFlags))
                {
                    bool successfulAcquire = Baselib_SystemSemaphore_TryTimedAcquire(semaphore->setSemaphore, timeoutInMilliseconds);
                    Detail_Baselib_EventSemaphore_SemaphoreBased_OnWakeup(semaphore, successfulAcquire);
                    return successfulAcquire;
                }
            }
        }
        while (!Detail_Baselib_EventSemaphore_IsSet(state.parts.numWaitingForSetAndStateFlags));

        return true;
    }
}

#endif // !PLATFORM_FUTEX_NATIVE_SUPPORT
