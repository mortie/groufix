/**
 * This file is part of groufix.
 * Copyright (c) Stef Velzel. All rights reserved.
 *
 * groufix : graphics engine produced by Stef Velzel.
 * www     : <www.vuzzel.nl>
 */

#include "groufix/core.h"
#include <assert.h>
#include <limits.h>


#define _GFX_MAP_FMT(gfxName, vkName) \
	do { \
		if (!_gfx_device_map_format(device, gfxName, vkName)) { \
			gfx_log_error("Could not map "#gfxName" to "#vkName"."); \
			goto clean; \
		} \
	} while(0)

#define _GFX_GET_FORMAT(elem) \
	(*(GFXFormat*)(elem))

#define _GFX_GET_VK_FORMAT(elem) \
	(*(VkFormat*)((char*)(elem) + sizeof(GFXFormat)))

#define _GFX_GET_VK_FORMAT_PROPERTIES(elem) \
	(*(VkFormatProperties*)((char*)(elem) + sizeof(GFXFormat) + sizeof(VkFormat)))

#define _GFX_GET_DISTANCE(fmta, fmtb) \
	((unsigned int)GFX_DIFF((fmta).comps[0], (fmtb).comps[0]) + \
	(unsigned int)GFX_DIFF((fmta).comps[1], (fmtb).comps[1]) + \
	(unsigned int)GFX_DIFF((fmta).comps[2], (fmtb).comps[2]) + \
	(unsigned int)GFX_DIFF((fmta).comps[3], (fmtb).comps[3]))

#define _GFX_GET_FEATURES(vkProps) \
	(((vkProps).bufferFeatures & VK_FORMAT_FEATURE_VERTEX_BUFFER_BIT ? \
		GFX_FORMAT_VERTEX_BUFFER : (GFXFormatFeatures)0) | \
	((vkProps).bufferFeatures & VK_FORMAT_FEATURE_UNIFORM_TEXEL_BUFFER_BIT ? \
		GFX_FORMAT_UNIFORM_TEXEL_BUFFER : (GFXFormatFeatures)0) | \
	((vkProps).bufferFeatures & VK_FORMAT_FEATURE_STORAGE_TEXEL_BUFFER_BIT ? \
		GFX_FORMAT_STORAGE_TEXEL_BUFFER : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_BIT ? \
		GFX_FORMAT_SAMPLED_IMAGE : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT ? \
		GFX_FORMAT_SAMPLED_IMAGE_LINEAR : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_MINMAX_BIT ? \
		GFX_FORMAT_SAMPLED_IMAGE_MINMAX : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_STORAGE_IMAGE_BIT ? \
		GFX_FORMAT_STORAGE_IMAGE : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BIT ? \
		GFX_FORMAT_ATTACHMENT : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_COLOR_ATTACHMENT_BLEND_BIT ? \
		GFX_FORMAT_ATTACHMENT_BLEND : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_SRC_BIT ? \
		GFX_FORMAT_IMAGE_READ : (GFXFormatFeatures)0) | \
	((vkProps).optimalTilingFeatures & VK_FORMAT_FEATURE_TRANSFER_DST_BIT ? \
		GFX_FORMAT_IMAGE_WRITE : (GFXFormatFeatures)0))


/****************************
 * Pushes an element to the format dictionary, mapping a groufix format
 * constant to a Vulkan format enumeration, including format properties.
 * @return Non-zero on success.
 *
 * Skips insertion if the format is not supported at all (!).
 */
static int _gfx_device_map_format(_GFXDevice* device,
                                  GFXFormat fmt, VkFormat vkFmt)
{
	VkFormatProperties props;
	_groufix.vk.GetPhysicalDeviceFormatProperties(
		device->vk.device, vkFmt, &props);

	if (
		props.linearTilingFeatures ||
		props.optimalTilingFeatures ||
		props.bufferFeatures)
	{
		if (!gfx_vec_push(&device->formats, 1, NULL))
			return 0;

		void* elem = gfx_vec_at(&device->formats, device->formats.size-1);
		_GFX_GET_FORMAT(elem) = fmt;
		_GFX_GET_VK_FORMAT(elem) = vkFmt;
		_GFX_GET_VK_FORMAT_PROPERTIES(elem) = props;
	}

	return 1;
}

/****************************/
int _gfx_device_init_formats(_GFXDevice* device)
{
	assert(device != NULL);

	// Initialize the format 'dictionary'.
	// i.e. a vector storing { GFXFormat, VkFormat, VkFormatProperties }.
	// We cannot use an actual map because we want to perform fuzzy search.
	gfx_vec_init(&device->formats,
		sizeof(GFXFormat) +
		sizeof(VkFormat) +
		sizeof(VkFormatProperties));

	_GFX_MAP_FMT(GFX_FORMAT_R4G4_UNORM, VK_FORMAT_R4G4_UNORM_PACK8);
	_GFX_MAP_FMT(GFX_FORMAT_R4G4B4A4_UNORM, VK_FORMAT_R4G4B4A4_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_B4G4R4A4_UNORM, VK_FORMAT_B4G4R4A4_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_R5G6B5_UNORM, VK_FORMAT_R5G6B5_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_B5G6R5_UNORM, VK_FORMAT_B5G6R5_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_R5G5B5A1_UNORM, VK_FORMAT_R5G5B5A1_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_B5G5R5A1_UNORM, VK_FORMAT_B5G5R5A1_UNORM_PACK16);
	_GFX_MAP_FMT(GFX_FORMAT_A1R5G5B5_UNORM, VK_FORMAT_A1R5G5B5_UNORM_PACK16);

	_GFX_MAP_FMT(GFX_FORMAT_R8_UNORM, VK_FORMAT_R8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8_SNORM, VK_FORMAT_R8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8_USCALED, VK_FORMAT_R8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8_SSCALED, VK_FORMAT_R8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8_UINT, VK_FORMAT_R8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8_SINT, VK_FORMAT_R8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8_SRGB, VK_FORMAT_R8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_R8G8_UNORM, VK_FORMAT_R8G8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_SNORM, VK_FORMAT_R8G8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_USCALED, VK_FORMAT_R8G8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_SSCALED, VK_FORMAT_R8G8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_UINT, VK_FORMAT_R8G8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_SINT, VK_FORMAT_R8G8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8_SRGB, VK_FORMAT_R8G8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_UNORM, VK_FORMAT_R8G8B8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_SNORM, VK_FORMAT_R8G8B8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_USCALED, VK_FORMAT_R8G8B8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_SSCALED, VK_FORMAT_R8G8B8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_UINT, VK_FORMAT_R8G8B8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_SINT, VK_FORMAT_R8G8B8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8_SRGB, VK_FORMAT_R8G8B8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_UNORM, VK_FORMAT_B8G8R8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_SNORM, VK_FORMAT_B8G8R8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_USCALED, VK_FORMAT_B8G8R8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_SSCALED, VK_FORMAT_B8G8R8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_UINT, VK_FORMAT_B8G8R8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_SINT, VK_FORMAT_B8G8R8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8_SRGB, VK_FORMAT_B8G8R8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_UNORM, VK_FORMAT_R8G8B8A8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_R8G8B8A8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_USCALED, VK_FORMAT_R8G8B8A8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_SSCALED, VK_FORMAT_R8G8B8A8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_UINT, VK_FORMAT_R8G8B8A8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_SINT, VK_FORMAT_R8G8B8A8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R8G8B8A8_SRGB, VK_FORMAT_R8G8B8A8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_B8G8R8A8_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_SNORM, VK_FORMAT_B8G8R8A8_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_USCALED, VK_FORMAT_B8G8R8A8_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_SSCALED, VK_FORMAT_B8G8R8A8_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_UINT, VK_FORMAT_B8G8R8A8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_SINT, VK_FORMAT_B8G8R8A8_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_B8G8R8A8_SRGB, VK_FORMAT_B8G8R8A8_SRGB);

	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_UNORM, VK_FORMAT_A8B8G8R8_UNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_SNORM, VK_FORMAT_A8B8G8R8_SNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_USCALED, VK_FORMAT_A8B8G8R8_USCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_SSCALED, VK_FORMAT_A8B8G8R8_SSCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_UINT, VK_FORMAT_A8B8G8R8_UINT_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_SINT, VK_FORMAT_A8B8G8R8_SINT_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A8B8G8R8_SRGB, VK_FORMAT_A8B8G8R8_SRGB_PACK32);

	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_UNORM, VK_FORMAT_A2R10G10B10_UNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_SNORM, VK_FORMAT_A2R10G10B10_SNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_USCALED, VK_FORMAT_A2R10G10B10_USCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_SSCALED, VK_FORMAT_A2R10G10B10_SSCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_UINT, VK_FORMAT_A2R10G10B10_UINT_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2R10G10B10_SINT, VK_FORMAT_A2R10G10B10_SINT_PACK32);

	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_UNORM, VK_FORMAT_A2B10G10R10_UNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_SNORM, VK_FORMAT_A2B10G10R10_SNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_USCALED, VK_FORMAT_A2B10G10R10_USCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_SSCALED, VK_FORMAT_A2B10G10R10_SSCALED_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_UINT, VK_FORMAT_A2B10G10R10_UINT_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_A2B10G10R10_SINT, VK_FORMAT_A2B10G10R10_SINT_PACK32);

	_GFX_MAP_FMT(GFX_FORMAT_R16_UNORM, VK_FORMAT_R16_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16_SNORM, VK_FORMAT_R16_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16_USCALED, VK_FORMAT_R16_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16_SSCALED, VK_FORMAT_R16_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16_UINT, VK_FORMAT_R16_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16_SINT, VK_FORMAT_R16_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16_SFLOAT, VK_FORMAT_R16_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R16G16_UNORM, VK_FORMAT_R16G16_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_SNORM, VK_FORMAT_R16G16_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_USCALED, VK_FORMAT_R16G16_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_SSCALED, VK_FORMAT_R16G16_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_UINT, VK_FORMAT_R16G16_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_SINT, VK_FORMAT_R16G16_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16_SFLOAT, VK_FORMAT_R16G16_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_UNORM, VK_FORMAT_R16G16B16_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_SNORM, VK_FORMAT_R16G16B16_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_USCALED, VK_FORMAT_R16G16B16_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_SSCALED, VK_FORMAT_R16G16B16_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_UINT, VK_FORMAT_R16G16B16_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_SINT, VK_FORMAT_R16G16B16_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16_SFLOAT, VK_FORMAT_R16G16B16_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_UNORM, VK_FORMAT_R16G16B16A16_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_SNORM, VK_FORMAT_R16G16B16A16_SNORM);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_USCALED, VK_FORMAT_R16G16B16A16_USCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_SSCALED, VK_FORMAT_R16G16B16A16_SSCALED);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_UINT, VK_FORMAT_R16G16B16A16_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_SINT, VK_FORMAT_R16G16B16A16_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R16G16B16A16_SFLOAT, VK_FORMAT_R16G16B16A16_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R32_UINT, VK_FORMAT_R32_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32_SINT, VK_FORMAT_R32_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32_SFLOAT, VK_FORMAT_R32_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R32G32_UINT, VK_FORMAT_R32G32_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32_SINT, VK_FORMAT_R32G32_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32_UINT, VK_FORMAT_R32G32B32_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32_SINT, VK_FORMAT_R32G32B32_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32A32_UINT, VK_FORMAT_R32G32B32A32_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32A32_SINT, VK_FORMAT_R32G32B32A32_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R32G32B32A32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R64_UINT, VK_FORMAT_R64_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64_SINT, VK_FORMAT_R64_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64_SFLOAT, VK_FORMAT_R64_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R64G64_UINT, VK_FORMAT_R64G64_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64_SINT, VK_FORMAT_R64G64_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64_SFLOAT, VK_FORMAT_R64G64_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64_UINT, VK_FORMAT_R64G64B64_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64_SINT, VK_FORMAT_R64G64B64_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64_SFLOAT, VK_FORMAT_R64G64B64_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64A64_UINT, VK_FORMAT_R64G64B64A64_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64A64_SINT, VK_FORMAT_R64G64B64A64_SINT);
	_GFX_MAP_FMT(GFX_FORMAT_R64G64B64A64_SFLOAT, VK_FORMAT_R64G64B64A64_SFLOAT);

	_GFX_MAP_FMT(GFX_FORMAT_B10G11R11_UFLOAT, VK_FORMAT_B10G11R11_UFLOAT_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_E5B9G9R9_UFLOAT, VK_FORMAT_E5B9G9R9_UFLOAT_PACK32);

	_GFX_MAP_FMT(GFX_FORMAT_D16_UNORM, VK_FORMAT_D16_UNORM);
	_GFX_MAP_FMT(GFX_FORMAT_X8_D24_UNORM, VK_FORMAT_X8_D24_UNORM_PACK32);
	_GFX_MAP_FMT(GFX_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT);
	_GFX_MAP_FMT(GFX_FORMAT_S8_UINT, VK_FORMAT_S8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_D16_UNORM_S8_UINT, VK_FORMAT_D16_UNORM_S8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_D24_UNORM_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT);
	_GFX_MAP_FMT(GFX_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D32_SFLOAT_S8_UINT);

	_GFX_MAP_FMT(GFX_FORMAT_BC1_RGB_UNORM, VK_FORMAT_BC1_RGB_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC1_RGB_SRGB, VK_FORMAT_BC1_RGB_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC1_RGBA_UNORM, VK_FORMAT_BC1_RGBA_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC1_RGBA_SRGB, VK_FORMAT_BC1_RGBA_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC2_UNORM, VK_FORMAT_BC2_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC2_SRGB, VK_FORMAT_BC2_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC3_UNORM, VK_FORMAT_BC3_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC3_SRGB, VK_FORMAT_BC3_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC4_UNORM, VK_FORMAT_BC4_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC4_SNORM, VK_FORMAT_BC4_SNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC5_UNORM, VK_FORMAT_BC5_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC5_SNORM, VK_FORMAT_BC5_SNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC6_UFLOAT, VK_FORMAT_BC6H_UFLOAT_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC6_SFLOAT, VK_FORMAT_BC6H_SFLOAT_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC7_UNORM, VK_FORMAT_BC7_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_BC7_SRGB, VK_FORMAT_BC7_SRGB_BLOCK);

	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8_UNORM, VK_FORMAT_ETC2_R8G8B8_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8_SRGB, VK_FORMAT_ETC2_R8G8B8_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8A1_UNORM, VK_FORMAT_ETC2_R8G8B8A1_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8A1_SRGB, VK_FORMAT_ETC2_R8G8B8A1_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8A8_UNORM, VK_FORMAT_ETC2_R8G8B8A8_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ETC2_R8G8B8A8_SRGB, VK_FORMAT_ETC2_R8G8B8A8_SRGB_BLOCK);

	_GFX_MAP_FMT(GFX_FORMAT_EAC_R11_UNORM, VK_FORMAT_EAC_R11_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_EAC_R11_SNORM, VK_FORMAT_EAC_R11_SNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_EAC_R11G11_UNORM, VK_FORMAT_EAC_R11G11_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_EAC_R11G11_SNORM, VK_FORMAT_EAC_R11G11_SNORM_BLOCK);

	_GFX_MAP_FMT(GFX_FORMAT_ASTC_4x4_UNORM, VK_FORMAT_ASTC_4x4_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_4x4_SRGB, VK_FORMAT_ASTC_4x4_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_5x4_UNORM, VK_FORMAT_ASTC_5x4_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_5x4_SRGB, VK_FORMAT_ASTC_5x4_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_5x5_UNORM, VK_FORMAT_ASTC_5x5_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_5x5_SRGB, VK_FORMAT_ASTC_5x5_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_6x5_UNORM, VK_FORMAT_ASTC_6x5_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_6x5_SRGB, VK_FORMAT_ASTC_6x5_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_6x6_UNORM, VK_FORMAT_ASTC_6x6_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_6x6_SRGB, VK_FORMAT_ASTC_6x6_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x5_UNORM, VK_FORMAT_ASTC_8x5_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x5_SRGB, VK_FORMAT_ASTC_8x5_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x6_UNORM, VK_FORMAT_ASTC_8x6_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x6_SRGB, VK_FORMAT_ASTC_8x6_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x8_UNORM, VK_FORMAT_ASTC_8x8_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_8x8_SRGB, VK_FORMAT_ASTC_8x8_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x5_UNORM, VK_FORMAT_ASTC_10x5_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x5_SRGB, VK_FORMAT_ASTC_10x5_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x6_UNORM, VK_FORMAT_ASTC_10x6_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x6_SRGB, VK_FORMAT_ASTC_10x6_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x8_UNORM, VK_FORMAT_ASTC_10x8_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x8_SRGB, VK_FORMAT_ASTC_10x8_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x10_UNORM, VK_FORMAT_ASTC_10x10_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_10x10_SRGB, VK_FORMAT_ASTC_10x10_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_12x10_UNORM, VK_FORMAT_ASTC_12x10_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_12x10_SRGB, VK_FORMAT_ASTC_12x10_SRGB_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_12x12_UNORM, VK_FORMAT_ASTC_12x12_UNORM_BLOCK);
	_GFX_MAP_FMT(GFX_FORMAT_ASTC_12x12_SRGB, VK_FORMAT_ASTC_12x12_SRGB_BLOCK);

	return 1;


	// Clean on failure.
