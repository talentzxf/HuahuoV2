#pragma once

#include "Internal/CoreMacros.h"
#include <cstdlib>
#include <vector>
#include <string>
#include "Utilities/ReflectableEnum.h"

REFLECTABLE_ENUM(GLExt,
    // AMD
    kGL_AMD_vertex_shader_layer,

    // ANDROID
    kGL_ANDROID_extension_pack_es31a,

    // APPLE
    kGL_APPLE_color_buffer_packed_float,
    kGL_APPLE_framebuffer_multisample,
    kGL_APPLE_texture_format_BGRA8888,
    kGL_APPLE_texture_max_level,
    kGL_APPLE_texture_packed_float,

    // ARB
    kGL_ARB_clear_buffer_object,
    kGL_ARB_compute_shader,
    kGL_ARB_copy_buffer,
    kGL_ARB_copy_image,
    kGL_ARB_depth_clamp,
    kGL_ARB_depth_texture,
    kGL_ARB_direct_state_access,
    kGL_ARB_draw_buffers_blend,
    kGL_ARB_draw_elements_base_vertex,
    kGL_ARB_draw_indirect,
    kGL_ARB_draw_instanced,
    kGL_ARB_ES2_compatibility,
    kGL_ARB_ES3_compatibility,
    kGL_ARB_ES3_1_compatibility,
    kGL_ARB_ES3_2_compatibility,
    kGL_ARB_framebuffer_blit,
    kGL_ARB_framebuffer_object,
    kGL_ARB_framebuffer_sRGB,
    kGL_ARB_geometry_shader4,
    kGL_ARB_get_program_binary,
    kGL_ARB_internalformat_query2,
    kGL_ARB_invalidate_subdata,
    kGL_ARB_map_buffer_range,
    kGL_ARB_sampler_objects,
    kGL_ARB_separate_shader_objects,
    kGL_ARB_shader_image_load_store,
    kGL_ARB_shader_storage_buffer_object,
    kGL_ARB_shader_viewport_layer_array,
    kGL_ARB_sparse_texture,
    kGL_ARB_tessellation_shader,
    kGL_ARB_texture_compression_bptc,
    kGL_ARB_texture_compression_rgtc,
    kGL_ARB_texture_cube_map_array,
    kGL_ARB_texture_mirror_clamp_to_edge,
    kGL_ARB_texture_storage,
    kGL_ARB_texture_swizzle,
    kGL_ARB_texture_view,
    kGL_ARB_timer_query,
    kGL_ARB_transform_feedback3,
    kGL_ARB_uniform_buffer_object,
    kGL_ARB_vertex_array_object,
    kGL_ARB_vertex_buffer_object,

    // ATI
    kGL_ATI_meminfo,
    kGL_ATI_texture_mirror_once,

    // EXT
    kGL_EXT_blend_minmax,
    kGL_EXT_clip_cull_distance,
    kGL_EXT_color_buffer_float,
    kGL_EXT_color_buffer_half_float,
    kGL_EXT_copy_image,
    kGL_EXT_debug_label,
    kGL_EXT_debug_marker,
    kGL_EXT_direct_state_access,
    kGL_EXT_discard_framebuffer,
    kGL_EXT_disjoint_timer_query,
    kGL_EXT_draw_buffers,
    kGL_EXT_draw_elements_base_vertex,
    kGL_EXT_draw_instanced,
    kGL_EXT_float_blend,
    kGL_EXT_framebuffer_multisample_blit_scaled,
    kGL_EXT_geometry_shader,
    kGL_EXT_map_buffer_range,
    kGL_EXT_multisampled_render_to_texture,
    kGL_EXT_packed_depth_stencil,
    kGL_EXT_pvrtc_sRGB,
    kGL_EXT_render_snorm,
    kGL_EXT_separate_shader_objects,
    kGL_EXT_shader_texture_lod,
    kGL_EXT_shadow_samplers,
    kGL_EXT_sparse_texture,
    kGL_EXT_sRGB,
    kGL_EXT_sRGB_write_control,
    kGL_EXT_tessellation_shader,
    kGL_EXT_texture_buffer,
    kGL_EXT_texture_compression_astc_decode_mode,
    kGL_EXT_texture_compression_astc_decode_mode_rgb9e5,
    kGL_EXT_texture_compression_rgtc,
    kGL_EXT_texture_compression_s3tc,
    kGL_EXT_texture_cube_map_array,
    kGL_EXT_texture_filter_anisotropic,
    kGL_EXT_texture_format_BGRA8888,
    kGL_EXT_texture_mirror_clamp,
    kGL_EXT_texture_mirror_clamp_to_edge,
    kGL_EXT_texture_norm16,
    kGL_EXT_texture_rg,
    kGL_EXT_texture_sRGB_decode,
    kGL_EXT_texture_sRGB_R8,
    kGL_EXT_texture_sRGB_RG8,
    kGL_EXT_texture_storage,
    kGL_EXT_texture_swizzle,
    kGL_EXT_texture_type_2_10_10_10_REV,
    kGL_EXT_texture_view,

    // GOOGLE
    kGL_GOOGLE_depth_texture,

    // IMG
    kGL_IMG_multisampled_render_to_texture,
    kGL_IMG_texture_compression_pvrtc,
    kGL_IMG_texture_format_BGRA8888,

    // KHR
    kGL_KHR_blend_equation_advanced,
    kGL_KHR_blend_equation_advanced_coherent,
    kGL_KHR_debug,
    kGL_KHR_texture_compression_astc_ldr,
    kGL_KHR_texture_compression_astc_hdr,
    kGL_KHR_texture_compression_astc_sliced_3d,

    // MESA
    kGLX_MESA_query_renderer,

    // NV
    kGL_NV_blend_equation_advanced,
    kGL_NV_blend_equation_advanced_coherent,
    kGL_NV_conservative_raster,
    kGL_NV_coverage_sample,
    kGL_NV_depth_nonlinear,
    kGL_NV_draw_buffers,
    kGL_NV_draw_instanced,
    kGL_NV_fbo_color_attachments,
    kGL_NV_framebuffer_blit,
    kGL_NV_framebuffer_multisample,
    kGL_NV_packed_float,
    kGL_NV_read_buffer,
    kGL_NV_sRGB_formats,
    kGL_NV_timer_query,
    kGL_NV_viewport_array2,

    // NVX
    kGL_NVX_gpu_memory_info,

    // OES
    kGL_OES_compressed_ETC1_RGB8_texture,
    kGL_OES_copy_image,
    kGL_OES_depth24,
    kGL_OES_depth_texture,
    kGL_OES_depth_texture_cube_map,
    kGL_OES_draw_elements_base_vertex,
    kGL_OES_EGL_image,
    kGL_OES_element_index_uint,
    kGL_OES_fbo_render_mipmap,
    kGL_OES_geometry_shader,
    kGL_OES_get_program_binary,
    kGL_OES_mapbuffer,
    kGL_OES_packed_depth_stencil,
    kGL_OES_tessellation_shader,
    kGL_OES_texture_3D,
    kGL_OES_texture_cube_map_array,
    kGL_OES_texture_float,
    kGL_OES_texture_float_linear,
    kGL_OES_texture_half_float,
    kGL_OES_texture_half_float_linear,
    kGL_OES_texture_npot,
    kGL_OES_texture_view,
    kGL_OES_vertex_array_object,
    kGL_OES_vertex_half_float,


    // OVR
    kGL_OVR_multiview,
    kGL_OVR_multiview_multisampled_render_to_texture,

    // WEBGL
    kGL_WEBGL_depth_texture
);

