/*
 * Vulkan
 *
 * Copyright (C) 2014 LunarG, Inc.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * Authors:
 *   Courtney Goeltzenleuchter <courtney@lunarg.com>
 *   Chia-I Wu <olv@lunarg.com>
 */

#include "genhw/genhw.h"
#include "compiler/pipeline/pipeline_compiler_interface.h"
#include "cmd.h"
#include "format.h"
#include "shader.h"
#include "pipeline.h"
#include "mem.h"

static int translate_blend_func(VkBlendOp func)
{
   switch (func) {
   case VK_BLEND_OP_ADD:                return GEN6_BLENDFUNCTION_ADD;
   case VK_BLEND_OP_SUBTRACT:           return GEN6_BLENDFUNCTION_SUBTRACT;
   case VK_BLEND_OP_REVERSE_SUBTRACT:   return GEN6_BLENDFUNCTION_REVERSE_SUBTRACT;
   case VK_BLEND_OP_MIN:                return GEN6_BLENDFUNCTION_MIN;
   case VK_BLEND_OP_MAX:                return GEN6_BLENDFUNCTION_MAX;
   default:
      assert(!"unknown blend func");
      return GEN6_BLENDFUNCTION_ADD;
   };
}

static int translate_blend(VkBlend blend)
{
   switch (blend) {
   case VK_BLEND_ZERO:                     return GEN6_BLENDFACTOR_ZERO;
   case VK_BLEND_ONE:                      return GEN6_BLENDFACTOR_ONE;
   case VK_BLEND_SRC_COLOR:                return GEN6_BLENDFACTOR_SRC_COLOR;
   case VK_BLEND_ONE_MINUS_SRC_COLOR:      return GEN6_BLENDFACTOR_INV_SRC_COLOR;
   case VK_BLEND_DEST_COLOR:               return GEN6_BLENDFACTOR_DST_COLOR;
   case VK_BLEND_ONE_MINUS_DEST_COLOR:     return GEN6_BLENDFACTOR_INV_DST_COLOR;
   case VK_BLEND_SRC_ALPHA:                return GEN6_BLENDFACTOR_SRC_ALPHA;
   case VK_BLEND_ONE_MINUS_SRC_ALPHA:      return GEN6_BLENDFACTOR_INV_SRC_ALPHA;
   case VK_BLEND_DEST_ALPHA:               return GEN6_BLENDFACTOR_DST_ALPHA;
   case VK_BLEND_ONE_MINUS_DEST_ALPHA:     return GEN6_BLENDFACTOR_INV_DST_ALPHA;
   case VK_BLEND_CONSTANT_COLOR:           return GEN6_BLENDFACTOR_CONST_COLOR;
   case VK_BLEND_ONE_MINUS_CONSTANT_COLOR: return GEN6_BLENDFACTOR_INV_CONST_COLOR;
   case VK_BLEND_CONSTANT_ALPHA:           return GEN6_BLENDFACTOR_CONST_ALPHA;
   case VK_BLEND_ONE_MINUS_CONSTANT_ALPHA: return GEN6_BLENDFACTOR_INV_CONST_ALPHA;
   case VK_BLEND_SRC_ALPHA_SATURATE:       return GEN6_BLENDFACTOR_SRC_ALPHA_SATURATE;
   case VK_BLEND_SRC1_COLOR:               return GEN6_BLENDFACTOR_SRC1_COLOR;
   case VK_BLEND_ONE_MINUS_SRC1_COLOR:     return GEN6_BLENDFACTOR_INV_SRC1_COLOR;
   case VK_BLEND_SRC1_ALPHA:               return GEN6_BLENDFACTOR_SRC1_ALPHA;
   case VK_BLEND_ONE_MINUS_SRC1_ALPHA:     return GEN6_BLENDFACTOR_INV_SRC1_ALPHA;
   default:
      assert(!"unknown blend factor");
      return GEN6_BLENDFACTOR_ONE;
   };
}

static int translate_compare_func(VkCompareOp func)
{
    switch (func) {
    case VK_COMPARE_OP_NEVER:         return GEN6_COMPAREFUNCTION_NEVER;
    case VK_COMPARE_OP_LESS:          return GEN6_COMPAREFUNCTION_LESS;
    case VK_COMPARE_OP_EQUAL:         return GEN6_COMPAREFUNCTION_EQUAL;
    case VK_COMPARE_OP_LESS_EQUAL:    return GEN6_COMPAREFUNCTION_LEQUAL;
    case VK_COMPARE_OP_GREATER:       return GEN6_COMPAREFUNCTION_GREATER;
    case VK_COMPARE_OP_NOT_EQUAL:     return GEN6_COMPAREFUNCTION_NOTEQUAL;
    case VK_COMPARE_OP_GREATER_EQUAL: return GEN6_COMPAREFUNCTION_GEQUAL;
    case VK_COMPARE_OP_ALWAYS:        return GEN6_COMPAREFUNCTION_ALWAYS;
    default:
      assert(!"unknown compare_func");
      return GEN6_COMPAREFUNCTION_NEVER;
    }
}

static int translate_stencil_op(VkStencilOp op)
{
    switch (op) {
    case VK_STENCIL_OP_KEEP:       return GEN6_STENCILOP_KEEP;
    case VK_STENCIL_OP_ZERO:       return GEN6_STENCILOP_ZERO;
    case VK_STENCIL_OP_REPLACE:    return GEN6_STENCILOP_REPLACE;
    case VK_STENCIL_OP_INC_CLAMP:  return GEN6_STENCILOP_INCRSAT;
    case VK_STENCIL_OP_DEC_CLAMP:  return GEN6_STENCILOP_DECRSAT;
    case VK_STENCIL_OP_INVERT:     return GEN6_STENCILOP_INVERT;
    case VK_STENCIL_OP_INC_WRAP:   return GEN6_STENCILOP_INCR;
    case VK_STENCIL_OP_DEC_WRAP:   return GEN6_STENCILOP_DECR;
    default:
      assert(!"unknown stencil op");
      return GEN6_STENCILOP_KEEP;
    }
}

struct intel_pipeline_create_info {
    VkGraphicsPipelineCreateInfo         graphics;
    VkPipelineVertexInputStateCreateInfo vi;
    VkPipelineIaStateCreateInfo          ia;
    VkPipelineDsStateCreateInfo          db;
    VkPipelineCbStateCreateInfo          cb;
    VkPipelineRsStateCreateInfo          rs;
    VkPipelineTessStateCreateInfo        tess;
    VkPipelineMsStateCreateInfo          ms;
    VkPipelineVpStateCreateInfo          vp;

    VkComputePipelineCreateInfo          compute;

    VkPipelineShaderStageCreateInfo      vs;
    VkPipelineShaderStageCreateInfo      tcs;
    VkPipelineShaderStageCreateInfo      tes;
    VkPipelineShaderStageCreateInfo      gs;
    VkPipelineShaderStageCreateInfo      fs;
};

/* in S1.3 */
struct intel_pipeline_sample_position {
    int8_t x, y;
};

static uint8_t pack_sample_position(const struct intel_dev *dev,
                                    const struct intel_pipeline_sample_position *pos)
{
    return (pos->x + 8) << 4 | (pos->y + 8);
}

