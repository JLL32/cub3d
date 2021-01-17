#include "cub3D.h"
#include <stdlib.h>

typedef struct s_vars
{
	void *mlx;
	void *win;
} t_vars;

typedef struct s_data
{
	void *img;
	int *addr;
	int width;
	int height;
	int bits_per_pixel;
	int line_length;
	int endian;
} t_data;

typedef int t_hex_color;

enum colors
{
	red = 0x0FF0000,
	maroon = 0x0800000,
	yellow = 0x0FFFF00,
	blue = 0x00000FF,
	white = 0xFFFFFFF,
	green = 0x0008000,
	black = 0x0000000,
	light_blue = 0xADD8E6,
	gray = 0x5A5A5A,
};

t_data img;

t_vars vars;

typedef struct s_texture
{
	void *add;
	int width;
	int height;
} t_texture;

t_data textures[5];

typedef struct s_sprite
{
	double x;
	double y;
	int texture;
} t_sprite;

#define num_sprites 8

t_sprite sprite[num_sprites] =
	{
		//some pillars around the map
		{21.5, 1.5, 4},
		{15.5, 1.5, 4},
		{16.0, 1.8, 4},
		{16.2, 1.2, 4},
		{3.5, 2.5, 4},
		{9.5, 15.5, 4},
		{10.0, 15.1, 4},
		{10.5, 15.8, 4},
};

unsigned int buffer[screen_height][screen_width]; // y-coordinates first because it works per scanline

// 1D ZBuffer
double z_buffer[screen_width];

// arrays used to sort the sprites
int sprite_order[num_sprites];
double sprite_distance[num_sprites];

// function used to sort the sprites
void sort_sprites(int *order, double *dist, int amount);

int draw(t_player *player);
void my_mlx_pixel_put(t_data *data, int x, int y, int color)
{
	if (y < screen_height && y >= 0)
		data->addr[(y)*screen_width + (x)] = color;
}


int key_press(int keycode, t_player *player)
{

	//TODO: we'll have to make it check against characters of the map instead of numbers
	if (keycode == KEY_UP)
		move_up(player);
	if (keycode == KEY_DOWN)
		move_down(player);
	if (keycode == KEY_RIGHT)
		move_right(player);
	if (keycode == KEY_LEFT)
		move_left(player);

	if (keycode == KEY_D)
		turn_right(player);
	if (keycode == KEY_A)
		turn_left(player);

	if (keycode == KEY_ESC)
	{
		mlx_destroy_image(vars.mlx, textures[0].img);
		mlx_destroy_image(vars.mlx, textures[1].img);
		mlx_destroy_image(vars.mlx, textures[2].img);
		mlx_destroy_image(vars.mlx, textures[3].img);
		mlx_destroy_image(vars.mlx, textures[4].img);
		mlx_destroy_image(vars.mlx, img.img);
		mlx_clear_window(vars.mlx, vars.win);
		exit(EXIT_SUCCESS);
	}

	// TODO: don't forget to add escape and cross-red events which
	// are quiting the game and cleaning after (imgs, textures, sprites...)

	draw(player);
	return 0;
}

//sort algorithm
//sort the sprites based on distance
typedef struct s_pair
{
	double first;
	int second;
} t_pair;

void swap(t_pair *xp, t_pair *yp)
{
	t_pair temp = *xp;
	*xp = *yp;
	*yp = temp;
}
// A function to implement bubble sort
void pair_sort(t_pair arr[], int n)
{
	int i, j;
	for (i = 0; i < n - 1; i++)
		for (j = 0; j < n - i - 1; j++)
			if (arr[j].first > arr[j + 1].first)
				swap(&arr[j], &arr[j + 1]);
}

void sort_sprites(int *order, double *dist, int amount)
{
	/* std::vector<std::pair<double, int>> sprites(amount); */
	t_pair sprites[amount];
	for (int i = 0; i < amount; i++)
	{
		sprites[i].first = dist[i];
		sprites[i].second = order[i];
	}
	pair_sort(sprites, amount);
	// restore in reverse order to go from farthest to nearest
	for (int i = 0; i < amount; i++)
	{
		dist[i] = sprites[amount - i - 1].first;
		order[i] = sprites[amount - i - 1].second;
	}
}

