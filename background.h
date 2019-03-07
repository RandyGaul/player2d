#ifndef BACKGROUND_H
#define BACKGROUND_H

// The window is not resizable
#define BACKGROUND_SCALE 1.8f
#define BACKGROUND_SPRITE_LAYER 0

enum background_image_id_t
{
    BACKGROUND_IMAGE_ID_BACKGROUND = 154,
    BACKGROUND_IMAGE_ID_SKY,
};

struct background_t
{
    int background_id;
    int sky_id;
};

void background_init(background_t* background);
// Should we do a parallax effect? Maybe.
//void background_update(background_t* background, hero_t* hero);
void background_draw(background_t* background);

#endif // BACKGROUND_H

#ifdef BACKGROUND_IMPLEMENTATION
#ifndef BACKGROUND_IMPLEMENTATION_ONCE
#define BACKGROUND_IMPLEMENTATION_ONCE

#include <hero.h>
#include <sprite.h>

void background_init(background_t* background)
{
    background->background_id = BACKGROUND_IMAGE_ID_BACKGROUND;
    background->sky_id = BACKGROUND_IMAGE_ID_SKY;
}

void background_draw(background_t* background)
{
    sprite_t sky = make_sprite(background->sky_id, 0.0f, 0.0f, BACKGROUND_SCALE, 0, BACKGROUND_SPRITE_LAYER);
    push_sprite(sky);
    sprite_t back = make_sprite(background->background_id, 0.0f, 0.0f, BACKGROUND_SCALE, 0, BACKGROUND_SPRITE_LAYER);
    push_sprite(back);
}

#endif // BACKGROUND_IMPLEMENTATION_ONCE
#endif // BACKGROUND_IMPLEMENTATION
