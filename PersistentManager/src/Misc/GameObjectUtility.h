//
// Created by VincentZhang on 4/28/2022.
//

#ifndef PERSISTENTMANAGER_GAMEOBJECTUTILITY_H
#define PERSISTENTMANAGER_GAMEOBJECTUTILITY_H
#include "BaseClasses/GameObject.h"
#include <string>

/// Adds a component by type or className to the game object.
/// This method does several checks and returns null if any of them fail.
/// - Class has to be derived from Component
/// - Class has to be not already added to the game object or be allowed to be added multiple times (ComponentRequirement.cpp)
/// On failure this method returns NULL and if error != null an error string.
/// This method automatically orders filters by their sort priority.
/// * Default properties are only setup in edit mode
BaseComponent* AddComponent(GameObject& go, const HuaHuo::Type* componentType, /*ScriptingClassPtr klass,*/ std::string* error = NULL, /*AwakeFromLoadQueue* queue = nullptr,*/ const char* componentName = NULL);
BaseComponent* AddComponent(GameObject& go, const char* className, std::string* error = NULL/*, AwakeFromLoadQueue* queue = nullptr*/);
//BaseComponent* AddComponentUnchecked(GameObject& go, const HuaHuo::Type* componentType, MonoScriptPtr script, std::string* error, AwakeFromLoadQueue* queue = nullptr);
//BaseComponent* AddComponentUnchecked(GameObject& go, const HuaHuo::Type* componentType, ScriptingClassPtr klass, std::string* error, AwakeFromLoadQueue* queue = nullptr);


//template<class T>
//inline T& AddComponentUnchecked(GameObject& go, ScriptingClassPtr klass = NULL, std::string* error = NULL)
//{
//    return static_cast<T*>(AddComponentUnchecked(go, TypeOf<T>(), klass, error));
//}
//
//template<class T>
//inline T& AddComponentUnchecked(GameObject& go, MonoScriptPtr script, std::string* error = NULL)
//{
//    return static_cast<T*>(AddComponentUnchecked(go, TypeOf<T>(), script, error));
//}

template<typename TComponent>
inline TComponent* AddTransformComponentUnchecked(GameObject& gameObject)
{
    BaseComponent* AddTransformComponentUnchecked(GameObject& gameObject, const HuaHuo::Type* type);
    return static_cast<TComponent*>(AddTransformComponentUnchecked(gameObject, TypeOf<TComponent>()));
}

/// Creates a game object with name. Add's a null terminated list of components by className.
/// Errors when a component can't be added!
GameObject& CreateGameObject(const std::string& name, const char* componentName, ...);
GameObject& CreateGameObjectWithVAList(const std::string& name, const char* componentName, va_list componentList);
GameObject& CreateGameObjectWithHideFlags(const std::string& name, bool isActive, Object::HideFlags flags, const char* componentName, ...);

#endif //PERSISTENTMANAGER_GAMEOBJECTUTILITY_H
