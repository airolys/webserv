/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   request.cpp                                        :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*   By: acharra <acharra@student.42.fr>          +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:16 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:28:16 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Request.hpp"

Request::Request()
    : _body(""), _fullRequest(false), _fullHeader(false), _fullBody(false), _fullEncoding(false), _isChunked(false), _inChunk(0), _fullBodyLength(0)
{
}

Request::~Request() {}

// Getters
std::string Request::getMethod() const { return _method; }
const std::string& Request::getPath() const { return _path; }
std::string Request::getVersion() const { return _version; }
std::string Request::getBody() const { return _body; }
std::string Request::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end()) {
        return it->second;
    }
    return "";
}

std::string Request::getQuery() const
{
    return _query;
}

std::size_t Request::getBodyLength() const
{
    return (_fullBodyLength);
}

bool Request::isFullBody() const { return _fullBody; }

// Member functions
void Request::parse(const std::vector<char>& request, const std::vector<Vserv*>& vservs)
{
    _buffer.append(request.begin(), request.end());
    std::string line;
    try
    {
        check_full_request(line);
        check_full_header(line);
        std::string contentLength = getHeader("Content-Length");
        if (!contentLength.empty())
        {
            Vserv* vserv = findMatchingVserv(vservs, *this);
            if (!vserv)
                throw std::runtime_error("400");
            size_t maxBodysize = vserv->getMaxBodySize();
            char* end;
            size_t requestSize = std::strtoul(contentLength.c_str(), &end, 10);
            if (*end != '\0' || end == contentLength.c_str())
            {
                throw std::runtime_error("400");
            }
            if (requestSize > maxBodysize)
            {
                throw std::runtime_error("413");
            }
        }
        check_chunked();
        if (!_fullBody && _fullEncoding)
        {
            if (_isChunked)
                parse_chunked_body();
            else
                parse_body();
        }
        if (_fullBody)
        {
            if (getHeader("Content-Type").find("multipart/form-data") != std::string::npos)
                parseMultipartFormData();
        }
    }
    catch (const std::exception& e)
    {
        _fullBody = true;
        throw;
    }
}
void Request::parseMultipartFormData()
{
    std::string contentType = getHeader("Content-Type");
    size_t boundaryPos = contentType.find("boundary=");
    if (boundaryPos == std::string::npos)
        throw std::runtime_error("400");

    std::string boundary = contentType.substr(boundaryPos + 9);
    std::string delimiter = "--" + boundary;

    size_t start = _body.find(delimiter);
    if (start == std::string::npos)
        throw std::runtime_error("400");
    
    start += delimiter.length();
    start += 2;
    size_t headersEnd = _body.find("\r\n\r\n", start);
    if (headersEnd == std::string::npos)
        throw std::runtime_error("400");
    start = headersEnd + 4;
    size_t end = _body.find(delimiter + "--", start);
    if (end == std::string::npos)
        throw std::runtime_error("400");

    _body = _body.substr(start, end - start - 2);
}

void Request::check_full_request(std::string &line)
{
    if (!_fullRequest)
    {
        if (!getNewLine(line))
            return;
        if (!parse_request(line))
            throw std::runtime_error("400");
        _fullRequest = true;
    }
}

void Request::check_full_header(std::string &line)
{
    if (!_fullHeader && _fullRequest)
    {
        while (getNewLine(line) && !_fullHeader)
        {
            if (line.empty())
            {
                _fullHeader = true;
                return ; 
            }
            if (!parse_header(line))
                throw std::runtime_error("400");
        }
    }
}

void Request::check_chunked()
{
    if (!_fullEncoding && _fullHeader)
    {
        std::map<std::string, std::string>::iterator temp = _headers.find("Content-Length");
        if (temp != _headers.end())
        {
            std::stringstream(temp->second) >> _fullBodyLength;
            _fullEncoding = true;
        }
        else
        {
            temp = _headers.find("Transfer-Encoding");
            if (temp == _headers.end() && _method != "POST")
            {
                _fullBody = true;
            }
            _isChunked = true;
            _fullEncoding = true;
        }
    }
}

bool Request::parse_request(const std::string& line)
{
    std::istringstream request_line(line);
    request_line >> _method >> _path >> _version;
    std::size_t queryStart = _path.find("?");
    if (queryStart != std::string::npos)
    {
        _query = _path.substr(queryStart+1);
        _path.resize(queryStart);
    }
    if (_method.empty() || _path.empty() || _version.empty())
        throw std::runtime_error("400");
    if (_version != "HTTP/1.1")
        throw std::runtime_error("505");
    if  (_method != "GET" && _method != "POST" && _method != "DELETE")
        throw std::runtime_error("405");
    return true;
}

bool Request::parse_header(const std::string& line)
{
    size_t colon_pos = line.find(':');
    if (colon_pos == std::string::npos || colon_pos == 0 || colon_pos == line.size() - 1)
        return false;
    std::string header_name = line.substr(0, colon_pos);
    std::string header_value = line.substr(colon_pos + 1);
    header_name.erase(0, header_name.find_first_not_of(" \t"));
    header_name.erase(header_name.find_last_not_of(" \t") + 1);
    header_value.erase(0, header_value.find_first_not_of(" \t"));
    header_value.erase(header_value.find_last_not_of(" \t") + 1);
    if (header_name.length() < 1 || header_value.length() < 1)
        throw std::runtime_error("400");
    _headers[header_name] = header_value;
    return true;
}

void Request::parse_chunked_body()
{
    std::string line;
    while (!_fullBody)
    {
        if (!_inChunk)
        {
            char* numEnd;
            if (!getNewLine(line))
                break;
            _chunkSize = std::strtoul(line.c_str(), &numEnd, 16);
            if (strcmp(numEnd, ""))
                throw std::runtime_error("413");
            if (_chunkSize == 0)
            {
                _fullBody = true;
                break;
            }
            _inChunk = true;
        }
        if (_inChunk)
        {
            if (!getChunk(line))
                break;
            _body.append(line);
            _fullBodyLength += _chunkSize;
            _inChunk = false;
        }
    }
}

void Request::parse_body()
{
    if (_buffer.size() >= _fullBodyLength)
    {
        _body.append(_buffer.substr(0, _fullBodyLength));
        _buffer.erase(0, _fullBodyLength);
        _fullBody = true;
    }
}

// Printers
void Request::print()
{
    std::cout << GOOD_INFO << "Method: " << BOLD<< _method << RESET << " Path: "<< BOLD << _path << RESET << " Version: " << BOLD << _version << RESET << std::endl;
}

// Helpers
bool Request::getNewLine(std::string& newline)
{
    newline.clear();
    if (_buffer.size() >= MAX_HEADER_SIZE)
        throw std::runtime_error("400");
    std::size_t lineEnd = _buffer.find("\r\n");
    if (lineEnd == std::string::npos)
        return false;
    newline.assign(_buffer.data(), lineEnd);
    _buffer.erase(0, lineEnd + 2);
    return true;
}

bool Request::getChunk(std::string& line)
{
    line.clear();
    std::size_t chunkEnd = _buffer.find("\r\n");
    if (chunkEnd == std::string::npos || chunkEnd != _chunkSize)
        return false;
    line = _buffer.substr(0, chunkEnd);
    _buffer.erase(0, chunkEnd + 2);
    return true;
}

Vserv* Request::findMatchingVserv(const std::vector<Vserv*>& vservs, const Request& request)
{
    for (std::vector<Vserv*>::const_iterator it = vservs.begin(); it != vservs.end(); ++it)
    {
        if ((*it)->matchByName(request.getHeader("Host")))
            return *it;
    }
    return NULL;
}
