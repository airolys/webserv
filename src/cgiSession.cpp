/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiSession.cpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:26:30 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:26:30 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */


#include "../include/cgiSession.hpp"

cgiSession::cgiSession(int clientfd, int childfd, int pipein, int pipeout, const std::string& towrite, std::time_t currentTime)
    :   _shouldDie(false), _clientfd(clientfd), _childfd(childfd), _pipein(pipein), _pipeout(pipeout), _deathtime(currentTime + MAX_CGI_LIFETIME), _lastWrite(currentTime), _toWrite(towrite)
{
    std::cout << GOOD_INFO << "new cgiSession created for client: " << BOLD << _clientfd << RESET;
    std::cout << " child fd: " << BOLD << _childfd << RESET << std::endl;
}

cgiSession::~cgiSession()
{
    std::cout << GOOD_INFO << "Death of cgiSession created for client: " << BOLD << _clientfd << RESET;
    std::cout << " child fd: " << BOLD << _childfd << RESET << std::endl;

    if (_pipein != -1)
    {
        close (_pipein);
    }
    if (_pipeout != -1)
    {
        close (_pipeout);
    }
}

int cgiSession::getClientFd() const
{
    return (_clientfd);
}

int cgiSession::getChildFd() const
{
    return (_childfd);
}

int cgiSession::getPipeIn() const
{
    return (_pipein);
}

int cgiSession::getPipeOut() const
{
    return (_pipeout);
}

bool    cgiSession::isDead()
{
    if (_shouldDie)
        return (true);
    std::time_t now = std::time(NULL);
    if (now >= _deathtime)
    {
        std::cout << INFO_ALERT << "CgiSession (" << BOLD << _childfd << RESET <<  ") exeeded allowed time to complete!" << std::endl;
        _shouldDie = true;
        return (true);
    }
    if (now - _lastWrite >= CGI_TIMEOUT)
    {
        std::cout << INFO_ALERT << "CgiSession (" << BOLD << _childfd << RESET <<  ") didn't progress in the allowed time!" << std::endl;
        _shouldDie = true;
        return (true);
    }
    return (false);
}

void    cgiSession::update()
{
    std::time(&_lastWrite);
}

//return true if nothing more to send
bool    cgiSession::writeTocgi()
{
    std::size_t n = write(_pipein, _toWrite.c_str(), _toWrite.length()<WRITE_SIZE? _toWrite.length() : WRITE_SIZE);
    if (n < _toWrite.length())
    {
        _toWrite.erase(0, n);
        return (false);
    }
    _toWrite.erase(0, n);
    close(_pipein);
    _pipein = -1;
    return (true);
}

//return true if all was received
bool    cgiSession::receiveFromCgi()
{
    char    buffer[READ_SIZE + 1];
    
    int n = read(_pipeout, buffer, READ_SIZE);
    if (n == -1)
    {
        std::cerr << ERROR_WARN << "read from pipe error!" << std::endl; 
    }
    if (n == READ_SIZE)
    {
        _received.append(buffer, n);
        return (false);
    }
    _received.append(buffer, n);
    close(_pipeout);
    _pipeout = -1;
    return (true);
}

void    cgiSession::shouldDie()
{
    _shouldDie = true;
}

std::string& cgiSession::getReceived()
{
    return(_received);
}
