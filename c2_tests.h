

#if 0
	static int my_x;
	static int my_y;
	static int capture_mouse = 1;
	if (space_is_pressed) {
		capture_mouse = !capture_mouse;
	}
	if (capture_mouse) {
		my_x = mx;
		my_y = my;
	}
	c2Capsule cap;
	cap.a = c2V(my_x, my_y);
	cap.b = c2V(my_x, my_y - 30.0f);
	cap.r = 11.0f;

	c2Poly poly;
	poly.verts[0] = c2V(16 - 50, -24);
	poly.verts[1] = c2V(32 - 50, -24);
	poly.verts[2] = c2V(32 - 50, -8);
	poly.verts[3] = c2V(16 - 50, -8);
	poly.count = 4;
	c2Norms(poly.verts, poly.norms, 4);

	draw_capsule(cap);
	draw_poly(poly);

	c2Manifold m;
	c2CapsuletoPolyManifold(cap, &poly, 0, &m);
	//c2Collide(&cap, 0, C2_CAPSULE, &poly, 0, C2_POLY, &m);
	//c2Collide(&poly, 0, C2_POLY, &cap, 0, C2_CAPSULE, &m);
	if (m.count) draw_manifold(m);
#endif

#if 0
	static int my_x;
	static int my_y;
	static int capture_mouse = 1;
	if (space_is_pressed) {
		capture_mouse = !capture_mouse;
	}
	if (capture_mouse) {
		my_x = mx;
		my_y = my;
	}
	c2Capsule cap;
	cap.a = c2V(my_x, my_y);
	cap.b = c2V(my_x, my_y - 30.0f);
	cap.r = 11.0f;

	c2AABB bb;
	bb.min = c2V(-10, -10);
	bb.max = c2V(10, 10);

	draw_capsule(cap);
	draw_aabb(c2(bb));

	c2Manifold m;
	c2AABBtoCapsuleManifold(bb, cap, &m);
	//c2Collide(&bb, 0, C2_AABB, &cap, 0, C2_CAPSULE, &m);
	//c2Collide(&cap, 0, C2_CAPSULE, &bb, 0, C2_AABB, &m);
	if (m.count) draw_manifold(m);
#endif

#if 0
	static int my_x;
	static int my_y;
	static int capture_mouse = 1;
	if (space_is_pressed) {
		capture_mouse = !capture_mouse;
	}
	if (capture_mouse) {
		my_x = mx;
		my_y = my;
	}
	c2Capsule cap;
	cap.a = c2V(my_x, my_y);
	cap.b = c2V(my_x, my_y - 30.0f);
	cap.r = 11.0f;

	c2Poly poly;
	poly.verts[0] = c2V(16 - 50, -30);
	poly.verts[1] = c2V(50 - 50, -30);
	poly.verts[2] = c2V(50 - 50, -8);
	poly.count = 3;
	c2Norms(poly.verts, poly.norms, 4);

	draw_capsule(cap);
	draw_poly(poly);
	gl_line(gfx, cap.a.x, cap.a.y, 0, cap.b.x, cap.b.y, 0);

	c2Manifold m;
	c2CapsuletoPolyManifold(cap, &poly, 0, &m);
	//c2Collide(&cap, 0, C2_CAPSULE, &poly, 0, C2_POLY, &m);
	//c2Collide(&poly, 0, C2_POLY, &cap, 0, C2_CAPSULE, &m);
	if (m.count) draw_manifold(m);
#endif
