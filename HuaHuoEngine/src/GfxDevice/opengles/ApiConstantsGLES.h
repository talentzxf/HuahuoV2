#pragma once

#include "ApiTypeGLES.h"

#if PLATFORM_WEBGL
#   include "PlatformDependent/WebGL/Source/gles/enum.h"
#else

static const GLenum GL_TRUE                                 = 1;
static const GLenum GL_FALSE                                = 0;

static const GLenum GL_MAP_READ_BIT                         = 0x0001;
static const GLenum GL_MAP_WRITE_BIT                        = 0x0002;
static const GLenum GL_MAP_INVALIDATE_RANGE_BIT             = 0x0004;
static const GLenum GL_MAP_INVALIDATE_BUFFER_BIT            = 0x0008;
static const GLenum GL_MAP_FLUSH_EXPLICIT_BIT               = 0x0010;
static const GLenum GL_MAP_UNSYNCHRONIZED_BIT               = 0x0020;

static const GLenum GL_COLOR                                = 0x1800;
static const GLenum GL_DEPTH                                = 0x1801;
static const GLenum GL_STENCIL                              = 0x1802;

static const GLenum GL_DEPTH_STENCIL_ATTACHMENT             = 0x821A;

static const GLenum GL_CULL_FACE_MODE                       = 0x0B45;
static const GLenum GL_CULL_FACE                            = 0x0B44;
static const GLenum GL_FRONT                                = 0x0404;
static const GLenum GL_BACK                                 = 0x0405;
static const GLenum GL_BACK_LEFT                            = 0x0402;
static const GLenum GL_BACK_RIGHT                           = 0x0403;
static const GLenum GL_STEREO                               = 0x0C33;
static const GLenum GL_CW                                   = 0x0900;
static const GLenum GL_CCW                                  = 0x0901;

static const GLenum GL_ACTIVE_TEXTURE                       = 0x84E0;
static const GLenum GL_TEXTURE0                             = 0x84C0;

static const GLenum GL_VALIDATE_STATUS                      = 0x8B83;
static const GLenum GL_DELETE_STATUS                        = 0x8B80;
static const GLenum GL_CURRENT_PROGRAM                      = 0x8B8D;

static const GLenum GL_DRAW_FRAMEBUFFER_BINDING             = 0x8CA6;
static const GLenum GL_READ_FRAMEBUFFER_BINDING             = 0x8CAA;
static const GLenum GL_FRAMEBUFFER_BINDING                  = 0x8CA6;

static const GLenum GL_ACTIVE_ATTRIBUTES                    = 0x8B89;
static const GLenum GL_ACTIVE_ATTRIBUTE_MAX_LENGTH          = 0x8B8A;
static const GLenum GL_VERTEX_ATTRIB_ARRAY_ENABLED          = 0x8622;
static const GLenum GL_VERTEX_ATTRIB_ARRAY_TYPE             = 0x8625;
static const GLenum GL_VERTEX_ATTRIB_ARRAY_NORMALIZED       = 0x886A;
static const GLenum GL_VERTEX_ATTRIB_ARRAY_INTEGER          = 0x88FD;
static const GLenum GL_VERTEX_ATTRIB_ARRAY_LONG             = 0x874E;

static const GLenum GL_TYPE                                 = 0x92FA;
static const GLenum GL_ARRAY_SIZE                           = 0x92FB;
static const GLenum GL_LOCATION                             = 0x930E;
static const GLenum GL_PROGRAM_INPUT                        = 0x92E3;

static const GLenum GL_READ_ONLY                            = 0x88B8;
static const GLenum GL_WRITE_ONLY                           = 0x88B9;
static const GLenum GL_READ_WRITE                           = 0x88BA;

static const GLenum GL_ONE                                  = 1;
static const GLenum GL_ZERO                                 = 0;

static const GLenum GL_DEPTH_BUFFER_BIT                     = 0x00000100;
static const GLenum GL_STENCIL_BUFFER_BIT                   = 0x00000400;
static const GLenum GL_COLOR_BUFFER_BIT                     = 0x00004000;
static const GLenum GL_COVERAGE_BUFFER_BIT_NV               = 0x00008000;

static const GLenum GL_INTERLEAVED_ATTRIBS                  = 0x8C8C;

static const GLenum GL_POINTS                               = 0x0000;
static const GLenum GL_LINES                                = 0x0001;
static const GLenum GL_LINE_LOOP                            = 0x0002;
static const GLenum GL_LINE_STRIP                           = 0x0003;
static const GLenum GL_TRIANGLES                            = 0x0004;
static const GLenum GL_TRIANGLE_STRIP                       = 0x0005;
static const GLenum GL_TRIANGLE_FAN                         = 0x0006;

static const GLenum GL_TEXTURE_IMMUTABLE_FORMAT             = 0x912F;
static const GLenum GL_TEXTURE_MAX_LEVEL                    = 0x813D;

static const GLenum GL_TEXTURE_2D                           = 0x0DE1;
static const GLenum GL_TEXTURE_3D                           = 0x806F;
static const GLenum GL_TEXTURE_CUBE_MAP                     = 0x8513;
static const GLenum GL_TEXTURE_2D_ARRAY                     = 0x8C1A;
static const GLenum GL_TEXTURE_CUBE_MAP_ARRAY               = 0x9009;
static const GLenum GL_TEXTURE_2D_MULTISAMPLE               = 0x9100;
static const GLenum GL_TEXTURE_2D_MULTISAMPLE_ARRAY         = 0x9102;
static const GLenum GL_TEXTURE_BUFFER                       = 0x8C2A;
static const GLenum GL_TEXTURE_RECTANGLE                    = 0x84F5;
static const GLenum GL_TEXTURE_EXTERNAL_OES                 = 0x8D65;