void intel_pipeline_init_default_sample_patterns(const struct intel_dev *dev,
                                                 uint8_t *pat_1x, uint8_t *pat_2x,
                                                 uint8_t *pat_4x, uint8_t *pat_8x,
                                                 uint8_t *pat_16x)
{
    static const struct intel_pipeline_sample_position default_1x[1] = {
        {  0,  0 },
    };
    static const struct intel_pipeline_sample_position default_2x[2] = {
        { -4, -4 },
        {  4,  4 },
    };
    static const struct intel_pipeline_sample_position default_4x[4] = {
        { -2, -6 },
        {  6, -2 },
        { -6,  2 },
        {  2,  6 },
    };
    static const struct intel_pipeline_sample_position default_8x[8] = {
        { -1,  1 },
        {  1,  5 },
        {  3, -5 },
        {  5,  3 },
        { -7, -1 },
        { -3, -7 },
        {  7, -3 },
        { -5,  7 },
    };
    static const struct intel_pipeline_sample_position default_16x[16] = {
        {  0,  2 },
        {  3,  0 },
        { -3, -2 },
        { -2, -4 },
        {  4,  3 },
        {  5,  1 },
        {  6, -1 },
        {  2, -6 },
        { -4,  5 },
        { -5, -5 },
        { -1, -7 },
        {  7, -3 },
        { -7,  4 },
        {  1, -8 },
        { -6,  6 },
        { -8,  7 },
    };
    int i;

    pat_1x[0] = pack_sample_position(dev, default_1x);
    for (i = 0; i < 2; i++)
        pat_2x[i] = pack_sample_position(dev, &default_2x[i]);
    for (i = 0; i < 4; i++)
        pat_4x[i] = pack_sample_position(dev, &default_4x[i]);
    for (i = 0; i < 8; i++)
        pat_8x[i] = pack_sample_position(dev, &default_8x[i]);
    for (i = 0; i < 16; i++)
        pat_16x[i] = pack_sample_position(dev, &default_16x[i]);
}

struct intel_pipeline_shader *intel_pipeline_shader_create_meta(struct intel_dev *dev,
                                                                enum intel_dev_meta_shader id)
{
    struct intel_pipeline_shader *sh;
    VkResult ret;

    sh = intel_alloc(dev, sizeof(*sh), 0, VK_SYSTEM_ALLOC_TYPE_INTERNAL);
    if (!sh)
        return NULL;
    memset(sh, 0, sizeof(*sh));

    ret = intel_pipeline_shader_compile_meta(sh, dev->gpu, id);
    if (ret != VK_SUCCESS) {
        intel_free(dev, sh);
        return NULL;
    }

    switch (id) {
    case INTEL_DEV_META_VS_FILL_MEM:
    case INTEL_DEV_META_VS_COPY_MEM:
    case INTEL_DEV_META_VS_COPY_MEM_UNALIGNED:
        sh->max_threads = intel_gpu_get_max_threads(dev->gpu,
                VK_SHADER_STAGE_VERTEX);
        break;
    default:
        sh->max_threads = intel_gpu_get_max_threads(dev->gpu,
                VK_SHADER_STAGE_FRAGMENT);
        break;
    }

    return sh;
}

void intel_pipeline_shader_destroy(struct intel_dev *dev,
                                   struct intel_pipeline_shader *sh)
{
    intel_pipeline_shader_cleanup(sh, dev->gpu);
    intel_free(dev, sh);
}

static VkResult pipeline_build_shader(struct intel_pipeline *pipeline,
                                        const VkPipelineShaderStageCreateInfo *sh_info,
                                        struct intel_pipeline_shader *sh)
{
    VkResult ret;

    const struct intel_ir* ir = intel_shader(sh_info->shader)->ir;

    ret = intel_pipeline_shader_compile(sh,
            pipeline->dev->gpu, pipeline->pipeline_layout, sh_info, ir);

    if (ret != VK_SUCCESS)
        return ret;

    sh->max_threads =
        intel_gpu_get_max_threads(pipeline->dev->gpu, sh_info->stage);

    /* 1KB aligned */
    sh->scratch_offset = u_align(pipeline->scratch_size, 1024);
    pipeline->scratch_size = sh->scratch_offset +
        sh->per_thread_scratch_size * sh->max_threads;

    pipeline->active_shaders |= 1 << sh_info->stage;

    return VK_SUCCESS;
}

static VkResult pipeline_build_shaders(struct intel_pipeline *pipeline,
                                         const struct intel_pipeline_create_info *info)
{
    VkResult ret = VK_SUCCESS;

    if (ret == VK_SUCCESS && info->vs.shader.handle)
        ret = pipeline_build_shader(pipeline, &info->vs, &pipeline->vs);
    if (ret == VK_SUCCESS && info->tcs.shader.handle)
        ret = pipeline_build_shader(pipeline, &info->tcs,&pipeline->tcs);
    if (ret == VK_SUCCESS && info->tes.shader.handle)
        ret = pipeline_build_shader(pipeline, &info->tes,&pipeline->tes);
    if (ret == VK_SUCCESS && info->gs.shader.handle)
        ret = pipeline_build_shader(pipeline, &info->gs, &pipeline->gs);
    if (ret == VK_SUCCESS && info->fs.shader.handle)
        ret = pipeline_build_shader(pipeline, &info->fs, &pipeline->fs);

    if (ret == VK_SUCCESS && info->compute.cs.shader.handle) {
        ret = pipeline_build_shader(pipeline,
                &info->compute.cs, &pipeline->cs);
    }

    return ret;
}
static uint32_t *pipeline_cmd_ptr(struct intel_pipeline *pipeline, int cmd_len)
{
    uint32_t *ptr;

    assert(pipeline->cmd_len + cmd_len < INTEL_PSO_CMD_ENTRIES);
    ptr = &pipeline->cmds[pipeline->cmd_len];
    pipeline->cmd_len += cmd_len;
    return ptr;
}

