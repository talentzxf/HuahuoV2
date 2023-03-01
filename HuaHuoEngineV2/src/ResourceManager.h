//
// Created by VincentZhang on 2023-02-28.
//

#ifndef HUAHUOENGINEV2_RESOURCEMANAGER_H
#define HUAHUOENGINEV2_RESOURCEMANAGER_H


#include "TypeSystem/Object.h"

class ResourceManager : public Object {
REGISTER_CLASS(ResourceManager);

DECLARE_OBJECT_SERIALIZE();
public:
    ResourceManager(MemLabelId label, ObjectCreationMode mode)
            : Super(label, mode) {

    }

    static ResourceManager *GetDefaultResourceManager();

    /**
     *
     * @return true - Registered successfully, no duplicate. false - There's a duplication.
     */
    bool RegisterFile(std::string &fileName);

    UInt32 GetDataSize(std::string &fileName) {
        return mFileNameDataMap[fileName].size();
    }

    std::vector<UInt8>& GetFileData(std::string &fileName){
        return mFileNameDataMap[fileName];
    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

private:
    std::map<std::string, vector<UInt8> > mFileNameDataMap;
};

ResourceManager *GetDefaultResourceManager();


#endif //HUAHUOENGINEV2_RESOURCEMANAGER_H
