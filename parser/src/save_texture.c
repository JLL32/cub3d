/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   save_texture.c                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: jll32 <jll32@student.42.fr>                +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2020/10/19 19:56:14 by mobaz             #+#    #+#             */
/*   Updated: 2021/02/07 17:07:47 by jll32            ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../parser.h"

char	*save_texture(char **texture)
{
	char *texture_path;
	static int tex_num;
	int i;

	if (tex_num > 4)
		ft_error(NULL, "Error\nDuplicate texture");
	tex_num++;
	i = 1;
	while (texture[i])
		i++;
	if (i != 2)
		ft_error(NULL, "Error\nTexture information is incorrect\n");
	texture_path = ft_strdup(texture[1]);
	free_double_pointer(texture);
	return (texture_path);
}
