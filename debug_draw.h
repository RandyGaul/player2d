#ifndef DEBUGDRAW_H
#define DEBUGDRAW_H

struct color_t
{
	float r;
	float g;
	float b;
};

struct vertex_t
{
	v2 pos;
	color_t col;
};

void draw_poly(v2* verts, int count)
{
	for (int i = 0; i < count; ++i)
	{
		int iA = i;
		int iB = (i + 1) % count;
		v2 a = verts[iA];
		v2 b = verts[iB];
		gl_line(gfx, a.x, a.y, 0, b.x, b.y, 0);
	}
}

void draw_aabb(v2 a, v2 b)
{
	v2 c = v2(a.x, b.y);
	v2 d = v2(b.x, a.y);
	gl_line(gfx, a.x, a.y, 0, c.x, c.y, 0);
	gl_line(gfx, c.x, c.y, 0, b.x, b.y, 0);
	gl_line(gfx, b.x, b.y, 0, d.x, d.y, 0);
	gl_line(gfx, d.x, d.y, 0, a.x, a.y, 0);
}

void draw_half_circle(v2 a, v2 b)
{
	v2 u = b - a;
	float r = len(u);
	u = skew(u);
	v2 v = ccw90(u);
	v2 s = v + a;
	c2m m;
	m.x = v2c2(norm(u));
	m.y = v2c2(norm(v));

	int kSegs = 20;
	float theta = 0;
	float inc = 3.14159265f / (float)kSegs;
	c2v p0;
	c2SinCos(theta, &p0.y, &p0.x);
	p0 = c2Mulvs(p0, r);
	p0 = c2Add(c2Mulmv(m, p0), v2c2(a));
	for (int i = 0; i < kSegs; ++i)
	{
		theta += inc;
		c2v p1;
		c2SinCos(theta, &p1.y, &p1.x);
		p1 = c2Mulvs(p1, r);
		p1 = c2Add(c2Mulmv(m, p1), v2c2(a));
		gl_line(gfx, p0.x, p0.y, 0, p1.x, p1.y, 0);
		p0 = p1;
	}
}

void draw_capsule(capsule_t capsule)
{
	c2v a; c2v b; float r;
	a = capsule.a;
	b = capsule.b;
	r = capsule.r;
	c2v n = c2Norm(c2Sub(b, a));
	draw_half_circle(c2v2(a), c2v2(c2Add(a, c2Mulvs(n, -r))));
	draw_half_circle(c2v2(b), c2v2(c2Add(b, c2Mulvs(n, r))));
	c2v p0 = c2Add(a, c2Mulvs(c2Skew(n), r));
	c2v p1 = c2Add(b, c2Mulvs(c2CCW90(n), -r));
	gl_line(gfx, p0.x, p0.y, 0, p1.x, p1.y, 0);
	p0 = c2Add(a, c2Mulvs(c2Skew(n), -r));
	p1 = c2Add(b, c2Mulvs(c2CCW90(n), r));
	gl_line(gfx, p0.x, p0.y, 0, p1.x, p1.y, 0);
}

void draw_circle(circle_t circle)
{
	c2v p = v2c2(circle.p);
	float r = circle.r;
	int kSegs = 40;
	float theta = 0;
	float inc = 3.14159265f * 2.0f / (float)kSegs;
	float px, py;
	c2SinCos(theta, &py, &px);
	px *= r; py *= r;
	px += p.x; py += p.y;
	for (int i = 0; i <= kSegs; ++i)
	{
		theta += inc;
		float x, y;
		c2SinCos(theta, &y, &x);
		x *= r; y *= r;
		x += p.x; y += p.y;
		gl_line(gfx, x, y, 0, px, py, 0);
		px = x; py = y;
	}
}

void draw_manifold(c2Manifold m)
{
	c2v n = m.n;
	gl_line_color(gfx, 1.0f, 0.2f, 0.4f);
	for (int i = 0; i < m.count; ++i)
	{
		c2v p = m.contact_points[i];
		float d = m.depths[i];
		circle_t circle;
		circle.p = c2v2(p);
		circle.r = 3.0f;
		draw_circle(circle);
		gl_line(gfx, p.x, p.y, 0, p.x + n.x * d, p.y + n.y * d, 0);
	}
}

#endif // DEBUGDRAW_H
