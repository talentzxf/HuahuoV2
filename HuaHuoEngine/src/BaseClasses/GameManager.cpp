#include "GameManager.h"
// #include "Runtime/Serialize/TransferFunctions/SerializeTransfer.h"
#include "ManagerContext.h"

//void GameManager::ThreadedCleanup()
//{
//}
//
//void GameManager::MainThreadCleanup()
//{
//    for (int i = 0; i < ManagerContext::kManagerCount; i++)
//    {
//        if (GetManagerContext().m_Managers[i] == this)
//            SetManagerPtrInContext(i, NULL);
//    }
//    Super::MainThreadCleanup();
//}

//void LevelGameManager::ThreadedCleanup() {}
//void GlobalGameManager::ThreadedCleanup() {}

template<class TransferFunction>
void LevelGameManager::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
}

template<class TransferFunction>
void GlobalGameManager::Transfer(TransferFunction& transfer)
{
    Super::Transfer(transfer);
}

char const* GlobalGameManager::GetName() const
{
    return GetTypeName();
}

GameManager* GetGameManagerIfExists(int index)
{
    GameManager* manager = static_cast<GameManager*>(GetManagerPtrFromContext(index));
//    Assert(manager == dynamic_pptr_cast<GameManager*>(GetManagerPtrFromContext(index)));
    return manager;
}

LevelGameManager::LevelGameManager(/*MemLabelId label,*/ ObjectCreationMode mode) : Super(/*label,*/ mode)
{}

GlobalGameManager::GlobalGameManager(/*MemLabelId label,*/ ObjectCreationMode mode) : Super(/*label,*/ mode)
{}


IMPLEMENT_REGISTER_CLASS(LevelGameManager, 11);
IMPLEMENT_REGISTER_CLASS(GlobalGameManager, 6);
IMPLEMENT_REGISTER_CLASS(GameManager, 9);

IMPLEMENT_OBJECT_SERIALIZE(LevelGameManager);
IMPLEMENT_OBJECT_SERIALIZE(GlobalGameManager);

INSTANTIATE_TEMPLATE_TRANSFER(LevelGameManager);
INSTANTIATE_TEMPLATE_TRANSFER(GlobalGameManager);
