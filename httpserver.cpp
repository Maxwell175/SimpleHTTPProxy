#include "httpserver.h"

HTTPServer::HTTPServer(QObject *parent) :
    QTcpServer(parent)
{
}

void HTTPServer::startServer()
{
    if(listen(QHostAddress::Any, 1234))
    {
        qDebug() << "Server: started";
    }
    else
    {
        qDebug() << "Server: not started!";
    }
}

void HTTPServer::incomingConnection(qintptr socketDescriptor)
{
    HTTPClient *worker = new HTTPClient(socketDescriptor);
    worker->run();
}
