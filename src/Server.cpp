/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Server.cpp                                         :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:37 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:37 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Server.hpp"

Server::Server() {}

Server::Server(const std::string configfile)
{
    _epollfd = 0;

    std::cout << GOOD_INFO << "Starting conf parsing (" << BOLD << configfile << RESET << ')' << std::endl;
    std::string file = configfile;
    std::ifstream ifs(file.c_str(), std::ios::in);
    std::vector<std::string> lines;

    if (!ifs.is_open())
    {
        std::cerr << ERROR_WARN << "Error opening/reading conf file (" << BOLD << configfile << RESET << ')' << std::endl;
        return ;
    }
    lines = get_lines(ifs);
    ifs.close();
    while (lines.size())
    {
        Vserv* newstuff = NULL;
        try{
            newstuff = new Vserv(lines);
            if (!newstuff)
                throw std::string("Error in the config file");
            this->addVserv( newstuff);
        } catch (std::exception& e)
        {
            delete newstuff;
            throw std::invalid_argument(e.what());
        }
    }
    printAllServer();
}

Server::~Server()
{
    std::cout << GOOD_INFO << "Cleaning up server" << std::endl;
    close(_epollfd);
    for (std::map<int, Request*>::iterator it = _requests.begin(); it != _requests.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    for (std::map<int, Vserv*>::iterator it = _listenfds.begin(); it != _listenfds.end(); ++it)
    {
        close(it->first);
        delete it->second;
    }
    for (std::map<int, Response*>::iterator it = _responses.begin(); it != _responses.end(); ++it)
    {
        delete it->second;
    }
    for (std::vector<Vserv*>::iterator it = _vservs.begin(); it != _vservs.end(); ++it)
    {
        delete *it;
    }
    _vservs.clear();
    _requests.clear();
    _listenfds.clear();
    _responses.clear();
}

//setters
void Server::addVserv(Vserv* vs) {_vservs.push_back(vs);}
void Server::setEpollfd(int fd) {_epollfd = fd;}
void Server::newRequest(int fd) {_requests[fd] = new Request(); }
void Server::newResponse(int fd) {_responses[fd] = new Response(this, fd); }
void Server::newError(int fd, int statusCode) {_responses[fd] = new Response(statusCode, this, fd); }
void Server::newCgiSession(int clientfd, int childfd, int pipeIn, int pipeOut, const std::string& body)
{
    cgiSession* curr = new cgiSession(clientfd, childfd, pipeIn, pipeOut, body);
    _cgisessions[clientfd] = curr;
    _cgi_sessions_in[pipeIn] = curr;
    _cgi_sessions_out[pipeOut] = curr;

    struct epoll_event ev;
    setNonBlocking(pipeIn);
    ev.events =  EPOLLOUT;
    ev.data.fd = pipeIn;
    if(epoll_ctl(getEpollfd(), EPOLL_CTL_ADD, pipeIn, &ev) == -1)
        throw std::runtime_error("epoll_ctl error");

    setNonBlocking(pipeOut);
    ev.events =  EPOLLIN;
    ev.data.fd = pipeOut;
    if(epoll_ctl(getEpollfd(), EPOLL_CTL_ADD, pipeOut, &ev) == -1)
        throw std::runtime_error("epoll_ctl error");

}

//getters
std::vector<Vserv*>& Server::getVservs() {return _vservs;}

Vserv *Server::getVserv(std::size_t at)
{
    if (at >= _vservs.size())
        return NULL;
    return(_vservs[at]);
}

std::size_t Server::servAmount(void) {return (_vservs.size());}
int Server::getEpollfd(void) {return (_epollfd);}
Request *Server::getRequest(int fd)
{
    std::map<int, Request*>::iterator tmp = _requests.find(fd);
    if (tmp == _requests.end())
        return NULL;
    return (tmp->second);
}
Response *Server::getResponse(int fd)
{
    std::map<int, Response*>::iterator tmp = _responses.find(fd);
    if (tmp == _responses.end())
        return NULL;
    return (tmp->second);

}

//printers
void Server::printAllServer()
{
    std::vector<Vserv*>::iterator it = _vservs.begin();
    for (size_t i = 0; it != _vservs.end(); ++it, ++i)
       (*it)->printInfo();
}

void    Server::deleteClient(int fd)
{
    delete getRequest(fd);
    _requests.erase(fd);
    delete getResponse(fd);
    _responses.erase(fd);
    _listenfds.erase(fd);
    if (_cgisessions.find(fd) != _cgisessions.end())
    {
        cgiSession  *cgi = _cgisessions.find(fd)->second;
        std::cout << INFO_ALERT << "Cgi session still active and to cull on client (" << BOLD << fd << RESET << ") deletion" << std::endl;
        _cgi_sessions_in.erase(cgi->getPipeIn());
        _cgi_sessions_out.erase(cgi->getPipeOut());
        cgi->shouldDie();
    }
}

//member functions set up the server
void Server::setup()
{
    int i = 0;
    int status = 0;
    Vserv*  curr = getVserv(i);
    while (curr && status != -1)
    {
        create_listening_sock(*curr);
        add_sock_to_epoll(*curr);
        curr = getVserv(++i);
    }
    std::cout << GOOD_INFO << "All servers listening" << std::endl;
}

void Server::add_sock_to_epoll(Vserv& vs)
{
    if (_epollfd == 0)
    {
        _epollfd = epoll_create1(0);
        if (_epollfd == -1)
           throw std::runtime_error("epoll_create error");
        std::cout << GOOD_INFO << "epoll fd: " << BOLD << _epollfd << RESET << " created" << std::endl;
    }
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = vs.getlisten();
    if (epoll_ctl(_epollfd, EPOLL_CTL_ADD, vs.getlisten(), &ev) == -1)
        throw std::runtime_error("epoll_ctl error");
    std::cout << GOOD_INFO << "Server: " << BOLD << vs.get_name() << RESET << " listening on: " << BOLD << vs.get_host() <<RESET<<":"<<BOLD<< vs.get_port() << RESET << " (fd:" << BOLD << vs.getlisten() << RESET << ')'<< std::endl;
}

void Server::create_listening_sock(Vserv& vs)
{
    int listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (listenfd < 0)
        throw std::runtime_error("socket error");
    vs.setlisten(listenfd);
    setNonBlocking(listenfd);
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(vs.get_port());
    if (inet_pton(AF_INET, vs.get_host().c_str(), &server_addr.sin_addr) <= 0)
        throw std::runtime_error("Invalid address/Address not supported");
    int opt = 1;
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("Error setting socket options");
    if (bind(listenfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        throw std::runtime_error("Bind fail/invalid address used");
    }
    if (listen(listenfd, 1000) == -1)
        throw std::runtime_error("listen error");
    std::cout << GOOD_INFO << "nonblocking socket: " << BOLD << listenfd << RESET << " ready" << std::endl;
}

void    Server::setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
        throw std::runtime_error("fcntl error");
    flags |= O_NONBLOCK;
    if (fcntl(fd, F_SETFL, flags) == -1)
        throw std::runtime_error("fcntl error");
}

//member functions run the server

void Server::run()
{
    struct epoll_event  events[NB_EPOLL_EVENTS];
    std::cout << GOOD_INFO << "webserv is running, epoll fd: " << BOLD << getEpollfd()  << RESET << std::endl;
    while (true)
    {
        int event_amount = epoll_wait(getEpollfd(), events, NB_EPOLL_EVENTS, TIMEOUT_EPOLL_WAIT);
        if (event_amount == -1)
            throw std::runtime_error("Epoll wait error");
        checkAndReapCgi();
        for (int i=0; i<event_amount; i++)
        {
            if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                handleSocketClose(events[i].data.fd);
                continue;
            }
            handleEvent(events[i]);
        }
    }
}

