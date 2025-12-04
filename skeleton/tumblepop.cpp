// Muhammad Haseeb
// Muhammad Ahsaan Khurshid
#include <iostream>
#include <fstream>
#include <cmath>
#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <SFML/Window.hpp>

using namespace sf;
using namespace std;

int screen_x = 1152;
int screen_y = 896;

// player collision with enemies and  resetting
void enemies_collision(bool is_player_facing_right, float &player_x, float &player_y, float enemy_x, float enemy_y, int speed)
{
	int tem_player_x;
	if (is_player_facing_right && speed < 0)
	{
		tem_player_x = player_x;
	}
	else
		tem_player_x = player_x + 32;

	float x_dis = abs(tem_player_x - enemy_x);
	float y_dis = abs(player_y - enemy_y);

	// now checking the collision
	if (x_dis <= 40 && y_dis <= 40)
	{
		player_x = 66;
		player_y = 762;
	}
}

// ghost movement function
// void ghost_move(float &ghost_x, float &g_speed, float &ghost_y, Sprite &ghost_Sprite, int low, int high)
// {
// 	ghost_x += g_speed;
// 	if (ghost_x < low)
// 	{
// 		g_speed = 2;
// 		ghost_Sprite.setScale(-2, 2);
// 		ghost_x += 50;
// 	}
// 	if (ghost_x > high)
// 	{
// 		g_speed = -2;
// 		ghost_Sprite.setScale(2, 2);
// 		ghost_x -= 50;
// 	}
// 	ghost_Sprite.setPosition(ghost_x, ghost_y);
// }

// enemy movement
void enemy_move(float enemy_x[], float enemy_speed[], float enemy_y[], Sprite enemy_Sprite[], float low[], float high[], int total_skeletons)
{
	for (int i = 0; i < total_skeletons; i++)
	{
		enemy_x[i] += enemy_speed[i];
		if (enemy_x[i] < low[i])
		{
			enemy_speed[i] = 2;
			enemy_Sprite[i].setScale(-2, 2);
			enemy_x[i] += 16;
		}
		if (enemy_x[i] > high[i])
		{
			enemy_speed[i] = -2;
			enemy_Sprite[i].setScale(2, 2);
			enemy_x[i] -= 16;
		}
		enemy_Sprite[i].setPosition(enemy_x[i], enemy_y[i]);
	}
}

void display_level(RenderWindow &window, char **lvl, Texture &bgTex, Sprite &bgSprite, Texture &blockTexture, Sprite &blockSprite, const int height, const int width, const int cell_size)
{
	window.draw(bgSprite);

	for (int i = 0; i < height; i += 1)
	{
		for (int j = 0; j < width; j += 1)
		{

			if (lvl[i][j] == '#')
			{
				blockSprite.setPosition(j * cell_size, i * cell_size);
				window.draw(blockSprite);
			}
		}
	}
}

void player_gravity(char **lvl, float &offset_x, float &offset_y, float &velocityY, bool &onGround, const float &gravity, float &terminal_Velocity, float &player_x, float &player_y, const int cell_size, int &Pheight, int &Pwidth, bool &ignoring_tiles)
{

	offset_y = player_y;
	offset_y += velocityY;
	if (velocityY < 0) // moving up
	{
		// calculating row and cols
		int row = (int)offset_y / cell_size;
		int col_right = (int)(offset_x + Pwidth) / cell_size;
		int col_left = (int)(offset_x) / cell_size;
		// rows at which there isn't jumping
		bool hard_row = (row == 9 || row == 11);
		// mid col and rows                        column:   0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 of row: 9
		bool hard_col = (col_right > 6 && col_left < 11); // # # # # # . . # # # #  .  .  #  #  #  #  #
		// using these rows and col to set the blocked region (no jumping zone)
		bool blocked_region = (hard_row && hard_col);
		bool block_check = false;
		int col = (col_left < 7 ? col_right : col_left);
		if (hard_row && lvl[row][col] == '#')
			block_check = true;			   // we found that player is touching the blocks in the blocked region
		if (block_check && blocked_region) // check both blocked region and block check
			velocityY = 0;
		else
		{
			if (offset_y >= 0) // offset_y sometimes become negetive bcz of jumpstrength
			{
				player_y = offset_y;
				onGround = false;
			}
			else
				velocityY = 0;
		}
	}
	else
	{
		char bottom_left_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x) / cell_size];
		char bottom_right_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)((player_x) + Pwidth) / cell_size];
		char bottom_mid_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)((player_x) + Pwidth / 2) / cell_size];
		if ((bottom_left_down == '#' || bottom_mid_down == '#' || bottom_right_down == '#') && !ignoring_tiles)
		{
			onGround = true;
		}
		else
		{
			player_y = offset_y;
			onGround = false;
		}
	}
	if (!onGround)
	{
		velocityY += gravity;
		if (velocityY >= terminal_Velocity)
			velocityY = terminal_Velocity;
	}
	else
	{
		velocityY = 0;
	}

	if (ignoring_tiles)
	{
		char bottom_left_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)(player_x) / cell_size];
		char bottom_right_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)((player_x) + Pwidth) / cell_size];
		char bottom_mid_down = lvl[(int)(offset_y + Pheight) / cell_size][(int)((player_x) + Pwidth / 2) / cell_size];
		if (bottom_left_down == '.' || bottom_mid_down == '.' || bottom_right_down == '.')
		{
			ignoring_tiles = false;
		}
		else
		{
			player_y = offset_y;
			onGround = false;
		}
	}
}