static const GLenum GL_SAMPLER_2D                               = 0x8B5E;
static const GLenum GL_SAMPLER_3D                               = 0x8B5F;
static const GLenum GL_SAMPLER_CUBE                             = 0x8B60;
static const GLenum GL_SAMPLER_2D_SHADOW                        = 0x8B62;
static const GLenum GL_SAMPLER_CUBE_SHADOW                      = 0x8DC5;
static const GLenum GL_SAMPLER_2D_ARRAY                         = 0x8DC1;
static const GLenum GL_SAMPLER_2D_ARRAY_SHADOW                  = 0x8DC4;
static const GLenum GL_SAMPLER_CUBE_MAP_ARRAY                   = 0x900C;
static const GLenum GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW            = 0x900D;
static const GLenum GL_INT_SAMPLER_2D                           = 0x8DCA;
static const GLenum GL_INT_SAMPLER_2D_ARRAY                     = 0x8DCF;
static const GLenum GL_INT_SAMPLER_3D                           = 0x8DCB;
static const GLenum GL_INT_SAMPLER_CUBE                         = 0x8DCC;
static const GLenum GL_INT_SAMPLER_CUBE_MAP_ARRAY               = 0x900E;
static const GLenum GL_UNSIGNED_INT_SAMPLER_2D                  = 0x8DD2;
static const GLenum GL_UNSIGNED_INT_SAMPLER_2D_ARRAY            = 0x8DD7;
static const GLenum GL_UNSIGNED_INT_SAMPLER_3D                  = 0x8DD3;
static const GLenum GL_UNSIGNED_INT_SAMPLER_CUBE                = 0x8DD4;
static const GLenum GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY      = 0x900F;
static const GLenum GL_SAMPLER_BUFFER                           = 0x8DC2;
static const GLenum GL_INT_SAMPLER_BUFFER                       = 0x8DD0;
static const GLenum GL_UNSIGNED_INT_SAMPLER_BUFFER              = 0x8DD8;
static const GLenum GL_SAMPLER_2D_MULTISAMPLE                   = 0x9108;
static const GLenum GL_SAMPLER_2D_MULTISAMPLE_ARRAY             = 0x910B;
static const GLenum GL_INT_SAMPLER_2D_MULTISAMPLE               = 0x9109;
static const GLenum GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY         = 0x910C;
static const GLenum GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE      = 0x910A;
static const GLenum GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY    = 0x910D;
static const GLenum GL_SAMPLER_2D_RECT                          = 0x8B63;
static const GLenum GL_SAMPLER_EXTERNAL_OES                     = 0x8D66;

static const GLenum GL_SAMPLER_BINDING                          = 0x8919; // OpenGL 3.3, OpenGL ES 3.0
static const GLenum GL_ELEMENT_ARRAY_BUFFER_BINDING             = 0x8895;
static const GLenum GL_ARRAY_BUFFER_BINDING                     = 0x8894;
static const GLenum GL_UNIFORM_BUFFER_BINDING                   = 0x8A28;
static const GLenum GL_COPY_READ_BUFFER_BINDING                 = 0x8F36;
static const GLenum GL_COPY_WRITE_BUFFER_BINDING                = 0x8F37;
static const GLenum GL_TRANSFORM_FEEDBACK_BUFFER_BINDING        = 0x8C8F;
static const GLenum GL_SHADER_STORAGE_BUFFER_BINDING            = 0x90D3;
static const GLenum GL_DISPATCH_INDIRECT_BUFFER_BINDING         = 0x90EF;
static const GLenum GL_DRAW_INDIRECT_BUFFER_BINDING             = 0x8F43;
static const GLenum GL_ATOMIC_COUNTER_BUFFER_BINDING            = 0x92C1;

static const GLenum GL_RENDERBUFFER                                     = 0x8D41;
static const GLenum GL_RENDERBUFFER_SAMPLES                             = 0x8CAB;
static const GLenum GL_RENDERBUFFER_WIDTH                               = 0x8D42;
static const GLenum GL_RENDERBUFFER_HEIGHT                              = 0x8D43;
static const GLenum GL_RENDERBUFFER_INTERNAL_FORMAT                     = 0x8D44;
static const GLenum GL_NUM_SAMPLE_COUNTS                                = 0x9380;

static const GLenum GL_COLOR_ATTACHMENT0                                = 0x8CE0;
static const GLenum GL_DEPTH_ATTACHMENT                                 = 0x8D00;
static const GLenum GL_STENCIL_ATTACHMENT                               = 0x8D20;
static const GLenum GL_COVERAGE_ATTACHMENT_NV                           = 0x8ED2;
static const GLenum GL_COVERAGE_COMPONENT4_NV                           = 0x8ED1;

