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

// sync box/capsule geom based on player's position
void player_sync_geometry(player2d_t* player)
{
	player->capsule.a = c2(player->pos + v2(0, PLAYER_HEIGHT / 2.0f - player->capsule.r));
	player->capsule.b = c2(player->pos + v2(0, -PLAYER_HEIGHT / 2.0f + player->capsule.r));
	player->box = make_aabb(player->pos, 40, 60);
}

// sweep player against the world geometry
// calculate time of impact (toi)
float player_sweep(capsule_t capsule, v2* n, v2* contact, v2 vel)
{
	float min_toi = 1;
	v2 min_toi_normal;
	v2 min_toi_contact;

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

			v2 toi_normal;
			v2 toi_contact;
			int iters;
			float toi = c2TOI(&tile_aabb, C2_AABB, 0, c2V(0, 0), &capsule, C2_CAPSULE, 0, c2(vel), 1, (c2v*)&toi_normal, (c2v*)&toi_contact, &iters);
			if (toi < min_toi) {
				min_toi = toi;
				min_toi_normal = toi_normal;
				min_toi_contact = toi_contact;
			}
		}
	}

	*n = min_toi_normal;
	*contact = min_toi_contact;
	return min_toi;
}

// Non-linear Hauss-Seidel (NGS)
// Simply means to adjust positions directly in an iterative fashion, along the solution vector.
// Used to *gently* press the player out of colliding configurations.
// This step is absolutely critical to ensure the TOI next frame can have "breathing room".
// The effect is to create a skin around  the player and make sure the player constantly
// "floats" just above all geometry.
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
	if (iters == max_iters) printf("NGS failed to converge.\n");
	player_copy.capsule.r = player->capsule.r;
	player_sync_geometry(&player_copy);
	*player = player_copy;
}

#endif // PLAYER2D_H