REFLECTABLE_ENUM(WebGLExt,
    // ANGLE
    kANGLE_instanced_arrays,

    // EXT
    kEXT_texture_filter_anisotropic,
    kEXT_sRGB,

    // WEBGL
    kWEBGL_color_buffer_float,
    kWEBGL_compressed_texture_astc_ldr,
    kWEBGL_compressed_texture_es3,
    kWEBGL_compressed_texture_etc1,
    kWEBGL_compressed_texture_pvrtc,
    kWEBGL_compressed_texture_s3tc,
    kWEBGL_compressed_texture_s3tc_srgb,
    kWEBGL_depth_texture,
    kWEBGL_draw_buffers,

    // WEBKIT
    kWEBKIT_EXT_texture_filter_anisotropic,
    kWEBKIT_WEBGL_compressed_texture_s3tc,
    kWEBKIT_WEBGL_compressed_texture_pvrtc
);

bool HasExtension(GLExt::ActualEnumType ext);
bool HasExtension(WebGLExt::ActualEnumType ext);

// Wrapper to query if an extension is present before the extensions are initialized.
// Useful, for example, to check for GLES31AEP.
// Note that it's way slower than HasExtension()
bool QueryExtensionSlow(GLExt::ActualEnumType ext);

void InitializeExtensions(const std::vector<std::string>& allExtensions);
std::string GetExtensionsString(const std::vector<std::string>& allExtensions);
