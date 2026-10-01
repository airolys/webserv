/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   Request.hpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:29:05 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:29:05 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef REQUEST_HPP
#define REQUEST_HPP

#include "includes.hpp"

class Request
{
    private:
        std::string _method;
        std::string _path;
        std::string _version;
        std::string _query;

        std::map<std::string, std::string>  _headers;
        std::string _body;

        bool    _fullRequest;
        bool    _fullHeader;
        bool    _fullBody;
        bool    _fullEncoding;

        std::string _buffer;
        bool        _isChunked;
        bool        _inChunk;
        std::size_t _chunkSize;
        std::size_t _fullBodyLength;

    public:
        Request();
        ~Request();

        std::string getMethod() const;
        const std::string& getPath() const;
        std::string getVersion() const;
        std::string getHeader(const std::string& key) const;
        std::string getBody() const;
        std::size_t getBodyLength() const;
        std::string getQuery() const;
        bool        isFullBody() const;

        //printers
        void    print();

        //member functions
        void    parse(const std::vector<char>& request, const std::vector<Vserv*>& vservs);
        void    parseMultipartFormData(void);
        void    check_full_request(std::string &line);
        void    check_full_header(std::string &line);
        void    check_chunked();
        bool    parse_request(const std::string& line);
        bool    parse_header(const std::string& line);
        void    parse_chunked_body();
        void    parse_body();

        //helpers
        bool    getNewLine(std::string &newline);
        bool    getChunk(std::string& line);
        Vserv *findMatchingVserv(const std::vector<Vserv*>& vservs, const Request& request);
};

#endif