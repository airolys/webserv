/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   parse.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:11 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:11 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/parse.hpp"

std::string KeywordString[] = {
    "server",
    "Name",
    "Port",
    "Host",
    "Client-max-body-size",
    "route",
    "Listing",
    "Allowed",
    "Forbidden",
    "Match",
    "Location",
    "Return",
    "Cgi-pass",
    "{",
    "}",
    "error_page"
};

//declared functions

//extract the first word as a keyword and the following non comment ones as a vector of values
std::pair<t_keywords, std::vector<std::string> >    make_line(std::string& line)
{
    std::istringstream linestream(line);

    std::vector<std::string> words;

    std::string temp;

    linestream >> temp;
    t_keywords key = id_keyword(temp);
    if (key == ERROR)
    {
        std::cerr << ERROR_WARN << "Unknow keyword: " << temp << std::endl;
        throw std::invalid_argument("Unknow keyword");

    }
    while (linestream >> temp)
    {
        if (temp[0] == '#')
            return (std::make_pair(key, words));
        words.push_back(temp);
    }
    return (std::make_pair(key, words));
}

//remove and return the first value of the vector if not empty
std::string pop_value(std::vector<std::string> &values)
{
    std::string out;
    if (values.empty())
        return ("");
    out = values.front();
    values.erase(values.begin());
    return (out);
}


//helpers
// Return true if the string contains only spaces, comments or is empty
bool    clear_line(const std::string& line)
{
    for (size_t i = 0; i < line.length(); ++i)
    {
        if (!isspace(line[i]))
        {
            if (line[i] == '#')
                return true;
            return false;
        }
    }
    return true;
}

std::vector<std::string>    get_lines(std::ifstream& file)
{
    std::vector<std::string>    lines;
    std::string tmp;

    while (file.peek() != EOF)
    {
        std::getline(file, tmp);
        if (clear_line(tmp))
            continue;
        lines.push_back(tmp);
    }
    return (lines);
}

t_keywords  id_keyword(std::string word)
{
	unsigned int			l = ERROR;

	for (unsigned int i=0; i<ERROR; i++)
	{
		if (KeywordString[i] == word)
			l = i;
	}
	return (static_cast<t_keywords>(l)) ;
}