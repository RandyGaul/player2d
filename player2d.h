#ifndef PLAYER2D_H
#define PLAYER2D_H

struct player2d_t
{
	v2 pos;
	aabb_t box;
	capsule_t capsule;
};

/*
	is on ground
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
