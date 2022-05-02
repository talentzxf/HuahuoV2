#pragma once

#if DEBUGMODE

#define ABORT_INVALID_FLOAT(value, varname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s assign attempt for '%s' is not valid. Input %s is { %s }.", #classname, #varname, GetName (), #varname, FloatToString (value).c_str ()), this); \
    return; \
}

#define ABORT_INVALID_ARG_FLOAT(value, varname, methodname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value).c_str ()), this); \
    return; \
}

#define RETURN_INVALID_ARG_FLOAT(value, varname, methodname, classname, returnvalue) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value).c_str ()), this); \
    return returnvalue; \
}

#ifdef VECTOR2_H
#define ABORT_INVALID_VECTOR2(value, varname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s assign attempt for '%s' is not valid. Input %s is { %s, %s }.", #classname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str ()), this); \
    return; \
}

#define ABORT_INVALID_ARG_VECTOR2(value, varname, methodname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s, %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str ()), this); \
    return; \
}

#define RETURN_INVALID_ARG_VECTOR2(value, varname, methodname, classname, returnvalue) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s, %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str ()), this); \
    return returnvalue; \
}

#endif

#ifdef VECTOR3_H
#define ABORT_INVALID_VECTOR3(value, varname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s assign attempt for '%s' is not valid. Input %s is { %s, %s, %s }.", #classname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str (), FloatToString (value.z).c_str ()), this); \
    return; \
}

#define ABORT_INVALID_ARG_VECTOR3(value, varname, methodname, classname) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s, %s, %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str (), FloatToString (value.z).c_str ()), this); \
    return; \
}

#define RETURN_INVALID_ARG_VECTOR3(value, varname, methodname, classname, returnvalue) \
if (!IsFinite (value)) \
{ \
    ErrorStringObject (Format ("%s.%s(%s) assign attempt for '%s' is not valid. Input %s is { %s, %s, %s }.", #classname, #methodname, #varname, GetName (), #varname, FloatToString (value.x).c_str (), FloatToString (value.y).c_str (), FloatToString (value.z).c_str ()), this); \
    return returnvalue; \
}
#endif

#ifdef QUATERNION_H
#define ABORT_INVALID_QUATERNION(value, varname, classname)   \
if (!IsFinite(value)) \
{ \
    ErrorStringObject (Format("%s.%s assign attempt for '%s' is not valid. Input rotation is { %s, %s, %s, %s }.", #classname, #varname, GetName(), FloatToString(value.x).c_str(), FloatToString(value.y).c_str(), FloatToString(value.z).c_str(), FloatToString(value.w).c_str()), this); \
    return; \
}

#define ABORT_INVALID_ARG_QUATERNION(value, varname, methodname, classname)    \
if (!IsFinite(value)) \
{ \
    ErrorStringObject (Format("%s.%s(%s) assign attempt for '%s' is not valid. Input rotation is { %s, %s, %s, %s }.", #classname, #methodname, #varname, GetName(), FloatToString(value.x).c_str(), FloatToString(value.y).c_str(), FloatToString(value.z).c_str(), FloatToString(value.w).c_str()), this); \
    return; \
}

#define RETURN_INVALID_ARG_QUATERNION(value, varname, methodname, classname, returnvalue)   \
if (!IsFinite(value)) \
{ \
    ErrorStringObject (Format("%s.%s(%s) assign attempt for '%s' is not valid. Input rotation is { %s, %s, %s, %s }.", #classname, #methodname, #varname, GetName(), FloatToString(value.x).c_str(), FloatToString(value.y).c_str(), FloatToString(value.z).c_str(), FloatToString(value.w).c_str()), this); \
    return returnvalue; \
}
#endif

#else

#define ABORT_INVALID_FLOAT(value, varname, classname)
#define ABORT_INVALID_VECTOR2(value, varname, classname)
#define ABORT_INVALID_VECTOR3(value, varname, classname)
#define ABORT_INVALID_QUATERNION(value, varname, classname)

#define ABORT_INVALID_ARG_FLOAT(value, varname, methodname, classname)
#define ABORT_INVALID_ARG_VECTOR2(value, varname, methodname, classname)
#define ABORT_INVALID_ARG_VECTOR3(value, varname, methodname, classname)
#define ABORT_INVALID_ARG_QUATERNION(value, varname, methodname, classname)

#define RETURN_INVALID_ARG_FLOAT(value, varname, methodname, classname, returnvalue)
#define RETURN_INVALID_ARG_VECTOR2(value, varname, methodname, classname, returnvalue)
#define RETURN_INVALID_ARG_VECTOR3(value, varname, methodname, classname, returnvalue)
#define RETURN_INVALID_ARG_QUATERNION(value, varname, methodname, classname, returnvalue)

#endif
