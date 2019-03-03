#ifndef MAP_H
#define MAP_H

#define TILE_WH 20

struct map_t
{
	int w, h, count;
	int* tiles;
};

int get_tile_id(map_t* map, int x, int y)
{
	return map->tiles[map->w * y + x];
}

aabb_t get_tile_bounds(map_t* map, int x, int y)
{
	float half_w = (float)(map->w * TILE_WH / 2);
	float half_h = (float)(map->h * TILE_WH / 2);
	float x0 = (float)x * TILE_WH - half_w;
	float y0 = (float)y * TILE_WH - half_h;
	return make_aabb(v2(x0, y0), TILE_WH, TILE_WH);
}

void draw_map(map_t* map)
{
	for (int i = 0; i < map->count; ++i)
	{
		int x = i % map->w;
		int y = i / map->h;
		aabb_t tile_box = get_tile_bounds(map, x, y);
		draw_aabb(tile_box);
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
	for (int i = 0; i < count; ++i)
	{
		fscanf(fp, "%d", map->tiles + i);
	}
}

void free_map(map_t* map)
{
	free(map->tiles);
	memset(map, 0, sizeof(map));
}

#endif // MAP_H
