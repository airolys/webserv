/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.hpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:19 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:29:19 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef SERVER_HPP
#define SERVER_HPP

#include "includes.hpp"
#include "Vserv.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "cgiSession.hpp"

class Server
{
    private:
        std::vector<Vserv*>         _vservs;
        int                         _epollfd;
        std::map<int, Vserv*>       _listenfds;
        std::map<int, Request*>     _requests;
        std::map<int, Response*>    _responses;
        std::map<int, cgiSession*>  _cgisessions;
        std::map<int, cgiSession*>  _cgi_sessions_in;
        std::map<int, cgiSession*>  _cgi_sessions_out;

        Server(void);
    public:

        Server(const std::string configfile);
        ~Server(void);

        //setters
        void    addVserv(Vserv* vs);
        void    setEpollfd(int fd);
        void    newRequest(int fd);
        void    newResponse(int fd);
        void    newError(int fd, int statusCode);
        void    newCgiSession(int clientfd, int childfd, int pipeIn, int pipeOut, const std::string& body);

        //getters
        int                     getEpollfd(void);
        Vserv*                  getVserv(std::size_t at);
        Response*               getResponse(int fd);
        Request*                getRequest(int fd);
        std::size_t             servAmount(void);
        std::vector<Vserv*>&    getVservs();
        
        //printers
        void    printAllServer();

        //cleanup
        void    deleteClient(int fd);

        //member functions set up the server
        void setup(void);
        void add_sock_to_epoll(Vserv& vs);
        void create_listening_sock(Vserv& vs);
        void setNonBlocking(int fd);

        //member functions run the server
        void run(void);
        void handleEvent(epoll_event event);
        void handleIN(int client_sock);
        void handleOUT(int client_sock);
        void handleNewConnection(int fd);
        void handleSocketClose(int socket_fd);
        void checkAndReapCgi();

        bool isCgiIn(int fd) const;
        bool isCgiOut(int fd) const;

        cgiSession* getCgiSessionIn(int fd);
        cgiSession* getCgiSessionOut(int fd);
};

#endif