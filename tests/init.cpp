// Copyright 2005, Google Inc.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


//  VK tests
//
//  Copyright (C) 2014 LunarG, Inc.
//
//  Permission is hereby granted, free of charge, to any person obtaining a
//  copy of this software and associated documentation files (the "Software"),
//  to deal in the Software without restriction, including without limitation
//  the rights to use, copy, modify, merge, publish, distribute, sublicense,
//  and/or sell copies of the Software, and to permit persons to whom the
//  Software is furnished to do so, subject to the following conditions:
//
//  The above copyright notice and this permission notice shall be included
//  in all copies or substantial portions of the Software.
//
//  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
//  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
//  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
//  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
//  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
//  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
//  DEALINGS IN THE SOFTWARE.


// Verify VK driver initialization

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <vulkan.h>

#include "vktestbinding.h"
#include "test_common.h"
#include "icd-spv.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

class VkTest : public ::testing::Test {
public:
    void CreateImageTest();
    void CreateCommandBufferTest();
    void CreatePipelineTest();
    void CreateShaderTest();
    void CreateShader(VkShader *pshader, VkShaderStage stage);

    VkDevice device() {return m_device->handle();}

protected:
    VkApplicationInfo app_info;
    VkInstance inst;
    VkPhysicalDevice objs[16];
    uint32_t gpu_count;

    uint32_t m_device_id;
    vk_testing::Device *m_device;
    VkPhysicalDeviceProperties props;
    std::vector<VkQueueFamilyProperties> queue_props;
    uint32_t graphics_queue_node_index;

