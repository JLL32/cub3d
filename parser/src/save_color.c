/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   save_color.c                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jll32 <jll32@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/19 18:49:03 by mobaz             #+#    #+#             */
/*   Updated: 2021/01/28 18:12:28 by jll32            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

// #include "../cub.h"
#include "../parser.h"

void	count_element(char **element)
{
	int el_count;

	el_count = 0;
	while (element[el_count])
		el_count++;
	if (el_count != 3)
		ft_error(NULL, "Error\nFloor count is incorrect\n");
}

static int	create_rgb(int r, int g, int b)
{
	if (r > 255 || g > 255 || b > 255)
		ft_error(NULL, "Error\nRgb value must not be more than 255");
	return (((r & 0xff) << 16) + ((g & 0xff) << 8) + (b & 0xff));
}

int	save_color(char **element)
{
	int color;
	char	**color_char;
	static int color_num;
	int		i;

	i = 0;
	while (element[i])
		i++;
	if (i != 2 || color_num > 1)
		ft_error(NULL, "Error\nColor information are incorrect\n");
	color_num++;
	i = 0;
	while (element[1][i])
	{
		if (element[1][i] != ',' && (element[1][i] < '0'
			|| element[1][i] > '9'))
			ft_error(NULL, "Error\nFloor information are incorrect\n");
		i++;
	}
	color_char = ft_split(element[1], ',');
	count_element(color_char);
	color = create_rgb(ft_atoi(color_char[0]), ft_atoi(color_char[1]),
					ft_atoi(color_char[2]));
	free_double_pointer(color_char);
	free_double_pointer(element);
	return (color);
}