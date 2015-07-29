/*
 * Vulkan Tests
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
 */

#include "vkrenderframework.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

VkRenderFramework::VkRenderFramework() :
    m_cmdBuffer(),
    m_renderPass(VK_NULL_HANDLE),
    m_framebuffer(VK_NULL_HANDLE),
    m_stateRaster( VK_NULL_HANDLE ),
    m_colorBlend( VK_NULL_HANDLE ),
    m_stateViewport( VK_NULL_HANDLE ),
    m_stateDepthStencil( VK_NULL_HANDLE ),
    m_width( 256.0 ),                   // default window width
    m_height( 256.0 ),                  // default window height
    m_render_target_fmt( VK_FORMAT_R8G8B8A8_UNORM ),
    m_depth_stencil_fmt( VK_FORMAT_UNDEFINED ),
    m_clear_via_load_op( true ),
    m_depth_clear_color( 1.0 ),
    m_stencil_clear_color( 0 ),
    m_depthStencil( NULL ),
    m_dbgCreateMsgCallback( VK_NULL_HANDLE ),
    m_dbgDestroyMsgCallback( VK_NULL_HANDLE ),
    m_globalMsgCallback( VK_NULL_HANDLE ),
    m_devMsgCallback( VK_NULL_HANDLE )
{

    memset(&m_renderPassBeginInfo, 0, sizeof(m_renderPassBeginInfo));
    m_renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;

    // clear the back buffer to dark grey
    m_clear_color.f32[0] = 0.25f;
    m_clear_color.f32[1] = 0.25f;
    m_clear_color.f32[2] = 0.25f;
    m_clear_color.f32[3] = 0.0f;
}

VkRenderFramework::~VkRenderFramework()
{

}

void VkRenderFramework::InitFramework()
{
    std::vector<const char*> instance_layer_names;
    std::vector<const char*> device_layer_names;
    std::vector<const char*> instance_extension_names;
    std::vector<const char*> device_extension_names;
    InitFramework(
                instance_layer_names, device_layer_names,
                instance_extension_names, device_extension_names);
}

void VkRenderFramework::InitFramework(
        std::vector<const char *> instance_layer_names,
        std::vector<const char *> device_layer_names,
        std::vector<const char *> instance_extension_names,
        std::vector<const char *> device_extension_names,
        PFN_vkDbgMsgCallback dbgFunction,
        void *userData)
{
    VkInstanceCreateInfo instInfo = {};
    std::vector<VkExtensionProperties> instance_extensions;
    std::vector<VkExtensionProperties> device_extensions;
    VkResult U_ASSERT_ONLY err;

    /* TODO: Verify requested extensions are available */

    instInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    instInfo.pNext = NULL;
    instInfo.pAppInfo = &app_info;
    instInfo.pAllocCb = NULL;
    instInfo.layerCount = instance_layer_names.size();
    instInfo.ppEnabledLayerNames = instance_layer_names.data();
    instInfo.extensionCount = instance_extension_names.size();
    instInfo.ppEnabledExtensionNames = instance_extension_names.data();
    err = vkCreateInstance(&instInfo, &this->inst);
    ASSERT_VK_SUCCESS(err);

    err = vkEnumeratePhysicalDevices(inst, &this->gpu_count, NULL);
    ASSERT_LE(this->gpu_count, ARRAY_SIZE(objs)) << "Too many gpus";
    ASSERT_VK_SUCCESS(err);
    err = vkEnumeratePhysicalDevices(inst, &this->gpu_count, objs);
    ASSERT_VK_SUCCESS(err);
    ASSERT_GE(this->gpu_count, 1) << "No GPU available";
    if (dbgFunction) {
        m_dbgCreateMsgCallback = (PFN_vkDbgCreateMsgCallback) vkGetInstanceProcAddr(this->inst, "vkDbgCreateMsgCallback");
        ASSERT_NE(m_dbgCreateMsgCallback, (PFN_vkDbgCreateMsgCallback) NULL) << "Did not get function pointer for DbgCreateMsgCallback";
        if (m_dbgCreateMsgCallback) {
            err = m_dbgCreateMsgCallback(this->inst,
                                         VK_DBG_REPORT_ERROR_BIT | VK_DBG_REPORT_WARN_BIT,
                                         dbgFunction,
                                         userData,
                                         &m_globalMsgCallback);
            ASSERT_VK_SUCCESS(err);

            m_dbgDestroyMsgCallback = (PFN_vkDbgDestroyMsgCallback) vkGetInstanceProcAddr(this->inst, "vkDbgDestroyMsgCallback");
            ASSERT_NE(m_dbgDestroyMsgCallback, (PFN_vkDbgDestroyMsgCallback) NULL) << "Did not get function pointer for DbgDestroyMsgCallback";
        }
    }

    /* TODO: Verify requested physical device extensions are available */
    m_device = new VkDeviceObj(0, objs[0], device_layer_names, device_extension_names);

    /* Now register callback on device */
    if (0) {
        if (m_dbgCreateMsgCallback) {
            err = m_dbgCreateMsgCallback(this->inst,
                                         VK_DBG_REPORT_ERROR_BIT | VK_DBG_REPORT_WARN_BIT,
                                         dbgFunction,
                                         userData,
                                         &m_devMsgCallback);
            ASSERT_VK_SUCCESS(err);
        }
    }
    m_device->get_device_queue();

    m_depthStencil = new VkDepthStencilObj();
}

void VkRenderFramework::ShutdownFramework()
{
    if (m_colorBlend) vkDestroyDynamicColorBlendState(device(), m_colorBlend);
    if (m_stateDepthStencil) vkDestroyDynamicDepthStencilState(device(), m_stateDepthStencil);
    if (m_stateRaster) vkDestroyDynamicRasterState(device(), m_stateRaster);
    if (m_cmdBuffer)
        delete m_cmdBuffer;
    if (m_cmdPool) vkDestroyCommandPool(device(), m_cmdPool);
    if (m_framebuffer) vkDestroyFramebuffer(device(), m_framebuffer);
    if (m_renderPass) vkDestroyRenderPass(device(), m_renderPass);

    if (m_globalMsgCallback) m_dbgDestroyMsgCallback(this->inst, m_globalMsgCallback);
    if (m_devMsgCallback) m_dbgDestroyMsgCallback(this->inst, m_devMsgCallback);

    if (m_stateViewport) {
        vkDestroyDynamicViewportState(device(), m_stateViewport);
    }
    while (!m_renderTargets.empty()) {
        vkDestroyAttachmentView(device(), m_renderTargets.back()->targetView());
        vkDestroyImage(device(), m_renderTargets.back()->image());
        vkFreeMemory(device(), m_renderTargets.back()->memory());
        m_renderTargets.pop_back();
    }

    delete m_depthStencil;
    while (!m_shader_modules.empty())
    {
        delete m_shader_modules.back();
        m_shader_modules.pop_back();
    }

    // reset the driver
    delete m_device;
    if (this->inst) vkDestroyInstance(this->inst);
}

