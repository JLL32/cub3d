/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   map_error_utils.c                                  :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: mobaz <mobaz@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/11/12 10:25:15 by mobaz             #+#    #+#             */
/*   Updated: 2020/11/12 10:27:04 by mobaz            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

//#include "../cub.h"
#include "../parser.h"

void		check_if_valid(t_config *config, int i, int j)
{
	char **map;

	map = config->map;
	if (!is_player(config, i, j) && !is_sprite(config, i, j) && map[i][j] != '1' && map[i][j] != '0'
		 && map[i][j] != ' ')
		ft_map_error(config->map, i, j);
}

void		check_next_row(char **map, int i, int j, char c, int curr_len)
{
	int next_len;

	next_len = 0;
	if (map[i + 1])
		next_len = ft_strlen(map[i + 1]);
	if (next_len >= curr_len)
	{
		if (map[i + 1][j] == c)
			ft_map_error(map, i, j);
	}
	else if (c == ' ' && j > next_len)
		ft_map_error(map, i, j);
}

void		check_prev_row(char **map, int i, int j, char c, int curr_len)
{
	if (g_prev_len >= curr_len)
	{
		if (map[i - 1][j] == c)
			ft_map_error(map, i, j);
	}
	else if (c == ' ' && j > g_prev_len)
		ft_map_error(map, i, j);
}

void		check_up_down(char **map, int i, int j, char c, int curr_len)
{
	check_next_row(map, i, j, c, curr_len);
	check_prev_row(map, i, j, c, curr_len);
}