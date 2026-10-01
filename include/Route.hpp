/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Route.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:14 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:29:14 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef ROUTE_HPP
#define ROUTE_HPP

#include "includes.hpp"

class Vserv;

class Route
{
private:
    unsigned int    _methods;
    std::string     _match;
    std::size_t     _match_len;
    std::string     _replace;
    Vserv *         _serv;
    std::string     _listing;
    std::string     _root;
    bool            _cgi_pass;

    std::vector<std::pair<std::string, std::string> > _cgi_list;

public:
    Route(Vserv &serv, std::vector<std::string>& lines);
    ~Route();

    //getters
    bool                isAllowed(METHODS method) const;
    const std::string&  get_match(void) const;
    const std::string&  get_replace(void) const;
    const std::string&  getIndexfile(void) const;
    bool                isListing(void) const;
    bool                cgipass(void) const;
    std::size_t         get_len() const;
    Vserv*              getServ() const;

    //printers
    void    printInfo();
    //member functions
    bool match(const std::string& to_match) const;
    std::string matchCgi(const std::string& ext);

};

#endif