static const GLenum GL_FRAMEBUFFER                                      = 0x8D40;
static const GLenum GL_READ_FRAMEBUFFER                                 = 0x8CA8;
static const GLenum GL_DRAW_FRAMEBUFFER                                 = 0x8CA9;
static const GLenum GL_SAMPLE_BUFFERS                                   = 0x80A8;
static const GLenum GL_STENCIL_INDEX                                    = 0x1901;
static const GLenum GL_STENCIL_INDEX8                                   = 0x8D48;
static const GLenum GL_DEPTH_COMPONENT32F                               = 0x8CAC;
static const GLenum GL_DEPTH32F_STENCIL8                                = 0x8CAD;
static const GLenum GL_FLOAT_32_UNSIGNED_INT_24_8_REV                   = 0x8DAD;

static const GLenum GL_FRAMEBUFFER_COMPLETE                             = 0x8CD5;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT                = 0x8CD6;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT        = 0x8CD7;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS                = 0x8CD9;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT               = 0x8CDA; // GL_EXT_framebuffer_object
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER               = 0x8CDB;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER               = 0x8CDC;
static const GLenum GL_FRAMEBUFFER_UNSUPPORTED                          = 0x8CDD;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE               = 0x8D56;

static const GLenum GL_VENDOR                                           = 0x1F00;
static const GLenum GL_EXTENSIONS                                       = 0x1F03;
static const GLenum GL_NUM_EXTENSIONS                                   = 0x821D;

static const GLenum GL_DEBUG_SOURCE_API                                 = 0x8246;
static const GLenum GL_DEBUG_SOURCE_WINDOW_SYSTEM                       = 0x8247;
static const GLenum GL_DEBUG_SOURCE_SHADER_COMPILER                     = 0x8248;
static const GLenum GL_DEBUG_SOURCE_THIRD_PARTY                         = 0x8249;
static const GLenum GL_DEBUG_SOURCE_APPLICATION                         = 0x824A;
static const GLenum GL_DEBUG_SOURCE_OTHER                               = 0x824B;
static const GLenum GL_DEBUG_TYPE_ERROR                                 = 0x824C;
static const GLenum GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR                   = 0x824D;
static const GLenum GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR                    = 0x824E;
static const GLenum GL_DEBUG_TYPE_PORTABILITY                           = 0x824F;
static const GLenum GL_DEBUG_TYPE_PERFORMANCE                           = 0x8250;
static const GLenum GL_DEBUG_TYPE_OTHER                                 = 0x8251;
static const GLenum GL_DEBUG_TYPE_MARKER                                = 0x8268;
static const GLenum GL_DEBUG_TYPE_PUSH_GROUP                            = 0x8269;
static const GLenum GL_DEBUG_TYPE_POP_GROUP                             = 0x826A;
static const GLenum GL_DEBUG_SEVERITY_NOTIFICATION                      = 0x826B;
static const GLenum GL_DEBUG_SEVERITY_HIGH                              = 0x9146;
static const GLenum GL_DEBUG_SEVERITY_MEDIUM                            = 0x9147;
static const GLenum GL_DEBUG_SEVERITY_LOW                               = 0x9148;
static const GLenum GL_DONT_CARE                                        = 0x1100;

static const GLenum GL_SAMPLES                                          = 0x80A9;
static const GLenum GL_READ_BUFFER                                      = 0x0C02;

static const GLenum GL_SCALED_RESOLVE_NICEST_EXT                        = 0x90BB;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE               = 0x8CD0;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_OBJECT_NAME               = 0x8CD1;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL             = 0x8CD2;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE     = 0x8CD3;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_TEXTURE_LAYER             = 0x8CD4;
static const GLenum GL_FRAMEBUFFER_ATTACHMENT_LAYERED                   = 0x8DA7;

static const GLenum GL_LINE                                             = 0x1B01;
static const GLenum GL_FILL                                             = 0x1B02;

static const GLenum GL_PATCHES                                          = 0x000E;

static const GLenum GL_INFO_LOG_LENGTH                                  = 0x8B84;
static const GLenum GL_COMPILE_STATUS                                   = 0x8B81;
static const GLenum GL_SHADER_TYPE                                      = 0x8B4F;
static const GLenum GL_SHADER_SOURCE_LENGTH                             = 0x8B88;

static const GLenum GL_TEXTURE_MAX_ANISOTROPY_EXT          = 0x84FE;
static const GLenum GL_TEXTURE_LOD_BIAS                    = 0x8501;
static const GLenum GL_TEXTURE_BASE_LEVEL                  = 0x813C;
static const GLenum GL_TEXTURE_SAMPLES                     = 0x9106;
static const GLenum GL_TEXTURE_WIDTH                       = 0x1000;
static const GLenum GL_TEXTURE_HEIGHT                      = 0x1001;
static const GLenum GL_TEXTURE_SWIZZLE_R                   = 0x8E42;
static const GLenum GL_TEXTURE_SWIZZLE_G                   = 0x8E43;
static const GLenum GL_TEXTURE_SWIZZLE_B                   = 0x8E44;
static const GLenum GL_TEXTURE_SWIZZLE_A                   = 0x8E45;

static const GLenum GL_TEXTURE_TARGET                      = 0x1006;

static const GLenum GL_FRONT_AND_BACK                      = 0x0408;

static const GLenum GL_PATCH_VERTICES                      = 0x8E72;

static const GLenum GL_UNPACK_ROW_LENGTH                   = 0x0CF2;
static const GLenum GL_PACK_ALIGNMENT                      = 0x0D05;
static const GLenum GL_UNPACK_ALIGNMENT                    = 0x0CF5;

