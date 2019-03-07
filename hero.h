#ifndef HERO_H
#define HERO_H

#include <cute_coroutine.h>

// This file just implements animation/rendering logic.
// The state machine is intended to be driven externally.

enum hero_state_t
{
	HERO_STATE_IDLE,
	HERO_STATE_RUN,
	HERO_STATE_JUMP,
};

enum facing_t
{
	FACING_LEFT,
	FACING_RIGHT
};

struct hero_t
{
	hero_state_t state;
	facing_t facing;

	coroutine_t co;
	int image_id;
};

void hero_set_state(hero_t* hero, hero_state_t state);
void hero_update(hero_t* hero, float dt);
void hero_draw(hero_t* hero, v2 pos);
void hero_init(hero_t* hero);

enum hero_image_id_t
{
	HERO_IMAGE_ID_IDLE0 = 140,
	HERO_IMAGE_ID_IDLE1,
	HERO_IMAGE_ID_IDLE2,
	HERO_IMAGE_ID_IDLE3,
};

#endif // HERO_H

#define HERO_IMPLEMENTATION
#ifdef HERO_IMPLEMENTATION
#ifndef HERO_IMPLEMENTATION_ONCE
#define HERO_IMPLEMENTATION_ONCE

#include <sprite.h>

void hero_set_state(hero_t* hero, hero_state_t state)
{
	hero->state = state;
	coroutine_init(&hero->co);
}

void hero_update(hero_t* hero, float dt)
{
	const float idle_time = 0.1f;
	const float idle_pause_time = 0.35f;
	coroutine_t* co = &hero->co;

	switch (hero->state)
	{
	case HERO_STATE_IDLE:
		COROUTINE_START(co);
		COROUTINE_CASE(co, idle_start);

			hero->image_id = HERO_IMAGE_ID_IDLE0;
			COROUTINE_WAIT(co, idle_pause_time, dt);

			hero->image_id = HERO_IMAGE_ID_IDLE1;
			COROUTINE_WAIT(co, idle_time, dt);

			hero->image_id = HERO_IMAGE_ID_IDLE2;
			COROUTINE_WAIT(co, idle_time, dt);

			hero->image_id = HERO_IMAGE_ID_IDLE3;
			COROUTINE_WAIT(co, idle_pause_time, dt);

			hero->image_id = HERO_IMAGE_ID_IDLE2;
			COROUTINE_WAIT(co, idle_time, dt);

			hero->image_id = HERO_IMAGE_ID_IDLE1;
			COROUTINE_WAIT(co, idle_time, dt);

		goto idle_start;
		COROUTINE_END(co);
		break;
	}
}

void hero_draw(hero_t* hero, v2 pos)
{
	sprite_t hero_sprite = make_sprite(hero->image_id, pos.x, pos.y, 1.0f, 0, 0);
	push_sprite(hero_sprite);
}

void hero_init(hero_t* hero)
{
	hero->state = HERO_STATE_IDLE;
	hero->facing = FACING_LEFT;
	coroutine_init(&hero->co);
	hero->image_id = ~0;
}

#endif // HERO_IMPLEMENTATION_ONCE
#endif // HERO_IMPLEMENTATION
