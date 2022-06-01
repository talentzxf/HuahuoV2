//
// Created by VincentZhang on 5/17/2022.
//

#ifndef HUAHUOENGINE_SHADERTAGS_H
#define HUAHUOENGINE_SHADERTAGS_H

struct ShaderTagID
{
    int id;

    explicit ShaderTagID() : id(0) {}
    explicit ShaderTagID(int id_) : id(id_) {}
    explicit ShaderTagID(const char* tagName);

    friend bool operator==(const ShaderTagID& lhs, const ShaderTagID& rhs) { return lhs.id == rhs.id; }
    friend bool operator!=(const ShaderTagID& lhs, const ShaderTagID& rhs) { return lhs.id != rhs.id; }
    friend bool operator<(const ShaderTagID& lhs, const ShaderTagID& rhs) { return lhs.id < rhs.id; }

    bool IsValid() const { return id > 0; }

    static ShaderTagID Invalid() { return ShaderTagID(); }
};


#endif //HUAHUOENGINE_SHADERTAGS_H
