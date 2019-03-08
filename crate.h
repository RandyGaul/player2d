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

struct crate_t {
    v2 pos;
    v2 old_pos;
    v2 vel;
    c2AABB aabb;
    int image_id;
    int on_ground;
};

void crate_sync_geometry(crate_t* crate);
void crate_update(crate_t* crate, float dt);
void crate_draw(crate_t* crate);
void crate_init(crate_t* crate, v2 pos);
void crate_ngs(crate_t* crate, capsule_t capsule);
// this is ripped straight from player2d :)
int crate_can_fall(crate_t* crate, int pixels_to_fall);

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
    // TODO: Figure out if the crate is on the ground or not
    if (crate->on_ground) {
        crate->vel.y = 0.0f;
        if (crate_can_fall(crate, 1)) {
            crate->on_ground = 0;
        }
    }
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
    crate->vel = v2(0.0f, -10.0f);
    aabb_t box = make_aabb(crate->pos, CRATE_WH, CRATE_WH);
    crate->aabb.min = c2(box.min);
    crate->aabb.max = c2(box.max);
    crate->image_id = CRATE_IMAGE_ID_BOX1_CROSS;
    crate->on_ground = 0;
}

// TODO: Collision with other crates
void crate_ngs(crate_t* crate, capsule_t capsule)
{
    int max_iters = 100;
	int iters = 0;
    int hit_something = 0;
	while (iters++ < 100)
	{
        // collision with player capsule
        c2Manifold m;
        c2AABBtoCapsuleManifold(crate->aabb, capsule, &m);
        if (m.count) {
            v2 n = c2(m.n);
            crate->pos -= n * 0.2f;
            hit_something = 1;
            crate_sync_geometry(crate);
        }

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
                    crate->pos -= n * corrective_factor;
                    crate->on_ground = 1;
                    crate_sync_geometry(crate);
                }
            }
        }
        if (hit_something) break;
	}
	if (iters == max_iters) printf("NGS failed to converge.\n");
    if (hit_something) {
        crate->vel = crate->pos - crate->old_pos;
    }
    crate_sync_geometry(crate);
    crate->old_pos = crate->pos;
}

int crate_can_fall(crate_t* crate, int pixels_to_fall)
{
    float min_toi = 1;

    v2 vel_down_10_pixels = v2(0, (float)-pixels_to_fall);

    for (int i = 0; i < map.count; ++i)
    {
        int x = i % map.w;
		int y = i / map.w;
		int id = get_tile_id(&map, x, y);
		if (id) {
			tile_t tile = get_tile(&map, x, y);
			if (!tile.id) continue;
			c2AABB tile_aabb = tile.u.box;

			v2 toi_normal;
			v2 toi_contact;
			int iters;
			float toi = c2TOI(&tile_aabb, C2_AABB, 0, c2V(0, 0), &crate->aabb, C2_AABB, 0, c2(vel_down_10_pixels), 1, 0, 0, &iters);
			if (toi < min_toi) {
				min_toi = toi;
			}
		}
    }

    return min_toi == 1.0f;
}

#endif // CRATE_IMPLEMENTATION_ONCE
#endif // CRATE_IMPLEMENTATION
