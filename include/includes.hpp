/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   includes.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:57 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:31:22 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef INCLUDES_HPP
#define INCLUDES_HPP 

#include <string>
#include <vector>
#include <list>
#include <map>
#include <sstream>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <cstdlib>
#include <cstring>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <signal.h>

// Forward declarations
class Vserv;
class Route;
class Request;
class Response;
class Server;
class cgiSession;


typedef enum keywords
{
    SERV = 0,
    NAME = 1,
    PORT = 2,
    HOST = 3,
    CMBS = 4,
    ROUTE = 5,
    LISTING = 6,
    AMETHODS = 7,
    FMETHODS = 8,
    MATCH = 9,
    LOCATION = 10,
    REDIR = 11,
    CGIPASS = 12,
    DELIM1 = 13,
    DELIM2 = 14,
    ERROR_PAGE = 15,
    ERROR = 16
}   t_keywords;

// Define color codes for terminal output
#define RESET   "\033[0m"
#define BOLD    "\033[1m"
#define UNDERLINE "\033[4m"

// Text colors
#define BLACK   "\033[30m"
#define RED     "\033[31m"
#define GREEN   "\033[32m"
#define YELLOW  "\033[33m"
#define BLUE    "\033[34m"
#define MAGENTA "\033[35m"
#define CYAN    "\033[36m"
#define WHITE   "\033[37m"

// Background colors
#define BG_BLACK   "\033[40m"
#define BG_RED     "\033[41m"
#define BG_GREEN   "\033[42m"
#define BG_YELLOW  "\033[43m"
#define BG_BLUE    "\033[44m"
#define BG_MAGENTA "\033[45m"
#define BG_CYAN    "\033[46m"
#define BG_WHITE   "\033[47m"

// High-intensity colors
#define BLACK_HI   "\033[90m"
#define RED_HI     "\033[91m"
#define GREEN_HI   "\033[92m"
#define YELLOW_HI  "\033[93m"
#define BLUE_HI    "\033[94m"
#define MAGENTA_HI "\033[95m"
#define CYAN_HI    "\033[96m"
#define WHITE_HI   "\033[97m"

// Background high-intensity colors
#define BG_BLACK_HI   "\033[100m"
#define BG_RED_HI     "\033[101m"
#define BG_GREEN_HI   "\033[102m"
#define BG_YELLOW_HI  "\033[103m"
#define BG_BLUE_HI    "\033[104m"
#define BG_MAGENTA_HI "\033[105m"
#define BG_CYAN_HI    "\033[106m"
#define BG_WHITE_HI   "\033[107m"

#define ERROR_WARN  BOLD << '[' << RED << "WARN" << RESET << BOLD << "] " << RESET

#define INFO_ALERT  BOLD << '[' << YELLOW << "INFO" << RESET << BOLD << "] " << RESET

#define GOOD_INFO   BOLD << '[' << GREEN << "INFO" << RESET << BOLD << "] " << RESET

typedef enum METHODS
{
    POST =  1 << 1,
    GET =  1 << 2,
    DELETE = 1 << 3
} methods_t;

#define READ_SIZE 1024
#define WRITE_SIZE 512
#define NB_EPOLL_EVENTS 1000
#define TIMEOUT_EPOLL_WAIT -1

#define MAX_REQUEST_SIZE 126
#define MAX_HEADER_SIZE 2048

#define DEFAULT_NAME "Webserv/1.1"
#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 1234
#define DEFAULT_MAX_BODY 1000000

#include "Server.hpp"
#include "parse.hpp"
#include "Request.hpp"
#include "Response.hpp"
#include "Route.hpp"
#include "Vserv.hpp"


#endif