void VkRenderFramework::InitState()
{
    VkResult err;

    m_render_target_fmt = VK_FORMAT_B8G8R8A8_UNORM;

    // create a raster state (solid, back-face culling)
    VkDynamicRasterStateCreateInfo raster = {};
    raster.sType = VK_STRUCTURE_TYPE_DYNAMIC_RASTER_STATE_CREATE_INFO;

    err = vkCreateDynamicRasterState( device(), &raster, &m_stateRaster );
    ASSERT_VK_SUCCESS(err);

    VkDynamicColorBlendStateCreateInfo blend = {};
    blend.sType = VK_STRUCTURE_TYPE_DYNAMIC_COLOR_BLEND_STATE_CREATE_INFO;
    blend.blendConst[0] = 1.0f;
    blend.blendConst[1] = 1.0f;
    blend.blendConst[2] = 1.0f;
    blend.blendConst[3] = 1.0f;
    err = vkCreateDynamicColorBlendState(device(), &blend, &m_colorBlend);
    ASSERT_VK_SUCCESS( err );

    VkDynamicDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_DYNAMIC_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.minDepthBounds = 0.f;
    depthStencil.maxDepthBounds = 1.f;
    depthStencil.stencilFrontRef = 0;
    depthStencil.stencilBackRef = 0;
    depthStencil.stencilReadMask = 0xff;
    depthStencil.stencilWriteMask = 0xff;

    err = vkCreateDynamicDepthStencilState( device(), &depthStencil, &m_stateDepthStencil );
    ASSERT_VK_SUCCESS( err );

    VkCmdPoolCreateInfo cmd_pool_info;
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO,
    cmd_pool_info.pNext = NULL,
    cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
    cmd_pool_info.flags = 0,
    err = vkCreateCommandPool(device(), &cmd_pool_info, &m_cmdPool);
    assert(!err);

    m_cmdBuffer = new VkCommandBufferObj(m_device, m_cmdPool);
}

void VkRenderFramework::InitViewport(float width, float height)
{
    VkResult err;

    VkViewport viewport;
    VkRect2D scissor;

    VkDynamicViewportStateCreateInfo viewportCreate = {};
    viewportCreate.sType = VK_STRUCTURE_TYPE_DYNAMIC_VIEWPORT_STATE_CREATE_INFO;
    viewportCreate.viewportAndScissorCount         = 1;
    viewport.originX  = 0;
    viewport.originY  = 0;
    viewport.width    = 1.f * width;
    viewport.height   = 1.f * height;
    viewport.minDepth = 0.f;
    viewport.maxDepth = 1.f;
    scissor.extent.width = (int32_t) width;
    scissor.extent.height = (int32_t) height;
    scissor.offset.x = 0;
    scissor.offset.y = 0;
    viewportCreate.pViewports = &viewport;
    viewportCreate.pScissors = &scissor;

    err = vkCreateDynamicViewportState( device(), &viewportCreate, &m_stateViewport );
    ASSERT_VK_SUCCESS( err );
    m_width = width;
    m_height = height;
}

void VkRenderFramework::InitViewport()
{
    InitViewport(m_width, m_height);
}
void VkRenderFramework::InitRenderTarget()
{
    InitRenderTarget(1);
}

void VkRenderFramework::InitRenderTarget(uint32_t targets)
{
    InitRenderTarget(targets, NULL);
}

void VkRenderFramework::InitRenderTarget(VkAttachmentBindInfo *dsBinding)
{
    InitRenderTarget(1, dsBinding);
}

void VkRenderFramework::InitRenderTarget(uint32_t targets, VkAttachmentBindInfo *dsBinding)
{
    std::vector<VkAttachmentDescription> attachments;
    std::vector<VkAttachmentReference> color_references;
    std::vector<VkAttachmentBindInfo> bindings;
    attachments.reserve(targets + (bool) dsBinding);
    color_references.reserve(targets);
    bindings.reserve(targets + (bool) dsBinding);

    VkAttachmentDescription att = {};
    att.sType = VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION;
    att.pNext = NULL;
    att.format = m_render_target_fmt;
    att.samples = 1;
    att.loadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;
    att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    att.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    att.finalLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkAttachmentReference ref = {};
    ref.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    m_renderPassClearValues.clear();
    VkClearValue clear = {};
    clear.color = m_clear_color;

    VkAttachmentBindInfo bind = {};
    bind.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    for (uint32_t i = 0; i < targets; i++) {
        attachments.push_back(att);

        ref.attachment = i;
        color_references.push_back(ref);

        m_renderPassClearValues.push_back(clear);

        VkImageObj *img = new VkImageObj(m_device);

        VkFormatProperties props;
        VkResult err;

        err = vkGetPhysicalDeviceFormatProperties(m_device->phy().handle(), m_render_target_fmt, &props);
        ASSERT_VK_SUCCESS(err);

        if (props.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->init((uint32_t)m_width, (uint32_t)m_height, m_render_target_fmt,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT,
            VK_IMAGE_TILING_LINEAR);
        }
        else if (props.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
            img->init((uint32_t)m_width, (uint32_t)m_height, m_render_target_fmt,
            VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT,
            VK_IMAGE_TILING_OPTIMAL);
        }
        else {
            FAIL() << "Neither Linear nor Optimal allowed for render target";
        }

        m_renderTargets.push_back(img);
        bind.view  = img->targetView();

        bindings.push_back(bind);
    }

    VkSubpassDescription subpass = {};
    subpass.sType = VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION;
    subpass.pNext = NULL;
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.flags = 0;
    subpass.inputCount = 0;
    subpass.inputAttachments = NULL;
    subpass.colorCount = targets;
    subpass.colorAttachments = color_references.data();
    subpass.resolveAttachments = NULL;

    if (dsBinding) {
        att.format = m_depth_stencil_fmt;
        att.loadOp = (m_clear_via_load_op) ? VK_ATTACHMENT_LOAD_OP_CLEAR : VK_ATTACHMENT_LOAD_OP_LOAD;;
        att.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
        att.stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
        att.initialLayout = dsBinding->layout;
        att.finalLayout = dsBinding->layout;
        attachments.push_back(att);

        clear.ds.depth = m_depth_clear_color;
        clear.ds.stencil = m_stencil_clear_color;
        m_renderPassClearValues.push_back(clear);

        bindings.push_back(*dsBinding);

        subpass.depthStencilAttachment.attachment = targets;
        subpass.depthStencilAttachment.layout = dsBinding->layout;
    } else {
        subpass.depthStencilAttachment.attachment = VK_ATTACHMENT_UNUSED;
    }

    subpass.preserveCount = 0;
    subpass.preserveAttachments = NULL;

    VkRenderPassCreateInfo rp_info = {};
    rp_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    rp_info.attachmentCount = attachments.size();
    rp_info.pAttachments = attachments.data();
    rp_info.subpassCount = 1;
    rp_info.pSubpasses = &subpass;

    vkCreateRenderPass(device(), &rp_info, &m_renderPass);

    // Create Framebuffer and RenderPass with color attachments and any depth/stencil attachment
    VkFramebufferCreateInfo fb_info = {};
    fb_info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    fb_info.pNext = NULL;
    fb_info.renderPass = m_renderPass;
    fb_info.attachmentCount = bindings.size();
    fb_info.pAttachments = bindings.data();
    fb_info.width = (uint32_t)m_width;
    fb_info.height = (uint32_t)m_height;
    fb_info.layers = 1;

    vkCreateFramebuffer(device(), &fb_info, &m_framebuffer);

    m_renderPassBeginInfo.renderPass = m_renderPass;
    m_renderPassBeginInfo.framebuffer = m_framebuffer;
    m_renderPassBeginInfo.renderArea.extent.width = m_width;
    m_renderPassBeginInfo.renderArea.extent.height = m_height;
    m_renderPassBeginInfo.attachmentCount = m_renderPassClearValues.size();
    m_renderPassBeginInfo.pAttachmentClearValues = m_renderPassClearValues.data();
}