static VkResult pipeline_build_ia(struct intel_pipeline *pipeline,
                                    const struct intel_pipeline_create_info* info)
{
    pipeline->topology = info->ia.topology;
    pipeline->disable_vs_cache = info->ia.disableVertexReuse;

    switch (info->ia.topology) {
    case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
        pipeline->prim_type = GEN6_3DPRIM_POINTLIST;
        break;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST:
        pipeline->prim_type = GEN6_3DPRIM_LINELIST;
        break;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP:
        pipeline->prim_type = GEN6_3DPRIM_LINESTRIP;
        break;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
        pipeline->prim_type = GEN6_3DPRIM_TRILIST;
        break;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
        pipeline->prim_type = GEN6_3DPRIM_TRISTRIP;
        break;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
        pipeline->prim_type = GEN6_3DPRIM_TRIFAN;
        break;
    case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_ADJ:
        pipeline->prim_type = GEN6_3DPRIM_LINELIST_ADJ;
        break;
    case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_ADJ:
        pipeline->prim_type = GEN6_3DPRIM_LINESTRIP_ADJ;
        break;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_ADJ:
        pipeline->prim_type = GEN6_3DPRIM_TRILIST_ADJ;
        break;
    case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_ADJ:
        pipeline->prim_type = GEN6_3DPRIM_TRISTRIP_ADJ;
        break;
    case VK_PRIMITIVE_TOPOLOGY_PATCH:
        if (!info->tess.patchControlPoints ||
            info->tess.patchControlPoints > 32)
            return VK_ERROR_BAD_PIPELINE_DATA;
        pipeline->prim_type = GEN7_3DPRIM_PATCHLIST_1 +
            info->tess.patchControlPoints - 1;
        break;
    default:
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    if (info->ia.primitiveRestartEnable) {
        pipeline->primitive_restart = true;
        pipeline->primitive_restart_index = info->ia.primitiveRestartIndex;
    } else {
        pipeline->primitive_restart = false;
    }

    return VK_SUCCESS;
}

static VkResult pipeline_build_rs_state(struct intel_pipeline *pipeline,
                                          const struct intel_pipeline_create_info* info)
{
    const VkPipelineRsStateCreateInfo *rs_state = &info->rs;
    bool ccw;

    pipeline->depthClipEnable = rs_state->depthClipEnable;
    pipeline->rasterizerDiscardEnable = rs_state->rasterizerDiscardEnable;

    if (rs_state->provokingVertex == VK_PROVOKING_VERTEX_FIRST) {
        pipeline->provoking_vertex_tri = 0;
        pipeline->provoking_vertex_trifan = 1;
        pipeline->provoking_vertex_line = 0;
    } else {
        pipeline->provoking_vertex_tri = 2;
        pipeline->provoking_vertex_trifan = 2;
        pipeline->provoking_vertex_line = 1;
    }

    switch (rs_state->fillMode) {
    case VK_FILL_MODE_POINTS:
        pipeline->cmd_sf_fill |= GEN7_SF_DW1_FRONTFACE_POINT |
                              GEN7_SF_DW1_BACKFACE_POINT;
        break;
    case VK_FILL_MODE_WIREFRAME:
        pipeline->cmd_sf_fill |= GEN7_SF_DW1_FRONTFACE_WIREFRAME |
                              GEN7_SF_DW1_BACKFACE_WIREFRAME;
        break;
    case VK_FILL_MODE_SOLID:
    default:
        pipeline->cmd_sf_fill |= GEN7_SF_DW1_FRONTFACE_SOLID |
                              GEN7_SF_DW1_BACKFACE_SOLID;
        break;
    }

    ccw = (rs_state->frontFace == VK_FRONT_FACE_CCW);
    /* flip the winding order */
    if (info->vp.clipOrigin == VK_COORDINATE_ORIGIN_LOWER_LEFT)
        ccw = !ccw;

    if (ccw) {
        pipeline->cmd_sf_fill |= GEN7_SF_DW1_FRONTWINDING_CCW;
        pipeline->cmd_clip_cull |= GEN7_CLIP_DW1_FRONTWINDING_CCW;
    }

    switch (rs_state->cullMode) {
    case VK_CULL_MODE_NONE:
    default:
        pipeline->cmd_sf_cull |= GEN7_SF_DW2_CULLMODE_NONE;
        pipeline->cmd_clip_cull |= GEN7_CLIP_DW1_CULLMODE_NONE;
        break;
    case VK_CULL_MODE_FRONT:
        pipeline->cmd_sf_cull |= GEN7_SF_DW2_CULLMODE_FRONT;
        pipeline->cmd_clip_cull |= GEN7_CLIP_DW1_CULLMODE_FRONT;
        break;
    case VK_CULL_MODE_BACK:
        pipeline->cmd_sf_cull |= GEN7_SF_DW2_CULLMODE_BACK;
        pipeline->cmd_clip_cull |= GEN7_CLIP_DW1_CULLMODE_BACK;
        break;
    case VK_CULL_MODE_FRONT_AND_BACK:
        pipeline->cmd_sf_cull |= GEN7_SF_DW2_CULLMODE_BOTH;
        pipeline->cmd_clip_cull |= GEN7_CLIP_DW1_CULLMODE_BOTH;
        break;
    }

    /* only GEN7+ needs cull mode in 3DSTATE_CLIP */
    if (intel_gpu_gen(pipeline->dev->gpu) == INTEL_GEN(6))
        pipeline->cmd_clip_cull = 0;

    return VK_SUCCESS;
}

static void pipeline_destroy(struct intel_obj *obj)
{
    struct intel_pipeline *pipeline = intel_pipeline_from_obj(obj);

    if (pipeline->active_shaders & SHADER_VERTEX_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->vs, pipeline->dev->gpu);
    }

    if (pipeline->active_shaders & SHADER_TESS_CONTROL_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->tcs, pipeline->dev->gpu);
    }

    if (pipeline->active_shaders & SHADER_TESS_EVAL_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->tes, pipeline->dev->gpu);
    }

    if (pipeline->active_shaders & SHADER_GEOMETRY_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->gs, pipeline->dev->gpu);
    }

    if (pipeline->active_shaders & SHADER_FRAGMENT_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->fs, pipeline->dev->gpu);
    }

    if (pipeline->active_shaders & SHADER_COMPUTE_FLAG) {
        intel_pipeline_shader_cleanup(&pipeline->cs, pipeline->dev->gpu);
    }

    intel_base_destroy(&pipeline->obj.base);
}

