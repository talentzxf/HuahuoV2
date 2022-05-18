#pragma once

#include "TypeSystem/Object.h"

/// Any game manager (eg. AudioManager, dynamicsmanager) that needs serialization
/// has to derive from either LevelGameManager or GlobalGameManager.
/// Every level contains its own GameManager for that Level (eg. Scene, PhysicsManager)
/// LevelGameManagers are destroyed and reloaded from the new scene when loading a new scene.
/// GlobalGameManagers are singletons and loaded on
/// startup of the gameplayer/editor (eg. InputManager, TagManager)

class EXPORT_COREMODULE GameManager : public Object
{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(GameManager);
public:

    GameManager(/*MemLabelId label,*/ ObjectCreationMode mode) : Super(/*label,*/ mode) {}
    // virtual void MainThreadCleanup() override;
    virtual bool ShouldWriteForBuild() const { return true; }

    ///@TODO: Get rid of this. I am not sure why this is not just done in the destructor / cleanup class
    virtual void NetworkOnApplicationQuit() { AssertString("not implemented"); }
    virtual void NetworkUpdate() { AssertString("not implemented"); }
};


class EXPORT_COREMODULE LevelGameManager : public GameManager
{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(LevelGameManager);
    DECLARE_OBJECT_SERIALIZE();
public:

    virtual char const* GetName() const override { return GetTypeName(); }


    LevelGameManager(/*MemLabelId label,*/ ObjectCreationMode mode);

    //  virtual ~LevelGameManager ();
};


class EXPORT_COREMODULE GlobalGameManager : public GameManager
{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(GlobalGameManager);
    DECLARE_OBJECT_SERIALIZE();
public:

    GlobalGameManager(/*MemLabelId label,*/ ObjectCreationMode mode);

    //  virtual ~GlobalGameManager ();

    virtual char const* GetName() const override;
};

GameManager* GetGameManagerIfExists(int index);

inline GameManager* CreateGameManager(const HuaHuo::Type* type)
{
    Object* o = Object::Produce(type);

    if(o == NULL){  //VZ: Work around here.
        return NULL;
    }

    o->Reset();
    o->AwakeFromLoad(kDefaultAwakeFromLoad);
    o->SetName(type->GetName());
    return static_cast<GameManager*>(o);
}

#define CALL_MANAGER_IF_EXISTS(x, func) { GameManager* _manager = GetGameManagerIfExists(x); if (_manager) _manager->func; }