VkDeviceObj::VkDeviceObj(uint32_t id, VkPhysicalDevice obj) :
    vk_testing::Device(obj), id(id)
{
    init();

    props = phy().properties();
    queue_props = phy().queue_properties().data();
}

VkDeviceObj::VkDeviceObj(uint32_t id,
        VkPhysicalDevice obj, std::vector<const char *> &layer_names,
        std::vector<const char *> &extension_names) :
    vk_testing::Device(obj), id(id)
{
    init(layer_names, extension_names);

    props = phy().properties();
    queue_props = phy().queue_properties().data();
}

void VkDeviceObj::get_device_queue()
{
    ASSERT_NE(true, graphics_queues().empty());
    m_queue = graphics_queues()[0]->handle();
}

VkDescriptorSetObj::VkDescriptorSetObj(VkDeviceObj *device) :
        m_device(device), m_nextSlot(0)
{

}

VkDescriptorSetObj::~VkDescriptorSetObj()
{
    delete m_set;
}

int VkDescriptorSetObj::AppendDummy()
{
    /* request a descriptor but do not update it */
    VkDescriptorTypeCount tc = {};
    tc.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
    tc.count = 1;
    m_type_counts.push_back(tc);

    return m_nextSlot++;
}

int VkDescriptorSetObj::AppendBuffer(VkDescriptorType type, VkConstantBufferObj &constantBuffer)
{
    VkDescriptorTypeCount tc = {};
    tc.type = type;
    tc.count = 1;
    m_type_counts.push_back(tc);

    m_writes.push_back(vk_testing::Device::write_descriptor_set(vk_testing::DescriptorSet(),
                m_nextSlot, 0, type, 1, &constantBuffer.m_descriptorInfo));

    return m_nextSlot++;
}

int VkDescriptorSetObj::AppendSamplerTexture( VkSamplerObj* sampler, VkTextureObj* texture)
{
    VkDescriptorTypeCount tc = {};
    tc.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    tc.count = 1;
    m_type_counts.push_back(tc);

    VkDescriptorInfo tmp = texture->m_descriptorInfo;
    tmp.sampler = sampler->handle();
    m_imageSamplerDescriptors.push_back(tmp);

    m_writes.push_back(vk_testing::Device::write_descriptor_set(vk_testing::DescriptorSet(),
                m_nextSlot, 0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1, NULL));

    return m_nextSlot++;
}

VkPipelineLayout VkDescriptorSetObj::GetPipelineLayout() const
{
    return m_pipeline_layout.handle();
}

VkDescriptorSet VkDescriptorSetObj::GetDescriptorSetHandle() const
{
    return m_set->handle();
}

