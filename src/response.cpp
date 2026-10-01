/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   response.cpp                                       :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: fgranger <fgranger@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/01/18 14:28:24 by fgranger          #+#    #+#             */
/*   Updated: 2025/01/18 14:34:58 by fgranger         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#include "../include/Response.hpp"
#include <fstream>
#include <sys/stat.h> // Include for stat

//constructor
Response::Response(Server* serv, int clientfd)
    : _client_fd(clientfd),
    _version("HTTP/1.1"),
    _statusCode(200),
    _statusMessage("OK"),
    _sent(0),
    _serv(serv),
    cgiStatus(NONE),
    sending_response(true)
{

}

Response::Response(int statusCode, Server* serv, int clientfd)
    : _client_fd(clientfd),
    _version("HTTP/1.1"),
    _statusCode(statusCode),
    _sent(0),
    _serv(serv),
    cgiStatus(NONE),
    sending_response(true)
{
    handleError(statusCode);
}

//destructors
Response::~Response() {}

//setters
void Response::setVersion(const std::string& version) {_version = version;}
void Response::setStatusCode(int statusCode) {_statusCode = statusCode;}
void Response::setStatusMessage(const std::string& statusMessage) {_statusMessage = statusMessage;}
void Response::addHeader(const std::string& key, const std::string& value) {_headers[key] = value;}
void Response::setBody(const std::string& body) {_body = body;}
void Response::setSent(ssize_t sent) {_sent = sent;}

//getters
std::string Response::getVersion() const {return _version;}
int Response::getStatusCode() const {return _statusCode;}
std::string Response::getStatusMessage() const {return _statusMessage;}
bool Response::isSendingResponse() const {return sending_response;}
ssize_t Response::getSent() const {return _sent;}
std::string Response::getBody() const {return _body;}
std::string Response::getHeader(const std::string& key) const
{
    std::map<std::string, std::string>::const_iterator it = _headers.find(key);
    if (it != _headers.end())
        return it->second;
    return "";
}

//member functions

void Response::generateResponse(Request& request, const std::vector<Vserv*>& vservs)
{
    request.print();
    // 2. Find matching virtual server and route
    Vserv* matchingVserv = findMatchingVserv(vservs, request);
    Route* matchingRoute = matchingVserv ? findMatchingRoute(matchingVserv, request) : NULL; 
    if (!matchingVserv || !matchingRoute)
    {
        handleError(404);
        return;
    }
    //3.prev Handle Cgi call if needed

    if (request.getPath().find('.') != std::string::npos && needCgi(request.getPath().substr(request.getPath().find('.')), matchingRoute))
    {
        handleCgi(request,matchingRoute);
        return ;
    }

    // 3. Handle different HTTP methods
    setVersion("HTTP/1.1");
    addHeader("Server", matchingVserv->get_name());
    addHeader("Date", getCurrentDate());

    std::string method = request.getMethod();
    if (method == "GET" && matchingRoute->isAllowed(GET))
        handleGET(request, matchingRoute, 0);
    else if (method == "POST" && matchingRoute->isAllowed(POST))
        handlePOST(request, matchingRoute);
    else if (method == "HEAD")
        handleHEAD(request, matchingRoute);
    else if (method == "DELETE" && matchingRoute->isAllowed(DELETE))
        handleDelete(request, matchingRoute);
    else
    {
        handleError(405);
        return;
    }
    // 4. Set content type based on file extension
    if (getHeader("Content-Type") == std::string(""))
    {
        std::string contentType = getContentType(request.getPath());
        addHeader("Content-Type", contentType);
    }

    // 5. Add security headers
    addHeader("X-Content-Type-Options", "nosniff");
    addHeader("X-XSS-Protection", "1; mode=block");

    // 6. Set content length
    addHeader("Content-Length", intToString(_body.size()));

    // 7. Handle connection
    if (request.getHeader("Connection") == "keep-alive")
    {
        addHeader("Connection", "keep-alive");
        addHeader("Keep-Alive", "timeout=5, max=100");
    }
    else
        addHeader("Connection", "close");
}

//Handle GET for a request and a matching route skipBody=1 to skip body
void Response::handleGET(const Request& request, Route* route, bool skipBody) {
    std::string filePath = route->get_replace();
    std::string requestPath = request.getPath();
    if (requestPath.find_last_of(".") != std::string::npos)
    {
        filePath += requestPath.substr(route->get_match().length());//version corrige
    }
    else if (route->isListing())
    {
        filePath += route->getIndexfile();
        addHeader("Content-Type", getContentType(filePath));
    }
    else
        return handleError(404);
    
    if (!fileExists(filePath))
    {
        handleError(404);
        return;
    }
    std::string content = readFile(filePath);
    if (content.empty())
    {
        handleError(500);
        return;
    }

    setStatusCode(200);
    setStatusMessage("OK");
    if (!skipBody)
        setBody(content);
}

void Response::handlePOST(const Request& request, Route* route) 
{
    // Check if path is directory
    std::string filePath = route->get_replace();
    std::string requestPath = request.getPath();
    filePath += requestPath.substr(route->get_match().length());
    // Handle directory case
    if (isDirectory(filePath))
    {
            handleError(404);
            return;
    }
    // Check if parent directory exists
    std::string dirPath = filePath.substr(0, filePath.find_last_of('/'));
    if (!isDirectory(dirPath))
    {
        handleError(404);
        return;
    }
    // Write content
    std::ofstream file(filePath.c_str(), std::ios::app);
    if (!file)
    {
        handleError(500);
        return;
    }

    file << request.getBody();
    //remove separtor and header
    file.close();

    // Send response
    setStatusCode(201);
    setStatusMessage("Created");
    addHeader("Content-Type", "text/html");
    setBody("<html><body><h1>POST request successful</h1></body></html>");
}

void Response::handleHEAD(const Request& request, Route* route)
{
    handleGET(request, route, 1);
    setBody("");
}

void Response::handleDelete(const Request& request, Route* route)
{
        // Check if path is directory
    std::string filePath = route->get_replace();
    std::string requestPath = request.getPath();
    filePath += requestPath.substr(route->get_match().length());
    // Handle directory case
    if (isDirectory(filePath))
    {
            handleError(404);
            return;
    }
    if (remove(filePath.c_str()) != 0)
        handleError(505);
    else
    {
        setStatusCode(204);
        setStatusMessage("No content");
    }
}

std::string Response::getContentType(const std::string& path) {
    std::string ext = path.substr(path.find_last_of(".") + 1);
    if (ext == "html") return "text/html";
    if (ext == "ico") return "image/x-icon";
    if (ext == "css") return "text/css";
    if (ext == "js") return "application/javascript";
    if (ext == "png") return "image/png";
    if (ext == "jpg" || ext == "jpeg") return "image/jpeg";
    if (ext == "webm") return "video/webm";
    return "text/plain";
}

inline std::string headerKeyToEnv(const std::string& key)
{
    std::string result = key;
    for (std::size_t i=0; i<key.size(); i++)
    {
        if (result[i] == '-')
            result[i] = '_';
        else
            result[i] = std::toupper(result[i]);
    }
    return (result);
}


void Response::setCgiEnv(const Request& request, Route* route)
{
    // Required PHP CGI variables
    setenv("REDIRECT_STATUS", "200", 1);
    setenv("SCRIPT_FILENAME", (route->get_replace() + request.getPath().substr(route->get_match().length())).c_str(), 1);
    
    // Standard CGI variables
    setenv("GATEWAY_INTERFACE", "CGI/1.1", 1);
    setenv("SERVER_PROTOCOL", "HTTP/1.1", 1);
    setenv("REQUEST_METHOD", request.getMethod().c_str(), 1);
    setenv("SCRIPT_NAME", request.getPath().c_str(), 1);
    setenv("QUERY_STRING", request.getQuery().c_str(), 1);
    setenv("CONTENT_LENGTH", intToString(request.getBodyLength()).c_str(), 1);
    setenv("CONTENT_TYPE", request.getHeader("Content-Type").c_str(), 1);

    // Headers
    for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it !=_headers.end(); ++it)
    {
        std::string key = headerKeyToEnv(it->first);
        std::string value = it->second;
        setenv(("HTTP_" + key).c_str(), value.c_str(), 1);
    }
    if (request.getHeader("Cookie") != "")
        setenv("HTTP_COOKIE", request.getHeader("Cookie").c_str(), 1);
}

void Response::handleCgi(const Request &request,Route *route)
{
    std::string scriptPath = route->get_replace() + request.getPath().substr(route->get_match().length());
    std::string ext = scriptPath.substr(scriptPath.find_last_of("."));
    std::string cgiExecutable = route->matchCgi(ext);
    if (cgiExecutable.empty())
    {
        std::cerr << ERROR_WARN << "No CGI executable found for extension: " << ext << std::endl;
        handleError(500);
        return;
    }
    int inputPipe[2], outputPipe[2];
    if (pipe(inputPipe) < 0 || pipe(outputPipe) < 0)
    {
        std::cerr << ERROR_WARN << "Pipe creation failure!" << std::endl;
        handleError(500);
        return;
    }
    pid_t   pid = fork();
    if (pid < 0)
    {
        std::cerr << ERROR_WARN << "Fork failure!" << std::endl;
        handleError(500);
        close(inputPipe[0]);
        close(inputPipe[1]);
        close(outputPipe[0]);
        close(outputPipe[1]);
        return ;
    }
    else if (pid == 0)
    {

        close(inputPipe[1]);
        close(outputPipe[0]);
        dup2(inputPipe[0], STDIN_FILENO);
        close(inputPipe[0]);
        dup2(outputPipe[1], STDOUT_FILENO);
        close(outputPipe[1]);
        setCgiEnv(request, route);
        char* args[] = {
            const_cast<char*>(cgiExecutable.c_str()),
            const_cast<char*>(scriptPath.c_str()),
            NULL
        };
        execv(cgiExecutable.c_str(), args);
        throw std::runtime_error("execve error");
    }
    else
    {
        cgiStatus = WAITING;
        Server  *server = _serv;
        close(inputPipe[0]);
        close(outputPipe[1]);
        server->newCgiSession(_client_fd, pid, inputPipe[1], outputPipe[0], request.getBody());
    }
}

bool Response::parseCGIOutput(const std::string& output) {
    size_t headerEnd = output.find("\r\n\r\n");
    if (headerEnd == std::string::npos)
    {
        std::cerr << ERROR_WARN << "Error: Could not parse CGI output" << std::endl;
        handleError(500);
        cgiStatus = DONE;
        return false;
    }
    std::string headers = output.substr(0, headerEnd);
    std::string body = output.substr(headerEnd + 4);

    // Parse headers
    std::istringstream headerStream(headers);
    std::string line;
    while (std::getline(headerStream, line) && !line.empty())
    {
        size_t colon = line.find(':');
        if (colon != std::string::npos) {
            std::string key = line.substr(0, colon);
            std::string value = line.substr(colon + 2);
            addHeader(key, value);
        }
    }
    setBody(body);
    setStatusCode(200);
    setStatusMessage("OK");
    cgiStatus = DONE;
    return (true);
}
//helpers

std::string Response::toString()
{
    if (_toSend.empty())
    {
        std::string response;
        response += _version + " " + intToString(_statusCode) + " " + _statusMessage + "\n";
        for (std::map<std::string, std::string>::const_iterator it = _headers.begin(); it != _headers.end(); ++it)
            response += it->first + ": " + it->second + "\n";
        response += "\n" + _body;
        _toSend = response;
    }
    return _toSend;
}

Vserv* Response::findMatchingVserv(const std::vector<Vserv*>& vservs, const Request& request)
{
    for (std::vector<Vserv*>::const_iterator it = vservs.begin(); it != vservs.end(); ++it)
    {
        if ((*it)->matchByName(request.getHeader("Host")))
            return *it;
    }
    return NULL;
}

Route* Response::findMatchingRoute(Vserv* vserv, const Request& request)
{
    Route *saved = NULL;
    for (std::list<Route*>::const_iterator it = vserv->getRoutes().begin(); it != vserv->getRoutes().end(); ++it)
    {
        if ((*it)->match(request.getPath()) && (!saved || (*it)->get_len() > saved->get_len())) 
            saved = *it;
    }
    return saved;
}

std::string Response::getCurrentDate()
{
    char buffer[100];
    time_t now = time(0);
    struct tm* timeinfo = gmtime(&now);
    strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", timeinfo);
    return std::string(buffer);
}

bool Response::fillFile(const std::string& path, const std::string& body)
{
    std::ofstream file(path.c_str(), std::ios::out | std::ios::trunc);
    
    if (!file)
        return false;
    file.write(body.c_str(), body.size());
    if (!file)
        return false;
    return true;
}

bool Response::needCgi(const std::string &ext, Route *route)
{
    if (!route->cgipass())
        return false;
    return route->matchCgi(ext).length();
}
std::string Response::readFile(const std::string &path)
{
    std::ifstream file(path.c_str());
    if (!file.is_open())
        return "";
    std::stringstream buffer;
    buffer << file.rdbuf();
    return buffer.str();
}

bool Response::fileExists(const std::string& path)
{
    struct stat buffer;
    return (stat(path.c_str(), &buffer) == 0);
}

bool Response::isDirectory(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

//other functions
std::string intToString(int value)
{
    std::ostringstream oss;
    oss << value;
    return oss.str();
}

void    Response::setToSend(std::string str)
{
    _toSend = str;
}

//return true if there remain more to be send
bool    Response::removefrontToSend(std::size_t amount)
{
    if (_toSend.length() > amount)
    {
        _toSend.erase(_toSend.begin(), _toSend.begin() + amount);
        return (true);
    }
    else
        return (false);
}

void Response::handleError(int statusCode)
{
    Vserv* vserv = findMatchingVserv(_serv->getVservs(), *(_serv->getRequest(_client_fd)));
    setStatusCode(statusCode);
    addHeader("Content-Type", "text/html");
    switch (statusCode)
    {
        case 400:
            setStatusMessage("Bad Request");
            setBody("<html><body><h1>400 Bad Request</h1></body></html>");
            break;
        case 403:
            setStatusMessage("Forbidden");
            setBody("<html><body><h1>403 Forbidden</h1></body></html>");
            break;
        case 404:
            setStatusMessage("Not Found");
            setBody("<html><body><h1>404 Not Found</h1></body></html>");
            break;
        case 405:
            setStatusMessage("Method Not Allowed");
            setBody("<html><body><h1>405 Method Not Allowed</h1></body></html>");
            break;
        case 413:
            setStatusMessage("Entity Too Large");
            setBody("<html><body><h1>413 Entity Too Large</h1></body></html>");
            break;
        case 500:
            setStatusMessage("Internal Server Error");
            setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
            break;
        default:
            setStatusCode(500);
            setStatusMessage("Internal Server Error");
            setBody("<html><body><h1>500 Internal Server Error</h1></body></html>");
            break;
    }
    if (vserv)
    {
        std::string path = vserv->getErrorPages(statusCode);
        if (path != "")
        {
            std::string content = readFile(path);
            if (content.empty())
                return;
            setBody(content);
        }
    }
}

cgiState_e  Response::getCgiStatus() const
{
    return (cgiStatus);
}

void Response::changeCgiStatus(cgiState_e state)
{
    cgiStatus = state;
}
