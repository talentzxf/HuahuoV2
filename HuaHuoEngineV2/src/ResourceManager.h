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

    std::string& GetMimeType(std::string &fileName){
        return mFileNameMimeMap[fileName];
    }

    std::vector<UInt8>& GetFileData(std::string &fileName){
        if(!mFileNameDataMap.contains(fileName)){
            printf("ResourceManager: File:%s has not been registered yet during GetFileData\n", fileName.c_str());

            // TODO: Throw exception here.
        }
        return mFileNameDataMap[fileName];
    }

    void AwakeFromLoad(AwakeFromLoadMode awakeMode) override;

    void SetFileData(const char* fileName, const char* mimeType, UInt8* pData, UInt32 dataSize);

private:
    std::map<std::string, vector<UInt8> > mFileNameDataMap;
    std::map<std::string, std::string> mFileNameMimeMap;
};

ResourceManager *GetDefaultResourceManager();

#endif //HUAHUOENGINEV2_RESOURCEMANAGER_H