static VkResult pipeline_validate(struct intel_pipeline *pipeline)
{
    /*
     * Validate required elements
     */
    if (!(pipeline->active_shaders & SHADER_VERTEX_FLAG)) {
        // TODO: Log debug message: Vertex Shader required.
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    /*
     * Tessalation control and evaluation have to both have a shader defined or
     * neither should have a shader defined.
     */
    if (((pipeline->active_shaders & SHADER_TESS_CONTROL_FLAG) == 0) !=
         ((pipeline->active_shaders & SHADER_TESS_EVAL_FLAG) == 0) ) {
        // TODO: Log debug message: Both Tess control and Tess eval are required to use tessalation
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    if ((pipeline->active_shaders & SHADER_COMPUTE_FLAG) &&
        (pipeline->active_shaders & (SHADER_VERTEX_FLAG | SHADER_TESS_CONTROL_FLAG |
                                     SHADER_TESS_EVAL_FLAG | SHADER_GEOMETRY_FLAG |
                                     SHADER_FRAGMENT_FLAG))) {
        // TODO: Log debug message: Can only specify compute shader when doing compute
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    /*
     * VK_PRIMITIVE_TOPOLOGY_PATCH primitive topology is only valid for tessellation pipelines.
     * Mismatching primitive topology and tessellation fails graphics pipeline creation.
     */
    if (pipeline->active_shaders & (SHADER_TESS_CONTROL_FLAG | SHADER_TESS_EVAL_FLAG) &&
        (pipeline->topology != VK_PRIMITIVE_TOPOLOGY_PATCH)) {
        // TODO: Log debug message: Invalid topology used with tessellation shader.
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    if ((pipeline->topology == VK_PRIMITIVE_TOPOLOGY_PATCH) &&
        (~pipeline->active_shaders & (SHADER_TESS_CONTROL_FLAG | SHADER_TESS_EVAL_FLAG))) {
        // TODO: Log debug message: Cannot use TOPOLOGY_PATCH on non-tessellation shader.
        return VK_ERROR_BAD_PIPELINE_DATA;
    }

    return VK_SUCCESS;
}

static void pipeline_build_urb_alloc_gen6(struct intel_pipeline *pipeline,
                                          const struct intel_pipeline_create_info *info)
{
    const struct intel_gpu *gpu = pipeline->dev->gpu;
    const int urb_size = ((gpu->gt == 2) ? 64 : 32) * 1024;
    const struct intel_pipeline_shader *vs = &pipeline->vs;
    const struct intel_pipeline_shader *gs = &pipeline->gs;
    int vs_entry_size, gs_entry_size;
    int vs_size, gs_size;

    INTEL_GPU_ASSERT(gpu, 6, 6);

    vs_entry_size = ((vs->in_count >= vs->out_count) ?
        vs->in_count : vs->out_count);
    gs_entry_size = (gs) ? gs->out_count : 0;

    /* in bytes */
    vs_entry_size *= sizeof(float) * 4;
    gs_entry_size *= sizeof(float) * 4;

    if (pipeline->active_shaders & SHADER_GEOMETRY_FLAG) {
        vs_size = urb_size / 2;
        gs_size = vs_size;
    } else {
        vs_size = urb_size;
        gs_size = 0;
    }

    /* 3DSTATE_URB */
    {
        const uint8_t cmd_len = 3;
        const uint32_t dw0 = GEN6_RENDER_CMD(3D, 3DSTATE_URB) |
                             (cmd_len - 2);
        int vs_alloc_size, gs_alloc_size;
        int vs_entry_count, gs_entry_count;
        uint32_t *dw;

        /* in 1024-bit rows */
        vs_alloc_size = (vs_entry_size + 128 - 1) / 128;
        gs_alloc_size = (gs_entry_size + 128 - 1) / 128;

        /* valid range is [1, 5] */
        if (!vs_alloc_size)
            vs_alloc_size = 1;
        if (!gs_alloc_size)
            gs_alloc_size = 1;
        assert(vs_alloc_size <= 5 && gs_alloc_size <= 5);

        /* valid range is [24, 256], multiples of 4 */
        vs_entry_count = (vs_size / 128 / vs_alloc_size) & ~3;
        if (vs_entry_count > 256)
            vs_entry_count = 256;
        assert(vs_entry_count >= 24);

        /* valid range is [0, 256], multiples of 4 */
        gs_entry_count = (gs_size / 128 / gs_alloc_size) & ~3;
        if (gs_entry_count > 256)
            gs_entry_count = 256;

        dw = pipeline_cmd_ptr(pipeline, cmd_len);

        dw[0] = dw0;
        dw[1] = (vs_alloc_size - 1) << GEN6_URB_DW1_VS_ENTRY_SIZE__SHIFT |
                vs_entry_count << GEN6_URB_DW1_VS_ENTRY_COUNT__SHIFT;
        dw[2] = gs_entry_count << GEN6_URB_DW2_GS_ENTRY_COUNT__SHIFT |
                (gs_alloc_size - 1) << GEN6_URB_DW2_GS_ENTRY_SIZE__SHIFT;
    }
}

static void pipeline_build_urb_alloc_gen7(struct intel_pipeline *pipeline,
                                          const struct intel_pipeline_create_info *info)
{
    const struct intel_gpu *gpu = pipeline->dev->gpu;
    const int urb_size = ((gpu->gt == 3) ? 512 :
                          (gpu->gt == 2) ? 256 : 128) * 1024;
    const struct intel_pipeline_shader *vs = &pipeline->vs;
    const struct intel_pipeline_shader *gs = &pipeline->gs;
    /* some space is reserved for PCBs */
    int urb_offset = ((gpu->gt == 3) ? 32 : 16) * 1024;
    int vs_entry_size, gs_entry_size;
    int vs_size, gs_size;

    INTEL_GPU_ASSERT(gpu, 7, 7.5);

    vs_entry_size = ((vs->in_count >= vs->out_count) ?
        vs->in_count : vs->out_count);
    gs_entry_size = (gs) ? gs->out_count : 0;

    /* in bytes */
    vs_entry_size *= sizeof(float) * 4;
    gs_entry_size *= sizeof(float) * 4;

    if (pipeline->active_shaders & SHADER_GEOMETRY_FLAG) {
        vs_size = (urb_size - urb_offset) / 2;
        gs_size = vs_size;
    } else {
        vs_size = urb_size - urb_offset;
        gs_size = 0;
    }

    /* 3DSTATE_URB_* */
    {
        const uint8_t cmd_len = 2;
        int vs_alloc_size, gs_alloc_size;
        int vs_entry_count, gs_entry_count;
        uint32_t *dw;

        /* in 512-bit rows */
        vs_alloc_size = (vs_entry_size + 64 - 1) / 64;
        gs_alloc_size = (gs_entry_size + 64 - 1) / 64;

        if (!vs_alloc_size)
            vs_alloc_size = 1;
        if (!gs_alloc_size)
            gs_alloc_size = 1;

        /* avoid performance decrease due to banking */
        if (vs_alloc_size == 5)
            vs_alloc_size = 6;

        /* in multiples of 8 */
        vs_entry_count = (vs_size / 64 / vs_alloc_size) & ~7;
        assert(vs_entry_count >= 32);

        gs_entry_count = (gs_size / 64 / gs_alloc_size) & ~7;

        if (intel_gpu_gen(gpu) >= INTEL_GEN(7.5)) {
            const int max_vs_entry_count =
                (gpu->gt >= 2) ? 1664 : 640;
            const int max_gs_entry_count =
                (gpu->gt >= 2) ? 640 : 256;
            if (vs_entry_count >= max_vs_entry_count)
                vs_entry_count = max_vs_entry_count;
            if (gs_entry_count >= max_gs_entry_count)
                gs_entry_count = max_gs_entry_count;
        } else {
            const int max_vs_entry_count =
                (gpu->gt == 2) ? 704 : 512;
            const int max_gs_entry_count =
                (gpu->gt == 2) ? 320 : 192;
            if (vs_entry_count >= max_vs_entry_count)
                vs_entry_count = max_vs_entry_count;
            if (gs_entry_count >= max_gs_entry_count)
                gs_entry_count = max_gs_entry_count;
        }

        dw = pipeline_cmd_ptr(pipeline, cmd_len*4);
        dw[0] = GEN7_RENDER_CMD(3D, 3DSTATE_URB_VS) | (cmd_len - 2);
        dw[1] = (urb_offset / 8192) << GEN7_URB_DW1_OFFSET__SHIFT |
                (vs_alloc_size - 1) << GEN7_URB_DW1_ENTRY_SIZE__SHIFT |
                vs_entry_count;

        dw += 2;
        if (gs_size)
            urb_offset += vs_size;
        dw[0] = GEN7_RENDER_CMD(3D, 3DSTATE_URB_GS) | (cmd_len - 2);
        dw[1] = (urb_offset  / 8192) << GEN7_URB_DW1_OFFSET__SHIFT |
                (gs_alloc_size - 1) << GEN7_URB_DW1_ENTRY_SIZE__SHIFT |
                gs_entry_count;

        dw += 2;
        dw[0] = GEN7_RENDER_CMD(3D, 3DSTATE_URB_HS) | (cmd_len - 2);
        dw[1] = (urb_offset / 8192)  << GEN7_URB_DW1_OFFSET__SHIFT;

        dw += 2;
        dw[0] = GEN7_RENDER_CMD(3D, 3DSTATE_URB_DS) | (cmd_len - 2);
        dw[1] = (urb_offset / 8192)  << GEN7_URB_DW1_OFFSET__SHIFT;
    }
}

static void pipeline_build_vertex_elements(struct intel_pipeline *pipeline,
                                           const struct intel_pipeline_create_info *info)
{
    const struct intel_pipeline_shader *vs = &pipeline->vs;
    uint8_t cmd_len;
    uint32_t *dw;
    uint32_t i, j;
    uint32_t attr_count;
    uint32_t attrs_processed;
    int comps[4];

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 6, 7.5);

    attr_count = u_popcountll(vs->inputs_read);
    cmd_len = 1 + 2 * attr_count;
    if (vs->uses & (INTEL_SHADER_USE_VID | INTEL_SHADER_USE_IID))
        cmd_len += 2;

    if (cmd_len == 1)
        return;

    dw = pipeline_cmd_ptr(pipeline, cmd_len);

    dw[0] = GEN6_RENDER_CMD(3D, 3DSTATE_VERTEX_ELEMENTS) |
            (cmd_len - 2);
    dw++;

    /* VERTEX_ELEMENT_STATE */
    for (i = 0, attrs_processed = 0; attrs_processed < attr_count; i++) {
        VkVertexInputAttributeDescription *attr = NULL;

        /*
         * The compiler will pack the shader references and then
         * indicate which locations are used via the bitmask in
         * vs->inputs_read.
         */
        if (!(vs->inputs_read & (1L << i))) {
            continue;
        }

        /*
         * For each bit set in the vs->inputs_read we'll need
         * to find the corresponding attribute record and then
         * set up the next HW vertex element based on that attribute.
         */
        for (j = 0; j < info->vi.attributeCount; j++) {
            if (info->vi.pVertexAttributeDescriptions[j].location == i) {
                attr = (VkVertexInputAttributeDescription *) &info->vi.pVertexAttributeDescriptions[j];
                attrs_processed++;
                break;
            }
        }
        assert(attr != NULL);

        const int format =
            intel_format_translate_color(pipeline->dev->gpu, attr->format);

        comps[0] = GEN6_VFCOMP_STORE_0;
        comps[1] = GEN6_VFCOMP_STORE_0;
        comps[2] = GEN6_VFCOMP_STORE_0;
        comps[3] = icd_format_is_int(attr->format) ?
            GEN6_VFCOMP_STORE_1_INT : GEN6_VFCOMP_STORE_1_FP;

        switch (icd_format_get_channel_count(attr->format)) {
        case 4: comps[3] = GEN6_VFCOMP_STORE_SRC; /* fall through */
        case 3: comps[2] = GEN6_VFCOMP_STORE_SRC; /* fall through */
        case 2: comps[1] = GEN6_VFCOMP_STORE_SRC; /* fall through */
        case 1: comps[0] = GEN6_VFCOMP_STORE_SRC; break;
        default:
            break;
        }

        assert(attr->offsetInBytes <= 2047);

        dw[0] = attr->binding << GEN6_VE_DW0_VB_INDEX__SHIFT |
                GEN6_VE_DW0_VALID |
                format << GEN6_VE_DW0_FORMAT__SHIFT |
                attr->offsetInBytes;

        dw[1] = comps[0] << GEN6_VE_DW1_COMP0__SHIFT |
                comps[1] << GEN6_VE_DW1_COMP1__SHIFT |
                comps[2] << GEN6_VE_DW1_COMP2__SHIFT |
                comps[3] << GEN6_VE_DW1_COMP3__SHIFT;

        dw += 2;
    }

    if (vs->uses & (INTEL_SHADER_USE_VID | INTEL_SHADER_USE_IID)) {
        comps[0] = (vs->uses & INTEL_SHADER_USE_VID) ?
            GEN6_VFCOMP_STORE_VID : GEN6_VFCOMP_STORE_0;
        comps[1] = (vs->uses & INTEL_SHADER_USE_IID) ?
            GEN6_VFCOMP_STORE_IID : GEN6_VFCOMP_NOSTORE;
        comps[2] = GEN6_VFCOMP_NOSTORE;
        comps[3] = GEN6_VFCOMP_NOSTORE;

        dw[0] = GEN6_VE_DW0_VALID;
        dw[1] = comps[0] << GEN6_VE_DW1_COMP0__SHIFT |
                comps[1] << GEN6_VE_DW1_COMP1__SHIFT |
                comps[2] << GEN6_VE_DW1_COMP2__SHIFT |
                comps[3] << GEN6_VE_DW1_COMP3__SHIFT;

        dw += 2;
    }
}

static void pipeline_build_fragment_SBE(struct intel_pipeline *pipeline,
                                        const struct intel_pipeline_create_info *info)
{
    const struct intel_pipeline_shader *fs = &pipeline->fs;
    uint8_t cmd_len;
    uint32_t *body;
    uint32_t attr_skip, attr_count;
    uint32_t vue_offset, vue_len;
    uint32_t i;

    // If GS is active, use its outputs
    const struct intel_pipeline_shader *src =
            (pipeline->active_shaders & SHADER_GEOMETRY_FLAG)
                    ? &pipeline->gs
                    : &pipeline->vs;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 6, 7.5);

    cmd_len = 14;

    if (intel_gpu_gen(pipeline->dev->gpu) >= INTEL_GEN(7))
        body = pipeline_cmd_ptr(pipeline, cmd_len);
    else
        body = pipeline->cmd_3dstate_sbe;

    assert(!fs->reads_user_clip || src->enable_user_clip);
    attr_skip = src->outputs_offset;
    if (src->enable_user_clip != fs->reads_user_clip) {
        attr_skip += 2;
    }
    assert(src->out_count >= attr_skip);
    attr_count = src->out_count - attr_skip;

    // LUNARG TODO: We currently are only handling 16 attrs;
    // ultimately, we need to handle 32
    assert(fs->in_count <= 16);
    assert(attr_count <= 16);

    vue_offset = attr_skip / 2;
    vue_len = (attr_count + 1) / 2;
    if (!vue_len)
        vue_len = 1;

    body[0] = GEN7_RENDER_CMD(3D, 3DSTATE_SBE) |
            (cmd_len - 2);

    // LUNARG TODO: If the attrs needed by the FS are exactly
    // what is written by the VS, we don't need to enable
    // swizzling, improving performance. Even if we swizzle,
    // we can improve performance by reducing vue_len to
    // just include the values needed by the FS:
    // vue_len = ceiling((max_vs_out + 1)/2)

    body[1] = GEN7_SBE_DW1_ATTR_SWIZZLE_ENABLE |
          fs->in_count << GEN7_SBE_DW1_ATTR_COUNT__SHIFT |
          vue_len << GEN7_SBE_DW1_URB_READ_LEN__SHIFT |
          vue_offset << GEN7_SBE_DW1_URB_READ_OFFSET__SHIFT;

    switch (info->rs.pointOrigin) {
    case VK_COORDINATE_ORIGIN_UPPER_LEFT:
        body[1] |= GEN7_SBE_DW1_POINT_SPRITE_TEXCOORD_UPPERLEFT;
        break;
    case VK_COORDINATE_ORIGIN_LOWER_LEFT:
        body[1] |= GEN7_SBE_DW1_POINT_SPRITE_TEXCOORD_LOWERLEFT;
        break;
    default:
        assert(!"unknown point origin");
        break;
    }

    uint16_t src_slot[fs->in_count];
    int32_t fs_in = 0;
    int32_t src_out = - (vue_offset * 2 - src->outputs_offset);
    for (i=0; i < 64; i++) {
        bool srcWrites = src->outputs_written & (1L << i);
        bool fsReads   = fs->inputs_read      & (1L << i);

        if (fsReads) {
            assert(src_out >= 0);
            assert(fs_in < fs->in_count);
            src_slot[fs_in] = src_out;

            if (!srcWrites) {
                // If the vertex shader did not write this input, we cannot
                // program the SBE to read it.  Our choices are to allow it to
                // read junk from a GRF, or get zero.  We're choosing zero.
                if (i >= fs->generic_input_start) {
                    src_slot[fs_in] = GEN8_SBE_SWIZ_CONST_0000 |
                                     GEN8_SBE_SWIZ_OVERRIDE_X |
                                     GEN8_SBE_SWIZ_OVERRIDE_Y |
                                     GEN8_SBE_SWIZ_OVERRIDE_Z |
                                     GEN8_SBE_SWIZ_OVERRIDE_W;
                }
            }

            fs_in += 1;
        }
        if (srcWrites) {
            src_out += 1;
        }
    }

    for (i = 0; i < 8; i++) {
        uint16_t hi, lo;

        /* no attr swizzles */
        if (i * 2 + 1 < fs->in_count) {
            lo = src_slot[i * 2];
            hi = src_slot[i * 2 + 1];
        } else if (i * 2 < fs->in_count) {
            lo = src_slot[i * 2];
            hi = 0;
        } else {
            hi = 0;
            lo = 0;
        }

        body[2 + i] = hi << GEN8_SBE_SWIZ_HIGH__SHIFT | lo;
    }

    if (info->ia.topology == VK_PRIMITIVE_TOPOLOGY_POINT_LIST)
        body[10] = fs->point_sprite_enables;
    else
        body[10] = 0;

    body[11] = 0; /* constant interpolation enables */
    body[12] = 0; /* WrapShortest enables */
    body[13] = 0;
}

static void pipeline_build_gs(struct intel_pipeline *pipeline,
                              const struct intel_pipeline_create_info *info)
{
    // gen7_emit_3DSTATE_GS done by cmd_pipeline
}

static void pipeline_build_hs(struct intel_pipeline *pipeline,
                              const struct intel_pipeline_create_info *info)
{
    const uint8_t cmd_len = 7;
    const uint32_t dw0 = GEN7_RENDER_CMD(3D, 3DSTATE_HS) | (cmd_len - 2);
    uint32_t *dw;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 7, 7.5);

    dw = pipeline_cmd_ptr(pipeline, cmd_len);
    dw[0] = dw0;
    dw[1] = 0;
    dw[2] = 0;
    dw[3] = 0;
    dw[4] = 0;
    dw[5] = 0;
    dw[6] = 0;
}

static void pipeline_build_te(struct intel_pipeline *pipeline,
                              const struct intel_pipeline_create_info *info)
{
    const uint8_t cmd_len = 4;
    const uint32_t dw0 = GEN7_RENDER_CMD(3D, 3DSTATE_TE) | (cmd_len - 2);
    uint32_t *dw;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 7, 7.5);

    dw = pipeline_cmd_ptr(pipeline, cmd_len);
    dw[0] = dw0;
    dw[1] = 0;
    dw[2] = 0;
    dw[3] = 0;
}

