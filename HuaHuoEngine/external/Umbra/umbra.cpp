#define UMBRA_FORCE_RELEASE_SPURS_JOB 1
#define UMBRA_DISABLE_DEFAULT_ALLOCATOR_CHECKS 1
#define UMBRA_COMP_NO_EXCEPTIONS 1		// old define (deprecated) to disable execeptions - unconditionally undefined in umbraPrivateDefs.hpp in version 3.3.16
#define UMBRA_SUPPORT_LEGACY_DATA 1
#define UMBRA_EXCEPTIONS_SUPPORTED 0
#define _HAS_EXCEPTIONS 0
#include "source/common/orbis/umbraOsOrbis.cpp"
#include "source/common/posix/umbraOsPosix.cpp"
#include "source/common/posix/umbraProcessPosix.cpp"
#include "source/common/windows/umbraOsWin32.cpp"
#include "source/common/windows/umbraProcessWin32.cpp"
#include "source/standard/Memory.cpp"
#include "source/standard/Predicates.cpp"
#include "source/standard/Sampling.cpp"
#include "source/common/umbraAABB.cpp"
#include "source/common/umbraBinStream.cpp"
#include "source/common/umbraBitMath.cpp"
#include "source/common/umbraBitOps.cpp"
#include "source/common/umbraChecksum.cpp"
#include "source/common/umbraCmdlineApp.cpp"
#include "source/common/umbraCRCStream.cpp"
#include "source/common/umbraDouble.cpp"
#include "source/common/umbraDPVSShared.cpp"
#include "source/common/umbraFile_ansic.cpp"
#include "source/common/umbraFileStream.cpp"
#include "source/common/umbraFloat.cpp"
#include "source/common/umbraIntersect.cpp"
#include "source/common/umbraHashGenerator.cpp"
#include "source/common/umbraLogger.cpp"
#include "source/common/umbraMatrix.cpp"
#include "source/common/umbraMemory.cpp"
#include "source/common/umbraPatcher.cpp"
#include "source/common/umbraPoolAllocator.cpp"
#include "source/common/umbraPrivateVersion.cpp"
#include "source/common/umbraPropertyFile.cpp"
#include "source/common/umbraRandom.cpp"
#include "source/common/umbraSerializer.cpp"
#include "source/common/umbraStatsHeap.cpp"
#include "source/common/umbraString.cpp"
#include "source/common/umbraThread.cpp"
#include "source/runtime/umbraBSPTree.cpp"
#include "source/runtime/umbraConnectivity.cpp"
#include "source/runtime/umbraCubemap.cpp"
#include "source/runtime/umbraDPVS.cpp"
#include "source/runtime/umbraPortalCull.cpp"
#include "source/runtime/umbraPortalRaster.cpp"
#include "source/runtime/umbraPortalRayTracer.cpp"
#include "source/runtime/umbraQueryApi.cpp"
#include "source/runtime/umbraQueryArgs.cpp"
#include "source/runtime/umbraQueryContext.cpp"
#include "source/runtime/umbraRuntimeInfoString.cpp"
#include "source/runtime/umbraRuntimeTomeGenerator.cpp"
#include "source/runtime/umbraShadows.cpp"
#include "source/runtime/umbraTome.cpp"
#include "source/runtime/umbraTomeApi.cpp"
#include "source/runtime/umbraTomeCollection.cpp"
#include "source/runtime/umbraTransformer.cpp"
