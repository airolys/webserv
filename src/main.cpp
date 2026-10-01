/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   main.cpp                                           :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:26:30 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:31:03 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/includes.hpp"

Server *webserv = NULL;

void sig_handler(int signum)
{
    (void)signum;
    if (webserv)
        delete webserv;
    exit(0);
}

int main(int argc, char *argv[])
{
    if (argc > 2)
    {
        std::cerr << ERROR_WARN << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return (EXIT_FAILURE);
    }
    std::string configFile = argc == 2 ? argv[1] : "default.conf";
    signal(SIGINT, sig_handler);
    try{
            webserv = new Server(configFile);
            webserv->setup();
            webserv->run();
        }
        catch (std::exception &e)
        {
                std::cerr << ERROR_WARN << e.what() << std::endl;
                delete webserv;
                return (EXIT_FAILURE);
        }
    delete webserv;
    return (EXIT_SUCCESS);
}

