/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   route.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:30 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:30 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Route.hpp"

Route::Route(Vserv& serv, std::vector<std::string>& lines)
    :  _methods(0), _match(""), _replace(""), _serv(&serv), _listing(""), _cgi_pass(false)
{
    std::pair<t_keywords, std::vector<std::string> > lineinfo = make_line(lines.front());
    lines.erase(lines.begin());
    while (lineinfo.first != DELIM2)
    {
        std::string value = pop_value(lineinfo.second);
        switch (lineinfo.first)
        {
        case LISTING:
        {
            _listing = value == "OFF" ? "" : value;
            break;
        }
        case AMETHODS:
        {
            if (value == "POST")
                _methods |= POST;
            else if (value == "GET")
                _methods |= GET;
            else if (value == "DELETE")
                _methods |= DELETE;
            else
                throw std::invalid_argument("unknown method keyword");
            break;
        }
        case FMETHODS:
        {
            if (value == "POST")
                _methods &= ~POST;
            else if (value == "GET")
                _methods &= ~GET;
            else if (value == "DELETE")
                _methods &= ~DELETE;
            else
                throw std::invalid_argument("unknown method keyword");
            break;
        }
        case MATCH:
        {
            _match = value;
            _match_len = _match.length();
            break;
        }
        case LOCATION:
        {
            _replace = value;
            break;
        }
        case REDIR:
        {
            _cgi_pass = true;
            break;
        }
        case CGIPASS:
        {
            _cgi_pass = true;
            _cgi_list.push_back(std::make_pair(value, pop_value(lineinfo.second)));
        }
        case DELIM1:
        {
            break;
        }
        case DELIM2:
        {
            return ;
            break;
        }
        default:
        {
            throw std::invalid_argument("Unknow Route keyword");
        }
        }
        if (lines.empty())
            throw std::invalid_argument("Missing end braces");
        if (lineinfo.second.empty())
        {
            lineinfo = make_line(lines.front());
            lines.erase(lines.begin());
        }
    }
    if (isListing())
    {
        std::string filename = _replace + _listing;
        if (access(filename.c_str(), R_OK))
            throw std::invalid_argument("index file unreachable");
    }
        
    if (!_match.length() || _replace.empty())
        throw std::invalid_argument("missing Route Info");
}

Route::~Route() {}

//getters
bool Route::isAllowed(METHODS method) const {return (_methods & method);}
const std::string &Route::get_match(void) const {return (_match);}
const std::string &Route::get_replace(void) const {return (_replace);}
bool    Route::isListing(void) const {return (_listing.length() > 0);}
const std::string&  Route::getIndexfile(void) const {return (_listing);}
bool    Route::cgipass(void) const {return (_cgi_pass);}
std::size_t Route::get_len() const {return (_match_len);}
Vserv *Route::getServ() const {return _serv;}

//printers
void Route::printInfo()
{
    std::cout << "\tRoute match: " << BOLD << _match << RESET <<'\n';
    std::cout << "\t\tLocation: " << BOLD <<_replace << RESET << '\n';
    std::cout << "\t\tListing: " << BOLD << _listing << RESET << '\n';
    std::cout << "\t\tMethods GET: " << (_methods & GET) << " POST: " << (_methods & POST) << " DEL: " << (_methods & DELETE) << std::endl; 
    
    std::cout << "\t\tCgi Pass: " << BOLD << (_cgi_pass ? "ON" : "OFF") << RESET << std::endl;
    if (_cgi_pass)
    {
        for (std::vector<std::pair<std::string, std::string> >::iterator it = _cgi_list.begin(); it != _cgi_list.end(); ++it)
        {
            std::cout << "\t\t\tExtension: " << BOLD << it->first << RESET << "\tExecutable name: " << BOLD << it->second << RESET << '\n'; 
        }
        std::cout << std::endl;
    }
}

//member functions
bool Route::match(const std::string& to_match) const
{
    return to_match.compare(0, _match_len, _match) == 0;
}

std::string Route::matchCgi(const std::string &ext)
{
    std::string realext = ext;
    if (ext[0] != '.')
        realext = std::string(".") + ext;
    for (std::vector<std::pair<std::string, std::string> >::iterator it = _cgi_list.begin(); it != _cgi_list.end(); ++it)
    {
        if(it->first == realext)
            return (it->second);
    }
    return std::string("");
}