static const GLenum GL_RED_BITS                            = 0x0D52;
static const GLenum GL_GREEN_BITS                          = 0x0D53;
static const GLenum GL_BLUE_BITS                           = 0x0D54;
static const GLenum GL_ALPHA_BITS                          = 0x0D55;
static const GLenum GL_DEPTH_BITS                          = 0x0D56;
static const GLenum GL_STENCIL_BITS                        = 0x0D57;
static const GLenum GL_COVERAGE_BUFFERS_NV                 = 0x8ED3;
static const GLenum GL_COVERAGE_SAMPLES_NV                 = 0x8ED4;

static const GLenum GL_TEXTURE_SPARSE_ARB                  = 0x91A6;
static const GLenum GL_NUM_SPARSE_LEVELS_ARB               = 0x91AA;
static const GLenum GL_VIRTUAL_PAGE_SIZE_X_ARB             = 0x9195;
static const GLenum GL_VIRTUAL_PAGE_SIZE_Y_ARB             = 0x9196;

static const GLenum GL_TEXTURE_SRGB_DECODE_EXT             = 0x8A48;

static const GLenum GL_NEVER                    = 0x0200;
static const GLenum GL_LESS                     = 0x0201;
static const GLenum GL_EQUAL                    = 0x0202;
static const GLenum GL_LEQUAL                   = 0x0203;
static const GLenum GL_GREATER                  = 0x0204;
static const GLenum GL_NOTEQUAL                 = 0x0205;
static const GLenum GL_GEQUAL                   = 0x0206;
static const GLenum GL_ALWAYS                   = 0x0207;

static const GLenum GL_TEXTURE_INTERNAL_FORMAT  = 0x1003;
static const GLenum GL_TEXTURE_COMPARE_MODE     = 0x884C;
static const GLenum GL_TEXTURE_COMPARE_FUNC     = 0x884D;
static const GLenum GL_COMPARE_REF_TO_TEXTURE   = 0x884E;

static const GLenum GL_QUERY_RESULT_AVAILABLE   = 0x8867;
static const GLenum GL_QUERY_RESULT             = 0x8866;
static const GLenum GL_GPU_DISJOINT_EXT         = 0x8FBB;
static const GLenum GL_TIME_ELAPSED             = 0x88BF;

static const GLenum GL_LINK_STATUS                          = 0x8B82;
static const GLenum GL_TESS_CONTROL_OUTPUT_VERTICES         = 0x8E75;
static const GLenum GL_GEOMETRY_LINKED_INPUT_TYPE           = 0x8917;

static const GLenum GL_PROGRAM_BINARY_RETRIEVABLE_HINT      = 0x8257;
static const GLenum GL_PROGRAM_BINARY_LENGTH                = 0x8741;
static const GLenum GL_PROGRAM_BINARY_FORMATS               = 0x87FF;
static const GLenum GL_NUM_PROGRAM_BINARY_FORMATS           = 0x87FE;

static const GLenum GL_STREAM_DRAW                          = 0x88E0;
static const GLenum GL_STATIC_DRAW                          = 0x88E4;
static const GLenum GL_DYNAMIC_DRAW                         = 0x88E8;
static const GLenum GL_STATIC_COPY                          = 0x88E6;

static const GLenum GL_BYTE                             = 0x1400;
static const GLenum GL_UNSIGNED_BYTE                    = 0x1401;
static const GLenum GL_SHORT                            = 0x1402;
static const GLenum GL_UNSIGNED_SHORT                   = 0x1403;
static const GLenum GL_INT                              = 0x1404;
static const GLenum GL_UNSIGNED_INT                     = 0x1405;
static const GLenum GL_FLOAT                            = 0x1406;
static const GLenum GL_FIXED                            = 0x140C;

static const GLenum GL_FLOAT_VEC2 = 0x8B50;
static const GLenum GL_FLOAT_VEC3 = 0x8B51;
static const GLenum GL_FLOAT_VEC4 = 0x8B52;
static const GLenum GL_INT_VEC2 = 0x8B53;
static const GLenum GL_INT_VEC3 = 0x8B54;
static const GLenum GL_INT_VEC4 = 0x8B55;
static const GLenum GL_UNSIGNED_INT_VEC2 = 0x8DC6;
static const GLenum GL_UNSIGNED_INT_VEC3 = 0x8DC7;
static const GLenum GL_UNSIGNED_INT_VEC4 = 0x8DC8;

static const GLenum GL_HIGH_FLOAT = 0x8DF2;

static const GLenum GL_RGBA8                            = 0x8058;
static const GLenum GL_SRGB8_ALPHA8                     = 0x8C43;
static const GLenum GL_DEPTH_COMPONENT16                = 0x81A5;
static const GLenum GL_DEPTH_COMPONENT24                = 0x81A6;
static const GLenum GL_DEPTH24_STENCIL8                 = 0x88F0;

static const GLenum GL_DEPTH_COMPONENT                  = 0x1902;
static const GLenum GL_ALPHA                            = 0x1906;
static const GLenum GL_RGB                              = 0x1907;
static const GLenum GL_RGBA                             = 0x1908;
static const GLenum GL_LUMINANCE                        = 0x1909;

static const GLenum GL_TEXTURE_WRAP_S                       = 0x2802;
static const GLenum GL_TEXTURE_WRAP_T                       = 0x2803;
static const GLenum GL_TEXTURE_WRAP_R                       = 0x8072;
static const GLenum GL_TEXTURE_MAG_FILTER                   = 0x2800;
static const GLenum GL_TEXTURE_MIN_FILTER                   = 0x2801;

