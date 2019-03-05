#ifndef PLAYER2D_H
#define PLAYER2D_H

#define PLAYER_HEIGHT 60.0f
#define PLAYER_WIDTH 15.0f

struct player2d_t
{
	v2 pos, vel;
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
