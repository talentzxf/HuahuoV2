#include "ManagerContext.h"
#include "TypeSystem/Object.h"
#include "BaseClasses/GameManager.h"
//#include "Runtime/Graphics/EmulatedJobs.h"
//#include "Runtime/Testing/Faking.h"

ManagerContext::ManagerContext()
{
    for (int i = 0; i < kManagerCount; i++)
        m_Managers[i] = NULL;
}

void ManagerContext::InitializeClasses()
{
    for (int i = 0; i < kManagerCount; i++)
    {
        m_ManagerTypes[i] = NULL;
        #if DEBUGMODE
        m_ManagerNames[i] = "Not initialized";
        #endif
    }

#if DEBUGMODE
    #define INIT_MANAGER_CLASS(x) Assert(m_ManagerTypes[k##x] == NULL); m_ManagerTypes[k##x] = Unity::Type::FindTypeByName(#x); m_ManagerNames[k##x] = #x;
#else
    #define INIT_MANAGER_CLASS(x) m_ManagerTypes[k##x] = HuaHuo::Type::FindTypeByName(#x);
#endif

    INIT_MANAGER_CLASS(PlayerSettings)
    INIT_MANAGER_CLASS(InputManager)
    INIT_MANAGER_CLASS(TagManager)
    INIT_MANAGER_CLASS(AudioManager)
    INIT_MANAGER_CLASS(ScriptMapper)
    INIT_MANAGER_CLASS(MonoManager)
    INIT_MANAGER_CLASS(GraphicsSettings)
    INIT_MANAGER_CLASS(TimeManager)
    INIT_MANAGER_CLASS(DelayedCallManager)
    INIT_MANAGER_CLASS(PhysicsManager)
    INIT_MANAGER_CLASS(BuildSettings)
    INIT_MANAGER_CLASS(QualitySettings)
    INIT_MANAGER_CLASS(ResourceManager)
    INIT_MANAGER_CLASS(NavMeshProjectSettings)
    INIT_MANAGER_CLASS(Physics2DSettings)
    INIT_MANAGER_CLASS(ClusterInputManager)
    INIT_MANAGER_CLASS(OcclusionCullingSettings)
    INIT_MANAGER_CLASS(RenderSettings)
    INIT_MANAGER_CLASS(LightmapSettings)
    INIT_MANAGER_CLASS(NavMeshSettings)
    INIT_MANAGER_CLASS(RuntimeInitializeOnLoadManager)
    INIT_MANAGER_CLASS(UnityConnectSettings)
#if ENABLE_TEXTURE_STREAMING
    INIT_MANAGER_CLASS(StreamingManager)
#endif
    INIT_MANAGER_CLASS(VFXManager)

#if UNITY_EDITOR
    for (int i = 0; i < kManagerCount; i++)
    {
        Assert(m_ManagerTypes[i] != NULL);
    }

    dynamic_array<const Unity::Type*> allDerivedClasses(kMemTempAlloc);
    TypeOf<GameManager>()->FindAllDerivedClasses(allDerivedClasses, Unity::Type::kOnlyNonAbstract);
    if (allDerivedClasses.size() != kManagerCount)
    {
        ErrorString("Number of GameManager classes does not match number of game managers registered.");
    }
#endif
}

static ManagerContext gContext;

bool IsManagerContextAvailable(int index)
{
    return gContext.m_Managers[index] != NULL;
}

Object& GetManagerFromContext(int index)
{
    // Don't fake this - fake GetManagerPtrFromContext instead, so that it doesn't matter which one callers are calling
    Object* result = GetManagerPtrFromContext(index);

#if DEBUGMODE
    if (result == NULL)
    {
        char const* managerName = gContext.m_ManagerNames[index];
        FatalErrorString(Format("GetManagerFromContext: pointer to object of manager '%s' is NULL (table index %d)", managerName, index));
    }
#endif

    return *result;
}

void ManagerContextInitializeClasses()
{
    gContext.InitializeClasses();
}

Object* GetManagerPtrFromContext(int index)
{
//    __FAKEABLE_FUNCTION__(GetManagerPtrFromContext, (index));
//
//    ASSERT_NOT_RUNNING_ON_EMULATED_JOB

#if DEBUGMODE
    if (index >= ManagerContext::kManagerCount)
        FatalErrorString("GetManagerFromContext: index for managers table is out of bounds");
#endif

    return gContext.m_Managers[index];
}

void SetManagerPtrInContext(int index, Object* ptr)
{
    gContext.m_Managers[index] = ptr;
}

const ManagerContext& GetManagerContext()
{
    return gContext;
}