static const GLenum GL_TEXTURE_CUBE_MAP_POSITIVE_X          = 0x8515;

static const GLenum GL_MAX_VERTEX_ATTRIBS                   = 0x8869;

static const GLenum GL_RED                                  = 0x1903;
static const GLenum GL_GREEN                                = 0x1904;
static const GLenum GL_BLUE                                 = 0x1905;
static const GLenum GL_NONE                                 = 0;

static const GLenum GL_RG                                       = 0x8227;
static const GLenum GL_BGRA                                     = 0x80E1;
static const GLenum GL_DEPTH_STENCIL                            = 0x84F9;
static const GLenum GL_RED_INTEGER                              = 0x8D94;
static const GLenum GL_RG_INTEGER                               = 0x8228;
static const GLenum GL_RGB_INTEGER                              = 0x8D98;
static const GLenum GL_RGBA_INTEGER                             = 0x8D99;

static const GLenum GL_HALF_FLOAT_OES                           = 0x8D61;
static const GLenum GL_HALF_FLOAT                               = 0x140B;
static const GLenum GL_COMPRESSED_RGBA8_ETC2_EAC                = 0x9278;
static const GLenum GL_COMPRESSED_RGB8_ETC2                     = 0x9274;

static const GLenum GL_SYNC_GPU_COMMANDS_COMPLETE               = 0x9117;
static const GLenum GL_ALREADY_SIGNALED                         = 0x911A;

static const GLenum GL_VERTEX_ATTRIB_ARRAY_BARRIER_BIT          = 0x00000001;
static const GLenum GL_ELEMENT_ARRAY_BARRIER_BIT                = 0x00000002;
static const GLenum GL_UNIFORM_BARRIER_BIT                      = 0x00000004;
static const GLenum GL_TEXTURE_FETCH_BARRIER_BIT                = 0x00000008;
static const GLenum GL_SHADER_IMAGE_ACCESS_BARRIER_BIT          = 0x00000020;
static const GLenum GL_COMMAND_BARRIER_BIT                      = 0x00000040;
static const GLenum GL_PIXEL_BUFFER_BARRIER_BIT                 = 0x00000080;
static const GLenum GL_TEXTURE_UPDATE_BARRIER_BIT               = 0x00000100;
static const GLenum GL_BUFFER_UPDATE_BARRIER_BIT                = 0x00000200;
static const GLenum GL_FRAMEBUFFER_BARRIER_BIT                  = 0x00000400;
static const GLenum GL_TRANSFORM_FEEDBACK_BARRIER_BIT           = 0x00000800;
static const GLenum GL_ATOMIC_COUNTER_BARRIER_BIT               = 0x00001000;
static const GLenum GL_SHADER_STORAGE_BARRIER_BIT               = 0x00002000;

static const GLenum GL_FUNC_ADD                 = 0x8006;
static const GLenum GL_BLEND_EQUATION           = 0x8009;
static const GLenum GL_BLEND_EQUATION_RGB       = 0x8009;
static const GLenum GL_BLEND_EQUATION_ALPHA     = 0x883D;
static const GLenum GL_FUNC_SUBTRACT            = 0x800A;
static const GLenum GL_FUNC_REVERSE_SUBTRACT    = 0x800B;

static const GLenum GL_MULTIPLY_KHR             = 0x9294;
static const GLenum GL_SCREEN_KHR               = 0x9295;
static const GLenum GL_OVERLAY_KHR              = 0x9296;
static const GLenum GL_DARKEN_KHR               = 0x9297;
static const GLenum GL_LIGHTEN_KHR              = 0x9298;
static const GLenum GL_COLORDODGE_KHR           = 0x9299;
static const GLenum GL_COLORBURN_KHR            = 0x929A;
static const GLenum GL_HARDLIGHT_KHR            = 0x929B;
static const GLenum GL_SOFTLIGHT_KHR            = 0x929C;
static const GLenum GL_DIFFERENCE_KHR           = 0x929E;
static const GLenum GL_EXCLUSION_KHR            = 0x92A0;
static const GLenum GL_HSL_HUE_KHR              = 0x92AD;
static const GLenum GL_HSL_SATURATION_KHR       = 0x92AE;
static const GLenum GL_HSL_COLOR_KHR            = 0x92AF;
static const GLenum GL_HSL_LUMINOSITY_KHR       = 0x92B0;

static const GLenum GL_MIN                          = 0x8007;
static const GLenum GL_MAX                          = 0x8008;

static const GLenum GL_CLEAR                        = 0x1500;
static const GLenum GL_AND                          = 0x1501;
static const GLenum GL_AND_REVERSE                  = 0x1502;
static const GLenum GL_COPY                         = 0x1503;
static const GLenum GL_AND_INVERTED                 = 0x1504;
static const GLenum GL_NOOP                         = 0x1505;
static const GLenum GL_XOR                          = 0x1506;
static const GLenum GL_OR                           = 0x1507;
static const GLenum GL_NOR                          = 0x1508;
static const GLenum GL_EQUIV                        = 0x1509;
static const GLenum GL_OR_REVERSE                   = 0x150B;
static const GLenum GL_COPY_INVERTED                = 0x150C;
static const GLenum GL_OR_INVERTED                  = 0x150D;
static const GLenum GL_NAND                         = 0x150E;
static const GLenum GL_SET                          = 0x150F;

