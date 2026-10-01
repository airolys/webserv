/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Response.hpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:10 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:31:19 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef RESPONSE_HPP
#define RESPONSE_HPP

#include <cstdio> //for remove
#include "includes.hpp"
#include "Request.hpp"
#include "Vserv.hpp"

enum    cgiState_e
{
    NONE,
    WAITING,
    DONE
};

class Response
{
    private:
        int _client_fd;
        std::string _version;
        int _statusCode;
        std::string _statusMessage;
        std::map<std::string, std::string> _headers;
        std::string _body;
        ssize_t _sent;

        std::string _toSend;
        Server*     _serv;

        cgiState_e  cgiStatus;
    public:
        bool sending_response;

        Response(Server* serv, int clientfd);
        Response(int statusCode, Server* serv, int clientfd);
        ~Response();

        //setters
        void setVersion(const std::string& version);
        void setStatusCode(int statusCode);
        void setStatusMessage(const std::string& statusMessage);
        void addHeader(const std::string& key, const std::string& value);
        void setBody(const std::string& body);
        void setSent(ssize_t sent);

        //getters
        std::string getVersion() const;
        int getStatusCode() const;
        std::string getStatusMessage() const;
        std::string getHeader(const std::string& key) const;
        std::string getBody() const;
        bool isSendingResponse() const;
        ssize_t getSent() const;

        //member functions
        void generateResponse(Request& request, const std::vector<Vserv*>& vservs);
        void handleGET(const Request& request, Route* route, bool skipBody);
        void handlePOST(const Request& request, Route* route);
        void handleHEAD(const Request& request, Route* route);
        void handleDelete(const Request& request, Route* route);
        void handleError(int statusCode);
        std::string getContentType(const std::string& path);

        void setCgiEnv(const Request& request,Route *route);
        bool parseCGIOutput(const std::string& output);
        void handleCgi(const Request& request, Route *route);

        //helpers
        std::string toString();
        Vserv* findMatchingVserv(const std::vector<Vserv*>& vservs, const Request& request);
        Route* findMatchingRoute(Vserv* vserv, const Request& request);
        std::string getCurrentDate();
        bool isDirectory(const std::string& path);
        bool fileExists(const std::string& path);
        std::string readFile(const std::string& path);
        bool fillFile(const std::string& path, const std::string& body);
        bool    needCgi(const std::string &ext, Route* route);

        void    setToSend(std::string str);
        bool    removefrontToSend(std::size_t amount);

        cgiState_e  getCgiStatus() const;
        void        changeCgiStatus(cgiState_e state);
};

#endif
