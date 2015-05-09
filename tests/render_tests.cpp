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

// Basic rendering tests

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <iostream>
#include <fstream>
using namespace std;

#include <vulkan.h>
#ifdef DUMP_STATE_DOT
#include "../layers/draw_state.h"
#endif
#ifdef PRINT_OBJECTS
#include "../layers/object_track.h"
#endif
#ifdef DEBUG_CALLBACK
#include <vkDbg.h>
#endif
#include "gtest-1.7.0/include/gtest/gtest.h"

#include "icd-spv.h"

#define GLM_FORCE_RADIANS
#include "glm/glm.hpp"
#include <glm/gtc/matrix_transform.hpp>

#include "vkrenderframework.h"
#ifdef DEBUG_CALLBACK
void VKAPI myDbgFunc(
    VK_DBG_MSG_TYPE     msgType,
    VkValidationLevel validationLevel,
    VkObject             srcObject,
    size_t               location,
    int32_t              msgCode,
    const char*          pMsg,
    void*                pUserData)
{
    switch (msgType)
    {
        case VK_DBG_MSG_WARNING:
            printf("CALLBACK WARNING : %s\n", pMsg);
            break;
        case VK_DBG_MSG_ERROR:
            printf("CALLBACK ERROR : %s\n", pMsg);
            break;
        default:
            printf("EATING Msg of type %u\n", msgType);
            break;
    }
}
#endif


#undef ASSERT_NO_FATAL_FAILURE
#define ASSERT_NO_FATAL_FAILURE(x) x

//--------------------------------------------------------------------------------------
// Mesh and VertexFormat Data
//--------------------------------------------------------------------------------------
struct Vertex
{
    float posX, posY, posZ, posW;    // Position data
    float r, g, b, a;                // Color
};

struct VertexUV
{
    float posX, posY, posZ, posW;    // Position data
    float u, v;                      // texture u,v
};

#define XYZ1(_x_, _y_, _z_)         (_x_), (_y_), (_z_), 1.f
#define UV(_u_, _v_)                (_u_), (_v_)

