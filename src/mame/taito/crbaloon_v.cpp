// license:BSD-3-Clause
// copyright-holders:Nicola Salmoria
/***************************************************************************

  video.c

  Functions to emulate the video hardware of the machine.

***************************************************************************/

#include "emu.h"
#include "crbaloon.h"


/***************************************************************************

  Convert the color PROMs into a more useable format.

  Crazy Balloon has no PROMs, the color code directly maps to a color:
  all bits are inverted
  bit 3 HALF (intensity)
  bit 2 BLUE
  bit 1 GREEN
  bit 0 RED

***************************************************************************/

void crbaloon_state::crbaloon_palette(palette_device &palette) const
{
	for (int i = 0; i < palette.entries(); i++)
	{
		uint8_t const pen = BIT(i, 0) ? (i >> 1) : 0x0f;

		int const h = BIT(~pen, 3) ? 0xff : 0x55;
		int const r = h * BIT(~pen, 0);
		int const g = h * BIT(~pen, 1);
		int const b = h * BIT(~pen, 2);

		palette.set_pen_color(i, rgb_t(r, g, b));
	}
}


void crbaloon_state::crbaloon_videoram_w(offs_t offset, uint8_t data)
{
	m_videoram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

void crbaloon_state::crbaloon_colorram_w(offs_t offset, uint8_t data)
{
	m_colorram[offset] = data;
	m_bg_tilemap->mark_tile_dirty(offset);
}

TILE_GET_INFO_MEMBER(crbaloon_state::get_bg_tile_info)
{
	int code = m_videoram[tile_index];
	int color = m_colorram[tile_index] & 0x0f;

	tileinfo.set(0, code, color, 0);
}

void crbaloon_state::video_start()
{
	m_bg_tilemap = &machine().tilemap().create(*m_gfxdecode, tilemap_get_info_delegate(*this, FUNC(crbaloon_state::get_bg_tile_info)), TILEMAP_SCAN_ROWS_FLIP_XY,  8, 8, 32, 32);

	save_item(NAME(m_collision_address));
	save_item(NAME(m_collision_address_clear));
}


uint16_t crbaloon_state::crbaloon_get_collision_address()
{
	return m_collision_address_clear ? 0xffff : m_collision_address;
}


void crbaloon_state::crbaloon_set_clear_collision_address(int _crbaloon_collision_address_clear)
{
	m_collision_address_clear = !_crbaloon_collision_address_clear; /* active LO */
}



void crbaloon_state::draw_sprite_and_check_collision(bitmap_ind16 &bitmap)
{
	int y;
	uint8_t code = m_spriteram[0] & 0x0f;
	uint8_t color = m_spriteram[0] >> 4;
	uint8_t sy = m_spriteram[2] - 32;

	uint8_t *gfx = memregion("gfx2")->base() + (code << 7);


	if (flip_screen())
		sy += 1;

	/* assume no collision */
	m_collision_address = 0xffff;

	for (y = 0x1f; y >= 0; y--)
	{
		int x;
		uint8_t data = 0;
		uint8_t sx = flip_screen() ? m_spriteram[1] + 1 : m_spriteram[1];

		for (x = 0x1f; x >= 0; x--)
		{
			int bit;

			if ((x & 0x07) == 0x07)
				/* get next byte to draw, but no drawing in VBLANK */
				data = (sy >= 0xe0) ? 0 : gfx[((x >> 3) << 5) | y];

			bit = data & 0x80;

			/* draw the current pixel, but check collision first */
			if (bit)
			{
				if (bitmap.pix(sy, sx) & 0x01)
					/* compute the collision address -- the +1 is via observation
					   of the game code, probably wrong for cocktail mode */
					m_collision_address = ((((sy ^ 0xff) >> 3) << 5) | ((sx ^ 0xff) >> 3)) + 1;

				bitmap.pix(sy, sx) = (color << 1) | 1;
			}

			sx = sx + 1;
			data = data << 1;
		}

		sy = sy + 1;
	}
}


uint32_t crbaloon_state::screen_update_crbaloon(screen_device &screen, bitmap_ind16 &bitmap, const rectangle &cliprect)
{
	m_bg_tilemap->draw(screen, bitmap, cliprect, 0, 0);

	draw_sprite_and_check_collision(bitmap);

	return 0;
}
