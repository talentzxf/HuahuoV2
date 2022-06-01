#include "IsPlaying.h"

/// Boolean variable that indicates whether we are in play mode (true)
/// or in editor mode (false).
/// Notice that we can be in play mode either because we're running
/// a built version of the project or because we've entered play mode inside the editor.
#if !HUAHUO_EDITOR
static bool gIsWorldPlaying = true;
#else
static bool gIsWorldPlaying = false;
#endif

bool IsWorldPlaying()
{
    // __FAKEABLE_FUNCTION__(IsWorldPlaying, ());

    return gIsWorldPlaying;
}

void SetIsWorldPlaying(bool isPlaying)
{
    gIsWorldPlaying = isPlaying;
}
