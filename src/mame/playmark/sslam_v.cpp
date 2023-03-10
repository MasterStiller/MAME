// license:BSD-3-Clause
// copyright-holders:Pierpaolo Prazzoli, Quench
/* Super Slam - Video Hardware */

#include "emu.h"
#include "sslam.h"


void sslam_state::draw_sprites(bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	gfx_element *gfx = m_gfxdecode->gfx(0);

	for (int i = 3; i < m_spriteram.length() - 3; i += 4)
	{
		const uint16_t *src = &m_spriteram[i];
		if (src[0] & 0x2000) break;

		int xpos = src[2] & 0x1ff;
		int ypos = src[0] & 0x01ff;
		int colr = (src[2] & 0xf000) >> 12;
		int eightbyeight = src[0] & 0x1000;
		int flipx = src[0] & 0x4000;
		int number = src[3];

		xpos -=16; xpos -=7; xpos += m_sprites_x_offset;
		ypos = 0xff - ypos;
		ypos -=16; ypos -=7;

		if(ypos < 0)
			ypos += 256;

		if(ypos >= 249)
			ypos -= 256;

		if (!eightbyeight)
		{
			if (flipx)
			{
				gfx->transpen(bitmap,cliprect,number,  colr,1,0,xpos+8,ypos,0);
				gfx->transpen(bitmap,cliprect,number+1,colr,1,0,xpos+8,ypos+8,0);
				gfx->transpen(bitmap,cliprect,number+2,colr,1,0,xpos,  ypos,0);
				gfx->transpen(bitmap,cliprect,number+3,colr,1,0,xpos,  ypos+8,0);
			}
			else
			{
				gfx->transpen(bitmap,cliprect,number,  colr,0,0,xpos,  ypos,0);
				gfx->transpen(bitmap,cliprect,number+1,colr,0,0,xpos,  ypos+8,0);
				gfx->transpen(bitmap,cliprect,number+2,colr,0,0,xpos+8,ypos,0);
				gfx->transpen(bitmap,cliprect,number+3,colr,0,0,xpos+8,ypos+8,0);
			}
		}
		else
		{
			if (flipx)
			{
				gfx->transpen(bitmap,cliprect,number ^ 2,colr,1,0,xpos,ypos,0);
			}
			else
			{
				gfx->transpen(bitmap,cliprect,number,colr,0,0,xpos,ypos,0);
			}
		}
	}
}


/* Text Layer */

TILE_GET_INFO_MEMBER(sslam_state::get_sslam_tx_tile_info)
{
	int code = m_tx_tileram[tile_index] & 0x0fff;
	int colr = m_tx_tileram[tile_index] & 0xf000;

	tileinfo.set(3,code+0xc000 ,colr >> 12,0);
}

void sslam_state::sslam_tx_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_tx_tileram[offset]);
	m_tx_tilemap->mark_tile_dirty(offset);
}

/* Middle Layer */

TILE_GET_INFO_MEMBER(sslam_state::get_sslam_md_tile_info)
{
	int code = m_md_tileram[tile_index] & 0x0fff;
	int colr = m_md_tileram[tile_index] & 0xf000;

	tileinfo.set(2,code+0x2000 ,colr >> 12,0);
}

void sslam_state::sslam_md_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_md_tileram[offset]);
	m_md_tilemap->mark_tile_dirty(offset);
}

/* Background Layer */

TILE_GET_INFO_MEMBER(sslam_state::get_sslam_bg_tile_info)
{
	int code = m_bg_tileram[tile_index] & 0x1fff;
	int colr = m_bg_tileram[tile_index] & 0xe000;

	tileinfo.set(1,code ,colr >> 13,0);
}

void sslam_state::sslam_bg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_tileram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(powerbls_state::get_powerbls_bg_tile_info)
{
	int code = m_bg_tileram[tile_index*2+1] & 0x0fff;
	int colr = (m_bg_tileram[tile_index*2+1] & 0xf000) >> 12;
	code |= (m_bg_tileram[tile_index*2] & 0x0f00) << 4;

	//(m_bg_tileram[tile_index*2] & 0x0f00) == 0xf000 ???

	tileinfo.set(1,code,colr,0);
}

void powerbls_state::powerbls_bg_tileram_w(offs_t offset, uint16_t data, uint16_t mem_mask)
{
	COMBINE_DATA(&m_bg_tileram[offset]);
	m_bg_tilemap->mark_tile_dirty(offset>>1);
}

void sslam_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sslam_state::get_sslam_bg_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_md_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sslam_state::get_sslam_md_tile_info)), TILEMAP_SCAN_ROWS, 16, 16, 32, 32);
	m_tx_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(sslam_state::get_sslam_tx_tile_info)), TILEMAP_SCAN_ROWS,  8,  8, 64, 64);

	m_md_tilemap->set_transparent_pen(0);
	m_tx_tilemap->set_transparent_pen(0);

	m_sprites_x_offset = 0;
	save_item(NAME(m_sprites_x_offset));
}

void powerbls_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(powerbls_state::get_powerbls_bg_tile_info)), TILEMAP_SCAN_ROWS, 8, 8, 64, 64);

	m_sprites_x_offset = -21;
	save_item(NAME(m_sprites_x_offset));
}

uint32_t sslam_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!(m_regs[6] & 1))
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_tx_tilemap->set_scrollx(0, m_regs[0]+1);  /* +0 looks better, but the real board has the left most pixel at the left edge shifted off screen */
	m_tx_tilemap->set_scrolly(0, (m_regs[1] & 0xff)+8);
	m_md_tilemap->set_scrollx(0, m_regs[2]+2);
	m_md_tilemap->set_scrolly(0, m_regs[3]+8);
	m_bg_tilemap->set_scrollx(0, m_regs[4]+4);
	m_bg_tilemap->set_scrolly(0, m_regs[5]+8);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);

	/* remove wraparound from the tilemap (used on title screen) */
	if (m_regs[2]+2 > 0x8c8)
	{
		rectangle md_clip;
		md_clip.min_x = cliprect.min_x;
		md_clip.max_x = cliprect.max_x - (m_regs[2]+2 - 0x8c8);
		md_clip.min_y = cliprect.min_y;
		md_clip.max_y = cliprect.max_y;

		m_md_tilemap->draw(screen, bitmap, md_clip, 0,0);
	}
	else
	{
		m_md_tilemap->draw(screen, bitmap, cliprect, 0,0);
	}

	draw_sprites(bitmap,cliprect);
	m_tx_tilemap->draw(screen, bitmap, cliprect, 0,0);
	return 0;
}

uint32_t powerbls_state::screen_update(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	if (!(m_regs[6] & 1))
	{
		bitmap.fill(m_palette->black_pen(), cliprect);
		return 0;
	}

	m_bg_tilemap->set_scrollx(0, m_regs[0]+21);
	m_bg_tilemap->set_scrolly(0, m_regs[1]-240);

	m_bg_tilemap->draw(screen, bitmap, cliprect, 0,0);
	draw_sprites(bitmap,cliprect);
	return 0;
}