int draw(t_player *player)
{
	for (int x = 0; x < screen_width; x++)
	{
		//calculate ray position and direction
		double camera_x = 2 * x / (double)screen_width - 1; //x-coordinate in camera space
		double ray_dir_x = player->dir_x + player->plane_x * camera_x;
		double ray_dir_y = player->dir_y + player->plane_y * camera_x;

		//which box of the map we're in
		int map_x = (int)player->pos_x;
		int map_y = (int)player->pos_y;

		//length of ray from current position to next x or y-side
		double side_dist_x;
		double side_dist_y;

		//length of ray from one x or y-side to next x or y-side
		double delta_dist_x = fabs(1.0 / ray_dir_x);
		double delta_dist_y = fabs(1.0 / ray_dir_y);
		double perp_wall_dist;

		//what direction to step in x or y-direction (either +1 or -1)
		int step_x;
		int step_y;

		int hit = 0; //was there a wall hit?
		int side;	 //was a NS or a EW wall hit?

		//calculate step and initial side_dist
		if (ray_dir_x < 0)
		{
			step_x = -1;
			side_dist_x = (player->pos_x - map_x) * delta_dist_x;
		}
		else
		{
			step_x = 1;
			side_dist_x = (map_x + 1.0 - player->pos_x) * delta_dist_x;
		}
		if (ray_dir_y < 0)
		{
			step_y = -1;
			side_dist_y = (player->pos_y - map_y) * delta_dist_y;
		}
		else
		{
			step_y = 1;
			side_dist_y = (map_y + 1.0 - player->pos_y) * delta_dist_y;
		}

		/*perform DDA*/
		while (hit == 0)
		{
			/*jump to next map square, or in x_direction, or in y-direction*/
			if (side_dist_x < side_dist_y)
			{
				side_dist_x += delta_dist_x;
				map_x += step_x;
				side = 0;
			}
			else
			{
				side_dist_y += delta_dist_y;
				map_y += step_y;
				side = 1;
			}
			/*check if ray has hit a wall*/
			if (world_map[map_x][map_y] > 0)
				hit = 1;
		}
		/*Calculate the distance projected on camera direction (Euclidean distance will give fisheye effect!)*/
		if (side == 0)
			perp_wall_dist = (map_x - player->pos_x + (1.0 - step_x) / 2) / ray_dir_x;
		else
			perp_wall_dist = (map_y - player->pos_y + (1.0 - step_y) / 2) / ray_dir_y;
		/* calculate line_height */
		int line_height = (int)(screen_height / perp_wall_dist);

		/*calculate the lowest and highest pixel fo fill in the current stripe*/
		int draw_start = -line_height / 2 + screen_height / 2;
		if (draw_start < 0)
			draw_start = 0;
		int draw_end = line_height / 2 + screen_height / 2;
		if (draw_end >= screen_height)
			draw_end = screen_height - 1;

		int tex_num;
		if (side == 1 && ray_dir_y > 0)
			tex_num = 0;
		else if (side == 1 && ray_dir_y < 0)
			tex_num = 2;
		else if (side == 0 && ray_dir_x > 0)
			tex_num = 1;
		else
			tex_num = 3;

		// calculate the value of wall_x
		double wall_x; // where exactly the wall was hit
		if (side == 0)
			wall_x = player->pos_y + perp_wall_dist * ray_dir_y;
		else
			wall_x = player->pos_x + perp_wall_dist * ray_dir_x;
		wall_x -= floor(wall_x);

		// x coordinate on the texture
		int tex_x = (int)(wall_x * (double)(tex_width));
		if (side == 0 && ray_dir_x > 0)
			tex_x = tex_width - tex_x - 1;
		if (side == 1 && ray_dir_y < 0)
			tex_x = tex_width - tex_x - 1;

		// How much to increase the texture coordinate per screen pixel
		double step = 1.0 * tex_height / line_height;
		// Starting texture coordinat
		double tex_pos = (draw_start - screen_height / 2.0 + line_height / 2.0) * step;

		int y = 0;
		while (y < draw_start)
		{
			my_mlx_pixel_put(&img, x, y, light_blue);
			y++;
		}
		while (y < draw_end)
		{
			// cast the texture coordinat to integer, and mask with (tex_height - 1) in case of overflow
			int text_y = (int)tex_pos & (tex_height - 1);
			tex_pos += step;
			int color = textures[tex_num].addr[tex_height * text_y + tex_x];
			// make color darker for y-sides: R, G, B byte each divided through two with a shift and an and
			if (side == 1)
				color = (color >> 1) & 8355711;
			my_mlx_pixel_put(&img, x, y, color);
			y++;
		}
		while (y < screen_height)
		{
			my_mlx_pixel_put(&img, x, y, gray);
			y++;
		}

		z_buffer[x] = perp_wall_dist;
	}

	// sprite casting
	// sorts sprites from for to close
	for (int i = 0; i < num_sprites; i++)
	{
		sprite_order[i] = i;
		sprite_distance[i] = ((player->pos_x - sprite[i].x) * (player->pos_x - sprite[i].x) + (player->pos_y - sprite[i].y) * (player->pos_y - sprite[i].y));
	}
	sort_sprites(sprite_order, sprite_distance, num_sprites);

	// after sorting the sprites do the projection and draw them
	for (int i = 0; i < num_sprites; i++)
	{
		// translate sprite position to relative to camera
		double sprite_x = sprite[sprite_order[i]].x - player->pos_x;
		double sprite_y = sprite[sprite_order[i]].y - player->pos_y;

		//transform sprite with the inverse camera matrix
		// [ planeX   dirX ] -1                                       [ dirY      -dirX ]
		// [               ]       =  1/(planeX*dirY-dirX*planeY) *   [                 ]
		// [ planeY   dirY ]                                          [ -planeY  planeX ]
		double inv_det = 1.0 / (player->plane_x * player->dir_y - player->dir_x * player->plane_y); // required for correct matrix multiplication

		double transform_x = inv_det * (player->dir_y * sprite_x - player->dir_x * sprite_y);
		double transform_y = inv_det * (-player->plane_y * sprite_x + player->plane_x * sprite_y);

		int sprite_screen_x = (int)((screen_width / 2) * (1 + transform_x / transform_y));

		// calculate the height of the sprite on screen
		int sprite_height = abs((int)(screen_height / transform_y));

		// calculate the lowest and highest pixels to fill in current stripe
		int draw_start_y = -sprite_height / 2 + screen_height / 2;
		if (draw_start_y < 0)
			draw_start_y = 0;
		int draw_end_y = sprite_height / 2 + screen_height / 2;
		if (draw_end_y >= screen_height)
			draw_end_y = screen_height - 1;

		//calculate width of the sprite
		int sprite_width = abs((int)(screen_height / transform_y));
		int draw_start_x = -sprite_width / 2 + sprite_screen_x;
		if (draw_start_x < 0)
			draw_start_x = 0;
		int draw_end_x = sprite_width / 2 + sprite_screen_x;
		if (draw_end_x >= screen_width)
			draw_end_x = screen_width - 1;

		//loop through every vertical stripe of the sprite on screen
		for (int stripe = draw_start_x; stripe < draw_end_x; stripe++)
		{
			int tex_x = (int)(256 * (stripe - (-sprite_width / 2 + sprite_screen_x)) * tex_width / sprite_width) / 256;
			//the conditions in the if are:
			//1) it's in front of camera plane so you don't see things behind you
			//2) it's on the screen (left)
			//3) it's on the screen (right)
			//4) ZBuffer, with perpendicular distance
			if (transform_y > 0 && stripe > 0 && stripe < screen_width && transform_y < z_buffer[stripe])
				for (int y = draw_start_y; y < draw_end_y; y++) //for every pixel of the current stripe
				{
					int d = (y)*256 - screen_height * 128 + sprite_height * 128; //256 and 128 factors to avoid floats
					int tex_y = ((d * tex_height) / sprite_height) / 256;
					int color = textures[sprite[sprite_order[i]].texture].addr[tex_width * tex_y + tex_x]; //get current color from the texture
					if ((color & 0x00FFFFFF) != 0)
						my_mlx_pixel_put(&img, stripe, y, color); //paint pixel if it isn't black, black is the invisible color
				}
		}
	}

	//Updates the screen.  Has to be called to view new pixels, but use only after
	//drawing the whole screen because it's slow.
	mlx_put_image_to_window(vars.mlx, vars.win, img.img, 0, 0);
	// speed modifiers
	player->move_speed = 0.2;
	player->rot_speed = 0.06;

	return 0;
}