    virtual void SetUp() {
        VkResult err;
        int i;

        this->app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        this->app_info.pNext = NULL;
        this->app_info.pAppName = "base";
        this->app_info.appVersion = 1;
        this->app_info.pEngineName = "unittest";
        this->app_info.engineVersion = 1;
        this->app_info.apiVersion = VK_API_VERSION;
        VkInstanceCreateInfo inst_info = {};
        inst_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        inst_info.pNext = NULL;
        inst_info.pAppInfo = &app_info;
        inst_info.pAllocCb = NULL;
        inst_info.layerCount = 0;
        inst_info.ppEnabledLayerNames = NULL;
        inst_info.extensionCount = 0;
        inst_info.ppEnabledExtensionNames = NULL;
        err = vkCreateInstance(&inst_info, &inst);
        ASSERT_VK_SUCCESS(err);
        err = vkEnumeratePhysicalDevices(inst, &this->gpu_count, NULL);
        ASSERT_VK_SUCCESS(err);
        ASSERT_LE(this->gpu_count, ARRAY_SIZE(objs)) << "Too many GPUs";
        err = vkEnumeratePhysicalDevices(inst, &this->gpu_count, objs);
        ASSERT_VK_SUCCESS(err);
        ASSERT_GE(this->gpu_count, (uint32_t) 1) << "No GPU available";

        m_device_id = 0;
        this->m_device = new vk_testing::Device(objs[m_device_id]);
        this->m_device->init();

        props = m_device->phy().properties();

        queue_props = this->m_device->phy().queue_properties();
        for (i = 0; i < queue_props.size(); i++) {
            if (queue_props[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                graphics_queue_node_index = i;
                break;
            }
        }
        ASSERT_LT(i, queue_props.size()) << "Could not find a Queue with Graphics support";
    }

    virtual void TearDown() {
        delete m_device;
        vkDestroyInstance(inst);
    }
};

TEST_F(VkTest, AllocMemory) {
    VkResult err;
    VkMemoryAllocInfo alloc_info = {};
    VkDeviceMemory gpu_mem;
    uint8_t *pData;

    alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO;
    alloc_info.allocationSize = 1024 * 1024; // 1MB
    alloc_info.memoryTypeIndex = 0;

    VkPhysicalDeviceMemoryProperties mem_props;
    vkGetPhysicalDeviceMemoryProperties(m_device->phy().handle(), &mem_props);

    err = m_device->phy().set_memory_type(((1 << mem_props.memoryTypeCount) - 1), &alloc_info, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    ASSERT_VK_SUCCESS(err);

    err = vkAllocMemory(device(), &alloc_info, &gpu_mem);
    ASSERT_VK_SUCCESS(err);

    err = vkMapMemory(device(), gpu_mem, 0, 0, 0, (void **) &pData);
    ASSERT_VK_SUCCESS(err);

    memset(pData, 0x55, alloc_info.allocationSize);
    EXPECT_EQ(0x55, pData[0]) << "Memory read not same as write";

    vkUnmapMemory(device(), gpu_mem);

    vkFreeMemory(device(), gpu_mem);
}

TEST_F(VkTest, Event) {
    VkEventCreateInfo event_info;
    VkEvent event;
    VkResult err;

    //        typedef struct VkEventCreateInfo_
    //        {
    //            VkStructureType                      sType;      // Must be VK_STRUCTURE_TYPE_EVENT_CREATE_INFO
    //            const void*                             pNext;      // Pointer to next structure
    //            VkFlags                               flags;      // Reserved
    //        } VkEventCreateInfo;
    memset(&event_info, 0, sizeof(event_info));
    event_info.sType = VK_STRUCTURE_TYPE_EVENT_CREATE_INFO;

    err = vkCreateEvent(device(), &event_info, &event);
    ASSERT_VK_SUCCESS(err);

    err = vkResetEvent(device(), event);
    ASSERT_VK_SUCCESS(err);

    err = vkGetEventStatus(device(), event);
    ASSERT_EQ(VK_EVENT_RESET, err);

    err = vkSetEvent(device(), event);
    ASSERT_VK_SUCCESS(err);

    err = vkGetEventStatus(device(), event);
    ASSERT_EQ(VK_EVENT_SET, err);

    // TODO: Test actual synchronization with command buffer event.

    // All done with event memory, clean up
    vkDestroyEvent(device(), event);
}

#define MAX_QUERY_SLOTS 10

TEST_F(VkTest, Query) {
    VkQueryPoolCreateInfo query_info;
    VkQueryPool query_pool;
    size_t query_result_size;
    uint32_t *query_result_data;
    VkResult err;

    //        typedef enum VkQueryType_
    //        {
    //            VK_QUERY_TYPE_OCCLUSION                                     = 0x00000000,
    //            VK_QUERY_TYPE_PIPELINE_STATISTICS                           = 0x00000001,

    //            VK_QUERY_TYPE_BEGIN_RANGE                              = VK_QUERY_TYPE_OCCLUSION,
    //            VK_QUERY_TYPE_END_RANGE                                = VK_QUERY_TYPE_PIPELINE_STATISTICS,
    //            VK_NUM_QUERY_TYPE                                      = (VK_QUERY_TYPE_END_RANGE - VK_QUERY_TYPE_BEGIN_RANGE + 1),
    //            VK_MAX_ENUM(VkQueryType_)
    //        } VkQueryType;

    //        typedef struct VkQueryPoolCreateInfo_
    //        {
    //            VkStructureType                      sType;      // Must be VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO
    //            const void*                             pNext;      // Pointer to next structure
    //            VkQueryType                          queryType;
    //            uint32_t                                slots;
    //        } VkQueryPoolCreateInfo;

    memset(&query_info, 0, sizeof(query_info));
    query_info.sType = VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO;
    query_info.queryType = VK_QUERY_TYPE_OCCLUSION;
    query_info.slots = MAX_QUERY_SLOTS;

    //        VkResult VKAPI vkCreateQueryPool(
    //            VkDevice                                  device,
    //            const VkQueryPoolCreateInfo*           pCreateInfo,
    //            VkQueryPool*                             pQueryPool);

    err = vkCreateQueryPool(device(), &query_info, &query_pool);
    ASSERT_VK_SUCCESS(err);

    // TODO: Test actual synchronization with command buffer event.
    // TODO: Create command buffer
    // TODO: vkCmdResetQueryPool
    // TODO: vkCmdBeginQuery
    // TODO: commands
    // TOOD: vkCmdEndQuery

    err = vkGetQueryPoolResults(device(), query_pool, 0, MAX_QUERY_SLOTS,
                                 &query_result_size, VK_NULL_HANDLE, 0);
    ASSERT_VK_SUCCESS(err);

    if (query_result_size > 0) {
        query_result_data = new uint32_t [query_result_size];
        err = vkGetQueryPoolResults(device(), query_pool, 0, MAX_QUERY_SLOTS,
                                     &query_result_size, query_result_data, 0);
        //ASSERT_VK_SUCCESS(err); TODO fix once actually submit queries

        // TODO: Test Query result data.

    }

    vkDestroyQueryPool(device(), query_pool);
}

void getQueue(vk_testing::Device *device, uint32_t queue_node_index, const char *qname)
{
    uint32_t que_idx;
    VkResult err;
    VkQueue queue;

    const VkQueueFamilyProperties props = device->phy().queue_properties()[queue_node_index];
    for (que_idx = 0; que_idx < props.queueCount; que_idx++) {
        // TODO: Need to add support for separate MEMMGR and work queues, including synchronization
        err = vkGetDeviceQueue(device->handle(), queue_node_index, que_idx, &queue);
        ASSERT_EQ(VK_SUCCESS, err) << "vkGetDeviceQueue: " << qname << " queue #" << que_idx << ": Failed with error: " << vk_result_string(err);
    }
}

void VkTest::CreateImageTest()
{
    VkResult err;
    VkImage image;
    uint32_t w, h, mipCount;
    VkFormat fmt;
    VkFormatProperties image_fmt;

    w =512;
    h = 256;
    mipCount = 0;

    uint32_t _w = w;
    uint32_t _h = h;
    while( ( _w > 0 ) || ( _h > 0 ) )
    {
        _w >>= 1;
        _h >>= 1;
        mipCount++;
    }

    fmt = VK_FORMAT_R8G8B8A8_UINT;
    // TODO: Pick known good format rather than just expect common format
    /*
     * XXX: What should happen if given NULL HANDLE for the pData argument?
     * We're not requesting VK_OBJECT_INFO_TYPE_MEMORY_REQUIREMENTS so there is
     * an expectation that pData is a valid pointer.
     * However, why include a returned size value? That implies that the
     * amount of data may vary and that doesn't work well for using a
     * fixed structure.
     */

    err = vkGetPhysicalDeviceFormatProperties(objs[m_device_id], fmt, &image_fmt);
    ASSERT_VK_SUCCESS(err);

//    typedef struct VkImageCreateInfo_
//    {
//        VkStructureType                      sType;                      // Must be VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO
//        const void*                             pNext;                      // Pointer to next structure.
//        VkImageType                          imageType;
//        VkFormat                              format;
//        VkExtent3D                            extent;
//        uint32_t                                mipLevels;
//        uint32_t                                arraySize;
//        uint32_t                                samples;
//        VkImageTiling                        tiling;
//        VkFlags                               usage;                      // VkImageUsageFlags
//        VkFlags                               flags;                      // VkImageCreateFlags
//    } VkImageCreateInfo;


    VkImageCreateInfo imageCreateInfo = {};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.format = fmt;
    imageCreateInfo.arraySize = 1;
    imageCreateInfo.extent.width = w;
    imageCreateInfo.extent.height = h;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.mipLevels = mipCount;
    imageCreateInfo.samples = 1;
    if (image_fmt.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        imageCreateInfo.tiling = VK_IMAGE_TILING_LINEAR;
    }
    else if (image_fmt.optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    }
    else {
        FAIL() << "Neither Linear nor Optimal allowed for color attachment";
    }
    imageCreateInfo.usage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                            VK_IMAGE_USAGE_SAMPLED_BIT;

//    VkResult VKAPI vkCreateImage(
//        VkDevice                                  device,
//        const VkImageCreateInfo*                pCreateInfo,
//        VkImage*                                  pImage);
    err = vkCreateImage(device(), &imageCreateInfo, &image);
    ASSERT_VK_SUCCESS(err);

    // Verify image resources
//    VkResult VKAPI vkGetImageSubresourceLayout(
//        VkImage                                   image,
//        const VkImageSubresource*                pSubresource,
//        VkSubresourceLayout*                     pLayout);
//    typedef struct VkSubresourceLayout_
//    {
//        VkDeviceSize                            offset;                 // Specified in bytes
//        VkDeviceSize                            size;                   // Specified in bytes
//        VkDeviceSize                            rowPitch;               // Specified in bytes
//        VkDeviceSize                            depthPitch;             // Specified in bytes
//    } VkSubresourceLayout;

//    typedef struct VkImageSubresource_
//    {
//        VkImageAspect                        aspect;
//        uint32_t                                mipLevel;
//        uint32_t                                arrayLayer;
//    } VkImageSubresource;


    if (image_fmt.linearTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT) {
        VkImageSubresource subresource = {};
        subresource.aspect = VK_IMAGE_ASPECT_COLOR;
        subresource.arrayLayer = 0;

        _w = w;
        _h = h;
        while ((_w > 0) || (_h > 0))
        {
            VkSubresourceLayout layout = {};
            err = vkGetImageSubresourceLayout(device(), image, &subresource, &layout);
            ASSERT_VK_SUCCESS(err);

            // TODO: 4 should be replaced with pixel size for given format
            EXPECT_LE(_w * 4, layout.rowPitch) << "Pitch does not match expected image pitch";
            _w >>= 1;
            _h >>= 1;

            subresource.mipLevel++;
        }
    }

    VkMemoryRequirements mem_req;
    VkDeviceMemory image_mem;

    err = vkGetImageMemoryRequirements(device(), image, &mem_req);
    ASSERT_VK_SUCCESS(err);

    if (mem_req.size) {

        //        VkResult VKAPI vkAllocMemory(
        //            VkDevice                                  device,
        //            const VkMemoryAllocInfo*                pAllocInfo,
        //            VkDeviceMemory*                             pMem);
        VkMemoryAllocInfo mem_info = {};

        mem_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOC_INFO;
        mem_info.pNext = NULL;
        mem_info.allocationSize = mem_req.size;
        mem_info.memoryTypeIndex = 0;

        err = m_device->phy().set_memory_type(mem_req.memoryTypeBits, &mem_info, 0);
        ASSERT_VK_SUCCESS(err);

        err = vkAllocMemory(device(), &mem_info, &image_mem);
        ASSERT_VK_SUCCESS(err);

        err = vkBindImageMemory(device(), image, image_mem, 0);
        ASSERT_VK_SUCCESS(err);
    }

//    typedef struct VkImageViewCreateInfo_
//    {
//        VkStructureType                      sType;                  // Must be VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO
//        const void*                             pNext;                  // Pointer to next structure
//        VkImage                               image;
//        VkImageViewType                     viewType;
//        VkFormat                              format;
//        VkChannelMapping                     channels;
//        VkImageSubresourceRange             subresourceRange;
//        float                                   minLod;
//    } VkImageViewCreateInfo;
    VkImageViewCreateInfo viewInfo = {};
    VkImageView view;
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = image;
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = fmt;

    viewInfo.channels.r = VK_CHANNEL_SWIZZLE_R;
    viewInfo.channels.g = VK_CHANNEL_SWIZZLE_G;
    viewInfo.channels.b = VK_CHANNEL_SWIZZLE_B;
    viewInfo.channels.a = VK_CHANNEL_SWIZZLE_A;

    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.arraySize = 1;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.mipLevels = 1;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

//    VkResult VKAPI vkCreateImageView(
//        VkDevice                                  device,
//        const VkImageViewCreateInfo*           pCreateInfo,
//        VkImageView*                             pView);

    err = vkCreateImageView(device(), &viewInfo, &view);
    ASSERT_VK_SUCCESS(err) << "vkCreateImageView failed";

    // TODO: Test image memory.

    // All done with image memory, clean up
    vkDestroyImageView(device(), view);
    vkDestroyImage(device(), image);

    if (mem_req.size) {
        vkFreeMemory(device(), image_mem);
    }
}

TEST_F(VkTest, CreateImage) {
    CreateImageTest();
}

void VkTest::CreateCommandBufferTest()
{
    VkResult err;
    VkCmdBufferCreateInfo info = {};
    VkCmdPool cmdPool;
    VkCmdBuffer cmdBuffer;

//    typedef struct VkCmdBufferCreateInfo_
//    {
//        VkStructureType                      sType;      // Must be VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO
//        const void*                             pNext;
//        VK_QUEUE_TYPE                          queueType;
//        VkFlags                               flags;
//    } VkCmdBufferCreateInfo;

    VkCmdPoolCreateInfo cmd_pool_info;
    cmd_pool_info.sType = VK_STRUCTURE_TYPE_CMD_POOL_CREATE_INFO,
    cmd_pool_info.pNext = NULL,
    cmd_pool_info.queueFamilyIndex = graphics_queue_node_index;
    cmd_pool_info.flags = 0,
    err = vkCreateCommandPool(device(), &cmd_pool_info, &cmdPool);
    ASSERT_VK_SUCCESS(err) << "vkCreateCommandPool failed";

    info.sType = VK_STRUCTURE_TYPE_CMD_BUFFER_CREATE_INFO;
    info.cmdPool = cmdPool;
    err = vkCreateCommandBuffer(device(), &info, &cmdBuffer);
    ASSERT_VK_SUCCESS(err) << "vkCreateCommandBuffer failed";

    vkDestroyCommandBuffer(device(), cmdBuffer);
    vkDestroyCommandPool(device(), cmdPool);
}

TEST_F(VkTest, TestCommandBuffer) {
    CreateCommandBufferTest();
}

void VkTest::CreateShader(VkShader *pshader, VkShaderStage stage)
{
    void *code;
    uint32_t codeSize;
    struct icd_spv_header *pSPV;
    VkResult err;

    codeSize = sizeof(struct icd_spv_header) + 100;
    code = malloc(codeSize);
    ASSERT_TRUE(NULL != code) << "malloc failed!";

    memset(code, 0, codeSize);

    // Indicate that this is SPV data.
    pSPV = (struct icd_spv_header *) code;
    pSPV->magic = ICD_SPV_MAGIC;
    pSPV->version = ICD_SPV_VERSION;

    VkShaderModuleCreateInfo moduleCreateInfo;
    VkShaderModule module;
    VkShaderCreateInfo createInfo;
    VkShader shader;

    moduleCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    moduleCreateInfo.pNext = NULL;
    moduleCreateInfo.pCode = code;
    moduleCreateInfo.codeSize = codeSize;
    moduleCreateInfo.flags = 0;
    err = vkCreateShaderModule(device(), &moduleCreateInfo, &module);
    ASSERT_VK_SUCCESS(err);

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_CREATE_INFO;
    createInfo.pNext = NULL;
    createInfo.module = module;
    createInfo.pName = "main";
    createInfo.flags = 0;
    createInfo.stage = stage;
    err = vkCreateShader(device(), &createInfo, &shader);
    ASSERT_VK_SUCCESS(err);

    vkDestroyShaderModule(device(), module);

    *pshader = shader;
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    vk_testing::set_error_callback(test_error_callback);
    return RUN_ALL_TESTS();
}
