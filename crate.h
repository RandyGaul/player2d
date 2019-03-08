#ifndef CRATE_H
#define CRATE_H

#define CRATE_WH 16.0f
// Same as the hero
#define CRATE_SPRITE_LAYER 2

// There are 3 different types of "boxes" in the *art*
enum crate_image_id_t
{
	CRATE_IMAGE_ID_BOX1_CROSS = 58,
	CRATE_IMAGE_ID_BOX1_SLASH,
	CRATE_IMAGE_ID_BOX1_PLAIN,

	CRATE_IMAGE_ID_BOX2_CROSS = 36,
	CRATE_IMAGE_ID_BOX2_SLASH,
	CRATE_IMAGE_ID_BOX2_PLAIN,

	CRATE_IMAGE_ID_BOX3_CROSS = 21,
	CRATE_IMAGE_ID_BOX3_SLASH,
	CRATE_IMAGE_ID_BOX3_PLAIN,
};

struct crate_t
{
	v2 pos;
	v2 old_pos;
	v2 vel;
	c2AABB aabb;
	int image_id;
};

void crate_sync_geometry(crate_t* crate);
void crate_update(crate_t* crate, float dt);
void crate_draw(crate_t* crate);
void crate_init(crate_t* crate, v2 pos);
void crate_ngs(crate_t* crate, capsule_t capsule);
void crate_vel_fixup(crate_t* crate, float dt);

#endif // CRATE_H

#ifdef CRATE_IMPLEMENTATION
#ifndef CRATE_IMPLEMENTATION_ONCE
#define CRATE_IMPLEMENTATION_ONCE

#include <sprite.h>

void crate_sync_geometry(crate_t* crate)
{
	aabb_t box = make_aabb(crate->pos, CRATE_WH, CRATE_WH);
	crate->aabb.min = c2(box.min);
	crate->aabb.max = c2(box.max);
}

void crate_update(crate_t* crate, float dt)
{
	// Large friction (well, actually it's damping).
	const float crate_damp_factor = 2.5f;
	crate->vel *= 1.0f / (1.0f + dt * crate_damp_factor);

	// Gravity.
	crate->vel += v2(0, -100.0f) * dt;

	// Make sure it's hard to shoot the crate super far for any reason.
	if (crate->vel.x > 5.0f) crate->vel.x = 5.0f;
	if (crate->vel.x < -5.0f) crate->vel.x = -5.0f;

	// Apply heavy damping on lower velocities.
	if (crate->vel.x < 1.0f) crate->vel.x *= 1.0f / (1.0f + dt * crate_damp_factor * crate_damp_factor);

	crate->old_pos = crate->pos;
	crate->pos += crate->vel * dt;
}

void crate_draw(crate_t* crate)
{
	sprite_t crate_sprite = make_sprite(crate->image_id, crate->pos.x, crate->pos.y, 1.0f, 0, CRATE_SPRITE_LAYER);
	push_sprite(crate_sprite);
}

void crate_init(crate_t* crate, v2 pos)
{
	crate->pos = pos;
	crate->old_pos = pos;
	aabb_t box = make_aabb(crate->pos, CRATE_WH, CRATE_WH);
	crate->aabb.min = c2(box.min);
	crate->aabb.max = c2(box.max);
	crate->image_id = CRATE_IMAGE_ID_BOX1_CROSS;
}

// TODO: Collision with other crates
void crate_ngs(crate_t* crate, capsule_t capsule)
{
	// collision with player capsule
	for (int i = 0; i < 10; ++i)
	{
		c2Manifold m;
		c2AABBtoCapsuleManifold(crate->aabb, capsule, &m);
		if (m.count) {
			v2 n = c2(m.n);
			crate->pos -= n * 0.025f * m.depths[0];
			crate_sync_geometry(crate);
		}
	}

	// Solve tiles *after* the player, so tile to crate has priority.
	// This prevents the player from pushing crates outside the map.
	int max_iters = 100;
	int iters = 0;
	int hit_something = 0;
	while (iters++ < 100)
	{
		for (int i = 0; i < map.count; ++i)
		{
			int x = i % map.w;
			int y = i / map.w;
			int id = get_tile_id(&map, x, y);
			if (id) {
				tile_t tile = get_tile(&map, x, y);
				if (!tile.id) continue;
				c2AABB tile_aabb = tile.u.box;

				c2Manifold m;
				c2AABBtoAABBManifold(crate->aabb, tile_aabb, &m);
				if (m.count) {
					hit_something = 1;
					v2 n = c2(m.n);
					const float corrective_factor = 0.2f;
					crate->pos -= n * corrective_factor * m.depths[0];
					crate_sync_geometry(crate);
				}
			}
		}
		if (!hit_something) break;
	}
	if (iters == max_iters) printf("NGS failed to converge.\n");
	crate_sync_geometry(crate);
}

void crate_vel_fixup(crate_t* crate, float inv_dt)
{
	v2 delta = crate->pos - crate->old_pos;
	crate->vel = delta * inv_dt;
}

#endif // CRATE_IMPLEMENTATION_ONCE
#endif // CRATE_IMPLEMENTATION