void VkDescriptorSetObj::CreateVKDescriptorSet(VkCommandBufferObj *cmdBuffer)
{
    // create VkDescriptorPool
    VkDescriptorPoolCreateInfo pool = {};
    pool.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool.count = m_type_counts.size();
    pool.pTypeCount = m_type_counts.data();
    init(*m_device, VK_DESCRIPTOR_POOL_USAGE_ONE_SHOT, 1, pool);

    // create VkDescriptorSetLayout
    vector<VkDescriptorSetLayoutBinding> bindings;
    bindings.resize(m_type_counts.size());
    for (int i = 0; i < m_type_counts.size(); i++) {
        bindings[i].descriptorType = m_type_counts[i].type;
        bindings[i].arraySize = m_type_counts[i].count;
        bindings[i].stageFlags = VK_SHADER_STAGE_ALL;
        bindings[i].pImmutableSamplers = NULL;
    }

    // create VkDescriptorSetLayout
    VkDescriptorSetLayoutCreateInfo layout = {};
    layout.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layout.count = bindings.size();
    layout.pBinding = bindings.data();

    m_layout.init(*m_device, layout);
    vector<const vk_testing::DescriptorSetLayout *> layouts;
    layouts.push_back(&m_layout);

    // create VkPipelineLayout
    VkPipelineLayoutCreateInfo pipeline_layout = {};
    pipeline_layout.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipeline_layout.descriptorSetCount = layouts.size();
    pipeline_layout.pSetLayouts = NULL;

    m_pipeline_layout.init(*m_device, pipeline_layout, layouts);

    // create VkDescriptorSet
    m_set = alloc_sets(*m_device, VK_DESCRIPTOR_SET_USAGE_STATIC, m_layout);

    // build the update array
    size_t imageSamplerCount = 0;
    for (std::vector<VkWriteDescriptorSet>::iterator it = m_writes.begin();
         it != m_writes.end(); it++) {
        it->destSet = m_set->handle();
        if (it->descriptorType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            it->pDescriptors = &m_imageSamplerDescriptors[imageSamplerCount++];
    }

    // do the updates
    m_device->update_descriptor_sets(m_writes);
}

VkImageObj::VkImageObj(VkDeviceObj *dev)
{
    m_device = dev;
    m_descriptorInfo.imageView = VK_NULL_HANDLE;
    m_descriptorInfo.imageLayout = VK_IMAGE_LAYOUT_GENERAL;
}

void VkImageObj::ImageMemoryBarrier(
        VkCommandBufferObj *cmd_buf,
        VkImageAspect aspect,
        VkFlags output_mask /*=
            VK_MEMORY_OUTPUT_HOST_WRITE_BIT |
            VK_MEMORY_OUTPUT_SHADER_WRITE_BIT |
            VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_COPY_BIT*/,
        VkFlags input_mask /*=
            VK_MEMORY_INPUT_HOST_READ_BIT |
            VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT |
            VK_MEMORY_INPUT_INDEX_FETCH_BIT |
            VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT |
            VK_MEMORY_INPUT_UNIFORM_READ_BIT |
            VK_MEMORY_INPUT_SHADER_READ_BIT |
            VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_COPY_BIT*/,
        VkImageLayout image_layout)
{
    const VkImageSubresourceRange subresourceRange = subresource_range(aspect, 0, 1, 0, 1);
    VkImageMemoryBarrier barrier;
    barrier = image_memory_barrier(output_mask, input_mask, layout(), image_layout,
                                   subresourceRange);

    VkImageMemoryBarrier *pmemory_barrier = &barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;

    // write barrier to the command buffer
    vkCmdPipelineBarrier(cmd_buf->handle(), src_stages, dest_stages, false, 1, (const void * const*)&pmemory_barrier);
}

void VkImageObj::SetLayout(VkCommandBufferObj *cmd_buf,
                         VkImageAspect aspect,
                         VkImageLayout image_layout)
{
    VkFlags output_mask, input_mask;
    const VkFlags all_cache_outputs =
            VK_MEMORY_OUTPUT_HOST_WRITE_BIT |
            VK_MEMORY_OUTPUT_SHADER_WRITE_BIT |
            VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_TRANSFER_BIT;
    const VkFlags all_cache_inputs =
            VK_MEMORY_INPUT_HOST_READ_BIT |
            VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT |
            VK_MEMORY_INPUT_INDEX_FETCH_BIT |
            VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT |
            VK_MEMORY_INPUT_UNIFORM_READ_BIT |
            VK_MEMORY_INPUT_SHADER_READ_BIT |
            VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_TRANSFER_BIT;

    if (image_layout == m_descriptorInfo.imageLayout) {
        return;
    }

    switch (image_layout) {
    case VK_IMAGE_LAYOUT_TRANSFER_SOURCE_OPTIMAL:
        output_mask = VK_MEMORY_OUTPUT_TRANSFER_BIT;
        input_mask = VK_MEMORY_INPUT_SHADER_READ_BIT | VK_MEMORY_INPUT_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_TRANSFER_DESTINATION_OPTIMAL:
        output_mask = VK_MEMORY_OUTPUT_TRANSFER_BIT;
        input_mask = VK_MEMORY_INPUT_SHADER_READ_BIT | VK_MEMORY_INPUT_TRANSFER_BIT;
        break;

    case VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL:
        output_mask = VK_MEMORY_OUTPUT_TRANSFER_BIT;
        input_mask = VK_MEMORY_INPUT_SHADER_READ_BIT | VK_MEMORY_INPUT_TRANSFER_BIT;
        break;

    default:
        output_mask =  all_cache_outputs;
        input_mask = all_cache_inputs;
        break;
    }

    ImageMemoryBarrier(cmd_buf, aspect, output_mask, input_mask, image_layout);
    m_descriptorInfo.imageLayout = image_layout;
}

void VkImageObj::SetLayout(VkImageAspect aspect,
                           VkImageLayout image_layout)
{
    VkResult U_ASSERT_ONLY err;

    if (image_layout == m_descriptorInfo.imageLayout) {
        return;
    }

    VkCmdPoolCreateInfo cmd_pool_info = {};
        cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO;
        cmd_pool_info.pNext = NULL;
        cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
        cmd_pool_info.flags = 0;
    vk_testing::CmdPool pool(*m_device, cmd_pool_info);
    VkCommandBufferObj cmd_buf(m_device, pool.handle());

    /* Build command buffer to set image layout in the driver */
    err = cmd_buf.BeginCommandBuffer();
    assert(!err);

    SetLayout(&cmd_buf, aspect, image_layout);

    err = cmd_buf.EndCommandBuffer();
    assert(!err);

    cmd_buf.QueueCommandBuffer();
}

bool VkImageObj::IsCompatible(VkFlags usage, VkFlags features)
{
    if ((usage & VK_IMAGE_USAGE_SAMPLED_BIT) &&
            !(features & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT))
        return false;

    return true;
}

void VkImageObj::init(uint32_t w, uint32_t h,
                      VkFormat fmt, VkFlags usage,
                      VkImageTiling requested_tiling,
                      VkMemoryPropertyFlags reqs)
{
    uint32_t mipCount;
    VkFormatProperties image_fmt;
    VkImageTiling tiling;
    VkResult err;

    mipCount = 0;

    uint32_t _w = w;
    uint32_t _h = h;
    while( ( _w > 0 ) || ( _h > 0 ) )
    {
        _w >>= 1;
        _h >>= 1;
        mipCount++;
    }

    err = vkGetPhysicalDeviceFormatProperties(m_device->phy().handle(), fmt, &image_fmt);
    ASSERT_VK_SUCCESS(err);

    if (requested_tiling == VK_IMAGE_TILING_LINEAR) {
        if (IsCompatible(usage, image_fmt.linearTilingFeatures)) {
            tiling = VK_IMAGE_TILING_LINEAR;
        } else if (IsCompatible(usage, image_fmt.optimalTilingFeatures)) {
            tiling = VK_IMAGE_TILING_OPTIMAL;
        } else {
            ASSERT_TRUE(false) << "Error: Cannot find requested tiling configuration";
        }
    } else if (IsCompatible(usage, image_fmt.optimalTilingFeatures)) {
        tiling = VK_IMAGE_TILING_OPTIMAL;
    } else if (IsCompatible(usage, image_fmt.linearTilingFeatures)) {
        tiling = VK_IMAGE_TILING_LINEAR;
    } else {
         ASSERT_TRUE(false) << "Error: Cannot find requested tiling configuration";
    }

    VkImageCreateInfo imageCreateInfo = vk_testing::Image::create_info();
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = fmt;
    imageCreateInfo.extent.width = w;
    imageCreateInfo.extent.height = h;
    imageCreateInfo.mipLevels = mipCount;
    imageCreateInfo.tiling = tiling;

    imageCreateInfo.usage = usage;

    vk_testing::Image::init(*m_device, imageCreateInfo, reqs);

    if (usage & VK_IMAGE_USAGE_SAMPLED_BIT) {
        SetLayout(VK_IMAGE_ASPECT_COLOR, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    } else {
        SetLayout(VK_IMAGE_ASPECT_COLOR, VK_IMAGE_LAYOUT_GENERAL);
    }
}

VkResult VkImageObj::CopyImage(VkImageObj &src_image)
{
    VkResult U_ASSERT_ONLY err;
    VkImageLayout src_image_layout, dest_image_layout;

    VkCmdPoolCreateInfo cmd_pool_info = {};
        cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO;
        cmd_pool_info.pNext = NULL;
        cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
        cmd_pool_info.flags = 0;
    vk_testing::CmdPool pool(*m_device, cmd_pool_info);
    VkCommandBufferObj cmd_buf(m_device, pool.handle());

    /* Build command buffer to copy staging texture to usable texture */
    err = cmd_buf.BeginCommandBuffer();
    assert(!err);

    /* TODO: Can we determine image aspect from image object? */
    src_image_layout = src_image.layout();
    src_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR, VK_IMAGE_LAYOUT_TRANSFER_SOURCE_OPTIMAL);

    dest_image_layout = this->layout();
    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR, VK_IMAGE_LAYOUT_TRANSFER_DESTINATION_OPTIMAL);

    VkImageCopy copy_region = {};
    copy_region.srcSubresource.aspect = VK_IMAGE_ASPECT_COLOR;
    copy_region.srcSubresource.arraySlice = 0;
    copy_region.srcSubresource.mipLevel = 0;
    copy_region.srcOffset.x = 0;
    copy_region.srcOffset.y = 0;
    copy_region.srcOffset.z = 0;
    copy_region.destSubresource.aspect = VK_IMAGE_ASPECT_COLOR;
    copy_region.destSubresource.arraySlice = 0;
    copy_region.destSubresource.mipLevel = 0;
    copy_region.destOffset.x = 0;
    copy_region.destOffset.y = 0;
    copy_region.destOffset.z = 0;
    copy_region.extent = src_image.extent();

    vkCmdCopyImage(cmd_buf.handle(),
                    src_image.handle(), src_image.layout(),
                    handle(), layout(),
                    1, &copy_region);

    src_image.SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR, src_image_layout);

    this->SetLayout(&cmd_buf, VK_IMAGE_ASPECT_COLOR, dest_image_layout);

    err = cmd_buf.EndCommandBuffer();
    assert(!err);

    cmd_buf.QueueCommandBuffer();

    return VK_SUCCESS;
}

