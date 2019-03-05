#ifndef PLAYER2D_H
#define PLAYER2D_H

#define PLAYER_HEIGHT 55.0f
#define PLAYER_WIDTH 15.0f

struct player2d_t
{
	v2 pos;
	v2 vel;
	aabb_t box;
	capsule_t capsule;
	int on_ground;
	int can_jump;
};

void player_sync_geometry(player2d_t* player)
{
	player->capsule.a = c2(player->pos + v2(0, PLAYER_HEIGHT / 2.0f - player->capsule.r));
	player->capsule.b = c2(player->pos + v2(0, -PLAYER_HEIGHT / 2.0f + player->capsule.r));
	player->box = make_aabb(player->pos, 40, 60);
}

void player_ngs(player2d_t* player)
{
	int max_iters = 100;
	int iters = 0;
	player2d_t player_copy = *player;
	player_copy.capsule.r += 0.05f;
	while (iters++ < 100)
	{
		int hit_something = 0;
		for (int i = 0; i <= map.count; ++i)
		{
			int x = i % map.w;
			int y = i / map.h;
			int id = get_tile_id(&map, x, y);
			if (id) {
				aabb_t tile_box = get_tile_bounds(&map, x, y);
				c2AABB tile_aabb;
				tile_aabb.min = c2(tile_box.min);
				tile_aabb.max = c2(tile_box.max);

				c2Manifold m;
				c2AABBtoCapsuleManifold(tile_aabb, player_copy.capsule, &m);
				if (m.count) {
					hit_something = 1;
					v2 n = c2(m.n);
					player_copy.pos += n * m.depths[0] * 0.2f;
					player_sync_geometry(&player_copy);
				}
			}
		}
		if (!hit_something) break;
	}
	if (iters == max_iters) printf("Uh oh! Hit max iters. Failed to converge.\n");
	player_copy.capsule.r = player->capsule.r;
	player_sync_geometry(&player_copy);
	*player = player_copy;
}

/*
	x is on ground
	x can jump
	  is on edge
	  swap from capsule (moving) to box (standing)
	  works for concave situations
	  wall jumping
	  wall sliding
	  corner grabbing
	  double jumping
	  friction on ground
	  different ground friction values
	  player friction
	  mixing the player friction with the ground friction
*/

#endif // PLAYER2D_H