void move_player(char **lvl, float &offset_x, float &player_x, float speed, bool player_Green, float &player_y, int PlayerHeight, const int cell_size, int PlayerWidth, bool is_player_facing_right)
{
	char mid, bottom;
	if (is_player_facing_right)
	{
		offset_x = player_x;
		offset_x += speed * (player_Green ? 1.5 : 1);
		mid = lvl[(int)(player_y + PlayerHeight / 2) / cell_size][(int)(offset_x + PlayerWidth) / cell_size];
		bottom = lvl[(int)(player_y + PlayerHeight) / cell_size][(int)(offset_x + PlayerWidth) / cell_size];
		if (!(mid == '#' || bottom == '#'))
		{
			player_x = offset_x;
		}
	}
	else
	{
		offset_x = player_x;
		offset_x -= speed * (player_Green ? 1.5 : 1);
		mid = lvl[(int)(player_y + PlayerHeight / 2) / cell_size][(int)(offset_x) / cell_size];
		bottom = lvl[(int)(player_y + PlayerHeight) / cell_size][(int)(offset_x) / cell_size];
		if (!(mid == '#' || bottom == '#'))
		{
			player_x = offset_x;
		}
	}
}

void player_animation(float &p_animation_timer, float p_animation_speed, int &p_frame_index)
{
	p_animation_timer += p_animation_speed;
	if (p_animation_timer >= 1.0f)
	{
		p_animation_timer = 0.0f;
		p_frame_index++;
		if (p_frame_index > 4)
			p_frame_index = 0;
	}
}

void display_player(RenderWindow &window, Texture &SpriteSheet, bool player_Green, bool is_player_facing_right, Sprite GreenplayerFrame[], Sprite YellowplayerFrame[], int p_frame_index, float player_x, float player_y, int PlayerWidth)
{
	if (player_Green)
	{
		GreenplayerFrame[p_frame_index].setScale(is_player_facing_right ? -2 : 2, 2);
		GreenplayerFrame[p_frame_index].setPosition(player_x + (is_player_facing_right ? PlayerWidth : 0), player_y);
		window.draw(GreenplayerFrame[p_frame_index]);
	}
	else
	{
		YellowplayerFrame[p_frame_index].setScale(is_player_facing_right ? -2 : 2, 2);
		YellowplayerFrame[p_frame_index].setPosition(player_x + (is_player_facing_right ? PlayerWidth : 0), player_y);
		window.draw(YellowplayerFrame[p_frame_index]);
	}
}

void display_fire(RenderWindow &window, Texture SpriteSheet, bool is_player_facing_right, float &player_x, float &player_y, int PlayerWidth, Sprite fire_Sprite, int FireWidth, float &fire_x, float &fire_y)
{
	fire_x = is_player_facing_right ? player_x + PlayerWidth + FireWidth : player_x - FireWidth;
	fire_y = player_y + 15;
	fire_Sprite.setPosition(fire_x, fire_y);
	fire_Sprite.setScale(is_player_facing_right ? -2 : 2, 2);
	window.draw(fire_Sprite);
}

