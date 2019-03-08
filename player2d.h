#ifndef PLAYER2D_H
#define PLAYER2D_H

#define PLAYER_HEIGHT 32.0f
#define PLAYER_HALF_WIDTH 11.0f

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
	player->capsule.a.y += 2.0f;
	player->capsule.b = c2(player->pos + v2(0, -PLAYER_HEIGHT / 2.0f + player->capsule.r));
	player->box = make_aabb(player->pos, PLAYER_HALF_WIDTH * 2.0f, PLAYER_HEIGHT);
}

// sweep player against the world geometry
// calculate time of impact (toi)
float player_sweep(capsule_t capsule, v2* n, v2* contact, v2 vel)
{
	float min_toi = 1;
	v2 min_toi_normal;
	v2 min_toi_contact;

	for (int i = 0; i < map.count; ++i)
	{
		int x = i % map.w;
		int y = i / map.w;
		int id = get_tile_id(&map, x, y);
		if (!is_empty_tile(id)) {
			tile_t tile = get_tile(&map, x, y);

			v2 toi_normal;
			v2 toi_contact;
			int iters;
			float toi = c2TOI(&tile.u, tile_id_to_c2_type(tile.id), 0, c2V(0, 0), &capsule, C2_CAPSULE, 0, c2(vel), 1, (c2v*)&toi_normal, (c2v*)&toi_contact, &iters);
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

// Non-linear Gauss-Seidel (NGS)
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
	const float skin_factor = 0.05f;
	player_copy.capsule.r += skin_factor;
	while (iters++ < 100)
	{
		int hit_something = 0;
		for (int i = 0; i < map.count; ++i)
		{
			int x = i % map.w;
			int y = i / map.w;
			int id = get_tile_id(&map, x, y);
			if (!is_empty_tile(id)) {
				tile_t tile = get_tile(&map, x, y);

				c2Manifold m;
				c2Collide(&tile.u, 0, tile_id_to_c2_type(tile.id), &player_copy.capsule, 0, C2_CAPSULE, &m);
				if (m.count) {
					hit_something = 1;
					v2 n = c2(m.n);
					const float corrective_factor = 0.2f;
					player_copy.pos += n * corrective_factor;
					player_sync_geometry(&player_copy);
					draw_manifold(m);
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

// shapecast downward to see if the player has space to fall, or not, using
// the player's AABB shape
int player_can_fall(player2d_t* player, int pixels_to_fall)
{
	float min_toi = 1;

	c2AABB player_aabb;
	player_aabb.min = c2(player->box.min);
	player_aabb.max = c2(player->box.max);

	v2 vel_down_10_pixels = v2(0, (float)-pixels_to_fall);

	for (int i = 0; i < map.count; ++i)
	{
		int x = i % map.w;
		int y = i / map.w;
		int id = get_tile_id(&map, x, y);
		if (!is_empty_tile(id)) {
			tile_t tile = get_tile(&map, x, y);

			v2 toi_normal;
			v2 toi_contact;
			int iters;
			float toi = c2TOI(&tile.u, tile_id_to_c2_type(tile.id), 0, c2V(0, 0), &player_aabb, C2_AABB, 0, c2(vel_down_10_pixels), 1, 0, 0, &iters);
			if (toi < min_toi) {
				min_toi = toi;
			}
		}
	}

	if (min_toi == 1.0f) {
		return 1;
	} else {
		return 0;
	}
}

#endif // PLAYER2D_H