static const Vertex g_vbData[] =
{
    { XYZ1( -1, -1, -1 ), XYZ1( 0.f, 0.f, 0.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },

    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( 1, -1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( 1, -1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( 1,  1,  1 ), XYZ1( 1.f, 1.f, 1.f ) },

    { XYZ1( 1,  1,  1 ), XYZ1( 1.f, 1.f, 1.f ) },
    { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( 1, -1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( 1, -1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },

    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1( -1, -1, -1 ), XYZ1( 0.f, 0.f, 0.f ) },

    { XYZ1( 1,  1,  1 ), XYZ1( 1.f, 1.f, 1.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },

    { XYZ1( 1, -1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( -1, -1, -1 ), XYZ1( 0.f, 0.f, 0.f ) },
};

static const Vertex g_vb_solid_face_colors_Data[] =
{
    { XYZ1( -1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1(  1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1(  1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
    { XYZ1(  1,  1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },

    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1(  1, -1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1(  1, -1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },
    { XYZ1(  1,  1,  1 ), XYZ1( 0.f, 1.f, 0.f ) },

    { XYZ1(  1,  1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1(  1,  1, -1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1(  1,  1, -1 ), XYZ1( 0.f, 0.f, 1.f ) },
    { XYZ1(  1, -1, -1 ), XYZ1( 0.f, 0.f, 1.f ) },

    { XYZ1( -1,  1,  1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 1.f, 1.f, 0.f ) },
    { XYZ1( -1, -1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },

    { XYZ1(  1,  1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1(  1,  1, -1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1(  1,  1, -1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( -1,  1,  1 ), XYZ1( 1.f, 0.f, 1.f ) },
    { XYZ1( -1,  1, -1 ), XYZ1( 1.f, 0.f, 1.f ) },

    { XYZ1( 1, -1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( 1, -1, -1 ), XYZ1( 0.f, 1.f, 1.f ) },
    { XYZ1( -1, -1, -1 ), XYZ1( 0.f, 1.f, 1.f ) },
};

static const VertexUV g_vb_texture_Data[] =
{
    { XYZ1( -1, -1, -1 ), UV( 0.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), UV( 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), UV( 1.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), UV( 1.f, 1.f ) },
    { XYZ1( -1, -1, -1 ), UV( 0.f, 0.f ) },
    { XYZ1( -1,  1, -1 ), UV( 0.f, 1.f ) },

    { XYZ1( -1, -1, -1 ), UV( 1.f, 0.f ) },
    { XYZ1(  1, -1, -1 ), UV( 0.f, 0.f ) },
    { XYZ1(  1,  1, -1 ), UV( 0.f, 1.f ) },
    { XYZ1( -1, -1, -1 ), UV( 1.f, 0.f ) },
    { XYZ1(  1,  1, -1 ), UV( 0.f, 1.f ) },
    { XYZ1( -1,  1, -1 ), UV( 1.f, 1.f ) },

    { XYZ1( -1, -1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1(  1, -1, -1 ), UV( 1.f, 0.f ) },
    { XYZ1( -1, -1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), UV( 0.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), UV( 0.f, 0.f ) },

    { XYZ1( -1,  1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1,  1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1( -1,  1,  1 ), UV( 0.f, 1.f ) },
    { XYZ1( -1,  1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1,  1, -1 ), UV( 1.f, 0.f ) },
    { XYZ1(  1,  1,  1 ), UV( 0.f, 0.f ) },

    { XYZ1(  1,  1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1(  1,  1,  1 ), UV( 0.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1(  1,  1, -1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1, -1, -1 ), UV( 1.f, 0.f ) },

    { XYZ1( -1,  1,  1 ), UV( 0.f, 1.f ) },
    { XYZ1(  1,  1,  1 ), UV( 1.f, 1.f ) },
    { XYZ1( -1, -1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1( -1, -1,  1 ), UV( 0.f, 0.f ) },
    { XYZ1(  1,  1,  1 ), UV( 1.f, 1.f ) },
    { XYZ1(  1, -1,  1 ), UV( 1.f, 0.f ) },
};

class VkRenderTest : public VkRenderFramework
{
public:

    void RotateTriangleVSUniform(glm::mat4 Projection, glm::mat4 View, glm::mat4 Model,
                                 VkConstantBufferObj *constantBuffer, VkCommandBufferObj *cmdBuffer);
    void GenericDrawPreparation(VkCommandBufferObj *cmdBuffer, VkPipelineObj &pipelineobj, VkDescriptorSetObj &descriptorSet);
    void InitDepthStencil();
    void VKTriangleTest(const char *vertShaderText, const char *fragShaderText, const bool rotate);

    VkResult BeginCommandBuffer(VkCommandBufferObj &cmdBuffer);
    VkResult EndCommandBuffer(VkCommandBufferObj &cmdBuffer);

protected:
    VkImage m_texture;
    VkImageView m_textureView;
    VkImageViewAttachInfo m_textureViewInfo;
    VkDeviceMemory m_textureMem;

    VkSampler m_sampler;


    virtual void SetUp() {

        this->app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        this->app_info.pNext = NULL;
        this->app_info.pAppName = "render_tests";
        this->app_info.appVersion = 1;
        this->app_info.pEngineName = "unittest";
        this->app_info.engineVersion = 1;
        this->app_info.apiVersion = VK_API_VERSION;

        memset(&m_textureViewInfo, 0, sizeof(m_textureViewInfo));
        m_textureViewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_ATTACH_INFO;

        InitFramework();
    }

    virtual void TearDown() {
        // Clean up resources before we reset
        ShutdownFramework();
    }
};

VkResult VkRenderTest::BeginCommandBuffer(VkCommandBufferObj &cmdBuffer)
{
    VkResult result;

    result = cmdBuffer.BeginCommandBuffer();

    /*
     * For render test all drawing happens in a single render pass
     * on a single command buffer.
     */
    if (VK_SUCCESS == result) {
        cmdBuffer.BeginRenderPass(renderPass(), framebuffer());
    }

    return result;
}

VkResult VkRenderTest::EndCommandBuffer(VkCommandBufferObj &cmdBuffer)
{
    VkResult result;

    cmdBuffer.EndRenderPass(renderPass());

    result = cmdBuffer.EndCommandBuffer();

    return result;
}


void VkRenderTest::GenericDrawPreparation(VkCommandBufferObj *cmdBuffer, VkPipelineObj &pipelineobj, VkDescriptorSetObj &descriptorSet)
{
    if (m_depthStencil->Initialized()) {
        cmdBuffer->ClearAllBuffers(m_clear_color, m_depth_clear_color, m_stencil_clear_color, m_depthStencil);
    } else {
        cmdBuffer->ClearAllBuffers(m_clear_color, m_depth_clear_color, m_stencil_clear_color, NULL);
    }

    cmdBuffer->PrepareAttachments();
    cmdBuffer->BindStateObject(VK_STATE_BIND_POINT_RASTER, m_stateRaster);
    cmdBuffer->BindStateObject(VK_STATE_BIND_POINT_VIEWPORT, m_stateViewport);
    cmdBuffer->BindStateObject(VK_STATE_BIND_POINT_COLOR_BLEND, m_colorBlend);
    cmdBuffer->BindStateObject(VK_STATE_BIND_POINT_DEPTH_STENCIL, m_stateDepthStencil);
    descriptorSet.CreateVKDescriptorSet(cmdBuffer);
    pipelineobj.CreateVKPipeline(descriptorSet);
    cmdBuffer->BindPipeline(pipelineobj);
    cmdBuffer->BindDescriptorSet(descriptorSet);
}

void VkRenderTest::RotateTriangleVSUniform(glm::mat4 Projection, glm::mat4 View, glm::mat4 Model,
                                            VkConstantBufferObj *constantBuffer, VkCommandBufferObj *cmdBuffer)
{
    int i;
    glm::mat4 MVP;
    int matrixSize = sizeof(MVP);
    VkResult err;

    /* Only do 3 positions to avoid back face cull */
    for (i = 0; i < 3; i++) {
        void *pData = constantBuffer->map();

        Model = glm::rotate(Model, glm::radians(22.5f), glm::vec3(0.0f, 1.0f, 0.0f));
        MVP = Projection * View * Model;
        memcpy(pData, (const void*) &MVP[0][0], matrixSize);

        constantBuffer->unmap();

        // submit the command buffer to the universal queue
        cmdBuffer->QueueCommandBuffer();

        err = vkQueueWaitIdle( m_device->m_queue );
        ASSERT_VK_SUCCESS( err );

        // Wait for work to finish before cleaning up.
        vkDeviceWaitIdle(m_device->device());

        assert(m_renderTargets.size() == 1);
        RecordImage(m_renderTargets[0]);
    }
}

void dumpMatrix(const char *note, glm::mat4 MVP)
{
    int i;

    printf("%s: \n", note);
    for (i=0; i<4; i++) {
        printf("%f, %f, %f, %f\n", MVP[i][0], MVP[i][1], MVP[i][2], MVP[i][3]);
    }
    printf("\n");
    fflush(stdout);
}

void dumpVec4(const char *note, glm::vec4 vector)
{
    printf("%s: \n", note);
        printf("%f, %f, %f, %f\n", vector[0], vector[1], vector[2], vector[3]);
    printf("\n");
    fflush(stdout);
}

struct vktriangle_vs_uniform {
    // Must start with MVP
    float   mvp[4][4];
    float   position[3][4];
    float   color[3][4];
};

void VkRenderTest::VKTriangleTest(const char *vertShaderText, const char *fragShaderText, const bool rotate)
{
#ifdef DEBUG_CALLBACK
    vkDbgRegisterMsgCallback(inst, myDbgFunc, NULL);
#endif
    // Create identity matrix
    int i;
    struct vktriangle_vs_uniform data;

    glm::mat4 Projection      = glm::mat4(1.0f);
    glm::mat4 View      = glm::mat4(1.0f);
    glm::mat4 Model      = glm::mat4(1.0f);
    glm::mat4 MVP = Projection * View * Model;
    const int matrixSize = sizeof(MVP);
    const int bufSize = sizeof(vktriangle_vs_uniform) / sizeof(float);
    memcpy(&data.mvp, &MVP[0][0], matrixSize);

    static const Vertex tri_data[] =
    {
        { XYZ1( -1, -1, 0 ), XYZ1( 1.f, 0.f, 0.f ) },
        { XYZ1( 1, -1, 0 ), XYZ1( 0.f, 1.f, 0.f ) },
        { XYZ1( 0,  1, 0 ), XYZ1( 0.f, 0.f, 1.f ) },
    };

    for (i=0; i<3; i++) {
        data.position[i][0] = tri_data[i].posX;
        data.position[i][1] = tri_data[i].posY;
        data.position[i][2] = tri_data[i].posZ;
        data.position[i][3] = tri_data[i].posW;
        data.color[i][0] = tri_data[i].r;
        data.color[i][1] = tri_data[i].g;
        data.color[i][2] = tri_data[i].b;
        data.color[i][3] = tri_data[i].a;
    }

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj constantBuffer(m_device, bufSize*2, sizeof(float), (const void*) &data);

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, constantBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);

    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);

    if (rotate)
        RotateTriangleVSUniform(Projection, View, Model, &constantBuffer, &cmdBuffer);

#ifdef PRINT_OBJECTS
    //uint64_t objTrackGetObjectCount(VkObjectType type)
    OBJ_TRACK_GET_OBJECTS_COUNT pObjTrackGetObjectsCount = (OBJ_TRACK_GET_OBJECTS_COUNT)vkGetProcAddr(gpu(), (char*)"objTrackGetObjectsCount");
    uint64_t numObjects = pObjTrackGetObjectsCount();
    //OBJ_TRACK_GET_OBJECTS pGetObjsFunc = vkGetProcAddr(gpu(), (char*)"objTrackGetObjects");
    printf("DEBUG : Number of Objects : %lu\n", numObjects);
    OBJ_TRACK_GET_OBJECTS pObjTrackGetObjs = (OBJ_TRACK_GET_OBJECTS)vkGetProcAddr(gpu(), (char*)"objTrackGetObjects");
    OBJTRACK_NODE* pObjNodeArray = (OBJTRACK_NODE*)malloc(sizeof(OBJTRACK_NODE)*numObjects);
    pObjTrackGetObjs(numObjects, pObjNodeArray);
    for (i=0; i < numObjects; i++) {
        printf("Object %i of type %s has objID (%p) and %lu uses\n", i, string_from_vulkan_object_type(pObjNodeArray[i].objType), pObjNodeArray[i].vkObj, pObjNodeArray[i].numUses);
    }
    free(pObjNodeArray);
#endif

}

TEST_F(VkRenderTest, VKTriangle_FragColor)
{
    static const char *vertShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout(binding = 0) uniform buf {\n"
        "        mat4 MVP;\n"
        "        vec4 position[3];\n"
        "        vec4 color[3];\n"
        "} ubuf;\n"
        "\n"
        "layout (location = 0) out vec4 outColor;\n"
        "\n"
        "void main() \n"
        "{\n"
        "   outColor = ubuf.color[gl_VertexID];\n"
        "   gl_Position = ubuf.MVP * ubuf.position[gl_VertexID];\n"
        "}\n";

    static const char *fragShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout (location = 0) in vec4 inColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = inColor;\n"
        "}\n";

    TEST_DESCRIPTION("VK-style shaders where fragment shader outputs to GLSL built-in gl_FragColor");
    VKTriangleTest(vertShaderText, fragShaderText, true);
}

TEST_F(VkRenderTest, VKTriangle_OutputLocation)
{
    static const char *vertShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout(binding = 0) uniform buf {\n"
        "        mat4 MVP;\n"
        "        vec4 position[3];\n"
        "        vec4 color[3];\n"
        "} ubuf;\n"
        "\n"
        "layout (location = 0) out vec4 outColor;\n"
        "\n"
        "void main() \n"
        "{\n"
        "   outColor = ubuf.color[gl_VertexID];\n"
        "   gl_Position = ubuf.MVP * ubuf.position[gl_VertexID];\n"
        "}\n";

    static const char *fragShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout (location = 0) in vec4 inColor;\n"
        "layout (location = 0) out vec4 outColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   outColor = inColor;\n"
        "}\n";

    TEST_DESCRIPTION("VK-style shaders where fragment shader outputs to output location 0, which should be the same as gl_FragColor");

    VKTriangleTest(vertShaderText, fragShaderText, true);
}
#ifndef _WIN32 // Implicit (for now at least) in WIN32 is that we are using the Nvidia driver and it won't consume SPIRV yet
TEST_F(VkRenderTest, SPV_VKTriangle)
{
    bool saved_use_spv = VkTestFramework::m_use_spv;

    static const char *vertShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout(binding = 0) uniform buf {\n"
        "        mat4 MVP;\n"
        "        vec4 position[3];\n"
        "        vec4 color[3];\n"
        "} ubuf;\n"
        "\n"
        "layout (location = 0) out vec4 outColor;\n"
        "\n"
        "void main() \n"
        "{\n"
        "   outColor = ubuf.color[gl_VertexID];\n"
        "   gl_Position = ubuf.MVP * ubuf.position[gl_VertexID];\n"
        "}\n";

    static const char *fragShaderText =
        "#version 140\n"
        "#extension GL_ARB_separate_shader_objects : enable\n"
        "#extension GL_ARB_shading_language_420pack : enable\n"
        "\n"
        "layout (location = 0) in vec4 inColor;\n"
        "\n"
        "void main()\n"
        "{\n"
        "   gl_FragColor = inColor;\n"
        "}\n";

    TEST_DESCRIPTION("VK-style shaders, but force test framework to compile shader to SPV and pass SPV to driver.");

    VkTestFramework::m_use_spv = true;

    VKTriangleTest(vertShaderText, fragShaderText, true);

    VkTestFramework::m_use_spv = saved_use_spv;
}
#endif
TEST_F(VkRenderTest, GreenTriangle)
{
    static const char *vertShaderText =
            "#version 130\n"
            "vec2 vertices[3];\n"
            "void main() {\n"
            "      vertices[0] = vec2(-1.0, -1.0);\n"
            "      vertices[1] = vec2( 1.0, -1.0);\n"
            "      vertices[2] = vec2( 0.0,  1.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
       "#version 130\n"
       "void main() {\n"
       "   gl_FragColor = vec4(0,1,0,1);\n"
       "}\n";

    TEST_DESCRIPTION("Basic shader that renders a fixed Green triangle coded as part of the vertex shader.");

    VKTriangleTest(vertShaderText, fragShaderText, false);
}
#ifndef _WIN32 // Implicit (for now at least) in WIN32 is that we are using the Nvidia driver and it won't consume SPIRV yet
TEST_F(VkRenderTest, SPV_GreenTriangle)
{
    bool saved_use_spv = VkTestFramework::m_use_spv;

    static const char *vertShaderText =
            "#version 130\n"
            "vec2 vertices[3];\n"
            "void main() {\n"
            "      vertices[0] = vec2(-1.0, -1.0);\n"
            "      vertices[1] = vec2( 1.0, -1.0);\n"
            "      vertices[2] = vec2( 0.0,  1.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
       "#version 130\n"
       "void main() {\n"
       "   gl_FragColor = vec4(0,1,0,1);\n"
       "}\n";

    TEST_DESCRIPTION("Same shader as GreenTriangle, but compiles shader to SPV and gives SPV to driver.");

    VkTestFramework::m_use_spv = true;
    VKTriangleTest(vertShaderText, fragShaderText, false);
    VkTestFramework::m_use_spv = saved_use_spv;
}
#endif
TEST_F(VkRenderTest, YellowTriangle)
{
    static const char *vertShaderText =
            "#version 130\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec4 colors[3];\n"
            "      colors[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "      colors[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "      colors[2] = vec4(0.0, 0.0, 1.0, 1.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 130\n"
            "void main() {\n"
            "  gl_FragColor = vec4(1.0, 1.0, 0.0, 1.0);\n"
            "}\n";

    VKTriangleTest(vertShaderText, fragShaderText, false);
}

TEST_F(VkRenderTest, QuadWithVertexFetch)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //XYZ1( -1, -1, -1 )
            "layout (location = 0) in vec4 pos;\n"
            //XYZ1( 0.f, 0.f, 0.f )
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = pos;\n"
            "}\n";


    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = color;\n"
            "}\n";



    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                      // binding ID
         sizeof(g_vbData[0]),              // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
         VK_VERTEX_INPUT_STEP_RATE_VERTEX // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BIND_ID;               // Binding ID
    vi_attribs[0].location = 0;                         // location, position
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BIND_ID;               // Binding ID
    vi_attribs[1].location = 1;                         // location, color
    vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[1].offsetInBytes = 1*sizeof(float)*4;     // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BIND_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);

    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleMRT)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 pos;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "}\n";

    static const char *fragShaderText =
            "#version 130\n"
            "void main() {\n"
            "   gl_FragData[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "   gl_FragData[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "}\n";
    const float vb_data[][2] = {
        { -1.0f, -1.0f },
        {  1.0f, -1.0f },
        { -1.0f,  1.0f }
    };

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device, sizeof(vb_data) / sizeof(vb_data[0]), sizeof(vb_data[0]), vb_data);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

#define MESH_BUF_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BUF_ID,                            // Binding ID
        sizeof(vb_data[0]),                     // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attrib;
    vi_attrib.binding = MESH_BUF_ID;            // index into vertexBindingDescriptions
    vi_attrib.location = 0;
    vi_attrib.format = VK_FORMAT_R32G32_SFLOAT;   // format of source data
    vi_attrib.offsetInBytes = 0;                // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(&vi_attrib, 1);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BUF_ID);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(2));

    VkPipelineCbAttachmentState att = {};
    att.blendEnable = VK_FALSE;
    att.format = m_render_target_fmt;
    att.channelWriteMask = 0xf;
    pipelineobj.AddColorAttachment(1, &att);

    VkCommandBufferObj cmdBuffer(m_device);

    cmdBuffer.AddRenderTarget(m_renderTargets[0]);
    cmdBuffer.AddRenderTarget(m_renderTargets[1]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, QuadWithIndexedVertexFetch)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout(location = 0) in vec4 pos;\n"
            "layout(location = 1) in vec4 inColor;\n"
            "layout(location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = pos;\n"
            "}\n";


    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout(location = 0) in vec4 color;\n"
            "void main() {\n"
            "   gl_FragColor = color;\n"
            "}\n";

    const Vertex g_vbData[] =
    {
        // first tri
        { XYZ1( -1, -1, -1 ), XYZ1( 0.f, 0.f, 0.f ) },  // LL: black
        { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },   // LR: red
        { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },  // UL: green

        // second tri
        { XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },  // UL: green
        { XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },   // LR: red
        { XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },   // UR: yellow
    };

    const uint16_t g_idxData[6] = {
        0, 1, 2,
        3, 4, 5,
    };

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkIndexBufferObj indexBuffer(m_device);
    indexBuffer.CreateAndInitBuffer(sizeof(g_idxData)/sizeof(g_idxData[0]), VK_INDEX_TYPE_UINT16, g_idxData);
    indexBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, indexBuffer);


#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                           // binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BIND_ID;               // binding ID from BINDING_DESCRIPTION array to use for this attribute
    vi_attribs[0].location = 0;                         // layout location of vertex attribute
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BIND_ID;               // binding ID from BINDING_DESCRIPTION array to use for this attribute
    vi_attribs[1].location = 1;                         // layout location of vertex attribute
    vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[1].offsetInBytes = 16;                   // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);
    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, MESH_BIND_ID);
    cmdBuffer.BindIndexBuffer(&indexBuffer, 0);

    // render two triangles
    cmdBuffer.DrawIndexed(0, 6, 0, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, GreyandRedCirclesonBlue)
{
    // This tests gl_FragCoord

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "layout (location = 1) out vec4 outColor2;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "   outColor = vec4(0.9, 0.9, 0.9, 1.0);\n"
            "   outColor2 = vec4(0.2, 0.2, 0.4, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //"#extension GL_ARB_fragment_coord_conventions : enable\n"
            //"layout (pixel_center_integer) in vec4 gl_FragCoord;\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 1) in vec4 color2;\n"
            "void main() {\n"
            "    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);\n"
            "    float dist_squared = dot(pos, pos);\n"
            "    gl_FragColor = (dist_squared < 400.0)\n"
            "        ? ((gl_FragCoord.y < 100.0) ? vec4(1.0, 0.0, 0.0, 0.0) : color)\n"
            "        : color2;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                           // binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[1];
    vi_attribs[0].binding = MESH_BIND_ID;               // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,1);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer,MESH_BIND_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangle
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, RedCirclesonBlue)
{
    // This tests that we correctly handle unread fragment inputs

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "layout (location = 1) out vec4 outColor2;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "   outColor = vec4(0.9, 0.9, 0.9, 1.0);\n"
            "   outColor2 = vec4(0.2, 0.2, 0.4, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //"#extension GL_ARB_fragment_coord_conventions : enable\n"
            //"layout (pixel_center_integer) in vec4 gl_FragCoord;\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 1) in vec4 color2;\n"
            "void main() {\n"
            "    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);\n"
            "    float dist_squared = dot(pos, pos);\n"
            "    gl_FragColor = (dist_squared < 400.0)\n"
            "        ? vec4(1.0, 0.0, 0.0, 1.0)\n"
            "        : color2;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                           // binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[1];
    vi_attribs[0].binding = MESH_BIND_ID;               // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,1);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer,MESH_BIND_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, GreyCirclesonBlueFade)
{
    // This tests reading gl_ClipDistance from FS

    static const char *vertShaderText =
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "out gl_PerVertex {\n"
            "    vec4 gl_Position;\n"
            "    float gl_ClipDistance[1];\n"
            "};\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "layout (location = 1) out vec4 outColor2;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "   outColor = vec4(0.9, 0.9, 0.9, 1.0);\n"
            "   outColor2 = vec4(0.2, 0.2, 0.4, 1.0);\n"
            "   float dists[3];\n"
            "      dists[0] = 0.0;\n"
            "      dists[1] = 1.0;\n"
            "      dists[2] = 1.0;\n"
            "   gl_ClipDistance[0] = dists[gl_VertexID % 3];\n"
            "}\n";


    static const char *fragShaderText =
            //"#version 140\n"
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //"#extension GL_ARB_fragment_coord_conventions : enable\n"
            //"layout (pixel_center_integer) in vec4 gl_FragCoord;\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 1) in vec4 color2;\n"
            "void main() {\n"
            "    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);\n"
            "    float dist_squared = dot(pos, pos);\n"
            "    gl_FragColor = (dist_squared < 400.0)\n"
            "        ? color * gl_ClipDistance[0]\n"
            "        : color2;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                           // binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[1];
    vi_attribs[0].binding = MESH_BIND_ID;               // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,1);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer,MESH_BIND_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, GreyCirclesonBlueDiscard)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "layout (location = 1) out vec4 outColor2;\n"
            "void main() {\n"
            "   gl_Position = pos;\n"
            "   outColor = vec4(0.9, 0.9, 0.9, 1.0);\n"
            "   outColor2 = vec4(0.2, 0.2, 0.4, 1.0);\n"
            "}\n";


    static const char *fragShaderText =
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //"#extension GL_ARB_fragment_coord_conventions : enable\n"
            //"layout (pixel_center_integer) in vec4 gl_FragCoord;\n"
            "layout (location = 0) in vec4 color;\n"
            "layout (location = 1) in vec4 color2;\n"
            "void main() {\n"
            "    vec2 pos = mod(gl_FragCoord.xy, vec2(50.0)) - vec2(25.0);\n"
            "    float dist_squared = dot(pos, pos);\n"
            "    if (dist_squared < 100.0)\n"
            "        discard;\n"
            "    gl_FragColor = (dist_squared < 400.0)\n"
            "        ? color\n"
            "        : color2;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                           // binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[1];
    vi_attribs[0].binding = MESH_BIND_ID;               // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                    // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs,1);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer,MESH_BIND_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}


TEST_F(VkRenderTest, TriangleVSUniform)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "\n"
            "layout(binding = 0) uniform buf {\n"
            "        mat4 MVP;\n"
            "} ubuf;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   gl_Position = ubuf.MVP * vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 130\n"
            "void main() {\n"
            "   gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    // Create identity matrix
    glm::mat4 Projection    = glm::mat4(1.0f);
    glm::mat4 View          = glm::mat4(1.0f);
    glm::mat4 Model         = glm::mat4(1.0f);
    glm::mat4 MVP = Projection * View * Model;
    const int matrixSize = sizeof(MVP) / sizeof(MVP[0]);

    VkConstantBufferObj MVPBuffer(m_device, matrixSize, sizeof(MVP[0]), (const void*) &MVP[0][0]);
    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    // Create descriptor set and attach the constant buffer to it
    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MVPBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    // cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);

    RotateTriangleVSUniform(Projection, View, Model, &MVPBuffer, &cmdBuffer);
}

TEST_F(VkRenderTest, MixTriangle)
{
    // This tests location applied to varyings. Notice that we have switched foo
    // and bar in the FS. The triangle should be blended with red, green and blue
    // corners.
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location=0) out vec4 bar;\n"
            "layout (location=1) out vec4 foo;\n"
            "layout (location=2) out float scale;\n"
            "vec2 vertices[3];\n"
            "void main() {\n"
            "      vertices[0] = vec2(-1.0, -1.0);\n"
            "      vertices[1] = vec2( 1.0, -1.0);\n"
            "      vertices[2] = vec2( 0.0,  1.0);\n"
            "vec4 colors[3];\n"
            "      colors[0] = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "      colors[1] = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "      colors[2] = vec4(0.0, 0.0, 1.0, 1.0);\n"
            "   foo = colors[gl_VertexID % 3];\n"
            "   bar = vec4(1.0, 1.0, 1.0, 1.0);\n"
            "   scale = 1.0;\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
           "#version 140\n"
           "#extension GL_ARB_separate_shader_objects : enable\n"
           "#extension GL_ARB_shading_language_420pack : enable\n"
           "layout (location = 1) in vec4 bar;\n"
           "layout (location = 0) in vec4 foo;\n"
           "layout (location = 2) in float scale;\n"
           "void main() {\n"
           "   gl_FragColor = bar * scale + foo * (1.0-scale);\n"
           "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendDummy();

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, QuadVertFetchAndVertID)
{
    // This tests that attributes work in the presence of gl_VertexID

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //XYZ1( -1, -1, -1 )
            "layout (location = 0) in vec4 pos;\n"
            //XYZ1( 0.f, 0.f, 0.f )
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   vec4 vertices[3];"
            "      vertices[gl_VertexID % 3] = pos;\n"
            "   gl_Position = vertices[(gl_VertexID + 3) % 3];\n"
            "}\n";


    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "void main() {\n"
            "   gl_FragColor = color;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BUF_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BUF_ID,                            // Binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32_SFLOAT;   // format of source data
    vi_attribs[0].offsetInBytes = 0;                // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[1].location = 1;
    vi_attribs[1].format = VK_FORMAT_R32G32_SFLOAT;   // format of source data
    vi_attribs[1].offsetInBytes = 16;                // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs, 2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BUF_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, QuadSparseVertFetch)
{
    // This tests that attributes work in the presence of gl_VertexID

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //XYZ1( -1, -1, -1 )
            "layout (location = 1) in vec4 pos;\n"
            "layout (location = 4) in vec4 inColor;\n"
            //XYZ1( 0.f, 0.f, 0.f )
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = pos;\n"
            "}\n";


    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "void main() {\n"
            "   gl_FragColor = color;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    struct VDATA
    {
        float t1, t2, t3, t4;            // filler data
        float posX, posY, posZ, posW;    // Position data
        float r, g, b, a;                // Color
    };
    const struct VDATA vData[] =
    {
        { XYZ1(0, 0, 0), XYZ1( -1, -1, -1 ), XYZ1( 0.f, 0.f, 0.f ) },
        { XYZ1(0, 0, 0), XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
        { XYZ1(0, 0, 0), XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
        { XYZ1(0, 0, 0), XYZ1( -1,  1, -1 ), XYZ1( 0.f, 1.f, 0.f ) },
        { XYZ1(0, 0, 0), XYZ1( 1, -1, -1 ), XYZ1( 1.f, 0.f, 0.f ) },
        { XYZ1(0, 0, 0), XYZ1( 1,  1, -1 ), XYZ1( 1.f, 1.f, 0.f ) },
    };

    VkConstantBufferObj meshBuffer(m_device,sizeof(vData)/sizeof(vData[0]),sizeof(vData[0]), vData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BUF_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BUF_ID,                            // Binding ID
        sizeof(vData[0]),                       // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BUF_ID;                                        // binding ID
    vi_attribs[0].location = 4;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;                         // format of source data
    vi_attribs[0].offsetInBytes = sizeof(float) * 4 * 2;   // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BUF_ID;                                        // binding ID
    vi_attribs[1].location = 1;
    vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;                         // format of source data
    vi_attribs[1].offsetInBytes = sizeof(float) * 4 * 1;   // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs, 2);
    pipelineobj.AddVertexInputBindings(&vi_binding, 1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BUF_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, MESH_BUF_ID);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriVertFetchDeadAttr)
{
    // This tests that attributes work in the presence of gl_VertexID
    // and a dead attribute in position 0. Draws a triangle with yellow,
    // red and green corners, starting at top and going clockwise.

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            //XYZ1( -1, -1, -1 )
            "layout (location = 0) in vec4 pos;\n"
            //XYZ1( 0.f, 0.f, 0.f )
            "layout (location = 1) in vec4 inColor;\n"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-1.0, -1.0);\n"
            "      vertices[1] = vec2( 1.0, -1.0);\n"
            "      vertices[2] = vec2( 0.0,  1.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";


    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 color;\n"
            "void main() {\n"
            "   gl_FragColor = color;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vbData)/sizeof(g_vbData[0]),sizeof(g_vbData[0]), g_vbData);
    meshBuffer.BufferMemoryBarrier();

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, meshBuffer);

#define MESH_BUF_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BUF_ID,                            // Binding ID
        sizeof(g_vbData[0]),                    // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;            // format of source data
    vi_attribs[0].offsetInBytes = 0;                // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[1].location = 1;
    vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;            // format of source data
    vi_attribs[1].offsetInBytes = 16;                // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs, 2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BUF_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render two triangles
    cmdBuffer.Draw(0, 6, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, CubeWithVertexFetchAndMVP)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140) uniform bufferVals {\n"
            "    mat4 mvp;\n"
            "} myBufferVals;\n"
            "layout (location = 0) in vec4 pos;\n"
            "layout (location = 1) in vec4 inColor;\n"
            "out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = inColor;\n"
            "   gl_Position = myBufferVals.mvp * pos;\n"
            "   gl_Position.y = -gl_Position.y;\n"
            "   gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;\n"
            "}\n";

    static const char *fragShaderText =
            "#version 130\n"
            "in vec4 color;\n"
            "void main() {\n"
            "   gl_FragColor = color;\n"
            "}\n";
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    glm::mat4 View       = glm::lookAt(
                           glm::vec3(0,3,10), // Camera is at (0,3,10), in World Space
                           glm::vec3(0,0,0), // and looks at the origin
                           glm::vec3(0,-1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                           );

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 MVP = Projection * View * Model;

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    m_depthStencil->Init(m_device, m_width, m_height);

    VkConstantBufferObj meshBuffer(m_device,sizeof(g_vb_solid_face_colors_Data)/sizeof(g_vb_solid_face_colors_Data[0]),
            sizeof(g_vb_solid_face_colors_Data[0]), g_vb_solid_face_colors_Data);

    const int buf_size = sizeof(MVP) / sizeof(float);

    VkConstantBufferObj MVPBuffer(m_device, buf_size, sizeof(MVP[0]), (const void*) &MVP[0][0]);
    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkPipelineDsStateCreateInfo ds_state;
    ds_state.depthTestEnable = VK_TRUE;
    ds_state.depthWriteEnable = VK_TRUE;
    ds_state.depthCompareOp = VK_COMPARE_OP_LESS_EQUAL;
    ds_state.depthBoundsEnable = VK_FALSE;
    ds_state.stencilTestEnable = VK_FALSE;
    ds_state.back.stencilDepthFailOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilFailOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilPassOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilCompareOp = VK_COMPARE_OP_ALWAYS;
    ds_state.format = VK_FORMAT_D32_SFLOAT;
    ds_state.front = ds_state.back;
    pipelineobj.SetDepthStencil(&ds_state);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, MVPBuffer);

#define MESH_BUF_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BUF_ID,                            // Binding ID
        sizeof(g_vbData[0]),                     // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX       // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[0].location = 0;
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT;            // format of source data
    vi_attribs[0].offsetInBytes = 0;                // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BUF_ID;            // binding ID
    vi_attribs[1].location = 1;
    vi_attribs[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;            // format of source data
    vi_attribs[1].offsetInBytes = 16;                // Offset of first element in bytes from base of vertex

    pipelineobj.AddVertexInputAttribs(vi_attribs, 2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BUF_ID);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));

    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));
    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangles
    cmdBuffer.Draw(0, 36, 0, 1);


    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, VSTexture)
{
    // The expected result from this test is a green and red triangle;
    // one red vertex on the left, two green vertices on the right.
    static const char *vertShaderText =
            "#version 130\n"
            "out vec4 texColor;\n"
            "uniform sampler2D surface;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec2 positions[3];"
            "      positions[0] = vec2( 0.0, 0.0);\n"
            "      positions[1] = vec2( 0.25, 0.1);\n"
            "      positions[2] = vec2( 0.1, 0.25);\n"
            "   vec2 samplePos = positions[gl_VertexID % 3];\n"
            "   texColor = textureLod(surface, samplePos, 0.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 130\n"
            "in vec4 texColor;\n"
            "void main() {\n"
            "   gl_FragColor = texColor;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);
    VkSamplerObj sampler(m_device);
    VkTextureObj texture(m_device);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif

    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}
TEST_F(VkRenderTest, TexturedTriangle)
{
    // The expected result from this test is a red and green checkered triangle
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) out vec2 samplePos;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec2 positions[3];"
            "      positions[0] = vec2( 0.0, 0.0);\n"
            "      positions[1] = vec2( 1.0, 0.0);\n"
            "      positions[2] = vec2( 1.0, 1.0);\n"
            "   samplePos = positions[gl_VertexID % 3];\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec2 samplePos;\n"
            "layout (binding = 0) uniform sampler2D surface;\n"
            "layout (location=0) out vec4 outColor;\n"
            "void main() {\n"
            "   vec4 texColor = textureLod(surface, samplePos, 0.0);\n"
            "   outColor = texColor;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);
    VkSamplerObj sampler(m_device);
    VkTextureObj texture(m_device);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}
TEST_F(VkRenderTest, TexturedTriangleClip)
{
    // The expected result from this test is a red and green checkered triangle
    static const char *vertShaderText =
            "#version 330\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) out vec2 samplePos;\n"
            "out gl_PerVertex {\n"
            "    vec4 gl_Position;\n"
            "    float gl_ClipDistance[1];\n"
            "};\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec2 positions[3];"
            "      positions[0] = vec2( 0.0, 0.0);\n"
            "      positions[1] = vec2( 1.0, 0.0);\n"
            "      positions[2] = vec2( 1.0, 1.0);\n"
            "   float dists[3];\n"
            "      dists[0] = 1.0;\n"
            "      dists[1] = 1.0;\n"
            "      dists[2] = -1.0;\n"
            "   gl_ClipDistance[0] = dists[gl_VertexID % 3];\n"
            "   samplePos = positions[gl_VertexID % 3];\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec2 samplePos;\n"
            "layout (binding = 0) uniform sampler2D surface;\n"
            "layout (location=0) out vec4 outColor;\n"
            "void main() {\n"
            //"   vec4 texColor = textureLod(surface, samplePos, 0.0 + gl_ClipDistance[0]);\n"
            "   vec4 texColor = textureLod(surface, samplePos, 0.0);\n"
            "   outColor = texColor;\n"
            "}\n";


    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);
    VkSamplerObj sampler(m_device);
    VkTextureObj texture(m_device);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}
TEST_F(VkRenderTest, FSTriangle)
{
    // The expected result from this test is a red and green checkered triangle
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) out vec2 samplePos;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec2 positions[3];"
            "      positions[0] = vec2( 0.0, 0.0);\n"
            "      positions[1] = vec2( 1.0, 0.0);\n"
            "      positions[2] = vec2( 1.0, 1.0);\n"
            "   samplePos = positions[gl_VertexID % 3];\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec2 samplePos;\n"
            "layout (binding = 0) uniform sampler2D surface;\n"
            "layout (location=0) out vec4 outColor;\n"
            "void main() {\n"
            "   vec4 texColor = textureLod(surface, samplePos, 0.0);\n"
            "   outColor = texColor;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);
    VkSamplerObj sampler(m_device);
    VkTextureObj texture(m_device);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}
TEST_F(VkRenderTest, SamplerBindingsTriangle)
{
    // This test sets bindings on the samplers
    // For now we are asserting that sampler and texture pairs
    // march in lock step, and are set via GLSL binding.  This can
    // and will probably change.
    // The sampler bindings should match the sampler and texture slot
    // number set up by the application.
    // This test will result in a blue triangle
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) out vec4 samplePos;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   vec2 positions[3];"
            "      positions[0] = vec2( 0.0, 0.0);\n"
            "      positions[1] = vec2( 1.0, 0.0);\n"
            "      positions[2] = vec2( 1.0, 1.0);\n"
            "   samplePos = vec4(positions[gl_VertexID % 3], 0.0, 0.0);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 samplePos;\n"
            "layout (binding = 0) uniform sampler2D surface0;\n"
            "layout (binding = 1) uniform sampler2D surface1;\n"
            "layout (binding = 12) uniform sampler2D surface2;\n"
            "void main() {\n"
            "   gl_FragColor = textureLod(surface2, samplePos.xy, 0.0);\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkSamplerObj sampler1(m_device);
    VkSamplerObj sampler2(m_device);
    VkSamplerObj sampler3(m_device);

    uint32_t tex_colors[2] = { 0xffff0000, 0xffff0000 };
    VkTextureObj texture1(m_device, tex_colors); // Red
    tex_colors[0] = 0xff00ff00; tex_colors[1] = 0xff00ff00;
    VkTextureObj texture2(m_device, tex_colors); // Green
    tex_colors[0] = 0xff0000ff; tex_colors[1] = 0xff0000ff;
    VkTextureObj texture3(m_device, tex_colors); // Blue

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler1, &texture1);
    descriptorSet.AppendSamplerTexture(&sampler2, &texture2);
    for (int i = 0; i < 10; i++)
        descriptorSet.AppendDummy();
    descriptorSet.AppendSamplerTexture(&sampler3, &texture3);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleVSUniformBlock)
{
    // The expected result from this test is a blue triangle

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) out vec4 outColor;\n"
            "layout (std140, binding = 0) uniform bufferVals {\n"
            "    vec4 red;\n"
            "    vec4 green;\n"
            "    vec4 blue;\n"
            "    vec4 white;\n"
            "} myBufferVals;\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   outColor = myBufferVals.blue;\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (location = 0) in vec4 inColor;\n"
            "void main() {\n"
            "   gl_FragColor = inColor;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    // Let's populate our buffer with the following:
    //     vec4 red;
    //     vec4 green;
    //     vec4 blue;
    //     vec4 white;
    const int valCount = 4 * 4;
    const float bufferVals[valCount] = { 1.0, 0.0, 0.0, 1.0,
                                         0.0, 1.0, 0.0, 1.0,
                                         0.0, 0.0, 1.0, 1.0,
                                         1.0, 1.0, 1.0, 1.0 };

    VkConstantBufferObj colorBuffer(m_device, valCount, sizeof(bufferVals[0]), (const void*) bufferVals);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, colorBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleFSUniformBlockBinding)
{
    // This test allows the shader to select which buffer it is
    // pulling from using layout binding qualifier.
    // There are corresponding changes in the compiler stack that
    // will select the buffer using binding directly.
    // The binding number should match the slot number set up by
    // the application.
    // The expected result from this test is a purple triangle

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform redVal   { vec4 color; } myRedVal\n;"
            "layout (std140, binding = 1) uniform greenVal { vec4 color; } myGreenVal\n;"
            "layout (std140, binding = 2) uniform blueVal  { vec4 color; } myBlueVal\n;"
            "layout (std140, binding = 3) uniform whiteVal { vec4 color; } myWhiteVal\n;"
            "void main() {\n"
            "   gl_FragColor = myBlueVal.color;\n"
            "   gl_FragColor += myRedVal.color;\n"
            "}\n";

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    // We're going to create a number of uniform buffers, and then allow
    // the shader to select which it wants to read from with a binding

    // Let's populate the buffers with a single color each:
    //    layout (std140, binding = 0) uniform bufferVals { vec4 red;   } myRedVal;
    //    layout (std140, binding = 1) uniform bufferVals { vec4 green; } myGreenVal;
    //    layout (std140, binding = 2) uniform bufferVals { vec4 blue;  } myBlueVal;
    //    layout (std140, binding = 3) uniform bufferVals { vec4 white; } myWhiteVal;

    const float redVals[4]   = { 1.0, 0.0, 0.0, 1.0 };
    const float greenVals[4] = { 0.0, 1.0, 0.0, 1.0 };
    const float blueVals[4]  = { 0.0, 0.0, 1.0, 1.0 };
    const float whiteVals[4] = { 1.0, 1.0, 1.0, 1.0 };

    const int redCount   = sizeof(redVals)   / sizeof(float);
    const int greenCount = sizeof(greenVals) / sizeof(float);
    const int blueCount  = sizeof(blueVals)  / sizeof(float);
    const int whiteCount = sizeof(whiteVals) / sizeof(float);

    VkConstantBufferObj redBuffer(m_device, redCount, sizeof(redVals[0]), (const void*) redVals);

    VkConstantBufferObj greenBuffer(m_device, greenCount, sizeof(greenVals[0]), (const void*) greenVals);

    VkConstantBufferObj blueBuffer(m_device, blueCount, sizeof(blueVals[0]), (const void*) blueVals);

    VkConstantBufferObj whiteBuffer(m_device, whiteCount, sizeof(whiteVals[0]), (const void*) whiteVals);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, redBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, greenBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, blueBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, whiteBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleFSAnonymousUniformBlockBinding)
{
    // This test is the same as TriangleFSUniformBlockBinding, but
    // it does not provide an instance name.
    // The expected result from this test is a purple triangle

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 430\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform redVal   { vec4 red; };"
            "layout (std140, binding = 1) uniform greenVal { vec4 green; };"
            "layout (std140, binding = 2) uniform blueVal  { vec4 blue; };"
            "layout (std140, binding = 3) uniform whiteVal { vec4 white; };"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = blue;\n"
            "   outColor += red;\n"
            "}\n";
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    // We're going to create a number of uniform buffers, and then allow
    // the shader to select which it wants to read from with a binding

    // Let's populate the buffers with a single color each:
    //    layout (std140, binding = 0) uniform bufferVals { vec4 red;   } myRedVal;
    //    layout (std140, binding = 1) uniform bufferVals { vec4 green; } myGreenVal;
    //    layout (std140, binding = 2) uniform bufferVals { vec4 blue;  } myBlueVal;
    //    layout (std140, binding = 3) uniform bufferVals { vec4 white; } myWhiteVal;

    const float redVals[4]   = { 1.0, 0.0, 0.0, 1.0 };
    const float greenVals[4] = { 0.0, 1.0, 0.0, 1.0 };
    const float blueVals[4]  = { 0.0, 0.0, 1.0, 1.0 };
    const float whiteVals[4] = { 1.0, 1.0, 1.0, 1.0 };

    const int redCount   = sizeof(redVals)   / sizeof(float);
    const int greenCount = sizeof(greenVals) / sizeof(float);
    const int blueCount  = sizeof(blueVals)  / sizeof(float);
    const int whiteCount = sizeof(whiteVals) / sizeof(float);

    VkConstantBufferObj redBuffer(m_device, redCount, sizeof(redVals[0]), (const void*) redVals);

    VkConstantBufferObj greenBuffer(m_device, greenCount, sizeof(greenVals[0]), (const void*) greenVals);

    VkConstantBufferObj blueBuffer(m_device, blueCount, sizeof(blueVals[0]), (const void*) blueVals);

    VkConstantBufferObj whiteBuffer(m_device, whiteCount, sizeof(whiteVals[0]), (const void*) whiteVals);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, redBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, greenBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, blueBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, whiteBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, CubeWithVertexFetchAndMVPAndTexture)
{
    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding=0) uniform bufferVals {\n"
            "    mat4 mvp;\n"
            "} myBufferVals;\n"
            "layout (location=0) in vec4 pos;\n"
            "layout (location=1) in vec2 input_uv;\n"
            "layout (location=0) out vec2 UV;\n"
            "void main() {\n"
            "   UV = input_uv;\n"
            "   gl_Position = myBufferVals.mvp * pos;\n"
            "   gl_Position.y = -gl_Position.y;\n"
            "   gl_Position.z = (gl_Position.z + gl_Position.w) / 2.0;\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (binding=1) uniform sampler2D surface;\n"
            "layout (location=0) out vec4 outColor;\n"
            "layout (location=0) in vec2 UV;\n"
            "void main() {\n"
            "    outColor= textureLod(surface, UV, 0.0);\n"
            "}\n";
    glm::mat4 Projection = glm::perspective(glm::radians(45.0f), 1.0f, 0.1f, 100.0f);

    glm::mat4 View       = glm::lookAt(
                           glm::vec3(0,3,10), // Camera is at (0,3,10), in World Space
                           glm::vec3(0,0,0), // and looks at the origin
                           glm::vec3(0,1,0)  // Head is up (set to 0,-1,0 to look upside-down)
                           );

    glm::mat4 Model = glm::mat4(1.0f);

    glm::mat4 MVP = Projection * View * Model;
    int num_verts = sizeof(g_vb_texture_Data) / sizeof(g_vb_texture_Data[0]);


    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());
    m_depthStencil->Init(m_device, m_width, m_height);

    VkConstantBufferObj meshBuffer(m_device, num_verts,
            sizeof(g_vb_texture_Data[0]), g_vb_texture_Data);
    meshBuffer.BufferMemoryBarrier();

    const int buf_size = sizeof(MVP) / sizeof(float);

    VkConstantBufferObj mvpBuffer(m_device, buf_size, sizeof(MVP[0]), (const void*) &MVP[0][0]);
    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);
    VkSamplerObj sampler(m_device);
    VkTextureObj texture(m_device);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mvpBuffer);
    descriptorSet.AppendSamplerTexture(&sampler, &texture);

#define MESH_BIND_ID 0
    VkVertexInputBindingDescription vi_binding = {
        MESH_BIND_ID,                      // binding ID
        sizeof(g_vb_texture_Data[0]),               // strideInBytes;  Distance between vertices in bytes (0 = no advancement)
        VK_VERTEX_INPUT_STEP_RATE_VERTEX  // stepRate;       // Rate at which binding is incremented
    };

    VkVertexInputAttributeDescription vi_attribs[2];
    vi_attribs[0].binding = MESH_BIND_ID;                 // Binding ID
    vi_attribs[0].location = 0;                           // location
    vi_attribs[0].format = VK_FORMAT_R32G32B32A32_SFLOAT; // format of source data
    vi_attribs[0].offsetInBytes = 0;                      // Offset of first element in bytes from base of vertex
    vi_attribs[1].binding = MESH_BIND_ID;                 // Binding ID
    vi_attribs[1].location = 1;                           // location
    vi_attribs[1].format = VK_FORMAT_R32G32_SFLOAT;       // format of source data
    vi_attribs[1].offsetInBytes = 16;                     // Offset of uv components

    pipelineobj.AddVertexInputAttribs(vi_attribs,2);
    pipelineobj.AddVertexInputBindings(&vi_binding,1);
    pipelineobj.AddVertexDataBuffer(&meshBuffer, MESH_BIND_ID);

    VkPipelineDsStateCreateInfo ds_state;
    ds_state.depthTestEnable = VK_TRUE;
    ds_state.depthWriteEnable = VK_TRUE;
    ds_state.depthCompareOp = VK_COMPARE_OP_LESS_EQUAL;
    ds_state.depthBoundsEnable = VK_FALSE;
    ds_state.stencilTestEnable = VK_FALSE;
    ds_state.back.stencilDepthFailOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilFailOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilPassOp = VK_STENCIL_OP_KEEP;
    ds_state.back.stencilCompareOp = VK_COMPARE_OP_ALWAYS;
    ds_state.format = VK_FORMAT_D32_SFLOAT;
    ds_state.front = ds_state.back;
    pipelineobj.SetDepthStencil(&ds_state);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget(m_depthStencil->BindInfo()));
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

    cmdBuffer.BindVertexBuffer(&meshBuffer, 0, 0);
#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, num_verts, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
    RotateTriangleVSUniform(Projection, View, Model, &mvpBuffer, &cmdBuffer);
}

TEST_F(VkRenderTest, TriangleMixedSamplerUniformBlockBinding)
{
    // This test mixes binding slots of textures and buffers, ensuring
    // that sparse and overlapping assignments work.
    // The expected result from this test is a purple triangle, although
    // you can modify it to move the desired result around.

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 430\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (binding = 0) uniform sampler2D surface0;\n"
            "layout (binding = 3) uniform sampler2D surface1;\n"
            "layout (binding = 1) uniform sampler2D surface2;\n"
            "layout (binding = 2) uniform sampler2D surface3;\n"


            "layout (std140, binding = 4) uniform redVal   { vec4 red; };"
            "layout (std140, binding = 6) uniform greenVal { vec4 green; };"
            "layout (std140, binding = 5) uniform blueVal  { vec4 blue; };"
            "layout (std140, binding = 7) uniform whiteVal { vec4 white; };"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = red * vec4(0.00001);\n"
            "   outColor += white * vec4(0.00001);\n"
            "   outColor += textureLod(surface2, vec2(0.5), 0.0)* vec4(0.00001);\n"
            "   outColor += textureLod(surface1, vec2(0.0), 0.0);//* vec4(0.00001);\n"
            "}\n";
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    const float redVals[4]   = { 1.0, 0.0, 0.0, 1.0 };
    const float greenVals[4] = { 0.0, 1.0, 0.0, 1.0 };
    const float blueVals[4]  = { 0.0, 0.0, 1.0, 1.0 };
    const float whiteVals[4] = { 1.0, 1.0, 1.0, 1.0 };

    const int redCount   = sizeof(redVals)   / sizeof(float);
    const int greenCount = sizeof(greenVals) / sizeof(float);
    const int blueCount  = sizeof(blueVals)  / sizeof(float);
    const int whiteCount = sizeof(whiteVals) / sizeof(float);

    VkConstantBufferObj redBuffer(m_device, redCount, sizeof(redVals[0]), (const void*) redVals);
    VkConstantBufferObj greenBuffer(m_device, greenCount, sizeof(greenVals[0]), (const void*) greenVals);
    VkConstantBufferObj blueBuffer(m_device, blueCount, sizeof(blueVals[0]), (const void*) blueVals);
    VkConstantBufferObj whiteBuffer(m_device, whiteCount, sizeof(whiteVals[0]), (const void*) whiteVals);

    uint32_t tex_colors[2] = { 0xff800000, 0xff800000 };
    VkSamplerObj sampler0(m_device);
    VkTextureObj texture0(m_device, tex_colors); // Light Red
    tex_colors[0] = 0xff000080; tex_colors[1] = 0xff000080;
    VkSamplerObj sampler2(m_device);
    VkTextureObj texture2(m_device, tex_colors); // Light Blue
    tex_colors[0] = 0xff008000; tex_colors[1] = 0xff008000;
    VkSamplerObj sampler4(m_device);
    VkTextureObj texture4(m_device, tex_colors); // Light Green

    // NOTE:  Bindings 1,3,5,7,8,9,11,12,14,16 work for this sampler, but 6 does not!!!
    // TODO:  Get back here ASAP and understand why.
    tex_colors[0] = 0xffff00ff; tex_colors[1] = 0xffff00ff;
    VkSamplerObj sampler7(m_device);
    VkTextureObj texture7(m_device, tex_colors); // Red and Blue

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler0, &texture0);
    descriptorSet.AppendSamplerTexture(&sampler2, &texture2);
    descriptorSet.AppendSamplerTexture(&sampler4, &texture4);
    descriptorSet.AppendSamplerTexture(&sampler7, &texture7);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, redBuffer);
    // swap blue and green
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, blueBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, greenBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, whiteBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleMatchingSamplerUniformBlockBinding)
{
    // This test matches binding slots of textures and buffers, requiring
    // the driver to give them distinct number spaces.
    // The expected result from this test is a red triangle, although
    // you can modify it to move the desired result around.

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "void main() {\n"
            "   vec2 vertices[3];"
            "      vertices[0] = vec2(-0.5, -0.5);\n"
            "      vertices[1] = vec2( 0.5, -0.5);\n"
            "      vertices[2] = vec2( 0.5,  0.5);\n"
            "   gl_Position = vec4(vertices[gl_VertexID % 3], 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 430\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (binding = 0) uniform sampler2D surface0;\n"
            "layout (binding = 1) uniform sampler2D surface1;\n"
            "layout (binding = 2) uniform sampler2D surface2;\n"
            "layout (binding = 3) uniform sampler2D surface3;\n"
            "layout (std140, binding = 4) uniform redVal   { vec4 red; };"
            "layout (std140, binding = 5) uniform greenVal { vec4 green; };"
            "layout (std140, binding = 6) uniform blueVal  { vec4 blue; };"
            "layout (std140, binding = 7) uniform whiteVal { vec4 white; };"
            "layout (location = 0) out vec4 outColor;\n"
            "void main() {\n"
            "   outColor = red;// * vec4(0.00001);\n"
            "   outColor += white * vec4(0.00001);\n"
            "   outColor += textureLod(surface1, vec2(0.5), 0.0)* vec4(0.00001);\n"
            "   outColor += textureLod(surface3, vec2(0.0), 0.0)* vec4(0.00001);\n"
            "}\n";
    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    const float redVals[4]   = { 1.0, 0.0, 0.0, 1.0 };
    const float greenVals[4] = { 0.0, 1.0, 0.0, 1.0 };
    const float blueVals[4]  = { 0.0, 0.0, 1.0, 1.0 };
    const float whiteVals[4] = { 1.0, 1.0, 1.0, 1.0 };

    const int redCount   = sizeof(redVals)   / sizeof(float);
    const int greenCount = sizeof(greenVals) / sizeof(float);
    const int blueCount  = sizeof(blueVals)  / sizeof(float);
    const int whiteCount = sizeof(whiteVals) / sizeof(float);

    VkConstantBufferObj redBuffer(m_device, redCount, sizeof(redVals[0]), (const void*) redVals);
    VkConstantBufferObj greenBuffer(m_device, greenCount, sizeof(greenVals[0]), (const void*) greenVals);
    VkConstantBufferObj blueBuffer(m_device, blueCount, sizeof(blueVals[0]), (const void*) blueVals);
    VkConstantBufferObj whiteBuffer(m_device, whiteCount, sizeof(whiteVals[0]), (const void*) whiteVals);

    uint32_t tex_colors[2] = { 0xff800000, 0xff800000 };
    VkSamplerObj sampler0(m_device);
    VkTextureObj texture0(m_device, tex_colors); // Light Red
    tex_colors[0] = 0xff000080; tex_colors[1] = 0xff000080;
    VkSamplerObj sampler2(m_device);
    VkTextureObj texture2(m_device, tex_colors); // Light Blue
    tex_colors[0] = 0xff008000; tex_colors[1] = 0xff008000;
    VkSamplerObj sampler4(m_device);
    VkTextureObj texture4(m_device, tex_colors); // Light Green
    tex_colors[0] = 0xffff00ff; tex_colors[1] = 0xffff00ff;
    VkSamplerObj sampler7(m_device);
    VkTextureObj texture7(m_device, tex_colors); // Red and Blue

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendSamplerTexture(&sampler0, &texture0);
    descriptorSet.AppendSamplerTexture(&sampler2, &texture2);
    descriptorSet.AppendSamplerTexture(&sampler4, &texture4);
    descriptorSet.AppendSamplerTexture(&sampler7, &texture7);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, redBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, greenBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, blueBuffer);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, whiteBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

TEST_F(VkRenderTest, TriangleUniformBufferLayout)
{
    // This test populates a buffer with a variety of different data
    // types, then reads them out with a shader.
    // The expected result from this test is a green triangle

    static const char *vertShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform mixedBuffer {\n"
            "    vec4 fRed;\n"
            "    vec4 fGreen;\n"
            "    layout(row_major) mat4 worldToProj;\n"
            "    layout(row_major) mat4 projToWorld;\n"
            "    layout(row_major) mat4 worldToView;\n"
            "    layout(row_major) mat4 viewToProj;\n"
            "    layout(row_major) mat4 worldToShadow[4];\n"
            "    float fZero;\n"
            "    float fOne;\n"
            "    float fTwo;\n"
            "    float fThree;\n"
            "    vec3 fZeroZeroZero;\n"
            "    float fFour;\n"
            "    vec3 fZeroZeroOne;\n"
            "    float fFive;\n"
            "    vec3 fZeroOneZero;\n"
            "    float fSix;\n"
            "    float fSeven;\n"
            "    float fEight;\n"
            "    float fNine;\n"
            "    vec2 fZeroZero;\n"
            "    vec2 fZeroOne;\n"
            "    vec4 fBlue;\n"
            "    vec2 fOneZero;\n"
            "    vec2 fOneOne;\n"
            "    vec3 fZeroOneOne;\n"
            "    float fTen;\n"
            "    float fEleven;\n"
            "    float fTwelve;\n"
            "    vec3 fOneZeroZero;\n"
            "    vec4 uvOffsets[4];\n"
            "};\n"
            "layout (location = 0) out vec4 color;"
            "void main() {\n"

            "   vec4 right = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "   vec4 wrong = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "   \n"

            // do some exact comparisons, even though we should
            // really have an epsilon involved.
            "   vec4 outColor = right;\n"
            "   if (fRed != vec4(1.0, 0.0, 0.0, 1.0))\n"
            "       outColor = wrong;\n"
            "   if (fGreen != vec4(0.0, 1.0, 0.0, 1.0))\n"
            "       outColor = wrong;\n"
            "   if (fBlue != vec4(0.0, 0.0, 1.0, 1.0))\n"
            "       outColor = wrong;\n"

            "   color = outColor;\n"

            // generic position stuff
            "   vec2 vertices;\n"
            "   int vertexSelector = gl_VertexID;\n"
            "   if (vertexSelector == 0)\n"
            "      vertices = vec2(-0.5, -0.5);\n"
            "   else if (vertexSelector == 1)\n"
            "      vertices = vec2( 0.5, -0.5);\n"
            "   else if (vertexSelector == 2)\n"
            "      vertices = vec2( 0.5, 0.5);\n"
            "   else\n"
            "      vertices = vec2( 0.0,  0.0);\n"
            "   gl_Position = vec4(vertices, 0.0, 1.0);\n"
            "}\n";

    static const char *fragShaderText =
            "#version 140\n"
            "#extension GL_ARB_separate_shader_objects : enable\n"
            "#extension GL_ARB_shading_language_420pack : enable\n"
            "layout (std140, binding = 0) uniform mixedBuffer {\n"
            "    vec4 fRed;\n"
            "    vec4 fGreen;\n"
            "    layout(row_major) mat4 worldToProj;\n"
            "    layout(row_major) mat4 projToWorld;\n"
            "    layout(row_major) mat4 worldToView;\n"
            "    layout(row_major) mat4 viewToProj;\n"
            "    layout(row_major) mat4 worldToShadow[4];\n"
            "    float fZero;\n"
            "    float fOne;\n"
            "    float fTwo;\n"
            "    float fThree;\n"
            "    vec3 fZeroZeroZero;\n"
            "    float fFour;\n"
            "    vec3 fZeroZeroOne;\n"
            "    float fFive;\n"
            "    vec3 fZeroOneZero;\n"
            "    float fSix;\n"
            "    float fSeven;\n"
            "    float fEight;\n"
            "    float fNine;\n"
            "    vec2 fZeroZero;\n"
            "    vec2 fZeroOne;\n"
            "    vec4 fBlue;\n"
            "    vec2 fOneZero;\n"
            "    vec2 fOneOne;\n"
            "    vec3 fZeroOneOne;\n"
            "    float fTen;\n"
            "    float fEleven;\n"
            "    float fTwelve;\n"
            "    vec3 fOneZeroZero;\n"
            "    vec4 uvOffsets[4];\n"
            "};\n"
            "layout (location = 0) in vec4 color;\n"
            "void main() {\n"
            "   vec4 right = vec4(0.0, 1.0, 0.0, 1.0);\n"
            "   vec4 wrong = vec4(1.0, 0.0, 0.0, 1.0);\n"
            "   \n"

            // start with VS value to ensure it passed
            "   vec4 outColor = color;\n"

            // do some exact comparisons, even though we should
            // really have an epsilon involved.
            "   if (fRed != vec4(1.0, 0.0, 0.0, 1.0))\n"
            "       outColor = wrong;\n"
            "   if (fGreen != vec4(0.0, 1.0, 0.0, 1.0))\n"
            "       outColor = wrong;\n"
            "   if (projToWorld[1] != vec4(0.0, 2.0, 0.0, 0.0))\n"
            "       outColor = wrong;\n"
            "   if (worldToShadow[2][1] != vec4(0.0, 7.0, 0.0, 0.0))\n"
            "       outColor = wrong;\n"
            "   if (fTwo != 2.0)\n"
            "       outColor = wrong;\n"
            "   if (fOneOne != vec2(1.0, 1.0))\n"
            "       outColor = wrong;\n"
            "   if (fTen != 10.0)\n"
            "       outColor = wrong;\n"
            "   if (uvOffsets[2] != vec4(0.9, 1.0, 1.1, 1.2))\n"
            "       outColor = wrong;\n"
            "   \n"
            "   gl_FragColor = outColor;\n"
            "}\n";


    const float mixedVals[196] = {   1.0, 0.0, 0.0, 1.0,   //        vec4 fRed;            // align
                                     0.0, 1.0, 0.0, 1.0,   //        vec4 fGreen;          // align
                                     1.0, 0.0, 0.0, 1.0,   //        layout(row_major) mat4 worldToProj;
                                     0.0, 1.0, 0.0, 1.0,   //        align
                                     0.0, 0.0, 1.0, 1.0,   //        align
                                     0.0, 0.0, 0.0, 1.0,   //        align
                                     2.0, 0.0, 0.0, 2.0,   //        layout(row_major) mat4 projToWorld;
                                     0.0, 2.0, 0.0, 2.0,   //        align
                                     0.0, 0.0, 2.0, 2.0,   //        align
                                     0.0, 0.0, 0.0, 2.0,   //        align
                                     3.0, 0.0, 0.0, 3.0,   //        layout(row_major) mat4 worldToView;
                                     0.0, 3.0, 0.0, 3.0,   //        align
                                     0.0, 0.0, 3.0, 3.0,   //        align
                                     0.0, 0.0, 0.0, 3.0,   //        align
                                     4.0, 0.0, 0.0, 4.0,   //        layout(row_major) mat4 viewToProj;
                                     0.0, 4.0, 0.0, 4.0,   //        align
                                     0.0, 0.0, 4.0, 4.0,   //        align
                                     0.0, 0.0, 0.0, 4.0,   //        align
                                     5.0, 0.0, 0.0, 5.0,   //        layout(row_major) mat4 worldToShadow[4];
                                     0.0, 5.0, 0.0, 5.0,   //        align
                                     0.0, 0.0, 5.0, 5.0,   //        align
                                     0.0, 0.0, 0.0, 5.0,   //        align
                                     6.0, 0.0, 0.0, 6.0,   //        align
                                     0.0, 6.0, 0.0, 6.0,   //        align
                                     0.0, 0.0, 6.0, 6.0,   //        align
                                     0.0, 0.0, 0.0, 6.0,   //        align
                                     7.0, 0.0, 0.0, 7.0,   //        align
                                     0.0, 7.0, 0.0, 7.0,   //        align
                                     0.0, 0.0, 7.0, 7.0,   //        align
                                     0.0, 0.0, 0.0, 7.0,   //        align
                                     8.0, 0.0, 0.0, 8.0,   //        align
                                     0.0, 8.0, 0.0, 8.0,   //        align
                                     0.0, 0.0, 8.0, 8.0,   //        align
                                     0.0, 0.0, 0.0, 8.0,   //        align
                                     0.0,                  //        float fZero;          // align
                                     1.0,                  //        float fOne;           // pack
                                     2.0,                  //        float fTwo;           // pack
                                     3.0,                  //        float fThree;         // pack
                                     0.0, 0.0, 0.0,        //        vec3 fZeroZeroZero;   // align
                                     4.0,                  //        float fFour;          // pack
                                     0.0, 0.0, 1.0,        //        vec3 fZeroZeroOne;    // align
                                     5.0,                  //        float fFive;          // pack
                                     0.0, 1.0, 0.0,        //        vec3 fZeroOneZero;    // align
                                     6.0,                  //        float fSix;           // pack
                                     7.0,                  //        float fSeven;         // align
                                     8.0,                  //        float fEight;         // pack
                                     9.0,                  //        float fNine;          // pack
                                     0.0,                  //        BUFFER
                                     0.0, 0.0,             //        vec2 fZeroZero;       // align
                                     0.0, 1.0,             //        vec2 fZeroOne;        // pack
                                     0.0, 0.0, 1.0, 1.0,   //        vec4 fBlue;           // align
                                     1.0, 0.0,             //        vec2 fOneZero;        // align
                                     1.0, 1.0,             //        vec2 fOneOne;         // pack
                                     0.0, 1.0, 1.0,        //        vec3 fZeroOneOne;     // align
                                     10.0,                 //        float fTen;           // pack
                                     11.0,                 //        float fEleven;        // align
                                     12.0,                 //        float fTwelve;        // pack
                                     0.0, 0.0,             //        BUFFER
                                     1.0, 0.0, 0.0,        //        vec3 fOneZeroZero;    // align
                                     0.0,                  //        BUFFER
                                     0.1, 0.2, 0.3, 0.4,   //        vec4 uvOffsets[4];
                                     0.5, 0.6, 0.7, 0.8,   //        align
                                     0.9, 1.0, 1.1, 1.2,   //        align
                                     1.3, 1.4, 1.5, 1.6,   //        align
                                  };

    ASSERT_NO_FATAL_FAILURE(InitState());
    ASSERT_NO_FATAL_FAILURE(InitViewport());

    const int constCount   = sizeof(mixedVals)   / sizeof(float);

    VkShaderObj vs(m_device,vertShaderText,VK_SHADER_STAGE_VERTEX, this);
    VkShaderObj ps(m_device,fragShaderText, VK_SHADER_STAGE_FRAGMENT, this);

    VkConstantBufferObj mixedBuffer(m_device, constCount, sizeof(mixedVals[0]), (const void*) mixedVals);

    VkPipelineObj pipelineobj(m_device);
    pipelineobj.AddShader(&vs);
    pipelineobj.AddShader(&ps);

    VkDescriptorSetObj descriptorSet(m_device);
    descriptorSet.AppendBuffer(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, mixedBuffer);

    ASSERT_NO_FATAL_FAILURE(InitRenderTarget());
    VkCommandBufferObj cmdBuffer(m_device);
    cmdBuffer.AddRenderTarget(m_renderTargets[0]);

    ASSERT_VK_SUCCESS(BeginCommandBuffer(cmdBuffer));

    GenericDrawPreparation(&cmdBuffer, pipelineobj, descriptorSet);

#ifdef DUMP_STATE_DOT
    DRAW_STATE_DUMP_DOT_FILE pDSDumpDot = (DRAW_STATE_DUMP_DOT_FILE)vkGetProcAddr(gpu(), (char*)"drawStateDumpDotFile");
    pDSDumpDot((char*)"triTest2.dot");
#endif
    // render triangle
    cmdBuffer.Draw(0, 3, 0, 1);

    // finalize recording of the command buffer
    EndCommandBuffer(cmdBuffer);
    cmdBuffer.QueueCommandBuffer();

    RecordImages(m_renderTargets);
}

int main(int argc, char **argv) {
    int result;

    ::testing::InitGoogleTest(&argc, argv);
    VkTestFramework::InitArgs(&argc, argv);

    ::testing::AddGlobalTestEnvironment(new TestEnvironment);

    result = RUN_ALL_TESTS();

    VkTestFramework::Finish();
    return result;
}