// collision of fire and enemies
bool isColliding(float f_x, float f_y, float f_w, float f_h, float e_x, float e_y, float e_w, float e_h)
{
	return !(f_x + f_w < e_x || f_x > e_x + e_w || f_y + f_h < e_y || f_y > e_y + e_h);
}

int main()
{
	RenderWindow window(VideoMode(screen_x, screen_y), "Tumble-POP", Style::Default);
	window.setVerticalSyncEnabled(true);
	window.setFramerateLimit(60);

	// level specifics
	const int cell_size = 64;
	const int height = 14;
	const int width = 18;
	char **lvl;

	// ----------All images----------

	// -----level and background textures and sprites-----
	Texture bgTex;
	Sprite bgSprite;

	bgTex.loadFromFile("Data/bg.png");
	bgSprite.setTexture(bgTex);
	bgSprite.setPosition(0, 0);

	Texture blockTexture;
	Sprite blockSprite;

	blockTexture.loadFromFile("Data/block1.png");
	blockSprite.setTexture(blockTexture);

	// -----Spritesheet loading-----
	Texture SpriteSheet;
	SpriteSheet.loadFromFile("../Assets/Sprites.png");

	// -----Player selection image-----
	Sprite g_Select_Sprite;
	g_Select_Sprite.setTexture(SpriteSheet);
	g_Select_Sprite.setTextureRect(IntRect(185, 42, 31, 39));

	Sprite y_Select_Sprite;
	y_Select_Sprite.setTexture(SpriteSheet);
	y_Select_Sprite.setTextureRect(IntRect(185, 231, 31, 39));

	// -----fire-----
	Sprite fire_Sprite;
	fire_Sprite.setTexture(SpriteSheet);
	fire_Sprite.setTextureRect(IntRect(349, 174, 48, 29));
	int FireWidth = 96;
	int FireHeight = 58;
	float fire_x = 0;
	float fire_y = 0;

	// -----Ghostsheet loading-----
	Texture ghostSheet;
	ghostSheet.loadFromFile("Data/Ghost.png");
	const int total_ghosts = 4;

	// -----ghost images-----
	// x coordinates of ghosts
	float ghost_x[total_ghosts] = {128, 128, 800, 800};
	// y coordinates of ghosts
	float ghost_y[total_ghosts] = {512, 256, 256, 512};
	// speed of ghosts
	float ghost_speed[total_ghosts] = {2, 2, 2, 2};  
	// left and right limits of ghosts
	float ghost_left_x[total_ghosts] = {64, 64, 840, 840};
	float ghost_right_x[total_ghosts] = {320, 320, 1090, 1090};
	// creating ghost sprites
	Sprite ghost_sprite[total_ghosts];
	for (int i = 0; i < total_ghosts; i++)
	{
		ghost_sprite[i].setTexture(ghostSheet);
		ghost_sprite[i].setTextureRect(IntRect(8, 8, 35, 32));
		ghost_sprite[i].setScale(-2, 2);
		ghost_sprite[i].setPosition(ghost_x[i], ghost_y[i]);
	}

	// -----Skeletonsheet loading-----
	Texture skeletonSheet;
	skeletonSheet.loadFromFile("Data/Skeleton.png");
	const int total_skeletons = 4;

	// -----skeleton images-----
	// x coordinates of skeletons
	float skeleton_x[total_skeletons] = {200, 800, 150, 900};
	// y coordinates of skeletons
	float skeleton_y[total_skeletons] = {120, 120, 370, 370};

	//float skeleton_timer[total_skeletons] = {0, 20, 40, 10};

	//float skeleton_velocity[total_skeletons] = {0, 0, 0, 0};
	float skeleton_speed[total_skeletons] = {2, -2, 2, -2};
	// left and right limits of skeletons
	float skeleton_left_x[total_skeletons] = {210, 670, 210, 750};
	float skeleton_right_x[total_skeletons] = {400, 926, 400, 926};
	// creating skeleton sprites
	Sprite skeleton_sprite[total_skeletons];
	for (int i = 0; i < total_skeletons; i++)
	{
		skeleton_sprite[i].setTexture(skeletonSheet);
		skeleton_sprite[i].setTextureRect(IntRect(8, 34, 32, 38));
		skeleton_sprite[i].setScale(2, 2);
		skeleton_sprite[i].setPosition(skeleton_x[i], skeleton_y[i]);
	}
	bool is_g_alive[total_ghosts] = {1, 1, 1, 1};
	bool is_s_alive[total_skeletons] = {1, 1, 1, 1};

	// ------Loading Skeleton------
	// Sprite skeleton_Sprite[5];
	// for (int i = 0; i < 5; i++)
	// {
	// 	skeleton_Sprite[i].setTexture(skeletonSheet);
	// 	if (i == 0)
	// 		skeleton_Sprite[i].setTextureRect(IntRect(8, 34, 32, 38));
	// 	else if (i == 1)
	// 		skeleton_Sprite[i].setTextureRect(IntRect(193, 33, 24, 39));
	// 	else if (i == 2)
	// 		skeleton_Sprite[i].setTextureRect(IntRect(223, 34, 26, 38));
	// 	else if (i == 3)
	// 		skeleton_Sprite[i].setTextureRect(IntRect(259, 33, 26, 39));
	// 	else if (i == 4)
	// 		skeleton_Sprite[i].setTextureRect(IntRect(291, 34, 31, 38));
	// }
	// Sprite skeletonFrame[5] = {skeleton_Sprite[0], skeleton_Sprite[1], skeleton_Sprite[2], skeleton_Sprite[3], skeleton_Sprite[4]};
	// int sk_frame_index = 0;
	// float sk_animation_timer = 0.0f;
	// const float sk_animation_speed = 0.15f;
	// float skeleton_x = 190;
	// float skeleton_y = 248;
	// int skeleton_width = 64;
	// int skeleton_height = 78;
	// bool is_skeleton_facing_right = true;

	// -----The title font-----
	Texture font_Texture;
	Sprite font_Sprite;

	font_Texture.loadFromFile("Data/font.png");
	font_Sprite.setTexture(font_Texture);
	font_Sprite.setPosition(176, 364);
	font_Sprite.setScale(2, 2);

	// ------Loading Green player------
	Sprite p1_Sprite[6];
	for (int i = 0; i < 6; i++)
	{
		p1_Sprite[i].setTexture(SpriteSheet);
		// Walking animation frames
		if (i == 0)
			p1_Sprite[i].setTextureRect(IntRect(16, 43, 31, 39));
		else if (i == 1)
			p1_Sprite[i].setTextureRect(IntRect(51, 43, 31, 39));
		else if (i == 2)
			p1_Sprite[i].setTextureRect(IntRect(84, 43, 31, 39));
		else if (i == 3)
			p1_Sprite[i].setTextureRect(IntRect(117, 43, 31, 39));
		else if (i == 4)
			p1_Sprite[i].setTextureRect(IntRect(150, 43, 31, 39));
		// showing Health image
		else if (i == 5)
			p1_Sprite[i].setTextureRect(IntRect(17, 18, 15, 14));
	}
	// ------Loading Yellow player------
	Sprite p2_Sprite[6];
	for (int i = 0; i < 6; i++)
	{
		p2_Sprite[i].setTexture(SpriteSheet);
		if (i == 0)
			p2_Sprite[i].setTextureRect(IntRect(16, 232, 31, 39));
		else if (i == 1)
			p2_Sprite[i].setTextureRect(IntRect(51, 232, 31, 39));
		else if (i == 2)
			p2_Sprite[i].setTextureRect(IntRect(84, 232, 31, 39));
		else if (i == 3)
			p2_Sprite[i].setTextureRect(IntRect(117, 232, 31, 39));
		else if (i == 4)
			p2_Sprite[i].setTextureRect(IntRect(150, 232, 31, 39));
		// showing Health image
		else if (i == 5)
			p2_Sprite[i].setTextureRect(IntRect(17, 207, 15, 14));
	}

	// -----arrays of both players sprites for animation walking----
	Sprite GreenplayerFrame[5] = {p1_Sprite[0], p1_Sprite[1], p1_Sprite[2], p1_Sprite[3], p1_Sprite[4]};
	Sprite YellowplayerFrame[5] = {p2_Sprite[0], p2_Sprite[1], p2_Sprite[2], p2_Sprite[3], p2_Sprite[4]};
	int p_frame_index = 0;
	float p_animation_timer = 0.0f;
	const float p_animation_speed = 0.15f; // Adjust the speed of animation

	// if it is true, then move to player selection screen
	bool isPlayerSelection = false;
	// player selected?
	bool player_Selected = false;
	// true -> green , false -> yellow
	bool player_Green = false;

	// -----Music initialisation-----
	Music lvlMusic;

	lvlMusic.openFromFile("Data/mus.ogg");
	lvlMusic.setVolume(20);
	lvlMusic.play();
	lvlMusic.setLoop(true);

	// -----player data-----
	float player_x = 382;
	float player_y = 763;

	float speed = 5;

	const float jumpStrength = -18; // Initial jump velocity
	const float gravity = 1;		// Gravity acceleration

	// bool isJumping = false; // Track if jumping

	// bool up_collide = false;
	// bool left_collide = false;
	// bool right_collide = false;

	bool onGround = false;
	bool is_player_facing_right = false; // to track player direction

	float offset_x = 0;
	float offset_y = 0;
	float velocityY = 0;

	float terminal_Velocity = 20;

	int PlayerHeight = 68;
	int PlayerWidth = 64;

	// for clampling the player when it reaches ground (down key press)
	bool reach_last_ground = false;
	bool ignoring_tiles = false;

	// char top_left = '\0';
	// char top_right = '\0';
	// char top_mid = '\0';

	// char left_mid = '\0';
	// char right_mid = '\0';

	// char bottom_left = '\0';
	// char bottom_right = '\0';
	// char bottom_mid = '\0';

	// char bottom_left_down = '\0';
	// char bottom_right_down = '\0';
	// char bottom_mid_down = '\0';

	// char top_right_up = '\0';
	// char top_mid_up = '\0';
	// char top_left_up = '\0';

	// creating level array
	lvl = new char *[height];
	for (int i = 0; i < height; i += 1)
	{
		lvl[i] = new char[width];
	}

	// -----storing '.' in the lvl array-----
	for (int i = 0; i < height; i++)
		for (int j = 0; j < width; j++)
			lvl[i][j] = '.';
	// -----Adding level 1 blocks-----
	for (int i = 0; i < width; i++)
		lvl[0][i] = '#'; // 0th row
	for (int i = 3; i < 15; i++)
		lvl[3][i] = '#'; // 3rd row
	for (int i = 8; i < 10; i++)
		lvl[4][i] = '#'; // 4rth row
	for (int i = 0; i < 5; i++)
		lvl[5][i] = '#'; // 5th row
	for (int i = 7; i < 11; i++)
		lvl[5][i] = '#'; // 5th row
	for (int i = 13; i < width; i++)
		lvl[5][i] = '#'; // 5th row
	for (int i = 6; i < 9; i++)
		lvl[i][7] = '#'; // 7th col
	for (int i = 3; i < 7; i++)
		lvl[7][i] = '#'; // 7th row
	for (int i = 11; i < 15; i++)
		lvl[7][i] = '#'; // 7th row
	for (int i = 0; i < 5; i++)
		lvl[9][i] = '#'; // 9th row
	for (int i = 7; i < 11; i++)
		lvl[9][i] = '#'; // 9th row
	for (int i = 13; i < width; i++)
		lvl[9][i] = '#'; // 9th row
	for (int i = 8; i < 10; i++)
		lvl[10][i] = '#'; // 10th row
	for (int i = 3; i < 15; i++)
		lvl[11][i] = '#'; // 11th row
	for (int i = 6; i < 9; i++)
		lvl[i][10] = '#'; // 10th col
	for (int i = 0; i < width; i++)
		lvl[height - 1][i] = '#'; // 13th row
	for (int i = 0; i < 14; i++)
		lvl[i][0] = '#'; // first column
	for (int i = 0; i < 14; i++)
		lvl[i][17] = '#'; // last column

	Event ev;
	// -----main loop-----
	while (window.isOpen())
	{
		while (window.pollEvent(ev))
		{
			if (ev.type == Event::Closed)
			{
				window.close();
			}

			if (ev.type == Event::KeyPressed)
			{
			}
		}
		// First show the tumblepop image
		if (!isPlayerSelection)
		{
			window.clear();
			window.draw(font_Sprite);
			window.display();

			if (ev.type == Event::KeyPressed && Keyboard::isKeyPressed(Keyboard::Enter))
				isPlayerSelection = true; // if true, then move to player selection screen
			continue;
		}
		// player selection screen
		if (isPlayerSelection && !player_Selected)
		{
			g_Select_Sprite.setPosition(300, 400);
			y_Select_Sprite.setPosition(700, 400);
			g_Select_Sprite.setScale(4, 4);
			y_Select_Sprite.setScale(4, 4);

			if (Keyboard::isKeyPressed(Keyboard::Right))
				player_Green = false; // Yellow player
			if (Keyboard::isKeyPressed(Keyboard::Left))
				player_Green = true; // Green player
			if (player_Green)
				g_Select_Sprite.setScale(5, 5);
			else
				y_Select_Sprite.setScale(5, 5);
			if (ev.type == Event::KeyPressed && Keyboard::isKeyPressed(Keyboard::Enter))
				player_Selected = true;

			window.clear();
			window.draw(g_Select_Sprite);
			window.draw(y_Select_Sprite);
			window.display();

			continue;
		}
		// presing escape to close
		if (Keyboard::isKeyPressed(Keyboard::Escape))
		{
			window.close();
		}
		// Movement of the player
		if (Keyboard::isKeyPressed(Keyboard::Left))
		{
			is_player_facing_right = false;
			move_player(lvl, offset_x, player_x, speed, player_Green, player_y, PlayerHeight, cell_size, PlayerWidth, is_player_facing_right);
		}
		if (Keyboard::isKeyPressed(Keyboard::Right))
		{
			is_player_facing_right = true;
			move_player(lvl, offset_x, player_x, speed, player_Green, player_y, PlayerHeight, cell_size, PlayerWidth, is_player_facing_right);
		}
		if (Keyboard::isKeyPressed(Keyboard::Up) && onGround)
		{
			velocityY = jumpStrength;
			onGround = false;
		}
		if ((int)(player_y + PlayerHeight) / cell_size >= 12)
		{
			reach_last_ground = true;
		}
		else
			reach_last_ground = false;
		// checking if player is in the blocked region
		int col = (int)(player_x + PlayerWidth / 2) / cell_size;
		int row = (int)(player_y + PlayerHeight / 2) / cell_size;
		if (!((row == 2 || row == 4) && col >= 7 && col <= 10))
		{
			// down movement
			if (Keyboard::isKeyPressed(Keyboard::Down) && onGround && !reach_last_ground)
			{
				velocityY = 10;
				ignoring_tiles = true;
			}
		}

		// Animation of the player
		if (Keyboard::isKeyPressed(Keyboard::Right) || Keyboard::isKeyPressed(Keyboard::Left))
		{
			player_animation(p_animation_timer, p_animation_speed, p_frame_index);
		}

		// enemy skeleton
		enemy_move(skeleton_x, skeleton_speed, skeleton_y, skeleton_sprite, skeleton_left_x, skeleton_right_x, total_skeletons);
		// ghost thigs calling for  different functions
		enemy_move(ghost_x, ghost_speed, ghost_y, ghost_sprite, ghost_left_x, ghost_right_x, total_ghosts);

		for (int i = 0; i < total_ghosts; i++)
		{
			// ghost_move(ghost_x[i],ghost_speed[i],ghost_y[i],ghost_sprite[i],ghost_left_x,ghost_right_x);
			if (is_g_alive[i])
				enemies_collision(is_player_facing_right, player_x, player_y, ghost_x[i], ghost_y[i], ghost_speed[i]);
		}

		// skeleton collision function call
		for (int i = 0; i < total_skeletons; i++)
		{
			if (is_s_alive[i])
				enemies_collision(is_player_facing_right, player_x, player_y, skeleton_x[i], skeleton_y[i], skeleton_speed[i]);
		}

		player_gravity(lvl, offset_x, offset_y, velocityY, onGround, gravity, terminal_Velocity, player_x, player_y, cell_size, PlayerHeight, PlayerWidth, ignoring_tiles);

		window.clear();
		display_level(window, lvl, bgTex, bgSprite, blockTexture, blockSprite, height, width, cell_size);

		// for health display
		if (player_Green)
		{
			p1_Sprite[5].setPosition(126, 70);
			p1_Sprite[5].setScale(4, 4);
			window.draw(p1_Sprite[5]);
		}
		else
		{
			p2_Sprite[5].setPosition(126, 70);
			p2_Sprite[5].setScale(4, 4);
			window.draw(p2_Sprite[5]);
		}

		display_player(window, SpriteSheet, player_Green, is_player_facing_right, GreenplayerFrame, YellowplayerFrame, p_frame_index, player_x, player_y, PlayerWidth);
		if (Keyboard::isKeyPressed(Keyboard::Space))
			display_fire(window, SpriteSheet, is_player_facing_right, player_x, player_y, PlayerWidth, fire_Sprite, FireWidth, fire_x, fire_y);

		// fire and enemies collision
		if (Keyboard::isKeyPressed(Keyboard::Space))
		{
			for (int i = 0; i < total_skeletons; i++)
			{
				bool hit = isColliding(fire_x, fire_y, FireWidth, FireHeight, skeleton_x[i], skeleton_y[i], 64, 76);
				if (hit)
				{
					is_s_alive[i] = 0;
					// skeleton_speed[i] = 0; // stop movement
				}
			}

			for (int i = 0; i < total_ghosts; i++)
			{
				bool hit = isColliding(fire_x, fire_y, FireWidth, FireHeight, ghost_x[i], ghost_y[i], 70, 64);
				if (hit)
				{
					is_g_alive[i] = 0;
					// ghost_speed[i] = 0; // stop movement
				}
			}
		}

		for (int i = 0; i < total_skeletons; i++)
		{
			if (is_s_alive[i])
				window.draw(skeleton_sprite[i]);
		}
		for (int i = 0; i < total_skeletons; i++)
		{
			if (is_g_alive[i])
				window.draw(ghost_sprite[i]);
		}

		// sk_animation_timer += sk_animation_speed;
		// if (is_skeleton_facing_right)
		// {
		// 	skeleton_x += 2;
		// }
		// else
		// {
		// 	skeleton_x -= 2;
		// }
		// char sk_bottom_mid, sk_bottom_right, sk_bottom_left, sk_right_mid, sk_left_mid;
		// sk_bottom_left = lvl[(int)(skeleton_y + skeleton_height) / cell_size][(int)(skeleton_x) / cell_size];
		// sk_bottom_right = lvl[(int)(skeleton_y + skeleton_height) / cell_size][(int)(skeleton_x + skeleton_width) / cell_size];
		// sk_bottom_mid = lvl[(int)(skeleton_y + skeleton_height) / cell_size][(int)(skeleton_x + skeleton_width / 2) / cell_size];
		// sk_right_mid = lvl[(int)(skeleton_y + skeleton_height / 2) / cell_size][(int)(skeleton_x + skeleton_width) / cell_size];
		// sk_left_mid = lvl[(int)(skeleton_y + skeleton_height / 2) / cell_size][(int)(skeleton_x) / cell_size];
		// if (sk_right_mid == '#')
		// {
		// 	is_skeleton_facing_right = false;
		// }
		// if (sk_left_mid == '#')
		// {
		// 	is_skeleton_facing_right = true;
		// }
		// if (!(sk_bottom_left == '#' || sk_bottom_mid == '#' || sk_bottom_right == '#'))
		// {
		// 	skeleton_y += 40;
		// }
		// if (sk_animation_timer >= 1.0f)
		// {
		// 	sk_animation_timer = 0.0f;
		// 	sk_frame_index++;
		// 	if (sk_frame_index > 4)
		// 		sk_frame_index = 0;
		// }
		// skeletonFrame[sk_frame_index].setScale(is_skeleton_facing_right ? -2 : 2, 2);
		// skeletonFrame[sk_frame_index].setPosition(skeleton_x + (is_skeleton_facing_right ? skeleton_width : 0), skeleton_y);
		// window.draw(skeletonFrame[sk_frame_index]);

		window.display();
	}

	// stopping music and deleting level array
	lvlMusic.stop();
	for (int i = 0; i < height; i++)
	{
		delete[] lvl[i];
	}

	delete[] lvl;
	return 0;
}
