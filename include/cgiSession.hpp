/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   cgiSession.hpp                                     :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:51 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:51 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef CGISESSION_HPP
# define CGISESSION_HPP

#include "includes.hpp"

#include <sys/wait.h>
#include <ctime>

//max cgi child life time in sec
#define MAX_CGI_LIFETIME    100
#define CGI_TIMEOUT         10

//a simple class to keep track if a child process timed out or took too long to finish
class cgiSession
{
    private:
        bool                _shouldDie;

        int                 _clientfd;
        int                 _childfd;
        int                 _pipein;
        int                 _pipeout;
        const std::time_t   _deathtime;
        std::time_t         _lastWrite;

        std::string         _toWrite;
        std::string         _received;

    public:
        cgiSession(int clientfd, int childfd, int pipein, int pipeout, const std::string& towrite, std::time_t currentTime = std::time(NULL));
        ~cgiSession();

        int getClientFd() const;
        int getChildFd() const;

        int getPipeIn() const;
        int getPipeOut() const;

        bool    writeTocgi();
        bool    receiveFromCgi();

        //check if the child timed out or took too long
        bool    isDead();
        //add more time until timeout
        void    update();

        void    shouldDie();

        std::string& getReceived();
};

#endif