VkTextureObj::VkTextureObj(VkDeviceObj *device, uint32_t *colors)
    :VkImageObj(device)
{
    m_device = device;
    const VkFormat tex_format = VK_FORMAT_B8G8R8A8_UNORM;
    uint32_t tex_colors[2] = { 0xffff0000, 0xff00ff00 };
    void *data;
    int32_t x, y;
    VkImageObj stagingImage(device);
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;

    stagingImage.init(16, 16, tex_format, VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT | VK_IMAGE_USAGE_TRANSFER_SOURCE_BIT, VK_IMAGE_TILING_LINEAR, reqs);
    VkSubresourceLayout layout = stagingImage.subresource_layout(subresource(VK_IMAGE_ASPECT_COLOR, 0, 0));

    if (colors == NULL)
        colors = tex_colors;

    memset(&m_descriptorInfo,0,sizeof(m_descriptorInfo));

    VkImageViewCreateInfo view = {};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    view.pNext = NULL;
    view.image = VK_NULL_HANDLE;
    view.viewType = VK_IMAGE_VIEW_TYPE_2D;
    view.format = tex_format;
    view.channels.r = VK_CHANNEL_SWIZZLE_R;
    view.channels.g = VK_CHANNEL_SWIZZLE_G;
    view.channels.b = VK_CHANNEL_SWIZZLE_B;
    view.channels.a = VK_CHANNEL_SWIZZLE_A;
    view.subresourceRange.aspect = VK_IMAGE_ASPECT_COLOR;
    view.subresourceRange.baseMipLevel = 0;
    view.subresourceRange.mipLevels = 1;
    view.subresourceRange.baseArraySlice = 0;
    view.subresourceRange.arraySize = 1;

    /* create image */
    init(16, 16, tex_format, VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_TRANSFER_DESTINATION_BIT, VK_IMAGE_TILING_OPTIMAL);

    /* create image view */
    view.image = handle();
    m_textureView.init(*m_device, view);
    m_descriptorInfo.imageView = m_textureView.handle();

    data = stagingImage.MapMemory();

    for (y = 0; y < extent().height; y++) {
        uint32_t *row = (uint32_t *) ((char *) data + layout.rowPitch * y);
        for (x = 0; x < extent().width; x++)
            row[x] = colors[(x & 1) ^ (y & 1)];
    }
    stagingImage.UnmapMemory();
    VkImageObj::CopyImage(stagingImage);
}

VkSamplerObj::VkSamplerObj(VkDeviceObj *device)
{
    m_device = device;

    VkSamplerCreateInfo samplerCreateInfo;
    memset(&samplerCreateInfo,0,sizeof(samplerCreateInfo));
    samplerCreateInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerCreateInfo.magFilter = VK_TEX_FILTER_NEAREST;
    samplerCreateInfo.minFilter = VK_TEX_FILTER_NEAREST;
    samplerCreateInfo.mipMode = VK_TEX_MIPMAP_MODE_BASE;
    samplerCreateInfo.addressU = VK_TEX_ADDRESS_WRAP;
    samplerCreateInfo.addressV = VK_TEX_ADDRESS_WRAP;
    samplerCreateInfo.addressW = VK_TEX_ADDRESS_WRAP;
    samplerCreateInfo.mipLodBias = 0.0;
    samplerCreateInfo.maxAnisotropy = 0;
    samplerCreateInfo.compareOp = VK_COMPARE_OP_NEVER;
    samplerCreateInfo.minLod = 0.0;
    samplerCreateInfo.maxLod = 0.0;
    samplerCreateInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    init(*m_device, samplerCreateInfo);
}

/*
 * Basic ConstantBuffer constructor. Then use create methods to fill in the details.
 */
VkConstantBufferObj::VkConstantBufferObj(VkDeviceObj *device)
{
    m_device = device;
    m_commandBuffer = 0;

    memset(&m_descriptorInfo,0,sizeof(m_descriptorInfo));
}

VkConstantBufferObj::~VkConstantBufferObj()
{
    // TODO: Should we call QueueRemoveMemReference for the constant buffer memory here?
    if (m_commandBuffer) {
        delete m_cmdPool;
        delete m_commandBuffer;
    }
}

VkConstantBufferObj::VkConstantBufferObj(VkDeviceObj *device, int constantCount, int constantSize, const void* data)
{
    m_device = device;
    m_commandBuffer = 0;

    memset(&m_descriptorInfo,0,sizeof(m_descriptorInfo));
    m_numVertices = constantCount;
    m_stride = constantSize;

    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    const size_t allocationSize = constantCount * constantSize;
    init_as_src_and_dst(*m_device, allocationSize, reqs);

    void *pData = memory().map();
    memcpy(pData, data, allocationSize);
    memory().unmap();

    // set up the buffer view for the constant buffer
    VkBufferViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    view_info.buffer = handle();
    view_info.viewType = VK_BUFFER_VIEW_TYPE_RAW;
    view_info.offset = 0;
    view_info.range  = allocationSize;
    m_bufferView.init(*m_device, view_info);

    this->m_descriptorInfo.bufferView = m_bufferView.handle();
}

void VkConstantBufferObj::Bind(VkCmdBuffer cmdBuffer, VkDeviceSize offset, uint32_t binding)
{
    vkCmdBindVertexBuffers(cmdBuffer, binding, 1, &handle(), &offset);
}


void VkConstantBufferObj::BufferMemoryBarrier(
        VkFlags outputMask /*=
            VK_MEMORY_OUTPUT_HOST_WRITE_BIT |
            VK_MEMORY_OUTPUT_SHADER_WRITE_BIT |
            VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_OUTPUT_COPY_BIT*/,
        VkFlags inputMask /*=
            VK_MEMORY_INPUT_HOST_READ_BIT |
            VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT |
            VK_MEMORY_INPUT_INDEX_FETCH_BIT |
            VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT |
            VK_MEMORY_INPUT_UNIFORM_READ_BIT |
            VK_MEMORY_INPUT_SHADER_READ_BIT |
            VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
            VK_MEMORY_INPUT_COPY_BIT*/)
{
    VkResult err = VK_SUCCESS;

    if (!m_commandBuffer)
    {
        m_fence.init(*m_device, vk_testing::Fence::create_info());
        VkCmdPoolCreateInfo cmd_pool_info = {};
            cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO;
            cmd_pool_info.pNext = NULL;
            cmd_pool_info.queueFamilyIndex = m_device->graphics_queue_node_index_;
            cmd_pool_info.flags = 0;
        m_cmdPool = new vk_testing::CmdPool(*m_device, cmd_pool_info);
        m_commandBuffer = new VkCommandBufferObj(m_device, m_cmdPool->handle());
    }
    else
    {
        m_device->wait(m_fence);
    }

    // open the command buffer
    VkCmdBufferBeginInfo cmd_buf_info = {};
    cmd_buf_info.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_BEGIN_INFO;
    cmd_buf_info.pNext = NULL;
    cmd_buf_info.flags = 0;

    err = m_commandBuffer->BeginCommandBuffer(&cmd_buf_info);
    ASSERT_VK_SUCCESS(err);

    VkBufferMemoryBarrier memory_barrier =
        buffer_memory_barrier(outputMask, inputMask, 0, m_numVertices * m_stride);
    VkBufferMemoryBarrier *pmemory_barrier = &memory_barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;

    // write barrier to the command buffer
    m_commandBuffer->PipelineBarrier(src_stages, dest_stages, false, 1, (const void **)&pmemory_barrier);

    // finish recording the command buffer
    err = m_commandBuffer->EndCommandBuffer();
    ASSERT_VK_SUCCESS(err);

    // submit the command buffer to the universal queue
    VkCmdBuffer bufferArray[1];
    bufferArray[0] = m_commandBuffer->GetBufferHandle();
    err = vkQueueSubmit( m_device->m_queue, 1, bufferArray, m_fence.handle() );
    ASSERT_VK_SUCCESS(err);
}

VkIndexBufferObj::VkIndexBufferObj(VkDeviceObj *device)
    : VkConstantBufferObj(device)
{

}

