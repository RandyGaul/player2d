#ifndef PLAYER2D_H
#define PLAYER2D_H

using capsule_t = c2Capsule;

struct player2d_t
{
	v2 pos;
	aabb_t box;
	capsule_t capsule;
};

/*
	is on ground
	is on edge

*/

#endif // PLAYER2D_H
