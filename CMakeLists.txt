cmake_minimum_required(VERSION 3.22.1)

project(VoxelGore VERSION 1.0.0 LANGUAGES CXX C)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG")
set(CMAKE_C_FLAGS_RELEASE "-O3 -DNDEBUG")

find_package(Vulkan REQUIRED)

# ============================================================
# Paths
# ============================================================

set(LUAJIT_ROOT "${CMAKE_SOURCE_DIR}")

set(LUAJIT_INCLUDE_DIR
        "${LUAJIT_ROOT}/include/luajit"
)

set(LUAJIT_LIBRARY
        "${CMAKE_SOURCE_DIR}/libs/android/libluajit.a"
)

message(STATUS "LuaJIT include: ${LUAJIT_INCLUDE_DIR}")
message(STATUS "LuaJIT library: ${LUAJIT_LIBRARY}")

# ============================================================
# Native App Glue
# ============================================================

add_library(native_app_glue STATIC
        ${ANDROID_NDK}/sources/android/native_app_glue/android_native_app_glue.c
)

set_target_properties(native_app_glue PROPERTIES
        LINKER_LANGUAGE C
)

# ============================================================
# Sources
# ============================================================

set(SOURCE_FILES

        # Root
        src/main_android.cpp

        src/Game.cpp
        src/WorldSession.cpp
        src/RenderWorld.cpp
        src/PlayerController.cpp
     
        # Engine
        src/Engine.cpp

        # vkcore - common
        src/vkcore/common/SystemError.cpp
        src/vkcore/common/vulkanFunctions.cpp

        # vkcore - commands
        src/vkcore/commands/CommandBuffer.cpp

        # vkcore - instances / debug
        src/vkcore/instances/LibraryLoader.cpp
        src/vkcore/instances/Instance.cpp
        src/vkcore/debug/DebugMessenger.cpp

        # vkcore - devices
        src/vkcore/devices/PhysicalDevice.cpp
        src/vkcore/devices/Device.cpp
        src/vkcore/devices/DeviceCreator.cpp

        # vkcore - present / sync / commands
        src/vkcore/present/Surface.cpp
        src/vkcore/present/Swapchain.cpp
        src/vkcore/sync/Semaphore.cpp
        src/vkcore/sync/Fence.cpp

        # vkcore - pipeline
        src/vkcore/pipeline/RenderPass.cpp
        src/vkcore/pipeline/Framebuffer.cpp
        src/vkcore/pipeline/PipelineLayout.cpp
        src/vkcore/pipeline/GraphicsPipelineCreator.cpp
        src/vkcore/pipeline/ShaderModule.cpp

        # vkcore - resources
        src/vkcore/resource/DescriptorSetLayout.cpp
        src/vkcore/resource/Buffer.cpp
        src/vkcore/resource/MemoryAllocator.cpp
        src/vkcore/resource/DeviceMemory.cpp
        src/vkcore/resource/MemoryBlock.cpp
        src/vkcore/resource/BufferAllocator.cpp
        src/vkcore/resource/BufferBlock.cpp
        src/vkcore/resource/memoryUtils.cpp
        src/vkcore/resource/ImageView.cpp
        src/vkcore/resource/Image.cpp
        src/vkcore/resource/Texture.cpp
        src/vkcore/resource/Sampler.cpp
        src/vkcore/resource/imageUtils.cpp
        src/vkcore/resource/SampledTexture.cpp
        src/vkcore/resource/DescriptorPool.cpp
        src/vkcore/resource/TransferContext.cpp
        src/vkcore/resource/FreeListAllocator.cpp

        # core
        src/core/InputAndroid.cpp
        src/core/CameraShake.cpp
        src/core/InputState.cpp
        src/core/Camera.cpp
        # game
        src/game/entity/EntityParser.cpp
        src/game/entity/EntityFactory.cpp
        src/game/entity/EntityDefinitionRegistry.cpp

        src/game/voxel/Chunk.cpp
        src/game/voxel/ChunkManager.cpp
        src/game/voxel/Block.cpp
        src/game/voxel/BlockParser.cpp
        src/game/voxel/BlockDebrisParser.cpp
        src/game/voxel/BlockManager.cpp
        src/game/physics/CollisionResolver.cpp

        src/game/lighting/ChannelProcessor.cpp
        src/game/lighting/Lighting.cpp
        src/game/lighting/LighChunk.cpp
        src/game/lighting/LightChunkStorage.cpp
        src/game/lighting/ChannelParser.cpp 
        src/game/lighting/ChannelRegistry.cpp 
        src/game/lighting/BlockLightCache.cpp 

        src/game/LibControl.cpp

        # gfx
        src/gfx/common/ShaderCompiler.cpp
        src/gfx/common/ShaderIncluder.cpp
        src/gfx/common/mesh/Mesh.cpp
        src/gfx/common/mesh/TranslucentMesh.cpp
        src/gfx/common/mesh/Model.cpp
        src/gfx/common/mesh/ModelCache.cpp
        src/gfx/common/mesh/ModelLoader.cpp

        src/gfx/common/texture/Atlas.cpp
        src/gfx/common/texture/ImageRegionAllocator.cpp
        src/gfx/common/texture/SpriteSheetParser.cpp
        src/gfx/common/texture/SpriteSheet.cpp
        src/gfx/common/texture/TextureLoader.cpp
        src/gfx/common/texture/TextureCache.cpp
        src/gfx/common/texture/TextureManager.cpp
        src/gfx/common/texture/MaterialManager.cpp

        src/gfx/render/block/BlockSurfaceRegistry.cpp
        src/gfx/render/block/BlockAnimation.cpp
        src/gfx/render/block/BlockAnimationParser.cpp
        src/gfx/render/block/BlockUvBuffer.cpp
        src/gfx/render/block/BlockPreviewRenderer.cpp
        src/gfx/render/block/BlockRenderData.cpp
        src/gfx/render/block/BlockRenderGroupRegistry.cpp

        src/gfx/render/billboard/BillboardBatch.cpp
        src/gfx/render/billboard/BillboardPipelines.cpp
        src/gfx/render/billboard/BillboardRenderBucket.cpp
        src/gfx/render/billboard/BillboardRenderer.cpp

        src/gfx/render/GameDataBinding.cpp
        src/gfx/render/chunk/ChunkMeshBuilder.cpp
        src/gfx/render/chunk/ChunkRenderer.cpp
        src/gfx/render/chunk/ChunkRenderPipelines.cpp
        src/gfx/render/entity/EntityRenderData.cpp
        src/gfx/render/entity/EntityRenderSystem.cpp
        src/gfx/render/ModelPipeline.cpp
        src/gfx/render/ModelRenderer.cpp

        src/gfx/render/particle/ParticleEmitter.cpp
        src/gfx/render/particle/ParticleEngine.cpp
        src/gfx/render/particle/BlockDebrisEmitter.cpp
        src/gfx/render/particle/ExplosionEmitter.cpp
        
        # UI
        src/gfx/render/ui/Node.cpp
        src/gfx/render/ui/Container.cpp
        src/gfx/render/ui/Page.cpp
        src/gfx/render/ui/NodeParser.cpp
        src/gfx/render/ui/Renderer.cpp
        src/gfx/render/ui/NodeBinding.cpp
        src/gfx/render/ui/LibGui.cpp
        src/gfx/render/ui/DescriptorSetRegistry.cpp

        # scripting
        src/script/LuaState.cpp
        src/script/LuaScript.cpp

        # third_party
        third_party/tinyxml2.cpp
)

