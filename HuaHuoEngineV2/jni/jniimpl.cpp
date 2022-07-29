//
// Created by VincentZhang on 2022-07-29.
//

#include "online_huahuo_backend_hhenginejni_HuahuoEngineJNIInterface.h"

JNIEXPORT jobject JNICALL Java_online_huahuo_backend_hhenginejni_HuahuoEngineJNIInterface_getProjectFileMetaInfo
        (JNIEnv * env, jclass callerClass, jstring path){
    jclass projectMetaClass = env->FindClass("online/huahuo/backend/hhenginejni");
    jobject newProjectMetaObj = env->AllocObject(projectMetaClass);

    const char* projectFilePath = env->GetStringUTFChars(path, NULL);

    printf("Got project file path:%s\n", projectFilePath);

    return newProjectMetaObj;
}