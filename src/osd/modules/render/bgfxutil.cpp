// license:BSD-3-Clause
// copyright-holders:Ryan Holtz
//============================================================
//
//  bgfxutil.cpp - BGFX renderer helper utils
//
//============================================================

// MAMEOS headers
#include "bgfxutil.h"
#include "copyutil.h"

#include "emucore.h"
#include "render.h"


const bgfx::Memory* bgfx_util::mame_texture_data_to_bgfx_texture_data(bgfx::TextureFormat::Enum &dst_format, uint32_t src_format, int rowpixels, int width_margin, int height, const rgb_t *palette, void *base, uint16_t &out_pitch, int &width_div_factor, int &width_mul_factor)
{
	bgfx::TextureInfo info;
	const bgfx::Memory *data = nullptr;
	uint8_t *adjusted_base = (uint8_t *)base;

	switch (src_format)
	{
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
			dst_format = bgfx::TextureFormat::BGRA8;
			width_div_factor = 2;
			width_mul_factor = 1;
			out_pitch = rowpixels * 2;
			bgfx::calcTextureSize(info, rowpixels / width_div_factor, height, 1, false, false, 1, dst_format);

			if (width_margin)
			{
				adjusted_base -= width_margin * 2;
			}
			data = bgfx::copy(adjusted_base, info.storageSize);
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
			dst_format = bgfx::TextureFormat::R8;
			width_div_factor = 1;
			width_mul_factor = 2;
			out_pitch = rowpixels * 2;
			bgfx::calcTextureSize(info, rowpixels * width_mul_factor, height, 1, false, false, 1, dst_format);

			if (width_margin)
			{
				adjusted_base -= width_margin * 2;
			}
			data = bgfx::copy(adjusted_base, info.storageSize);
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
			dst_format = bgfx::TextureFormat::BGRA8;
			width_div_factor = 1;
			width_mul_factor = 1;
			out_pitch = rowpixels * 4;
			bgfx::calcTextureSize(info, rowpixels, height, 1, false, false, 1, dst_format);

			if (width_margin)
			{
				adjusted_base -= width_margin * 4;
			}
			data = bgfx::copy(adjusted_base, info.storageSize);
			break;
	}

	return data;
}

const bgfx::Memory* bgfx_util::mame_texture_data_to_bgra32(uint32_t src_format, int width, int height, int rowpixels, const rgb_t *palette, void *base)
{
	const bgfx::Memory* mem = bgfx::alloc(rowpixels * height * 4);
	auto* dst = reinterpret_cast<uint32_t*>(mem->data);
	auto* src16 = reinterpret_cast<uint16_t*>(base);
	auto* src32 = reinterpret_cast<uint32_t*>(base);

	for (int y = 0; y < height; y++)
	{
		switch (src_format)
		{
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_PALETTE16):
			copy_util::copyline_palette16_to_bgra(dst, src16, width, palette);
			src16 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_YUY16):
			copy_util::copyline_yuy16_to_bgra(dst, src16, width, palette, 1);
			src16 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_ARGB32):
			copy_util::copyline_argb32_to_bgra(dst, src32, width, palette);
			src32 += rowpixels;
			break;
		case PRIMFLAG_TEXFORMAT(TEXFORMAT_RGB32):
			copy_util::copyline_rgb32_to_bgra(dst, src32, width, palette);
			src32 += rowpixels;
			break;
		default:
			break;
		}
		dst += width;
	}
	return mem;
}

uint64_t bgfx_util::get_blend_state(uint32_t blend)
{
	switch (blend)
	{
		case BLENDMODE_ALPHA:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_INV_SRC_ALPHA);
		case BLENDMODE_RGB_MULTIPLY:
			//return BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_INV_SRC_COLOR, BGFX_STATE_BLEND_ONE, BGFX_STATE_BLEND_ZERO);
			return BGFX_STATE_BLEND_FUNC_SEPARATE(BGFX_STATE_BLEND_DST_COLOR, BGFX_STATE_BLEND_ZERO, BGFX_STATE_BLEND_DST_ALPHA, BGFX_STATE_BLEND_ZERO);
		case BLENDMODE_ADD:
			return BGFX_STATE_BLEND_FUNC(BGFX_STATE_BLEND_SRC_ALPHA, BGFX_STATE_BLEND_ONE);
		default:
			return 0L;
	}
	return 0L;
}

void bgfx_util::find_prescale_factor(uint16_t width, uint16_t height, uint16_t max_prescale_size, uint16_t &xprescale, uint16_t &yprescale)
{
	while (xprescale > 1 && width * xprescale > max_prescale_size)
	{
		xprescale--;
	}
	while (yprescale > 1 && height * yprescale > max_prescale_size)
	{
		yprescale--;
	}
}
