


void InitPlatformer()
{
}

void UpdatePlatformer(float dt)
{
	//gl_line(gfx, 0, 0, 0, 100, 100, 0);

	circle_t circle;
	circle.p = v2(-50, 25);
	circle.r = 10;
	draw_circle(circle);

	circle_t circle2;
	circle2.p = v2(-10, 25);
	circle2.r = 10;
	draw_circle(circle2);
}