clean:
	gfx_log_error(
		"[ %s ] could not initialize format dictionary.",
		device->name);

	gfx_vec_clear(&device->formats);

	return 0;
}

/****************************/
VkFormat _gfx_resolve_format(_GFXDevice* device,
                             GFXFormat* fmt, const VkFormatProperties* props)
{
	assert(device != NULL);
	assert(fmt != NULL);

	VkFormat vkFmt = VK_FORMAT_UNDEFINED;
	GFXFormat gfxFmt = GFX_FORMAT_EMPTY;
	unsigned int dist = UINT_MAX;

	// Loop over all known formats of the device.
	for (size_t f = 0; f < device->formats.size; ++f)
	{
		void* elem = gfx_vec_at(&device->formats, f);
		VkFormatProperties* eProps = &_GFX_GET_VK_FORMAT_PROPERTIES(elem);

		// Match against the given format & minimal properties.
		if (
			!GFX_FORMAT_IS_CONTAINED(_GFX_GET_FORMAT(elem), *fmt) ||
			(props &&
				((props->linearTilingFeatures & eProps->linearTilingFeatures)
					!= props->linearTilingFeatures ||
				(props->optimalTilingFeatures & eProps->optimalTilingFeatures)
					!= props->optimalTilingFeatures ||
				(props->bufferFeatures & eProps->bufferFeatures)
					!= props->bufferFeatures)))
		{
			continue;
		}

		// Get 'closest' match.
		unsigned int d = _GFX_GET_DISTANCE(_GFX_GET_FORMAT(elem), *fmt);
		if (d < dist)
		{
			vkFmt = _GFX_GET_VK_FORMAT(elem);
			gfxFmt = _GFX_GET_FORMAT(elem);
			dist = d;
		}
	}

	// Return found data.
	*fmt = gfxFmt;
	return vkFmt;
}