static void pipeline_build_ds(struct intel_pipeline *pipeline,
                              const struct intel_pipeline_create_info *info)
{
    const uint8_t cmd_len = 6;
    const uint32_t dw0 = GEN7_RENDER_CMD(3D, 3DSTATE_DS) | (cmd_len - 2);
    uint32_t *dw;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 7, 7.5);

    dw = pipeline_cmd_ptr(pipeline, cmd_len);
    dw[0] = dw0;
    dw[1] = 0;
    dw[2] = 0;
    dw[3] = 0;
    dw[4] = 0;
    dw[5] = 0;
}

static void pipeline_build_depth_stencil(struct intel_pipeline *pipeline,
                                         const struct intel_pipeline_create_info *info)
{
    pipeline->cmd_depth_stencil = 0;

    if (info->db.stencilTestEnable) {
        pipeline->cmd_depth_stencil = 1 << 31 |
               translate_compare_func(info->db.front.stencilCompareOp) << 28 |
               translate_stencil_op(info->db.front.stencilFailOp) << 25 |
               translate_stencil_op(info->db.front.stencilDepthFailOp) << 22 |
               translate_stencil_op(info->db.front.stencilPassOp) << 19 |
               1 << 15 |
               translate_compare_func(info->db.back.stencilCompareOp) << 12 |
               translate_stencil_op(info->db.back.stencilFailOp) << 9 |
               translate_stencil_op(info->db.back.stencilDepthFailOp) << 6 |
               translate_stencil_op(info->db.back.stencilPassOp) << 3;
     }

    pipeline->stencilTestEnable = info->db.stencilTestEnable;

    /*
     * From the Sandy Bridge PRM, volume 2 part 1, page 360:
     *
     *     "Enabling the Depth Test function without defining a Depth Buffer is
     *      UNDEFINED."
     *
     * From the Sandy Bridge PRM, volume 2 part 1, page 375:
     *
     *     "A Depth Buffer must be defined before enabling writes to it, or
     *      operation is UNDEFINED."
     *
     * TODO We do not check these yet.
     */
    if (info->db.depthTestEnable) {
       pipeline->cmd_depth_test = GEN6_ZS_DW2_DEPTH_TEST_ENABLE |
               translate_compare_func(info->db.depthCompareOp) << 27;
    } else {
       pipeline->cmd_depth_test = GEN6_COMPAREFUNCTION_ALWAYS << 27;
    }

    if (info->db.depthWriteEnable)
       pipeline->cmd_depth_test |= GEN6_ZS_DW2_DEPTH_WRITE_ENABLE;
}

