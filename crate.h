#ifndef CRATE_H
#define CRATE_H

#define CRATE_WH 16.0f
// Same as the hero
#define CRATE_SPRITE_LAYER 2

// There are 3 different types of "boxes" in the *art*
enum crate_image_id_t
{
    CRATE_IMAGE_ID_BOX1_CROSS = 39,
    CRATE_IMAGE_ID_BOX1_SLASH,
    CRATE_IMAGE_ID_BOX1_PLAIN,

    CRATE_IMAGE_ID_BOX2_CROSS = 66,
    CRATE_IMAGE_ID_BOX2_SLASH,
    CRATE_IMAGE_ID_BOX2_PLAIN,

    CRATE_IMAGE_ID_BOX3_CROSS = 93,
    CRATE_IMAGE_ID_BOX3_SLASH,
    CRATE_IMAGE_ID_BOX3_PLAIN,
};

struct crate_t {
    v2 pos;
    v2 vel;
    c2AABB aabb;
    int image_id;
};

void crate_sync_geometry(crate_t* crate);
void crate_update(crate_t* crate, float dt);
void crate_draw(crate_t* crate);
void crate_init(crate_t* crate, v2 pos);
void crate_player_collision(crate_t* crate, player2d_t* player);

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
    crate->vel = v2(0.0f, -10.0f);
    aabb_t box = make_aabb(crate->pos, CRATE_WH, CRATE_WH);
    crate->aabb.min = c2(box.min);
    crate->aabb.max = c2(box.max);
    crate->image_id = CRATE_IMAGE_ID_BOX1_CROSS;
}

void crate_player_collision(crate_t* crate, player2d_t* player)
{
    c2Manifold m;
    c2AABBtoCapsuleManifold(crate->aabb, player->capsule, &m);
    if (m.count) {
        // player<->crate collision
    }
}

#endif // CRATE_IMPLEMENTATION_ONCE
#endif // CRATE_IMPLEMENTATION
