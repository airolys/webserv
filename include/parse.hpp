/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:01 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:29:01 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef PARSE_HPP
#define PARSE_HPP

#include "includes.hpp"
#include "Server.hpp"

//declared functions
std::pair<t_keywords, std::vector<std::string> >    make_line(std::string& line);
std::string pop_value(std::vector<std::string> &values);

//helpers
bool    clear_line(const std::string& line);
std::vector<std::string>    get_lines(std::ifstream& file);
t_keywords  id_keyword(std::string word);
#endif