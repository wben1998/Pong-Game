#define is_down(b) input -> buttons[b].is_down
#define pressed(b) (input -> buttons[b].is_down && input -> buttons[b].changed)
#define released(b) (!input -> buttons[b].is_down && input -> buttons[b].changed)

float player_1_p, player_1_dp, player_2_p, player_2_dp;
float arena_half_size_x = 85, arena_half_size_y = 45;
float player_half_size_x = 2.5, player_half_size_y = 12;
float ball_p_x, ball_p_y, ball_dp_x = 125, ball_dp_y, ball_half_size = 1;
int player_1_score, player_2_score = 0;

internal void simulate_player(float* p, float* dp, float ddp, float dt) {
	// Friction
	ddp -= *dp * 10.f;
	// Physics equations
	*p = *p + *dp * dt + ddp * dt * dt * .5f;
	*dp = *dp + ddp * dt;
	// Stopping player form moving out of bounds
	if (*p + player_half_size_y > arena_half_size_y) {
		*p = arena_half_size_y - player_half_size_y;
		*dp = 0; // Stops the player
	}
	else if (*p - player_half_size_y < -arena_half_size_y) {
		*p = -arena_half_size_y + player_half_size_y;
		*dp = 0; // Stops the player
	}
}

internal bool aabb_vs_aabb(float p1x, float p1y, float hs1x, float hs1y, float p2x, float p2y, float hs2x, float hs2y) {
	return (p1x + hs1x > p2x - hs2x &&
		p1x - hs1x < p2x + hs2x &&
		p1y + hs1y > p2y - hs2y &&
		p1y + hs1y < p2y + hs2y);
}

enum Gamemode {
	GM_MENU,
	GM_GAMEPLAY
};

Gamemode current_gamemode;
int hot_button;
bool enemy_is_ai;

internal void simulate_game(Input* input, float dt) {
	draw_rect(0, 0, arena_half_size_x, arena_half_size_y, 0x000000);
	draw_arena_borders(arena_half_size_x, arena_half_size_y, 0x000000);

	if (current_gamemode == GM_GAMEPLAY) {

		if (pressed(BUTTON_ESC)) {
			running = false;
		}

		float player_1_ddp = 0.f; //acceleration
		// AI bot
		if (!enemy_is_ai) {
			if (is_down(BUTTON_UP)) player_1_ddp += 2000;
			if (is_down(BUTTON_DOWN)) player_1_ddp -= 2000;
		}
		else {
			player_1_ddp = (ball_p_y - player_1_p) * 100;
			if (player_1_ddp > 1300) player_1_ddp = 1300;
			if (player_1_ddp < -1300 - 2.f) player_1_ddp = -1300;
		}

		float player_2_ddp = 0.f; //acceleration
		if (is_down(BUTTON_W)) player_2_ddp += 2000;
		if (is_down(BUTTON_S)) player_2_ddp -= 2000;

		simulate_player(&player_1_p, &player_1_dp, player_1_ddp, dt);
		simulate_player(&player_2_p, &player_2_dp, player_2_ddp, dt);

		// Simulate ball
		{
			// Ball
			ball_p_x += ball_dp_x * dt;
			ball_p_y += ball_dp_y * dt;

			// Player 1 paddle collision
			if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 80, player_1_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = 80 - player_half_size_x - ball_half_size;
				ball_dp_x *= -1;

				ball_dp_y = (ball_p_y - player_1_p) * 2 + player_1_dp * .75f; //set ball's y axis movement to player's speed based on location
			}
			// Player 2 paddle collision
			else if (aabb_vs_aabb(ball_p_x, ball_p_y, ball_half_size, ball_half_size, -80, player_2_p, player_half_size_x, player_half_size_y)) {
				ball_p_x = -80 + player_half_size_x + ball_half_size;
				ball_dp_x *= -1;

				ball_dp_y = (ball_p_y - player_2_p) * 2 + player_2_dp * .75f; //set ball's y axis movement to player's speed based on location
			}
			// Top and bottom of screen collision
			if (ball_p_y + ball_half_size > arena_half_size_y) {
				ball_p_y = arena_half_size_y - ball_half_size;
				ball_dp_y *= -1;
			}
			else if (ball_p_y - ball_half_size < -arena_half_size_y) {
				ball_p_y = -arena_half_size_y + ball_half_size;
				ball_dp_y *= -1;
			}
			// Left and right of screen collision - should reset ball
			if (ball_p_x + ball_half_size > arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_1_score++;
			}
			else if (ball_p_x - ball_half_size < -arena_half_size_x) {
				ball_dp_x *= -1;
				ball_dp_y = 0;
				ball_p_x = 0;
				ball_p_y = 0;
				player_2_score++;
			}
		}

		// Score rendering
		draw_number(player_1_score, -10, 40, 1.f, 0x0028ff);
		draw_number(player_2_score, 10, 40, 1.f, 0xff0000);

		// Render
		draw_rect(ball_p_x, ball_p_y, ball_half_size, ball_half_size, 0xffffff);
		// Player 1
		draw_rect(80, player_1_p, player_half_size_x, player_half_size_y, 0xff0000);
		// Player 2
		draw_rect(-80, player_2_p, player_half_size_x, player_half_size_y, 0x0028ff);

	}
	else {

		if (pressed(BUTTON_LEFT) || pressed(BUTTON_RIGHT)) {
			hot_button = !hot_button; //this lets the user switch the selected mode and see it visually
		}

		if (pressed(BUTTON_ENTER)) {
			current_gamemode = GM_GAMEPLAY;
			enemy_is_ai = hot_button ? 0 : 1;
		}

		if (pressed(BUTTON_ESC)) {
			running = false;
		}

		if (hot_button == 0) {
			draw_text("SINGLE PLAYER", -75, -20, .69, 0xff0000);
			draw_text("MULTIPLAYER", 12, -20, .69, 0xd2d7d7);
		}
		else {
			draw_text("SINGLE PLAYER", -75, -20, .69, 0xd2d7d7);
			draw_text("MULTIPLAYER", 12, -20, .69, 0xff0000);
		}
		draw_text("PONG GAME", -55, 35, 1.5, 0xffffff);
		draw_text("PRESS ESC TO EXIT", -35, 4, .5, 0xffffff);
		draw_text("BY BEN WANG", -30, 18, .69, 0xffffff);
	}
}