static const GLenum GL_ELEMENT_ARRAY_BUFFER         = 0x8893;
static const GLenum GL_ARRAY_BUFFER                 = 0x8892;
static const GLenum GL_UNIFORM_BUFFER               = 0x8A11;
static const GLenum GL_COPY_WRITE_BUFFER            = 0x8F37;
static const GLenum GL_COPY_READ_BUFFER             = 0x8F36;
static const GLenum GL_TRANSFORM_FEEDBACK_BUFFER    = 0x8C8E;
static const GLenum GL_SHADER_STORAGE_BUFFER        = 0x90D2;
static const GLenum GL_DISPATCH_INDIRECT_BUFFER     = 0x90EE;
static const GLenum GL_DRAW_INDIRECT_BUFFER         = 0x8F3F;
static const GLenum GL_ATOMIC_COUNTER_BUFFER        = 0x92C0;
static const GLenum GL_COMPUTE_WORK_GROUP_SIZE      = 0x8267;

static const GLenum GL_TEXTURE_BINDING_CUBE_MAP         = 0x8514;
static const GLenum GL_BLEND                            = 0x0BE2;
static const GLenum GL_DITHER                           = 0x0BD0;
static const GLenum GL_STENCIL_TEST                     = 0x0B90;
static const GLenum GL_DEPTH_TEST                       = 0x0B71;
static const GLenum GL_SCISSOR_TEST                     = 0x0C11;
static const GLenum GL_POLYGON_OFFSET_FILL              = 0x8037;
static const GLenum GL_SAMPLE_ALPHA_TO_COVERAGE         = 0x809E;
static const GLenum GL_FRAMEBUFFER_SRGB                 = 0x8DB9; // OpenGL 3.0 / ARB_framebuffer_object / EXT_framebuffer_srgb
static const GLenum GL_MULTISAMPLE                      = 0x809D; // OpenGL 1.3
static const GLenum GL_SAMPLE_ALPHA_TO_ONE              = 0x809F; // OpenGL 1.3
static const GLenum GL_PROGRAM_POINT_SIZE               = 0x8642; // OpenGL 3.2
static const GLenum GL_POLYGON_OFFSET_LINE              = 0x2A02; // OpenGL 1.1
static const GLenum GL_POLYGON_SMOOTH                   = 0x0B41; // OpenGL 1.1
static const GLenum GL_PRIMITIVE_RESTART                = 0x8F9D; // OpenGL 3.1
static const GLenum GL_PRIMITIVE_RESTART_FIXED_INDEX    = 0x8D69; // OpenGL 4.3 / OpenGL ES 3.0
static const GLenum GL_ALPHA_TEST                       = 0x0BC0; // OpenGL compatibility profile and GL_QCOM_alpha_test
static const GLenum GL_DEBUG_OUTPUT                     = 0x92E0; // KHR_debug
static const GLenum GL_DEBUG_OUTPUT_SYNCHRONOUS         = 0x8242; // KHR_debug
static const GLenum GL_LINE_SMOOTH                      = 0x0B20; // OpenGL line smoothing, only for OpenGL core and compatibility profile
static const GLenum GL_RASTERIZER_DISCARD               = 0x8C89; // OpenGL 3.2 / OpenGL ES 3.0
static const GLenum GL_TEXTURE_CUBE_MAP_SEAMLESS        = 0x884F; // OpenGL 3.2 / GL_ARB_seamless_cube_map
static const GLenum GL_DEPTH_CLAMP                      = 0x864F; // OpenGL 3.2 / GL_ARB_depth_clamp
static const GLenum GL_CONSERVATIVE_RASTER              = 0x9346; // GL_NV_conservative_raster

static const GLenum GL_FRAGMENT_SHADER              = 0x8B30;
static const GLenum GL_VERTEX_SHADER                = 0x8B31;
static const GLenum GL_GEOMETRY_SHADER              = 0x8DD9;
static const GLenum GL_TESS_EVALUATION_SHADER       = 0x8E87;
static const GLenum GL_TESS_CONTROL_SHADER          = 0x8E88;
static const GLenum GL_COMPUTE_SHADER               = 0x91B9;

static const GLenum GL_REPEAT                       = 0x2901;
static const GLenum GL_CLAMP_TO_EDGE                = 0x812F;
static const GLenum GL_MIRRORED_REPEAT              = 0x8370;
static const GLenum GL_MIRROR_CLAMP_TO_EDGE         = 0x8743; // from GL4.4 / EXT_texture_mirror_clamp / ATI_texture_mirror_once
static const GLenum GL_NEAREST                      = 0x2600;
static const GLenum GL_LINEAR                       = 0x2601;
static const GLenum GL_NEAREST_MIPMAP_NEAREST       = 0x2700;
static const GLenum GL_LINEAR_MIPMAP_NEAREST        = 0x2701;
static const GLenum GL_NEAREST_MIPMAP_LINEAR        = 0x2702;
static const GLenum GL_LINEAR_MIPMAP_LINEAR         = 0x2703;