int main(int argc, char *argv[argc])
{
	t_player player = create_player();

	vars.mlx = mlx_init();
	vars.win = mlx_new_window(vars.mlx, screen_width, screen_height, "Cub3D");
	img.img = mlx_new_image(vars.mlx, screen_width, screen_height);
	img.addr = (int *)mlx_get_data_addr(img.img, &img.bits_per_pixel, &img.line_length, &img.endian);

	textures[0].img = mlx_xpm_file_to_image(vars.mlx, "./assets/redbrick.xpm", &textures[0].width, &textures[0].height);
	textures[0].addr = (int *)mlx_get_data_addr(textures[0].img, &textures[0].bits_per_pixel, &textures[0].line_length, &textures[0].endian);

	textures[1].img = mlx_xpm_file_to_image(vars.mlx, "./assets/greystone.xpm", &textures[1].width, &textures[1].height);
	textures[1].addr = (int *)mlx_get_data_addr(textures[1].img, &textures[1].bits_per_pixel, &textures[1].line_length, &textures[1].endian);

	textures[2].img = mlx_xpm_file_to_image(vars.mlx, "./assets/bluestone.xpm", &textures[2].width, &textures[2].height);
	textures[2].addr = (int *)mlx_get_data_addr(textures[2].img, &textures[2].bits_per_pixel, &textures[2].line_length, &textures[2].endian);

	textures[3].img = mlx_xpm_file_to_image(vars.mlx, "./assets/mossy.xpm", &textures[3].width, &textures[3].height);
	textures[3].addr = (int *)mlx_get_data_addr(textures[3].img, &textures[3].bits_per_pixel, &textures[3].line_length, &textures[3].endian);

	textures[4].img = mlx_xpm_file_to_image(vars.mlx, "./assets/pillar.xpm", &textures[4].width, &textures[4].height);
	textures[4].addr = (int *)mlx_get_data_addr(textures[4].img, &textures[4].bits_per_pixel, &textures[4].line_length, &textures[4].endian);

	draw(&player);

	mlx_hook(vars.win, 2, 0, key_press, &player);

	mlx_loop(vars.mlx);

	return (0);
}
