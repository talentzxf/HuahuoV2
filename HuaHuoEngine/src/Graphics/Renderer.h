//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_RENDERER_H
#define HUAHUOENGINE_RENDERER_H


#include "Components/BaseComponent.h"
#include "Camera/BaseRenderer.h"
#include "Camera/SceneNode.h"

class Renderer : public BaseComponent, public BaseRenderer{
    REGISTER_CLASS_TRAITS(kTypeIsAbstract);
    REGISTER_CLASS(Renderer);
    DECLARE_OBJECT_SERIALIZE();
public:
    typedef std::vector<PPtr<Material> > MaterialArray;
    typedef std::vector<UInt32> IndexArray;

    Renderer(RendererType type, MemLabelId label, ObjectCreationMode mode);
    virtual int GetLayer() const override;

    const StaticBatchInfo& GetStaticBatchInfo() const { return m_RendererData.m_StaticBatchInfo; }
    bool IsPartOfStaticBatch() const { return IsPartOfBatch(m_RendererData.m_StaticBatchInfo); }

    // This must be called from the renderer subclass whenever anything is changed
    // that affects the bounding volume
    void BoundsChanged();

    virtual int GetMaterialCount() const override            { return m_Materials.size(); }
    virtual PPtr<Material> GetMaterial(int i) const override { return m_Materials[i]; }

    virtual void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    SceneHandle GetSceneHandle() { return m_SceneHandle; }

    bool IsInScene() const { return m_SceneHandle != kInvalidSceneHandle; }

    // A callback from the scene to notify
    // that the pointer to the cullnode associated with the renderer has changed
    void NotifySceneHandleChange(SceneHandle handle) { m_SceneHandle = handle; }

protected:
    // Should this renderer be in the scene?
    bool ShouldBeInScene() const { return m_Enabled && m_IsRenderable && IsActive(); }

    // Update the scene info for the renderer.
    // Registers/unregisters the renderer in the scene.
    void UpdateRenderer();
protected:
    MaterialArray       m_Materials;    ///< List of materials to use when rendering.
private:
    void AddToScene();
    void RemoveFromScene();
private:
    SceneHandle         m_SceneHandle;
    bool                m_Enabled;
    bool                m_IsRenderable;
};


#endif //HUAHUOENGINE_RENDERER_H
