//
// Created by VincentZhang on 5/13/2022.
//

#include "Camera.h"
#include "RendererScene.h"

void Camera::Render() {
    GetRendererScene().BeginCameraRender();
    StandaloneRender(Camera::kRenderFlagSetRenderTarget, "");
    GetRendererScene().EndCameraRender();
}

// No need to use fixed pipeline anymore
void Camera::StandaloneRender(RenderFlag renderFlags, const std::string& replacementTag){

}