void VkIndexBufferObj::CreateAndInitBuffer(int numIndexes, VkIndexType indexType, const void* data)
{
    VkFormat viewFormat;

    m_numVertices = numIndexes;
    m_indexType = indexType;
    switch (indexType) {
    case VK_INDEX_TYPE_UINT16:
        m_stride = 2;
        viewFormat = VK_FORMAT_R16_UINT;
        break;
    case VK_INDEX_TYPE_UINT32:
        m_stride = 4;
        viewFormat = VK_FORMAT_R32_UINT;
        break;
    default:
        assert(!"unknown index type");
        m_stride = 2;
        viewFormat = VK_FORMAT_R16_UINT;
        break;
    }

    const size_t allocationSize = numIndexes * m_stride;
    VkMemoryPropertyFlags reqs = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
    init_as_src_and_dst(*m_device, allocationSize, reqs);

    void *pData = memory().map();
    memcpy(pData, data, allocationSize);
    memory().unmap();

    // set up the buffer view for the constant buffer
    VkBufferViewCreateInfo view_info = {};
    view_info.sType = VK_STRUCTURE_TYPE_BUFFER_VIEW_CREATE_INFO;
    view_info.buffer = handle();
    view_info.viewType = VK_BUFFER_VIEW_TYPE_FORMATTED;
    view_info.format = viewFormat;
    view_info.offset = 0;
    view_info.range  = allocationSize;
    m_bufferView.init(*m_device, view_info);

    this->m_descriptorInfo.bufferView = m_bufferView.handle();
}

void VkIndexBufferObj::Bind(VkCmdBuffer cmdBuffer, VkDeviceSize offset)
{
    vkCmdBindIndexBuffer(cmdBuffer, handle(), offset, m_indexType);
}

VkIndexType VkIndexBufferObj::GetIndexType()
{
    return m_indexType;
}

VkPipelineShaderStageCreateInfo* VkShaderObj::GetStageCreateInfo()
{
    VkPipelineShaderStageCreateInfo *stageInfo = (VkPipelineShaderStageCreateInfo*) calloc( 1,sizeof(VkPipelineShaderStageCreateInfo) );
    stageInfo->sType                = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    stageInfo->stage                = m_stage;
    stageInfo->shader               = handle();

    return stageInfo;
}

VkShaderObj::VkShaderObj(VkDeviceObj *device, const char * shader_code, VkShaderStage stage, VkRenderFramework *framework)
{
    VkResult U_ASSERT_ONLY err = VK_SUCCESS;
    std::vector<unsigned int> spv;
    VkShaderCreateInfo createInfo;
    VkShaderModuleCreateInfo moduleCreateInfo;
    vk_testing::ShaderModule *module = new vk_testing::ShaderModule();
    size_t shader_len;

    m_stage = stage;
    m_device = device;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO;
    createInfo.pNext = NULL;

    if (framework->m_use_glsl) {

        shader_len = strlen(shader_code);
        moduleCreateInfo.codeSize = 3 * sizeof(uint32_t) + shader_len + 1;
        moduleCreateInfo.pCode = malloc(moduleCreateInfo.codeSize);
        moduleCreateInfo.flags = 0;

        /* try version 0 first: VkShaderStage followed by GLSL */
        ((uint32_t *) moduleCreateInfo.pCode)[0] = ICD_SPV_MAGIC;
        ((uint32_t *) moduleCreateInfo.pCode)[1] = 0;
        ((uint32_t *) moduleCreateInfo.pCode)[2] = stage;
        memcpy(((uint32_t *) moduleCreateInfo.pCode + 3), shader_code, shader_len + 1);

    } else {

        // Use Reference GLSL to SPV compiler
        framework->GLSLtoSPV(stage, shader_code, spv);
        moduleCreateInfo.pCode = spv.data();
        moduleCreateInfo.codeSize = spv.size() * sizeof(unsigned int);
        moduleCreateInfo.flags = 0;
    }

    err = module->init_try(*m_device, moduleCreateInfo);
    assert(VK_SUCCESS == err);

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.module = module->handle();
    createInfo.pName = "main";
    createInfo.flags = 0;

    err = init_try(*m_device, createInfo);
    assert(VK_SUCCESS == err);
    framework->m_shader_modules.push_back(module);
}

VkPipelineObj::VkPipelineObj(VkDeviceObj *device)
{
    m_device = device;

    m_vi_state.pNext                        = VK_NULL_HANDLE;
    m_vi_state.bindingCount                 = 0;
    m_vi_state.pVertexBindingDescriptions   = VK_NULL_HANDLE;
    m_vi_state.attributeCount               = 0;
    m_vi_state.pVertexAttributeDescriptions = VK_NULL_HANDLE;

    m_vertexBufferCount = 0;

    m_ia_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    m_ia_state.pNext = VK_NULL_HANDLE;
    m_ia_state.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    m_ia_state.primitiveRestartEnable = VK_FALSE;

    m_rs_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTER_STATE_CREATE_INFO;
    m_rs_state.pNext = VK_NULL_HANDLE;
    m_rs_state.depthClipEnable = VK_FALSE;
    m_rs_state.rasterizerDiscardEnable = VK_FALSE;
    m_rs_state.fillMode = VK_FILL_MODE_SOLID;
    m_rs_state.cullMode = VK_CULL_MODE_BACK;
    m_rs_state.frontFace = VK_FRONT_FACE_CW;

    memset(&m_cb_state,0,sizeof(m_cb_state));
    m_cb_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    m_cb_state.pNext = VK_NULL_HANDLE;
    m_cb_state.alphaToCoverageEnable = VK_FALSE;
    m_cb_state.logicOp = VK_LOGIC_OP_COPY;

    m_ms_state.pNext = VK_NULL_HANDLE;
    m_ms_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    m_ms_state.sampleMask = 1;                // Do we have to specify MSAA even just to disable it?
    m_ms_state.rasterSamples = 1;
    m_ms_state.minSampleShading = 0;
    m_ms_state.sampleShadingEnable = 0;

    m_vp_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    m_vp_state.pNext = VK_NULL_HANDLE;
    m_vp_state.viewportCount = 1;

    m_ds_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    m_ds_state.pNext = VK_NULL_HANDLE,
    m_ds_state.depthTestEnable      = VK_FALSE;
    m_ds_state.depthWriteEnable     = VK_FALSE;
    m_ds_state.depthBoundsEnable    = VK_FALSE;
    m_ds_state.depthCompareOp = VK_COMPARE_OP_LESS_EQUAL;
    m_ds_state.back.stencilDepthFailOp = VK_STENCIL_OP_KEEP;
    m_ds_state.back.stencilFailOp = VK_STENCIL_OP_KEEP;
    m_ds_state.back.stencilPassOp = VK_STENCIL_OP_KEEP;
    m_ds_state.back.stencilCompareOp = VK_COMPARE_OP_ALWAYS;
    m_ds_state.stencilTestEnable = VK_FALSE;
    m_ds_state.front = m_ds_state.back;
};

void VkPipelineObj::AddShader(VkShaderObj* shader)
{
    m_shaderObjs.push_back(shader);
}

void VkPipelineObj::AddVertexInputAttribs(VkVertexInputAttributeDescription* vi_attrib, int count)
{
    m_vi_state.pVertexAttributeDescriptions = vi_attrib;
    m_vi_state.attributeCount = count;
}

void VkPipelineObj::AddVertexInputBindings(VkVertexInputBindingDescription* vi_binding, int count)
{
    m_vi_state.pVertexBindingDescriptions = vi_binding;
    m_vi_state.bindingCount = count;
}

void VkPipelineObj::AddVertexDataBuffer(VkConstantBufferObj* vertexDataBuffer, int binding)
{
    m_vertexBufferObjs.push_back(vertexDataBuffer);
    m_vertexBufferBindings.push_back(binding);
    m_vertexBufferCount++;
}

