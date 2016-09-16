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
    HTTPThread *thread = new HTTPThread(socketDescriptor, this);
    connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
    thread->start();
}
