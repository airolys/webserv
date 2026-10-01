/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Vserv.hpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:23 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:29:23 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef VSERV_HPP
#define VSERV_HPP

#include "includes.hpp"
#include "Route.hpp"

class Route;

class Vserv
{
    private:
        std::string         _host;
        unsigned int        _port;
        std::string         _server_name;
        std::list<Route*>   _routes;
        std::map<int, std::string> _error_pages;
        long unsigned int   _maxbodysize;

        int        _listenfd;

        bool    checkConfig();
    public:

        Vserv(void);
        Vserv(std::vector<std::string>& lines);
        Vserv(const Vserv& vs);
        Vserv& operator=(const Vserv& vs);
        ~Vserv();

        //setters
        void    setlisten(int fd);
        void    addRoute(Route* route);

        //getters
        int                         getlisten();
        const std::string&          get_name() const;
        const std::string&          get_host() const;
        unsigned int                get_port() const;
        const std::list<Route*>&    getRoutes() const;
        long unsigned int           getMaxBodySize() const;
        std::string getErrorPages(int code);

        //printers
        void  printAllRoutes(void);
        void  printInfo(void);

        //member functions
        bool    matchByName(const std::string& name) const;
};

std::string intToString(int value);

#endif