// KHR_debug object type identifiers
static const GLenum GL_BUFFER                       = 0x82E0;
static const GLenum GL_SHADER                       = 0x82E1;
static const GLenum GL_PROGRAM                      = 0x82E2;
static const GLenum GL_VERTEX_ARRAY                 = 0x8074;
static const GLenum GL_QUERY                        = 0x82E3;
static const GLenum GL_PROGRAM_PIPELINE             = 0x82E4;
static const GLenum GL_SAMPLER                      = 0x82E6;
static const GLenum GL_TRANSFORM_FEEDBACK           = 0x8E22;
static const GLenum GL_TEXTURE                      = 0x1702;

static const GLenum GL_SRC_COLOR                = 0x0300;
static const GLenum GL_ONE_MINUS_SRC_COLOR      = 0x0301;
static const GLenum GL_SRC_ALPHA                = 0x0302;
static const GLenum GL_ONE_MINUS_SRC_ALPHA      = 0x0303;
static const GLenum GL_DST_ALPHA                = 0x0304;
static const GLenum GL_ONE_MINUS_DST_ALPHA      = 0x0305;
static const GLenum GL_DST_COLOR                = 0x0306;
static const GLenum GL_ONE_MINUS_DST_COLOR      = 0x0307;
static const GLenum GL_SRC_ALPHA_SATURATE       = 0x0308;

static const GLenum GL_KEEP         = 0x1E00;
static const GLenum GL_REPLACE      = 0x1E01;
static const GLenum GL_INCR         = 0x1E02;
static const GLenum GL_DECR         = 0x1E03;
static const GLenum GL_INVERT       = 0x150A;
static const GLenum GL_INCR_WRAP    = 0x8507;
static const GLenum GL_DECR_WRAP    = 0x8508;

static const GLenum GL_ETC1_RGB8_OES                                = 0x8D64;
static const GLenum GL_COMPRESSED_R11_EAC                           = 0x9270;
static const GLenum GL_COMPRESSED_SIGNED_R11_EAC                    = 0x9271;
static const GLenum GL_COMPRESSED_RG11_EAC                          = 0x9272;
static const GLenum GL_COMPRESSED_SIGNED_RG11_EAC                   = 0x9273;
static const GLenum GL_COMPRESSED_SRGB8_ETC2                        = 0x9275;
static const GLenum GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2     = 0x9276;
static const GLenum GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2    = 0x9277;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC             = 0x9279;

// -- ASTC --
static const GLenum GL_COMPRESSED_RGBA_ASTC_4x4 = 0x93B0;
static const GLenum GL_COMPRESSED_RGBA_ASTC_5x5 = 0x93B2;
static const GLenum GL_COMPRESSED_RGBA_ASTC_6x6 = 0x93B4;
static const GLenum GL_COMPRESSED_RGBA_ASTC_8x8 = 0x93B7;
static const GLenum GL_COMPRESSED_RGBA_ASTC_10x10 = 0x93BB;
static const GLenum GL_COMPRESSED_RGBA_ASTC_12x12 = 0x93BD;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4 = 0x93D0;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5 = 0x93D2;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6 = 0x93D4;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8 = 0x93D7;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10 = 0x93DB;
static const GLenum GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12 = 0x93DD;

// -- DXT --
static const GLenum GL_COMPRESSED_RGBA_S3TC_DXT1_EXT = 0x83F1;
static const GLenum GL_COMPRESSED_RGBA_S3TC_DXT3_EXT = 0x83F2;
static const GLenum GL_COMPRESSED_RGBA_S3TC_DXT5_EXT = 0x83F3;

// -- DXT sRGB: GL_EXT_texture_sRGB (GL), ANGLE_texture_compression_dxt (WebGL), NV_sRGB_formats (ES) --
static const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT = 0x8C4D;
static const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT = 0x8C4E;
static const GLenum GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT = 0x8C4F;

// -- GL_EXT_texture_compression_rgtc / GL_ARB_texture_compression_rgtc / Core32
static const GLenum GL_COMPRESSED_RED_RGTC1 = 0x8DBB;
static const GLenum GL_COMPRESSED_SIGNED_RED_RGTC1 = 0x8DBC;
static const GLenum GL_COMPRESSED_RG_RGTC2 = 0x8DBD;
static const GLenum GL_COMPRESSED_SIGNED_RG_RGTC2 = 0x8DBE;

// -- GL_ARB_texture_compression_bptc / Core42
static const GLenum GL_COMPRESSED_RGBA_BPTC_UNORM = 0x8E8C;
static const GLenum GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM = 0x8E8D;
static const GLenum GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT = 0x8E8E;
static const GLenum GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT = 0x8E8F;

// -- PVRTC --
static const GLenum GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG = 0x8C01;
static const GLenum GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG = 0x8C00;
static const GLenum GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG = 0x8C03;
static const GLenum GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG = 0x8C02;
static const GLenum GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT = 0x8A54;
static const GLenum GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT = 0x8A55;
static const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT = 0x8A56;
static const GLenum GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT = 0x8A57;