void    Server::handleEvent(epoll_event event)
{
    for (std::size_t i=0; i<servAmount(); i++)
    {
        if (getVserv(i)->getlisten() == event.data.fd)
        {
            std::cout << GOOD_INFO << "new client on: " << BOLD << getVserv(i)->get_name() << RESET << std::endl;
            handleNewConnection(event.data.fd);
            return ;
        }
    }
    if (event.events & EPOLLIN)
    {
        if (isCgiOut(event.data.fd))
        {
            cgiSession* curr = getCgiSessionOut(event.data.fd);
            curr->update();
            if (curr && curr->receiveFromCgi())
            {
                _cgi_sessions_out.erase(event.data.fd);
                _responses[curr->getClientFd()]->parseCGIOutput(curr->getReceived());
                curr->shouldDie();
            }
        }
        else
            handleIN(event.data.fd);
    }
    else if (event.events & EPOLLOUT)
    {
        if (isCgiIn(event.data.fd))
        {
            cgiSession* curr = getCgiSessionIn(event.data.fd);
            if (curr && curr->writeTocgi())
                _cgi_sessions_in.erase(event.data.fd);
        }
        else if (getResponse(event.data.fd) != NULL && getResponse(event.data.fd)->sending_response)
        {
            Response*   curr = getResponse(event.data.fd);
            if (curr->getCgiStatus() == NONE || curr->getCgiStatus() == DONE)
                handleOUT(event.data.fd);
        }
    }
}

void Server::handleSocketClose(int socket_fd)
{
    std::cout << GOOD_INFO << "Deleting client:" << BOLD << socket_fd << RESET << std::endl;
    if(epoll_ctl(getEpollfd(), EPOLL_CTL_DEL, socket_fd, NULL) == -1)
        throw std::runtime_error("epoll_ctl error");
    deleteClient(socket_fd);
    close(socket_fd);
}