static void pipeline_build_msaa(struct intel_pipeline *pipeline,
                                const struct intel_pipeline_create_info *info)
{
    uint32_t cmd, cmd_len;
    uint32_t *dw;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 6, 7.5);

    pipeline->sample_count = (info->ms.rasterSamples <= 1) ? 1 : info->ms.rasterSamples;

    /* 3DSTATE_SAMPLE_MASK */
    cmd = GEN6_RENDER_CMD(3D, 3DSTATE_SAMPLE_MASK);
    cmd_len = 2;

    dw = pipeline_cmd_ptr(pipeline, cmd_len);
    dw[0] = cmd | (cmd_len - 2);
    dw[1] = info->ms.sampleMask & ((1 << pipeline->sample_count) - 1);
    pipeline->cmd_sample_mask = dw[1];
}

static void pipeline_build_cb(struct intel_pipeline *pipeline,
                              const struct intel_pipeline_create_info *info)
{
    uint32_t i;

    INTEL_GPU_ASSERT(pipeline->dev->gpu, 6, 7.5);
    STATIC_ASSERT(ARRAY_SIZE(pipeline->cmd_cb) >= INTEL_MAX_RENDER_TARGETS*2);
    assert(info->cb.attachmentCount <= INTEL_MAX_RENDER_TARGETS);

    uint32_t *dw = pipeline->cmd_cb;

    for (i = 0; i < info->cb.attachmentCount; i++) {
        const VkPipelineCbAttachmentState *att = &info->cb.pAttachments[i];
        uint32_t dw0, dw1;


        dw0 = 0;
        dw1 = GEN6_RT_DW1_COLORCLAMP_RTFORMAT |
              GEN6_RT_DW1_PRE_BLEND_CLAMP |
              GEN6_RT_DW1_POST_BLEND_CLAMP;

        if (att->blendEnable) {
            dw0 = 1 << 31 |
                    translate_blend_func(att->blendOpAlpha) << 26 |
                    translate_blend(att->srcBlendAlpha) << 20 |
                    translate_blend(att->destBlendAlpha) << 15 |
                    translate_blend_func(att->blendOpColor) << 11 |
                    translate_blend(att->srcBlendColor) << 5 |
                    translate_blend(att->destBlendColor);

            if (att->blendOpAlpha != att->blendOpColor ||
                att->srcBlendAlpha != att->srcBlendColor ||
                att->destBlendAlpha != att->destBlendColor)
                dw0 |= 1 << 30;

            pipeline->dual_source_blend_enable = icd_pipeline_cb_att_needs_dual_source_blending(att);
        }

        if (info->cb.logicOpEnable && info->cb.logicOp != VK_LOGIC_OP_COPY) {
            int logicop;

            switch (info->cb.logicOp) {
            case VK_LOGIC_OP_CLEAR:            logicop = GEN6_LOGICOP_CLEAR; break;
            case VK_LOGIC_OP_AND:              logicop = GEN6_LOGICOP_AND; break;
            case VK_LOGIC_OP_AND_REVERSE:      logicop = GEN6_LOGICOP_AND_REVERSE; break;
            case VK_LOGIC_OP_AND_INVERTED:     logicop = GEN6_LOGICOP_AND_INVERTED; break;
            case VK_LOGIC_OP_NOOP:             logicop = GEN6_LOGICOP_NOOP; break;
            case VK_LOGIC_OP_XOR:              logicop = GEN6_LOGICOP_XOR; break;
            case VK_LOGIC_OP_OR:               logicop = GEN6_LOGICOP_OR; break;
            case VK_LOGIC_OP_NOR:              logicop = GEN6_LOGICOP_NOR; break;
            case VK_LOGIC_OP_EQUIV:            logicop = GEN6_LOGICOP_EQUIV; break;
            case VK_LOGIC_OP_INVERT:           logicop = GEN6_LOGICOP_INVERT; break;
            case VK_LOGIC_OP_OR_REVERSE:       logicop = GEN6_LOGICOP_OR_REVERSE; break;
            case VK_LOGIC_OP_COPY_INVERTED:    logicop = GEN6_LOGICOP_COPY_INVERTED; break;
            case VK_LOGIC_OP_OR_INVERTED:      logicop = GEN6_LOGICOP_OR_INVERTED; break;
            case VK_LOGIC_OP_NAND:             logicop = GEN6_LOGICOP_NAND; break;
            case VK_LOGIC_OP_SET:              logicop = GEN6_LOGICOP_SET; break;
            default:
                assert(!"unknown logic op");
                logicop = GEN6_LOGICOP_CLEAR;
                break;
            }

            dw1 |= GEN6_RT_DW1_LOGICOP_ENABLE |
                   logicop << GEN6_RT_DW1_LOGICOP_FUNC__SHIFT;
        }

        if (!(att->channelWriteMask & 0x1))
            dw1 |= GEN6_RT_DW1_WRITE_DISABLE_R;
        if (!(att->channelWriteMask & 0x2))
            dw1 |= GEN6_RT_DW1_WRITE_DISABLE_G;
        if (!(att->channelWriteMask & 0x4))
            dw1 |= GEN6_RT_DW1_WRITE_DISABLE_B;
        if (!(att->channelWriteMask & 0x8))
            dw1 |= GEN6_RT_DW1_WRITE_DISABLE_A;

        dw[2 * i] = dw0;
        dw[2 * i + 1] = dw1;
    }

    for (i=info->cb.attachmentCount; i < INTEL_MAX_RENDER_TARGETS; i++)
    {
        dw[2 * i] = 0;
        dw[2 * i + 1] = GEN6_RT_DW1_COLORCLAMP_RTFORMAT |
                GEN6_RT_DW1_PRE_BLEND_CLAMP |
                GEN6_RT_DW1_POST_BLEND_CLAMP |
                GEN6_RT_DW1_WRITE_DISABLE_R |
                GEN6_RT_DW1_WRITE_DISABLE_G |
                GEN6_RT_DW1_WRITE_DISABLE_B |
                GEN6_RT_DW1_WRITE_DISABLE_A;
    }

}


