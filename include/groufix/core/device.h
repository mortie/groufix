/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */


#ifndef GFX_CORE_DEVICE_H
#define GFX_CORE_DEVICE_H

#include "groufix/def.h"


/**
 * Physical device type.
 * From most preferred to least preferred.
 */
typedef enum GFXDeviceType
{
	GFX_DEVICE_DISCRETE_GPU,
	GFX_DEVICE_VIRTUAL_GPU,
	GFX_DEVICE_INTEGRATED_GPU,
	GFX_DEVICE_CPU,
	GFX_DEVICE_UNKNOWN

} GFXDeviceType;


/**
 * Physical device definition (e.g. a GPU).
 */
typedef struct GFXDevice
{
	// All read-only.
	GFXDeviceType type;
	const char*   name;

	bool available; // Zero if it does not support the required Vulkan version.


	// Device features.
	struct
	{
		bool indexUint32;
		bool cubeArray;
		bool geometryShader;
		bool tessellationShader;
		bool compressionBC;
		bool compressionETC2; // Includes EAC compression.
		bool compressionASTC;
		bool shaderClipDistance;
		bool shaderCullDistance;
		bool shaderInt8;
		bool shaderInt16;
		bool shaderInt64;
		bool shaderFloat16;
		bool shaderFloat64;
		bool shaderPushConstant8;
		bool shaderPushConstant16;
		bool shaderInputOutput16;
		bool samplerAnisotropy;
		bool samplerClampToEdgeMirror;
		bool samplerMinmax;

	} features;

	// Device limits.
	struct
	{
		uint32_t maxIndexUint32;
		uint32_t maxImageSize1D;   // For { width }.
		uint32_t maxImageSize2D;   // For { width, height }.
		uint32_t maxImageSize3D;   // For { width, height, depth }.
		uint32_t maxImageSizeCube; // For { width, height }.
		uint32_t maxImageLayers;
		uint32_t maxBufferTexels;
		uint32_t maxUniformBufferRange;
		uint32_t maxStorageBufferRange;
		uint32_t maxPushConstantSize;
		uint32_t maxBoundSets;
		uint32_t maxAttributes;
		uint32_t maxAttributeOffset;
		uint32_t maxAttributeStride;
		uint32_t maxPrimitiveBuffers;

		uint32_t maxStageUniformBuffers;
		uint32_t maxStageStorageBuffers;
		uint32_t maxStageSampledImages;
		uint32_t maxStageStorageImages;
		uint32_t maxStageSamplers;
		uint32_t maxStageAttachmentInputs;

		uint32_t maxSetUniformBuffers; // Includes dynamic.
		uint32_t maxSetStorageBuffers; // Includes dynamic.
		uint32_t maxSetUniformBuffersDynamic; // Only dynamic.
		uint32_t maxSetStorageBuffersDynamic; // Only dynamic.
		uint32_t maxSetSampledImages;
		uint32_t maxSetStorageImages;
		uint32_t maxSetSamplers;
		uint32_t maxSetAttachmentInputs;

		uint64_t minTexelBufferAlign;
		uint64_t minUniformBufferAlign;
		uint64_t minStorageBufferAlign;

		float maxMipLodBias;
		float maxAnisotropy;

		// Async-transfer image granularity (0,0,0 = only whole mip levels).
		struct { uint32_t x, y, z; } imageTransferGranularity;

	} limits;

} GFXDevice;


/**
 * Retrieves the number of initialized devices.
 * @return 0 if no devices were found.
 *
 * Can be called from any thread.
 */
GFX_API size_t gfx_get_num_devices(void);

/**
 * Retrieves an initialized device.
 * The primary device is always stored at index 0 and stays constant.
 * @param index Must be < gfx_get_num_devices().
 *
 * Can be called from any thread.
 */
GFX_API GFXDevice* gfx_get_device(size_t index);

/**
 * Retrieves the primary device.
 * This is equivalent to gfx_get_device(0).
 *
 * Can be called from any thread.
 */
GFX_API GFXDevice* gfx_get_primary_device(void);


#endif
