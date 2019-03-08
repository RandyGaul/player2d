#ifndef MAP_H
#define MAP_H

#define TILE_WH 16
#define MAP_SPRITE_LAYER 1

#include <cute_c2.h>

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

inline C2_TYPE tile_id_to_c2_type(int id)
{
	switch (id)
	{
	default: return C2_AABB;
	case -1: return C2_NONE;
	case 107: case 108: return C2_POLY;
	}
}

int get_tile_id(map_t* map, int x, int y);
int is_empty_tile(int id);
void get_tile_xy_from_world_pos(map_t* map, v2 pos, int* x, int* y);
aabb_t get_tile_bounds(map_t* map, int x, int y);
tile_t get_tile(map_t* map, int x, int y);
void debug_draw_map(map_t* map);
void draw_map(map_t* map);
void load_map(map_t* map, const char* path);
void save_map(const map_t* map, const char* path);
void free_map(map_t* map);

#endif // MAP_H

#ifdef MAP_IMPLEMENTATION
#ifndef MAP_IMPLEMENTATION_ONCE
#define MAP_IMPLEMENTATION_ONCE

#include <assert.h>
#include <cute_png.h>

int get_tile_id(map_t* map, int x, int y)
{
	assert(x < map->w);
	assert(y < map->h);
	return map->tiles[map->w * y + x];
}

int is_empty_tile(int id)
{
	switch (id)
	{
	default: return 0;
	case -1: case 118: case 119: case 120: case 121: case 133: case 134: case 135: case 20:
		return 1;
	}
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

	if (!is_empty_tile(tile.id)) {
		switch (tile.id)
		{
		default: tile.u.box = c2(bounds); break;
		case 107:
		{
			tile.u.poly.verts[0] = c2(top_left(bounds));
			tile.u.poly.verts[1] = c2(bottom_left(bounds));
			tile.u.poly.verts[2] = c2(bottom_right(bounds));
			tile.u.poly.verts[3] = c2((top_right(bounds) + bottom_right(bounds)) * 0.5f);
			tile.u.poly.count = 4;
			c2Norms(tile.u.poly.verts, tile.u.poly.norms, tile.u.poly.count);
		}	break;
		case 108:
		{
			tile.u.poly.verts[0] = c2((top_left(bounds) + bottom_left(bounds)) * 0.5f);
			tile.u.poly.verts[1] = c2(bottom_left(bounds));
			tile.u.poly.verts[2] = c2(bottom_right(bounds));
			tile.u.poly.count = 3;
			c2Norms(tile.u.poly.verts, tile.u.poly.norms, tile.u.poly.count);
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
		if (!is_empty_tile(tile.id)) {
			switch (tile.id)
			{
			default: draw_aabb(c2(tile.u.box)); break;
			case 107: case 108: draw_poly(tile.u.poly); break;
			}
		}
	}
}

void draw_map(map_t* sprite_map)
{
	for (int i = 0; i < sprite_map->count; ++i)
	{
		int x = i % sprite_map->w;
		int y = i / sprite_map->w;
		int id = get_tile_id(sprite_map, x, y);
		if (id != ~0) {
			aabb_t bounds = get_tile_bounds(sprite_map, x, y);
			sprite_t sprite = make_sprite(id, center(bounds).x, center(bounds).y, 1.0f, 0, MAP_SPRITE_LAYER);
			push_sprite(sprite);
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
	fclose(fp);
}

void save_map(const map_t* map, const char* path)
{
	FILE* fp = fopen(path, "w");
	fprintf(fp, "%d\n", map->w);
	fprintf(fp, "%d\n\n", map->h);
	for (int y = map->h - 1; y >= 0; --y)
	{
		for (int x = 0; x < map->w; ++x)
		{
			fprintf(fp, "%d%s", map->tiles[y * map->w + x], x != map->w - 1 ? "\t" : "");
		}
		if (y) fprintf(fp, "\n");
	}
	fclose(fp);
}

void free_map(map_t* map)
{
	free(map->tiles);
	memset(map, 0, sizeof(map));
}

#endif // MAP_IMPLEMENTATION_ONCE
#endif // MAP_IMPLEMENTATION
