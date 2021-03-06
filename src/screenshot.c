/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   screenshot.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: ael-hach <ael-hach@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2021/02/09 15:18:07 by ael-hach          #+#    #+#             */
/*   Updated: 2021/02/09 16:09:10 by ael-hach         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../cub.h"

static char		*make_bmp_header(t_bitmapheader *header, t_resolution res)
{
	char	*buf;

	buf = ft_calloc(54, 1);
	header->bit_count = 24;
	header->width_in_bytes = ((res.width * header->bit_count + 31)
	/ 32) * 4;
	header->image_size = header->width_in_bytes * res.height;
	header->size = 54 + header->image_size;
	header->off_bits = 54;
	header->info_size = 40;
	header->planes = 1;
	header->width = res.width;
	header->height = res.height;
	ft_memcpy(buf, "BM", 2);
	ft_memcpy(buf + 2, &(header->size), 4);
	ft_memcpy(buf + 10, &(header->off_bits), 4);
	ft_memcpy(buf + 14, &(header->info_size), 4);
	ft_memcpy(buf + 18, &(header->width), 4);
	ft_memcpy(buf + 22, &(header->height), 4);
	ft_memcpy(buf + 26, &(header->planes), 2);
	ft_memcpy(buf + 28, &(header->bit_count), 2);
	ft_memcpy(buf + 34, &(header->image_size), 4);
	return (buf);
}

static int		*get_colors(int color)
{
	int *colors;

	colors = malloc(3 * sizeof(int));
	colors[0] = ((color >> 16) & 0xFF);
	colors[1] = ((color >> 8) & 0xFF);
	colors[2] = ((color) & 0xFF);
	return (colors);
}

static char		*make_img_buff(t_bitmapheader *header, t_game game)
{
	char	*buf;
	int		i;
	int		j;
	int		*colors;

	buf = malloc(header->image_size);
	i = header->height - 1;
	while (i > 0)
	{
		j = 0;
		while (j < header->width)
		{
			colors = get_colors(game.win_buffer.addr[((game.res.height - i)
			* game.res.width) + j]);
			buf[i * header->width_in_bytes + j * 3 + 2] = colors[0];
			buf[i * header->width_in_bytes + j * 3 + 1] = colors[1];
			buf[i * header->width_in_bytes + j * 3 + 0] = colors[2];
			free(colors);
			j++;
		}
		i--;
	}
	return (buf);
}

void			take_screenshot(t_game game)
{
	t_bitmapheader	header;
	char			*header_str;
	char			*img_buf;

	header.fd = open("./screenshot.bmp", O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR);
	header_str = make_bmp_header(&header, game.res);
	img_buf = make_img_buff(&header, game);
	write(header.fd, header_str, 54);
	write(header.fd, img_buf, header.image_size);
	free(header_str);
	free(img_buf);
}
