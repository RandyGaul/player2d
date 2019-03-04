#ifndef PLATFORMER_H
#define PLATFORMER_H

struct player2d_t
{
	v2 pos, vel;
	aabb_t box;
	capsule_t capsule;
	float height;
	bool onGround;
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

void InitPlatformer();

void UpdatePlatformer(float dt);

#endif // PLATFORMER_H
