//
// Created by VincentZhang on 4/8/2022.
//

#ifndef HUAHUOENGINE_MEMORYMACROSDETAILS_H
#define HUAHUOENGINE_MEMORYMACROSDETAILS_H

template<typename T>
class NewWithLabelConstructor{
public:
    NewWithLabelConstructor(int align, const char* file, int line)
    {
    }

    T* construct() { return new T(); }
};
#define NEW_AS_ROOT_WITH_LABEL_CONSTRUCT(...) construct(__VA_ARGS__)
#endif //HUAHUOENGINE_MEMORYMACROSDETAILS_H