/****************************/
GFX_API GFXFormatFeatures gfx_format_support(GFXFormat fmt,
                                             GFXDevice* device)
{
	_GFXDevice* dev;
	_GFX_GET_DEVICE(dev, device);

	GFXFormatFeatures features = 0;

	// Loop over all known formats of the device.
	for (size_t f = 0; f < dev->formats.size; ++f)
	{
		void* elem = gfx_vec_at(&dev->formats, f);

		// Match against the given format.
		if (!GFX_FORMAT_IS_CONTAINED(_GFX_GET_FORMAT(elem), fmt))
			continue;

		// If a match, add supported features.
		features |= _GFX_GET_FEATURES(_GFX_GET_VK_FORMAT_PROPERTIES(elem));
	}

	return features;
}

/****************************/
GFX_API GFXFormat gfx_format_fuzzy(GFXFormat fmt, GFXFuzzyFlags flags,
                                   GFXFormatFeatures features,
                                   GFXDevice* device)
{
	_GFXDevice* dev;
	_GFX_GET_DEVICE(dev, device);

	GFXFormat retFmt = GFX_FORMAT_EMPTY;
	int contained = 0;
	unsigned int dist = UINT_MAX;

	// Loop over all known formats of the device.
	for (size_t f = 0; f < dev->formats.size; ++f)
	{
		void* elem = gfx_vec_at(&dev->formats, f);
		GFXFormat* eFmt = &_GFX_GET_FORMAT(elem);

		// Match against the given format type/order and minimal features.
		// Note this is not the same as GFX_FORMAT_IS_CONTAINED(*eFmt, fmt)!
		// Containment checks for bit depth as well, we do not care about
		// the depth here, we fuzzy search over _ALL_ depths!
		GFXFormatFeatures eFeatures =
			_GFX_GET_FEATURES(_GFX_GET_VK_FORMAT_PROPERTIES(elem));

		if (
			(features & eFeatures) != features ||
			(eFmt->type & fmt.type) != eFmt->type ||
			(GFX_FORMAT_IS_COMPRESSED(*eFmt) ?
				eFmt->order != fmt.order :
				(eFmt->order & fmt.order) != eFmt->order))
		{
			continue;
		}

		// We do however match against given bit depth requirements.
		if (
			((flags & GFX_FUZZY_MIN_DEPTH) &&
				(eFmt->comps[0] < fmt.comps[0] ||
				eFmt->comps[1] < fmt.comps[1] ||
				eFmt->comps[2] < fmt.comps[2] ||
				eFmt->comps[3] < fmt.comps[3])) ||
			((flags & GFX_FUZZY_MAX_DEPTH) &&
				(eFmt->comps[0] > fmt.comps[0] ||
				eFmt->comps[1] > fmt.comps[1] ||
				eFmt->comps[2] > fmt.comps[2] ||
				eFmt->comps[3] > fmt.comps[3])))
		{
			continue;
		}

		// Get 'closest' match.
		// We always prefer contained formats.
		int cont = GFX_FORMAT_IS_CONTAINED(*eFmt, fmt);
		unsigned int d = _GFX_GET_DISTANCE(*eFmt, fmt);

		if (contained ? (cont && d < dist) : (cont || d < dist))
		{
			retFmt = *eFmt;
			contained = cont;
			dist = d;
		}
	}

	return retFmt;
}