static VkResult pipeline_build_all(struct intel_pipeline *pipeline,
                                     const struct intel_pipeline_create_info *info)
{
    VkResult ret;

    ret = pipeline_build_shaders(pipeline, info);
    if (ret != VK_SUCCESS)
        return ret;

    if (info->vi.bindingCount > ARRAY_SIZE(pipeline->vb) ||
        info->vi.attributeCount > ARRAY_SIZE(pipeline->vb))
        return VK_ERROR_BAD_PIPELINE_DATA;

    if (info->vp.clipOrigin != VK_COORDINATE_ORIGIN_UPPER_LEFT) {
        assert(!"only VK_COORDINATE_ORIGIN_UPPER_LEFT is supported");
        return VK_ERROR_INVALID_VALUE;
    }

    if (info->vp.depthMode != VK_DEPTH_MODE_ZERO_TO_ONE) {
        assert(!"only VK_DEPTH_MODE_ZERO_TO_ONE is supported");
        return VK_ERROR_INVALID_VALUE;
    }

    pipeline->vb_count = info->vi.bindingCount;
    memcpy(pipeline->vb, info->vi.pVertexBindingDescriptions,
            sizeof(pipeline->vb[0]) * pipeline->vb_count);

    pipeline_build_vertex_elements(pipeline, info);
    pipeline_build_fragment_SBE(pipeline, info);
    pipeline_build_msaa(pipeline, info);
    pipeline_build_depth_stencil(pipeline, info);

    if (intel_gpu_gen(pipeline->dev->gpu) >= INTEL_GEN(7)) {
        pipeline_build_urb_alloc_gen7(pipeline, info);
        pipeline_build_gs(pipeline, info);
        pipeline_build_hs(pipeline, info);
        pipeline_build_te(pipeline, info);
        pipeline_build_ds(pipeline, info);

        pipeline->wa_flags = INTEL_CMD_WA_GEN6_PRE_DEPTH_STALL_WRITE |
                             INTEL_CMD_WA_GEN6_PRE_COMMAND_SCOREBOARD_STALL |
                             INTEL_CMD_WA_GEN7_PRE_VS_DEPTH_STALL_WRITE |
                             INTEL_CMD_WA_GEN7_POST_COMMAND_CS_STALL |
                             INTEL_CMD_WA_GEN7_POST_COMMAND_DEPTH_STALL;
    } else {
        pipeline_build_urb_alloc_gen6(pipeline, info);

        pipeline->wa_flags = INTEL_CMD_WA_GEN6_PRE_DEPTH_STALL_WRITE |
                             INTEL_CMD_WA_GEN6_PRE_COMMAND_SCOREBOARD_STALL;
    }

    ret = pipeline_build_ia(pipeline, info);

    if (ret == VK_SUCCESS)
        ret = pipeline_build_rs_state(pipeline, info);

    if (ret == VK_SUCCESS) {
        pipeline_build_cb(pipeline, info);
        pipeline->cb_state = info->cb;
        pipeline->tess_state = info->tess;
    }

    return ret;
}