void VkPipelineObj::AddColorAttachment(uint32_t binding, const VkPipelineColorBlendAttachmentState *att)
{
    if (binding+1 > m_colorAttachments.size())
    {
        m_colorAttachments.resize(binding+1);
    }
    m_colorAttachments[binding] = *att;
}

void VkPipelineObj::SetDepthStencil(VkPipelineDepthStencilStateCreateInfo *ds_state)
{
    m_ds_state.depthTestEnable = ds_state->depthTestEnable;
    m_ds_state.depthWriteEnable = ds_state->depthWriteEnable;
    m_ds_state.depthBoundsEnable = ds_state->depthBoundsEnable;
    m_ds_state.depthCompareOp = ds_state->depthCompareOp;
    m_ds_state.stencilTestEnable = ds_state->stencilTestEnable;
    m_ds_state.back = ds_state->back;
    m_ds_state.front = ds_state->front;
}

VkResult VkPipelineObj::CreateVKPipeline(VkDescriptorSetObj &descriptorSet, VkRenderPass render_pass)
{
    VkGraphicsPipelineCreateInfo info = {};

    VkPipelineShaderStageCreateInfo* shaderCreateInfo;

    info.stageCount = m_shaderObjs.size();
    info.pStages = new VkPipelineShaderStageCreateInfo[info.stageCount];

    for (int i=0; i<m_shaderObjs.size(); i++)
    {
        shaderCreateInfo = m_shaderObjs[i]->GetStageCreateInfo();
        memcpy((void*)&info.pStages[i], shaderCreateInfo, sizeof(VkPipelineShaderStageCreateInfo));
    }

    if (m_vi_state.attributeCount && m_vi_state.bindingCount) {
        m_vi_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        info.pVertexInputState = &m_vi_state;
    } else {
        info.pVertexInputState = NULL;
    }

    info.sType  = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    info.pNext  = NULL;
    info.flags  = 0;
    info.layout = descriptorSet.GetPipelineLayout();

    m_cb_state.attachmentCount = m_colorAttachments.size();
    m_cb_state.pAttachments = m_colorAttachments.data();

    info.renderPass        = render_pass;
    info.subpass           = 0;
    info.pTessellationState   = NULL;
    info.pInputAssemblyState  = &m_ia_state;
    info.pViewportState       = &m_vp_state;
    info.pRasterState         = &m_rs_state;
    info.pMultisampleState    = &m_ms_state;
    info.pDepthStencilState   = &m_ds_state;
    info.pColorBlendState     = &m_cb_state;

    return init_try(*m_device, info);
}

VkCommandBufferObj::VkCommandBufferObj(VkDeviceObj *device, VkCmdPool pool)
{
    m_device = device;

    init(*device, vk_testing::CmdBuffer::create_info(pool));
}

VkCmdBuffer VkCommandBufferObj::GetBufferHandle()
{
    return handle();
}

VkResult VkCommandBufferObj::BeginCommandBuffer(VkCmdBufferBeginInfo *pInfo)
{
    begin(pInfo);
    return VK_SUCCESS;
}

VkResult VkCommandBufferObj::BeginCommandBuffer()
{
    begin();
    return VK_SUCCESS;
}

VkResult VkCommandBufferObj::EndCommandBuffer()
{
    end();
    return VK_SUCCESS;
}

void VkCommandBufferObj::PipelineBarrier(VkPipelineStageFlags src_stages,  VkPipelineStageFlags dest_stages, VkBool32 byRegion, uint32_t memBarrierCount, const void* const* ppMemBarriers)
{
    vkCmdPipelineBarrier(handle(), src_stages, dest_stages, byRegion, memBarrierCount, ppMemBarriers);
}

void VkCommandBufferObj::ClearAllBuffers(VkClearColorValue clear_color, float depth_clear_color, uint32_t stencil_clear_color,
                                          VkDepthStencilObj *depthStencilObj)
{
    uint32_t i;
    const VkFlags output_mask =
        VK_MEMORY_OUTPUT_HOST_WRITE_BIT |
        VK_MEMORY_OUTPUT_SHADER_WRITE_BIT |
        VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT |
        VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_MEMORY_OUTPUT_TRANSFER_BIT;
    const VkFlags input_mask = 0;

    // whatever we want to do, we do it to the whole buffer
    VkImageSubresourceRange srRange = {};
    srRange.aspect = VK_IMAGE_ASPECT_COLOR;
    srRange.baseMipLevel = 0;
    srRange.mipLevels = VK_LAST_MIP_LEVEL;
    srRange.baseArraySlice = 0;
    srRange.arraySize = VK_LAST_ARRAY_SLICE;

    VkImageMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memory_barrier.outputMask = output_mask;
    memory_barrier.inputMask = input_mask;
    memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
    memory_barrier.subresourceRange = srRange;
    VkImageMemoryBarrier *pmemory_barrier = &memory_barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;

    for (i = 0; i < m_renderTargets.size(); i++) {
        memory_barrier.image = m_renderTargets[i]->image();
        memory_barrier.oldLayout = m_renderTargets[i]->layout();
        vkCmdPipelineBarrier( handle(), src_stages, dest_stages, false, 1, (const void * const*)&pmemory_barrier);
        m_renderTargets[i]->layout(memory_barrier.newLayout);

        vkCmdClearColorImage(handle(),
               m_renderTargets[i]->image(), VK_IMAGE_LAYOUT_GENERAL,
               &clear_color, 1, &srRange );

    }

    if (depthStencilObj)
    {
        VkImageSubresourceRange dsRange = {};
        dsRange.aspect = VK_IMAGE_ASPECT_DEPTH;
        dsRange.baseMipLevel = 0;
        dsRange.mipLevels = VK_LAST_MIP_LEVEL;
        dsRange.baseArraySlice = 0;
        dsRange.arraySize = VK_LAST_ARRAY_SLICE;

        // prepare the depth buffer for clear

        memory_barrier.oldLayout = depthStencilObj->BindInfo()->layout;
        memory_barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
        memory_barrier.image = depthStencilObj->handle();
        memory_barrier.subresourceRange = dsRange;

        vkCmdPipelineBarrier( handle(), src_stages, dest_stages, false, 1, (const void * const*)&pmemory_barrier);

        vkCmdClearDepthStencilImage(handle(),
                                    depthStencilObj->handle(), VK_IMAGE_LAYOUT_GENERAL,
                                    depth_clear_color,  stencil_clear_color,
                                    1, &dsRange);

        // prepare depth buffer for rendering
        memory_barrier.image = depthStencilObj->handle();
        memory_barrier.oldLayout = VK_IMAGE_LAYOUT_GENERAL;
        memory_barrier.newLayout = depthStencilObj->BindInfo()->layout;
        memory_barrier.subresourceRange = dsRange;
        vkCmdPipelineBarrier( handle(), src_stages, dest_stages, false, 1, (const void * const*)&pmemory_barrier);
    }
}

void VkCommandBufferObj::FillBuffer(VkBuffer buffer, VkDeviceSize offset, VkDeviceSize fill_size, uint32_t data)
{
    vkCmdFillBuffer( handle(), buffer, offset, fill_size, data);
}

