# player2d

An example demo showing one way to implement swept 2D character controller from scratch. The [cute_headers](https://github.com/RandyGaul/cute_headers) repository is heavily used to implement the low-level guts of all algorithms in the demo, such as sprite batching, image loading, music/sound fx playing, and collisions.

![screenshot 1](/screenshots/fabulous_demo.gif?raw=true)

# Controls

* Press wasd in the demo to move the player
* The demo prints out controls for the editor to stdout - press RIGHT-CLICK TO enable the editor
* Press G to turn ON/OFF debug rendering

![screenshot 2](/screenshots/fabulous_capsule.gif?raw=true)

# Swept Character Controller

The overall strategy is to make use of a few algorithms to implement the character controller:

1. GJK (Gilbert Johnson Keerthi) - Compute closest points between two disjoint shapes.
2. Conservative Advancement - Compute the Time of Impact (TOI) between two moving shapes.
3. Non-linear Gauss Seidel - An iterative algorithm to solve non-linear big matrix problems.

The first two algorithms are completely wrapped behind a black-box implementation thanks to the [cute_c2.h](https://github.com/RandyGaul/cute_headers) header, in `c2TOI` and `c2GJK`. `c2TOI` implements my own take on the Conservative Advancement algorithm.

The third algorithm sounds fancy, but simply means to gently press shapes apart along the solution vector (axis of minimum separation) in an iterative fashion. Typically it looks something like:

	func ngs() {
        for each iteration
	        for each collision
	            push shapes about by resolution_factor along the solution vector
	}

The non-linearity comes in when adjusting positions directly. In order to use NGS the solution vector must be computed. A nice theorem to do so is the Separating Axis Theorem, which leads to the majority of the [cute_c2.h](https://github.com/RandyGaul/cute_headers) header for computing manifolds between two intersecting shapes. A manifold is a structure that describes how two shapes intersect.

Roughly speaking, the overall character controller follows these steps:

1. Sweep player against the world to find TOI.
2. Move player to TOI.
3. Use NGS to apply a "skin factor" and avoid colliding scenarios.
4. Apply "slide along wall" function to player velocity.
5. Cut timestep down by TOI, and go back to step 1. if any time remains.

# Skin Factor

Since a TOI algorithm is used, it is very important to let the TOI algorithm breathe between each frame. After a TOI is computed and the player is moved to the TOI, the TOI algorithm will quite likely fail if immediately called. For example, say the player has run into a wall, but it was a glancing blow. The player has a lot of remaining velocity, and is supposed to slide along the wall to exhaust the rest of the timestep. Once the player is moved to the initial TOI, the next TOI will likely be 0 since the player is numerically in an intersecting configuration.

One solution to this scenario is to move the player to the TOI, and then use NGS to gently press the player away from all geometry, keeping a small numeric "skin buffer" of space around the player at all times. This gives the TOI algorithm breathing room to slide along walls and continue moving to search the entire timestep.

# Pros/Cons to this Strategy

Pros:

* Very numerically robust, and can handle majority of arbitrary geometry gracefully.
* The swept nature can handle high speeds and avoid tunneling. This is great for super fast dashes, lasers, or explosions.
* All behavior can be completely fine-tuned in any way.
* Any collision detection solution can work, even pre-built physics engines (for example: using a kinematic rigid body in the Bullet Physics library).

Cons:

* Requires a bit of knowledge about the underlying algorithms to identify root causes of bugs.
* Lots of mathematics and geometry.
* Lots of work to implement robustly.
* Some sensitivity to "acute" angles for NGS. Not explicitly handled in this demo (for simplicity). Instead, high NGS iterations are used. Ideally an explicit code path can detect acute cases and apply a specific solution.

# Case Study - Standing on Edges

![screenshot 3](/screenshots/edge_fall.gif?raw=true)

Here is an example of nuanced behavior. This character controller is designed to primarily use the capsule for interaction with the world. However, I wanted the player to be able to stand on the edge of ledges without slipping on the capsule's round surface, to get a "retro feel" when platforming.

The strategy I chose was to disable gravity when the player is detected on the ground. Then, I use an AABB shape and find a TOI with a downward velocity. If I find an acceptable TOI, I re-apply gravity and clear the players "on_ground" flag.

By crafting a strong API around `c2TOI` it becomes very simple to create these custom predicates, such as "can fall". Here's the source to check whether or not the player can fall:

```cpp
// shapecast downward to see if the player has space to fall, or not, using
// the player's AABB shape
int player_can_fall(player2d_t* player, int pixels_to_fall)
{
	float min_toi = 1;

	c2AABB player_aabb;
	player_aabb.min = c2(player->box.min);
	player_aabb.max = c2(player->box.max);

	v2 vel_down_10_pixels = v2(0, (float)-pixels_to_fall);

	for (int i = 0; i < map.count; ++i)
	{
		int x = i % map.w;
		int y = i / map.w;
		int id = get_tile_id(&map, x, y);
		if (!is_empty_tile(id)) {
			tile_t tile = get_tile(&map, x, y);

			v2 toi_normal;
			v2 toi_contact;
			int iters;
			float toi = c2TOI(&tile.u, tile_id_to_c2_type(tile.id), 0, c2V(0, 0), &player_aabb, C2_AABB, 0, c2(vel_down_10_pixels), 1, 0, 0, &iters);
			if (toi < min_toi) {
				min_toi = toi;
			}
		}
	}

	if (min_toi == 1.0f) {
		return 1;
	} else {
		return 0;
	}
}
```

# Zero Gravity Strategy

A great thing about disabling gravity when "on ground" is that "floaty" artifacts can be avoided entirely. Here is what I mean by "floaty artifacts":

![screenshot 4](/screenshots/float.gif?raw=true)

In the above image gravity is always applied, and gravity is used to keep the player on the ground. If instead gravity is turned off, and an "on ground" flag is used, a different implementation can be made. Instead, it's possible to raycast downward from the player's center, and "follow" the ground explicitly. This way the player can run on a flat surface, and onto a sloped surface, without floating in the air at all, and without using a huge gravity value.

I have not yet implemented this feature in the demo, and instead simply deal with "floaty" artifacts. They can easy be seen when the player runs down the sloped tiles in the demo.

# Some Ideas for Extended Features

Lots of features can be tacked onto the demo here to spice things up. The sweep/NGS functions in the demo are especially useful to use as work-horses to implement features. Here are some random ideas I think would be pretty cool:

* Player dash effect (the TOI functions will gracefully handle high speeds)
* Raycast downwards while "on ground" to follow slopes gracefully
* Crate stacking
* One-way platforms (top left of demo - unimplemented)
* Standing on top of crates
* Slide-dash - use a different collider depending on the controller state (like a sideways capsule while sliding)
* Ducking/crouching
* Wall-grabbing or wall-sliding
* Double jumping
* Corner grabbing
* Many more!

# License

Do whatever you want with the code! The character controller here can be a great start to implementing your own character controller in your own game. I do not have plans to expand the demo much further, but, if anyone comes along and wants to submit a pull-request to implement more features, I will be more than happy to take a look and try merging it in. The above list of ideas for extended features is a good list of things I'd be especially interested in seeing come alive :)

# Special Thanks

Special thanks to these people for contributing to this demo:

* bsvercl - implementing crates, and many other various goodies in the demo
* Collin Meredith - funky music
* Bakudas - Animation for the player
* JPKotzeGames - The tile drawings
* Max Klassen - Help with the initial setup of the demo
