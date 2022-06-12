#include "SceneManager/HuaHuoScene.h"
#include "SceneManager/SceneManager.h"
#include "Export/Events/ScriptEventManager.h"
#include "HuaHuoEngine.h"
#include "Components/Transform/Transform.h"
#include "ObjectStore.h"
#include "Shapes/LineShape.h"
#ifdef HUAHUO_EDITOR
#include "Editor/SceneInspector.h"
#include "Editor/SceneView.h"
#include "Editor/Utility/SceneRootTransformArray.h"
#endif
#include "glue.cpp"