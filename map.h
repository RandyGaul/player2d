#ifndef MAP_H
#define MAP_H

#define TILE_WH 16

#include <assert.h>
#include <cute_c2.h>
#include <cute_png.h>

struct tile_t
{
	int id;
	union
	{
		c2AABB box;
		c2Poly poly;
	} u;
};

struct map_t
{
	int w, h, count;
	int* tiles;
};

int get_tile_id(map_t* map, int x, int y)
{
	assert(x < map->w);
	assert(y < map->h);
	return map->tiles[map->w * y + x];
}

void get_tile_xy_from_world_pos(map_t* map, v2 pos, int* x, int* y)
{
	float half_w = (float)(map->w * TILE_WH / 2);
	float half_h = (float)(map->h * TILE_WH / 2);
	*x = (int)((pos.x + half_w) / TILE_WH);
	*y = (int)((pos.y + half_h) / TILE_WH);
}

aabb_t get_tile_bounds(map_t* map, int x, int y)
{
	float half_w = (float)(map->w * TILE_WH / 2);
	float half_h = (float)(map->h * TILE_WH / 2);
	float x0 = (float)x * TILE_WH - half_w;
	float y0 = (float)y * TILE_WH - half_h;
	aabb_t bounds = make_aabb(v2(x0 + TILE_WH / 2, y0 + TILE_WH / 2), TILE_WH, TILE_WH);
	return bounds;
}

tile_t get_tile(map_t* map, int x, int y)
{
	aabb_t bounds = get_tile_bounds(map, x, y);
	tile_t tile;
	tile.id = get_tile_id(map, x, y);

	if (tile.id) {
		switch (tile.id)
		{
		default: tile.u.box = c2(bounds); break;
		case 5:
		{
			tile.u.poly.verts[0] = c2(top_left(bounds));
			tile.u.poly.verts[1] = c2(bottom_left(bounds));
			tile.u.poly.verts[2] = c2(bottom_right(bounds));
			tile.u.poly.count = 3;
			c2Norms(tile.u.poly.verts, tile.u.poly.norms, 3);
		}	break;
		}
	}

	return tile;
}

void debug_draw_map(map_t* map)
{
	for (int i = 0; i < map->count; ++i)
	{
		int x = i % map->w;
		int y = i / map->w;
		tile_t tile = get_tile(map, x, y);
		if (tile.id) {
			switch (tile.id)
			{
			default: draw_aabb(c2(tile.u.box)); break;
			case 5: draw_poly(tile.u.poly); break;
			}
		}
	}
}

void load_map(map_t* map, const char* path)
{
	FILE* fp = fopen(path, "r");
	fscanf(fp, "%d", &map->w);
	fscanf(fp, "%d", &map->h);
	int count = map->w * map->h;
	map->tiles = (int*)malloc(sizeof(int) * count);
	map->count = count;
	for (int y = map->h - 1; y >= 0; --y)
	{
		for (int x = 0; x < map->w; ++x)
		{
			fscanf(fp, "%d", map->tiles + y * map->w + x);
		}
	}

	for (int y = map->h - 1; y >= 0; --y)
	{
		for (int x = 0; x < map->w; ++x)
		{
			tile_t tile = get_tile(map, x, y);
			printf("%3d", tile.id);
		}
		printf("\n");
	}
}

void free_map(map_t* map)
{
	free(map->tiles);
	memset(map, 0, sizeof(map));
}

#endif // MAP_H
