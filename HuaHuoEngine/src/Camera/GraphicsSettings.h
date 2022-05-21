//
// Created by VincentZhang on 5/16/2022.
//

#ifndef HUAHUOENGINE_GRAPHICSSETTINGS_H
#define HUAHUOENGINE_GRAPHICSSETTINGS_H


#include "TypeSystem/ObjectDefines.h"
#include "BaseClasses/GameManager.h"

class GraphicsSettings :public GlobalGameManager{
    REGISTER_CLASS(GraphicsSettings);
    DECLARE_OBJECT_SERIALIZE();
public:
    GraphicsSettings(MemLabelId label, ObjectCreationMode mode);

    static void InitializeClass();
    static void CleanupClass();
};


#endif //HUAHUOENGINE_GRAPHICSSETTINGS_H
