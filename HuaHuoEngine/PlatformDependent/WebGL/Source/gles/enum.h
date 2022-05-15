#pragma once

static const GLenum GL_BACK_LEFT                            = 0x0402;
static const GLenum GL_BACK_RIGHT                           = 0x0403;
static const GLenum GL_STEREO                               = 0x0C33;

static const GLenum GL_VERTEX_ATTRIB_ARRAY_LONG             = 0x874E;

static const GLenum GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT               = 0x8CDA; // GL_EXT_framebuffer_object
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER               = 0x8CDB;
static const GLenum GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER               = 0x8CDC;

static const GLenum GL_SCALED_RESOLVE_NICEST_EXT                        = 0x90BB;

static const GLenum GL_LINE                                             = 0x1B01;
static const GLenum GL_FILL                                             = 0x1B02;

static const GLenum GL_TEXTURE_LOD_BIAS                    = 0x8501;

static const GLenum GL_TEXTURE_TARGET                      = 0x1006;

static const GLenum GL_TEXTURE_SPARSE_ARB                  = 0x91A6;
static const GLenum GL_NUM_SPARSE_LEVELS_ARB               = 0x91AA;
static const GLenum GL_VIRTUAL_PAGE_SIZE_X_ARB             = 0x9195;
static const GLenum GL_VIRTUAL_PAGE_SIZE_Y_ARB             = 0x9196;

static const GLenum GL_TIME_ELAPSED             = 0x88BF;

static const GLenum GL_GEOMETRY_LINKED_INPUT_TYPE           = 0x8917;

static const GLenum GL_BGRA                                     = 0x80E1;

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

static const GLenum GL_FRAMEBUFFER_SRGB                 = 0x8DB9; // OpenGL 3.0 / ARB_framebuffer_object / EXT_framebuffer_srgb
static const GLenum GL_MULTISAMPLE                      = 0x809D; // OpenGL 1.3
static const GLenum GL_PROGRAM_POINT_SIZE               = 0x8642; // OpenGL 3.2
static const GLenum GL_POLYGON_OFFSET_LINE              = 0x2A02; // OpenGL 1.1
static const GLenum GL_LINE_SMOOTH                      = 0x0B20; // OpenGL line smoothing, only for OpenGL core and compatibility profile
static const GLenum GL_TEXTURE_CUBE_MAP_SEAMLESS        = 0x884F; // OpenGL 3.2 / GL_ARB_seamless_cube_map
static const GLenum GL_DEPTH_CLAMP                      = 0x864F; // OpenGL 3.2 / GL_ARB_depth_clamp
static const GLenum GL_CONSERVATIVE_RASTER              = 0x9346; // GL_NV_conservative_raster

static const GLenum GL_MIRROR_CLAMP_TO_EDGE         = 0x8743; // from GL4.4 / EXT_texture_mirror_clamp / ATI_texture_mirror_once

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

static const GLenum GL_ALPHA8 = 0x803C;
static const GLenum GL_ALPHA16 = 0x803E;
static const GLenum GL_RGBA16 = 0x805B;
static const GLenum GL_R16 = 0x822A;
static const GLenum GL_RG16 = 0x822C;
static const GLenum GL_RGB16 = 0x8054;

static const GLenum GL_R16_SNORM = 0x8F98;
static const GLenum GL_RG16_SNORM = 0x8F99;
static const GLenum GL_RGB16_SNORM = 0x8F9A;
static const GLenum GL_RGBA16_SNORM = 0x8F9B;

// GL_EXT_texture_compression_astc_decode_mode
static const GLenum GL_TEXTURE_ASTC_DECODE_PRECISION_EXT = 0x8F69;
