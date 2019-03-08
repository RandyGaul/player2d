# player2d

An example demo showing one way to implement swept 2D character controller from scratch. The [cute_headers](https://github.com/RandyGaul/cute_headers) repository is heavily used to implement the low-level guts of all algorithms in the demo, such as sprite batching, image loading, music/sound fx playing, and collisions.

# Controls

* Press wasd in the demo to move the player
* The demo prints out controls for the editor to stdout - press RIGHT-CLICK TO enable the editor
* Press G to turn ON/OFF debug rendering

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