// -- Uncompressed internal formats
static const GLenum GL_RGB8 = 0x8051;
static const GLenum GL_R8 = 0x8229;
static const GLenum GL_RG8 = 0x822B;
static const GLenum GL_RGBA32F = 0x8814;
static const GLenum GL_RGB32F = 0x8815;
static const GLenum GL_RG32F = 0x8230;
static const GLenum GL_R32F = 0x822E;
static const GLenum GL_RGBA16F = 0x881A;
static const GLenum GL_RGB16F = 0x881B;
static const GLenum GL_RG16F = 0x822F;
static const GLenum GL_R16F = 0x822D;
static const GLenum GL_RGB10_A2 = 0x8059;
static const GLenum GL_RGB10_A2UI = 0x906F;
static const GLenum GL_ALPHA8 = 0x803C;
static const GLenum GL_ALPHA16 = 0x803E;
static const GLenum GL_RGBA16 = 0x805B;
static const GLenum GL_R16 = 0x822A;
static const GLenum GL_RG16 = 0x822C;
static const GLenum GL_RGB16 = 0x8054;
static const GLenum GL_BGRA8_EXT = 0x93A1;
static const GLenum GL_RGBA4 = 0x8056;
static const GLenum GL_RGB5_A1 = 0x8057;
static const GLenum GL_RGB565 = 0x8D62;
static const GLenum GL_SRGB8 = 0x8C41;
static const GLenum GL_R11F_G11F_B10F = 0x8C3A;
static const GLenum GL_RGB9_E5 = 0x8C3D;
static const GLenum GL_DEPTH_COMPONENT16_NONLINEAR_NV = 0x8E2C;
static const GLenum GL_SR8_EXT = 0x8FBD;
static const GLenum GL_SRG8_EXT = 0x8FBE;

static const GLenum GL_R8_SNORM = 0x8F94;
static const GLenum GL_RG8_SNORM = 0x8F95;
static const GLenum GL_RGB8_SNORM = 0x8F96;
static const GLenum GL_RGBA8_SNORM = 0x8F97;
static const GLenum GL_R16_SNORM = 0x8F98;
static const GLenum GL_RG16_SNORM = 0x8F99;
static const GLenum GL_RGB16_SNORM = 0x8F9A;
static const GLenum GL_RGBA16_SNORM = 0x8F9B;

static const GLenum GL_RGBA32UI = 0x8D70;
static const GLenum GL_RGB32UI = 0x8D71;
static const GLenum GL_RGBA16UI = 0x8D76;
static const GLenum GL_RGB16UI = 0x8D77;
static const GLenum GL_RGBA8UI = 0x8D7C;
static const GLenum GL_RGB8UI = 0x8D7D;
static const GLenum GL_RGBA32I = 0x8D82;
static const GLenum GL_RGB32I = 0x8D83;
static const GLenum GL_RGBA16I = 0x8D88;
static const GLenum GL_RGB16I = 0x8D89;
static const GLenum GL_RGBA8I = 0x8D8E;
static const GLenum GL_RGB8I = 0x8D8F;
static const GLenum GL_R8I = 0x8231;
static const GLenum GL_R8UI = 0x8232;
static const GLenum GL_R16I = 0x8233;
static const GLenum GL_R16UI = 0x8234;
static const GLenum GL_R32I = 0x8235;
static const GLenum GL_R32UI = 0x8236;
static const GLenum GL_RG8I = 0x8237;
static const GLenum GL_RG8UI = 0x8238;
static const GLenum GL_RG16I = 0x8239;
static const GLenum GL_RG16UI = 0x823A;
static const GLenum GL_RG32I = 0x823B;
static const GLenum GL_RG32UI = 0x823C;

// -- Unsized internal formats --
static const GLenum GL_SRGB = 0x8C40;
static const GLenum GL_SRGB_ALPHA_EXT = 0x8C42;

// -- Types --
static const GLenum GL_UNSIGNED_INT_24_8 = 0x84FA;

static const GLenum GL_UNSIGNED_INT_10F_11F_11F_REV = 0x8C3B;
static const GLenum GL_UNSIGNED_INT_5_9_9_9_REV = 0x8C3E;

static const GLenum GL_UNSIGNED_INT_2_10_10_10_REV = 0x8368;
static const GLenum GL_UNSIGNED_INT_10_10_10_2 = 0x8036;
static const GLenum GL_INT_2_10_10_10_REV = 0x8D9F;

static const GLenum GL_UNSIGNED_SHORT_5_6_5_REV = 0x8364;
static const GLenum GL_UNSIGNED_SHORT_5_6_5 = 0x8363;
static const GLenum GL_UNSIGNED_SHORT_1_5_5_5_REV = 0x8366;
static const GLenum GL_UNSIGNED_SHORT_5_5_5_1 = 0x8034;
static const GLenum GL_UNSIGNED_SHORT_4_4_4_4_REV = 0x8365;
static const GLenum GL_UNSIGNED_SHORT_4_4_4_4 = 0x8033;

// -- For EXT_debug_label (~ES) --
static const GLenum GL_BUFFER_OBJECT_EXT = 0x9151;
static const GLenum GL_SHADER_OBJECT_EXT = 0x8B48;
static const GLenum GL_PROGRAM_OBJECT_EXT = 0x8B40;
static const GLenum GL_VERTEX_ARRAY_OBJECT_EXT = 0x9154;
static const GLenum GL_QUERY_OBJECT_EXT = 0x9153;

// -- GL_EXT_texture_sRGB_decode --
static const GLenum GL_DECODE_EXT = 0x8A49;
static const GLenum GL_SKIP_DECODE_EXT = 0x8A4A;

// GL_OVR_Multiview
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_VIEW_TARGETS_OVR = 0x9633;

// GL_EXT_texture_compression_astc_decode_mode
static const GLenum GL_TEXTURE_ASTC_DECODE_PRECISION_EXT = 0x8F69;

#endif