void VkCommandBufferObj::PrepareAttachments()
{
    uint32_t i;
    const VkFlags output_mask =
        VK_MEMORY_OUTPUT_HOST_WRITE_BIT |
        VK_MEMORY_OUTPUT_SHADER_WRITE_BIT |
        VK_MEMORY_OUTPUT_COLOR_ATTACHMENT_BIT |
        VK_MEMORY_OUTPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_MEMORY_OUTPUT_TRANSFER_BIT;
    const VkFlags input_mask =
        VK_MEMORY_INPUT_HOST_READ_BIT |
        VK_MEMORY_INPUT_INDIRECT_COMMAND_BIT |
        VK_MEMORY_INPUT_INDEX_FETCH_BIT |
        VK_MEMORY_INPUT_VERTEX_ATTRIBUTE_FETCH_BIT |
        VK_MEMORY_INPUT_UNIFORM_READ_BIT |
        VK_MEMORY_INPUT_SHADER_READ_BIT |
        VK_MEMORY_INPUT_COLOR_ATTACHMENT_BIT |
        VK_MEMORY_INPUT_DEPTH_STENCIL_ATTACHMENT_BIT |
        VK_MEMORY_INPUT_TRANSFER_BIT;

    VkImageSubresourceRange srRange = {};
    srRange.aspect = VK_IMAGE_ASPECT_COLOR;
    srRange.baseMipLevel = 0;
    srRange.mipLevels = VK_LAST_MIP_LEVEL;
    srRange.baseArraySlice = 0;
    srRange.arraySize = VK_LAST_ARRAY_SLICE;

    VkImageMemoryBarrier memory_barrier = {};
    memory_barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
    memory_barrier.outputMask = output_mask;
    memory_barrier.inputMask = input_mask;
    memory_barrier.newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    memory_barrier.subresourceRange = srRange;
    VkImageMemoryBarrier *pmemory_barrier = &memory_barrier;

    VkPipelineStageFlags src_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;
    VkPipelineStageFlags dest_stages = VK_PIPELINE_STAGE_ALL_GPU_COMMANDS;

    for(i=0; i<m_renderTargets.size(); i++)
    {
        memory_barrier.image = m_renderTargets[i]->image();
        memory_barrier.oldLayout = m_renderTargets[i]->layout();
        vkCmdPipelineBarrier( handle(), src_stages, dest_stages, false, 1, (const void * const*)&pmemory_barrier);
        m_renderTargets[i]->layout(memory_barrier.newLayout);
    }
}

void VkCommandBufferObj::BeginRenderPass(const VkRenderPassBeginInfo &info)
{
    vkCmdBeginRenderPass( handle(), &info, VK_RENDER_PASS_CONTENTS_INLINE);
}

void VkCommandBufferObj::EndRenderPass()
{
    vkCmdEndRenderPass(handle());
}

void VkCommandBufferObj::BindDynamicViewportState(VkDynamicViewportState viewportState)
{
    vkCmdBindDynamicViewportState( handle(), viewportState);
}

void VkCommandBufferObj::BindDynamicRasterState(VkDynamicRasterState rasterState)
{
    vkCmdBindDynamicRasterState( handle(), rasterState);
}

void VkCommandBufferObj::BindDynamicColorBlendState(VkDynamicColorBlendState colorBlendState)
{
    vkCmdBindDynamicColorBlendState( handle(), colorBlendState);
}

void VkCommandBufferObj::BindDynamicDepthStencilState(VkDynamicDepthStencilState depthStencilState)
{
    vkCmdBindDynamicDepthStencilState( handle(), depthStencilState);
}

void VkCommandBufferObj::AddRenderTarget(VkImageObj *renderTarget)
{
    m_renderTargets.push_back(renderTarget);
}

void VkCommandBufferObj::DrawIndexed(uint32_t firstIndex, uint32_t indexCount, int32_t vertexOffset, uint32_t firstInstance, uint32_t instanceCount)
{
    vkCmdDrawIndexed(handle(), firstIndex, indexCount, vertexOffset, firstInstance, instanceCount);
}

void VkCommandBufferObj::Draw(uint32_t firstVertex, uint32_t vertexCount, uint32_t firstInstance, uint32_t instanceCount)
{
    vkCmdDraw(handle(), firstVertex, vertexCount, firstInstance, instanceCount);
}

void VkCommandBufferObj::QueueCommandBuffer()
{
    VkFence nullFence = { VK_NULL_HANDLE };
    QueueCommandBuffer(nullFence);
}

void VkCommandBufferObj::QueueCommandBuffer(VkFence fence)
{
    VkResult err = VK_SUCCESS;

    // submit the command buffer to the universal queue
    err = vkQueueSubmit( m_device->m_queue, 1, &handle(), fence );
    ASSERT_VK_SUCCESS( err );

    err = vkQueueWaitIdle( m_device->m_queue );
    ASSERT_VK_SUCCESS( err );

    // Wait for work to finish before cleaning up.
    vkDeviceWaitIdle(m_device->device());
}

void VkCommandBufferObj::BindPipeline(VkPipelineObj &pipeline)
{
    vkCmdBindPipeline( handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline.handle() );
}

void VkCommandBufferObj::BindDescriptorSet(VkDescriptorSetObj &descriptorSet)
{
    VkDescriptorSet set_obj = descriptorSet.GetDescriptorSetHandle();

    // bind pipeline, vertex buffer (descriptor set) and WVP (dynamic buffer view)
    vkCmdBindDescriptorSets(handle(), VK_PIPELINE_BIND_POINT_GRAPHICS, descriptorSet.GetPipelineLayout(),
           0, 1, &set_obj, 0, NULL );
}

void VkCommandBufferObj::BindIndexBuffer(VkIndexBufferObj *indexBuffer, uint32_t offset)
{
    vkCmdBindIndexBuffer(handle(), indexBuffer->handle(), offset, indexBuffer->GetIndexType());
}

void VkCommandBufferObj::BindVertexBuffer(VkConstantBufferObj *vertexBuffer, VkDeviceSize offset, uint32_t binding)
{
    vkCmdBindVertexBuffers(handle(), binding, 1, &vertexBuffer->handle(), &offset);
}

VkDepthStencilObj::VkDepthStencilObj()
{
    m_initialized = false;
}
bool VkDepthStencilObj::Initialized()
{
    return m_initialized;
}

VkAttachmentBindInfo* VkDepthStencilObj::BindInfo()
{
    return &m_attachmentBindInfo;
}

void VkDepthStencilObj::Init(VkDeviceObj *device, int32_t width, int32_t height, VkFormat format)
{
    VkImageCreateInfo image_info;
    VkAttachmentViewCreateInfo view_info;

    m_device = device;
    m_initialized = true;
    m_depth_stencil_fmt = format;

    image_info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    image_info.pNext = NULL;
    image_info.imageType = VK_IMAGE_TYPE_2D;
    image_info.format = m_depth_stencil_fmt;
    image_info.extent.width = width;
    image_info.extent.height = height;
    image_info.extent.depth = 1;
    image_info.mipLevels = 1;
    image_info.arraySize = 1;
    image_info.samples = 1;
    image_info.tiling = VK_IMAGE_TILING_OPTIMAL;
    image_info.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_BIT;
    image_info.flags = 0;
    init(*m_device, image_info);

    view_info.sType = VK_STRUCTURE_TYPE_ATTACHMENT_VIEW_CREATE_INFO;
    view_info.pNext = NULL;
    view_info.image = VK_NULL_HANDLE;
    view_info.mipLevel = 0;
    view_info.baseArraySlice = 0;
    view_info.arraySize = 1;
    view_info.flags = 0;
    view_info.image = handle();
    m_attachmentView.init(*m_device, view_info);

    m_attachmentBindInfo.view = m_attachmentView.handle();
    m_attachmentBindInfo.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
}
