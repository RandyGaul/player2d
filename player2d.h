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