static VkResult pipeline_create_info_init(struct intel_pipeline_create_info  *info,
                                          const VkGraphicsPipelineCreateInfo *vkinfo)
{
    memset(info, 0, sizeof(*info));

    /*
     * Do we need to set safe defaults in case the app doesn't provide all of
     * the necessary create infos?
     */
    info->ms.rasterSamples    = 1;
    info->ms.sampleMask = 1;

    memcpy(&info->graphics, vkinfo, sizeof (info->graphics));

    void *dst;
    for (uint32_t i = 0; i < vkinfo->stageCount; i++) {
        const VkPipelineShaderStageCreateInfo *thisStage = &vkinfo->pStages[i];
        switch (thisStage->stage) {
            case VK_SHADER_STAGE_VERTEX:
                dst = &info->vs;
                break;
            case VK_SHADER_STAGE_TESS_CONTROL:
                dst = &info->tcs;
                break;
            case VK_SHADER_STAGE_TESS_EVALUATION:
                dst = &info->tes;
                break;
            case VK_SHADER_STAGE_GEOMETRY:
                dst = &info->gs;
                break;
            case VK_SHADER_STAGE_FRAGMENT:
                dst = &info->fs;
                break;
            case VK_SHADER_STAGE_COMPUTE:
                dst = &info->compute;
                break;
            default:
                return VK_ERROR_BAD_PIPELINE_DATA;
                break;
        }
        memcpy(dst, thisStage, sizeof(VkPipelineShaderStageCreateInfo));
    }

    if (vkinfo->pVertexInputState != NULL) {
        memcpy(&info->vi, vkinfo->pVertexInputState, sizeof (info->vi));
    }
    if (vkinfo->pIaState != NULL) {
        memcpy(&info->ia, vkinfo->pIaState, sizeof (info->ia));
    }
    if (vkinfo->pDsState != NULL) {
        memcpy(&info->db, vkinfo->pDsState, sizeof (info->db));
    }
    if (vkinfo->pCbState != NULL) {
        memcpy(&info->cb, vkinfo->pCbState, sizeof (info->cb));
    }
    if (vkinfo->pRsState != NULL) {
        memcpy(&info->rs, vkinfo->pRsState, sizeof (info->rs));
    }
    if (vkinfo->pTessState != NULL) {
        memcpy(&info->tess, vkinfo->pTessState, sizeof (info->tess));
    }
    if (vkinfo->pMsState != NULL) {
        memcpy(&info->ms, vkinfo->pMsState, sizeof (info->ms));
    }
    if (vkinfo->pVpState != NULL) {
        memcpy(&info->vp, vkinfo->pVpState, sizeof (info->vp));
    }
    if (vkinfo->pVpState != NULL) {
        memcpy(&info->vp, vkinfo->pVpState, sizeof (info->vp));
    }

    return VK_SUCCESS;
}

static VkResult graphics_pipeline_create(struct intel_dev *dev,
                                         const VkGraphicsPipelineCreateInfo *info_,
                                         struct intel_pipeline **pipeline_ret)
{
    struct intel_pipeline_create_info info;
    struct intel_pipeline *pipeline;
    VkResult ret;

    ret = pipeline_create_info_init(&info, info_);

    if (ret != VK_SUCCESS)
        return ret;

    pipeline = (struct intel_pipeline *) intel_base_create(&dev->base.handle,
                        sizeof (*pipeline), dev->base.dbg,
                        VK_OBJECT_TYPE_PIPELINE, info_, 0);
    if (!pipeline)
        return VK_ERROR_OUT_OF_HOST_MEMORY;

    pipeline->dev = dev;
    pipeline->pipeline_layout = intel_pipeline_layout(info.graphics.layout);

    pipeline->obj.destroy = pipeline_destroy;

    ret = pipeline_build_all(pipeline, &info);
    if (ret == VK_SUCCESS)
        ret = pipeline_validate(pipeline);
    if (ret != VK_SUCCESS) {
        pipeline_destroy(&pipeline->obj);
        return ret;
    }

    VkMemoryAllocInfo mem_reqs;
    mem_reqs.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO;
    mem_reqs.allocationSize = pipeline->scratch_size;
    mem_reqs.pNext = NULL;
    mem_reqs.memoryTypeIndex = 0;
    intel_mem_alloc(dev, &mem_reqs, &pipeline->obj.mem);

    *pipeline_ret = pipeline;
    return VK_SUCCESS;
}

ICD_EXPORT VkResult VKAPI vkCreatePipelineCache(
    VkDevice                                    device,
    const VkPipelineCacheCreateInfo*            pCreateInfo,
    VkPipelineCache*                            pPipelineCache)
{

    // non-dispatchable objects only need to be 64 bits currently
    *((uint64_t *)pPipelineCache) = 1;
    return VK_SUCCESS;
}

VkResult VKAPI vkDestroyPipelineCache(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache)
{
    return VK_SUCCESS;
}

ICD_EXPORT size_t VKAPI vkGetPipelineCacheSize(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache)
{
    return VK_ERROR_UNAVAILABLE;
}

ICD_EXPORT VkResult VKAPI vkGetPipelineCacheData(
    VkDevice                                    device,
    VkPipelineCache                             pipelineCache,
    void*                                       pData)
{
    return VK_ERROR_UNAVAILABLE;
}

ICD_EXPORT VkResult VKAPI vkMergePipelineCaches(
    VkDevice                                    device,
    VkPipelineCache                             destCache,
    uint32_t                                    srcCacheCount,
    const VkPipelineCache*                      pSrcCaches)
{
    return VK_ERROR_UNAVAILABLE;
}

ICD_EXPORT VkResult VKAPI vkCreateGraphicsPipelines(
    VkDevice                                  device,
    VkPipelineCache                           pipelineCache,
    uint32_t                                  count,
    const VkGraphicsPipelineCreateInfo*       pCreateInfos,
    VkPipeline*                               pPipelines)
{
    struct intel_dev *dev = intel_dev(device);
    uint32_t i;
    VkResult res;
    bool one_succeeded = false;

    for (i = 0; i < count; i++) {
        res =  graphics_pipeline_create(dev, &(pCreateInfos[i]),
            (struct intel_pipeline **) &(pPipelines[i]));
        //return NULL handle for unsuccessful creates
        if (res != VK_SUCCESS)
            pPipelines[i].handle = 0;
        else
            one_succeeded = true;
    }
    //return VK_SUCCESS if any of count creates succeeded
    if (one_succeeded)
        return VK_SUCCESS;
    else
        return res;
}

ICD_EXPORT VkResult VKAPI vkCreateComputePipelines(
    VkDevice                                  device,
    VkPipelineCache                           pipelineCache,
    uint32_t                                  count,
    const VkComputePipelineCreateInfo*        pCreateInfos,
    VkPipeline*                               pPipelines)
{
    return VK_ERROR_UNAVAILABLE;
}

ICD_EXPORT VkResult VKAPI vkDestroyPipeline(
    VkDevice                                device,
    VkPipeline                              pipeline)

 {
    struct intel_obj *obj = intel_obj(pipeline.handle);

    intel_mem_free(obj->mem);
    obj->destroy(obj);
    return VK_SUCCESS;
 }
