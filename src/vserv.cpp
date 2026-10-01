/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   vserv.cpp                                          :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:43 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:43 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Vserv.hpp"

std::string intToString(int value);

Vserv::Vserv() {}

Vserv::Vserv(std::vector<std::string>& lines)
    :   _host(DEFAULT_HOST), _port(DEFAULT_PORT), _server_name(DEFAULT_NAME), _maxbodysize(DEFAULT_MAX_BODY), _listenfd(-1)
{
    std::cout << GOOD_INFO << "filling server..." << std::endl;
    std::pair<t_keywords, std::vector<std::string> > lineinfo = make_line(lines.front());
    lines.erase(lines.begin());
    while (lineinfo.first != DELIM2)
    {
        std::string value = pop_value(lineinfo.second);
        switch (lineinfo.first)
        {
        case SERV:
            break;
        case NAME:
            _server_name = value;
            break;
        case PORT:
        {
            char* endptr;
            unsigned long result = std::strtoul(value.c_str(), &endptr, 10);
    
            if (*endptr != '\0')
            {
               throw std::invalid_argument("Invalid Port value");
            }
            _port = result;
            break;
        }
        case HOST:
            if (_host != DEFAULT_HOST)
                throw std::invalid_argument("Multiple Host");
            _host = value;
            break;
        case CMBS:
            _maxbodysize = std::strtoul((value).c_str(), NULL, 10);
            break;
        case ROUTE:
        {

            Route *newroute = new Route(*this, lines);
            this->addRoute(newroute);
            break;
        }
        case ERROR_PAGE:
        {
            int error_code = std::atoi(value.c_str());
            std::string path = pop_value(lineinfo.second);
            this->_error_pages[error_code] = path;
            break;
        }
        case DELIM1:
            break;
        case DELIM2:
            return;
        default:
            throw std::invalid_argument("Unknow Server keyword");
        }
        if (lines.empty())
        {
            throw std::invalid_argument("Missing end braces");
        }
        if (lineinfo.second.empty())
        {
            lineinfo = make_line(lines.front());
            lines.erase(lines.begin());
        }
    }
    checkConfig();
}

Vserv::Vserv(const Vserv& vs) {*this = vs;}

Vserv &Vserv::operator=(const Vserv &vs)
{
    if (&vs == this)
        return (*this);
    _host = vs.get_host();
    _port = vs.get_port();
    return (*this);
}

Vserv::~Vserv()
{
    for (std::list<Route*>::iterator it = _routes.begin(); it != _routes.end(); ++it)
    {
        delete *it;
    }
    if (_listenfd != -1)
        close(_listenfd);
    _routes.clear();
}


//setters
void    Vserv::addRoute(Route* route) {_routes.push_front(route);}
void Vserv::setlisten(int fd) {_listenfd = fd;}

//getters
int Vserv::getlisten() {return (_listenfd);}
const std::string &Vserv::get_name() const {return (_server_name);}
const std::string &Vserv::get_host() const {return (_host);}
unsigned int Vserv::get_port() const {return (_port);}
const std::list<Route*>& Vserv::getRoutes() const {return _routes;}
long unsigned int Vserv::getMaxBodySize() const {return _maxbodysize;}
std::string Vserv::getErrorPages(int code) {return _error_pages[code];}
//printers
void Vserv::printAllRoutes(void)
{
    std::list<Route*>::iterator it = _routes.begin();
    for (size_t i = 0; it != _routes.end(); ++it, ++i)
    {
        std::cout << (*it)->get_match() << std::endl;
        std::cout << (*it)->get_replace() << std::endl;
        std::cout << (*it)->isListing() << std::endl;
        std::cout << (*it)->cgipass() << std::endl;
    }
    return ;
}

void Vserv::printInfo(void)
{
    std::cout << GOOD_INFO << "ready to start" << '\n';
    std::cout << "\tServer: " << BOLD << _server_name << RESET << '\n';
    std::cout << "\tHost: " << BOLD << _host << RESET << '\n';
    std::cout << "\tPort: " << BOLD << _port << RESET << '\n';
    std::cout << "\tClient max body size: " << BOLD << _maxbodysize << RESET << std::endl;
    std::list<Route*>::iterator it = _routes.begin();
    for (size_t i = 0; it != _routes.end(); ++it, ++i)
        (*it)->printInfo();

}

//member functions
bool Vserv::matchByName(const std::string &name) const 
{
    std::string hostPort = _host + ":" + intToString(_port);
    return (name == hostPort);
}

bool Vserv::checkConfig()
{
    if (_host.empty())
        throw std::invalid_argument("Empty Host");
    if (_port == 0)
        throw std::invalid_argument("Port error");
    if (_routes.empty())
        throw std::invalid_argument("No routes provided");
    return (true);
}