void Server::handleNewConnection(int fd)
{
    struct epoll_event ev;
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);
    int client_sock = accept(fd, (struct sockaddr *)&client_addr, &client_len);
    
    if (client_sock == -1)
        throw std::runtime_error("Error accept new client");
    setNonBlocking(client_sock);
    ev.events = EPOLLIN | EPOLLOUT;
    ev.data.fd = client_sock;
    if(epoll_ctl(getEpollfd(), EPOLL_CTL_ADD, client_sock, &ev) == -1)
        throw std::runtime_error("epoll_ctl error");
    std::cout << GOOD_INFO << "New client " << BOLD << client_sock << RESET << " connected" << std::endl;
    newRequest(client_sock);
}

void Server::handleIN(int client_sock)
{
    char buffer[READ_SIZE + 1];
    int n = recv(client_sock, buffer, READ_SIZE, 0);
    if (n == -1)
    {
        std::cerr << "Error receiving" << std::endl;
        handleSocketClose(client_sock);
        return;
    }
    else if (n == 0)
    {
        std::cout << GOOD_INFO << "Client " << BOLD << client_sock << RESET << " disconnected!" << std::endl;
        handleSocketClose(client_sock);
        delete getRequest(client_sock);
        return;
    }
    else
    {
        std::vector<char> data(buffer, buffer + n);
        try
        {   
            if (!getResponse(client_sock))
            {
                getRequest(client_sock)->parse(data, getVservs());
                if (getRequest(client_sock)->isFullBody())
                    newResponse(client_sock);
            }
        }
        catch (const std::exception& e)
        {
            std::string errorMsg = e.what();
            char* end;
            int code = std::strtoul(errorMsg.c_str(), &end, 10);
            if (*end != '\0' || end == errorMsg.c_str())
                code = 500;
            newError(client_sock, code);
        }
    }
}

void Server::handleOUT(int client_sock)
{
    Response    *current = getResponse(client_sock);
    
    if (current->getCgiStatus() != DONE)
    {

        if (current->getStatusCode() == 200 && !current->getSent())
        {
            std::cout << GOOD_INFO << "Sending response..." << std::endl;
            current->generateResponse(*getRequest(client_sock), getVservs());
        }
    }
    if (current->getCgiStatus() == WAITING)
        return ;
    std::string responseStr = current->toString();
    ssize_t sent = send(client_sock, responseStr.c_str(), responseStr.size()>WRITE_SIZE? WRITE_SIZE : responseStr.size(), 0);
    if (sent == -1)
    {
        std::cerr << ERROR_WARN << "Error sending response" << std::endl;
        handleSocketClose(client_sock);
        return ;
    }
    current->sending_response = current->removefrontToSend(sent);
    current->setSent(current->getSent() + sent);
    if (current->sending_response == false && current->getHeader("Connection") == "close")
        handleSocketClose(client_sock);
    else if (current->sending_response == false)
    {
        std::cout << GOOD_INFO << "Response fully sent" << std::endl;
        handleSocketClose(client_sock);
    }
}

void    Server::checkAndReapCgi()
{
    cgiSession* current = NULL;
    for (std::map<int, cgiSession*>::iterator it=_cgisessions.begin(); it!=_cgisessions.end();)
    {
        current = it->second;
        if (current && current->isDead())
        {
            kill(current->getChildFd(), SIGTERM);
            std::cout << INFO_ALERT << "Reaping cgiSession (" << BOLD << current->getChildFd() << RESET << ") created for client " << BOLD << current->getClientFd() << RESET << std::endl;
            int socket = current->getPipeIn();
            if (socket != -1)
            {
                close (socket);
                _cgi_sessions_in.erase(socket);
            }
            socket = current->getPipeOut();
            if (socket != -1)
            {
                close (socket);
                _cgi_sessions_out.erase(socket);
            }
            if (current->getChildFd() == waitpid(current->getChildFd(), NULL, WNOHANG))
            {
                _cgisessions.erase(it++);
                Response    *curr = _responses[current->getClientFd()];
                if (curr)
                    curr->changeCgiStatus(DONE);
                delete current;
            }
            else
            {
                std::cout << INFO_ALERT << "CGI Session (" << BOLD << current->getChildFd() << RESET << ") not yet dead !" << std::endl;
                ++it;
            }    
        }
        else
            ++it;
    }
}

bool    Server::isCgiIn(int fd) const
{
    std::map<int, cgiSession*>::const_iterator curr = _cgi_sessions_in.find(fd);
    if (curr != _cgi_sessions_in.end())
        return (true);
    return (false);
}

bool    Server::isCgiOut(int fd) const
{
    std::map<int, cgiSession*>::const_iterator curr = _cgi_sessions_out.find(fd);
    if (curr != _cgi_sessions_out.end())
        return (true);
    return (false);
}

cgiSession* Server::getCgiSessionIn(int fd)
{
    std::map<int, cgiSession*>::iterator curr = _cgi_sessions_in.find(fd);
    if (curr != _cgi_sessions_in.end())
        return (curr->second);
    return (NULL);
}

cgiSession* Server::getCgiSessionOut(int fd)
{
    std::map<int, cgiSession*>::iterator curr = _cgi_sessions_out.find(fd);
    if (curr != _cgi_sessions_out.end())
        return (curr->second);
    return (NULL);
}