# ============================================================
# VoxelGore
# ============================================================

add_library(VoxelGore SHARED
        ${SOURCE_FILES}
)

# ============================================================
# Includes
# ============================================================

target_include_directories(VoxelGore PRIVATE

        # Vulkan
        ${Vulkan_INCLUDE_DIRS}

        # Android native_app_glue
        ${ANDROID_NDK}/sources/android/native_app_glue

        # Project / external headers
        ${CMAKE_SOURCE_DIR}/include

        # LuaJIT
        ${LUAJIT_INCLUDE_DIR}
)

# ============================================================
# Linker
# ============================================================

target_link_options(VoxelGore PRIVATE
        "-Wl,-u,ANativeActivity_onCreate"
)

target_link_libraries(VoxelGore PRIVATE

        # Android
        native_app_glue
        android
        log
        dl
        vulkan    

        # Standard math
        m

        # Vulkan
        ${Vulkan_LIBRARIES}

        # LuaJIT
        ${LUAJIT_LIBRARY}

        # shaderc
        shaderc
)

# ============================================================
# Compile definitions
# ============================================================

target_compile_definitions(VoxelGore PRIVATE

        VK_USE_PLATFORM_ANDROID_KHR
        VK_NO_PROTOTYPES

        GLM_ENABLE_EXPERIMENTAL
        GLM_FORCE_DEPTH_ZERO_TO_ONE
        GLM_FORCE_RADIANS

        STB_IMAGE_IMPLEMENTATION
        STB_IMAGE_STATIC

        TINYOBJLOADER_IMPLEMENTATION
        TINYOBJLOADER_DISABLE_FAST_FLOAT

        DISABLE_VVLS
)
 
# ============================================================
# Output
# ============================================================

set_target_properties(VoxelGore PROPERTIES
        OUTPUT_NAME "VoxelGore"
)

message(STATUS "Build type: ${CMAKE_BUILD_TYPE}")
message(STATUS "Output: ${CMAKE_BINARY_DIR}/libVoxelGore.so")