#include "SceneManager/HuaHuoScene.h"
#include "SceneManager/SceneManager.h"
#include "Export/Events/ScriptEventManager.h"
#include "HuaHuoEngine.h"
#include "Components/Transform/Transform.h"
#include "Graphics/ScriptableRenderLoop/ScriptableRenderContext.h"
#include "Export/Rendering/RenderPipeline.h"
#include "Export/Rendering/RenderPipelineManager.h"
#ifdef HUAHUO_EDITOR
#include "Editor/SceneInspector.h"
#include "Editor/SceneView.h"
#include "Editor/Utility/SceneRootTransformArray.h"
#endif
#include "glue.cpp"