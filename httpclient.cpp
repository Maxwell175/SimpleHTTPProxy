#include <string>

#include "httpclient.h"
#include "common.h"
#include "configmanager.h"


HTTPClient::HTTPClient(int socketDescriptor)
    : socketDescriptor(socketDescriptor)
{
    qDebug() << "HTTPClient()";
}

void HTTPClient::run()
{
    qDebug() << "HTTPClient::run()";

    ClientSocket = new QTcpSocket();

    if (!ClientSocket->setSocketDescriptor(socketDescriptor)) {
        emit error(ClientSocket->error());
        qDebug() << "setSocketDescriptor() error: " << ClientSocket->error();
        return;
    }

    if (ClientSocket->canReadLine()) {
        ClientReadyRead();
    } else {
        connect(ClientSocket, SIGNAL(readyRead()), this, SLOT(ClientReadyRead()), Qt::QueuedConnection);
    }
    connect(ClientSocket, SIGNAL(disconnected()), this, SLOT(ClientDisconnected()), Qt::QueuedConnection);

}

void HTTPClient::ClientReadyRead() {
    qDebug() << "HTTPClient::ClientReadyRead()";

    bool HostRead = false;
    bool ConnectionRead = false;
    while (ClientSocket->canReadLine()) {
        std::string currRead = ClientSocket->readLine().toStdString();
        currRead.pop_back(); // Remove CRLF from the end.
        currRead.pop_back();

        std::vector<std::string>* readParts = Split(currRead, ": ");

        if (readParts->at(0) == "Host") {
            recvHost = readParts->at(1);
            domain = Split(recvHost, ":")->at(0);

            HostRead = true;
        } else if (readParts->at(0) == "Connection") {
            // We dont support multiple requests per connection at this time.
            readParts->at(1) = "close";

            ConnectionRead = true;

            dataRead += "Connection: ";
            dataRead += readParts->at(1);
            dataRead += "\r\n";
        } else {
            dataRead += currRead;
            dataRead += "\r\n";
        }

        if (HostRead && ConnectionRead) {
            break;
        }
    }

    if (dataRead == "" || domain == "" || !HostRead || !ConnectionRead) {
        ClientSocket->write("HTTP/1.0 400 Bad Request\r\n\r\nInvalid request!");

        ClientSocket->disconnectFromHost();

        return;
    }

    qDebug() << "Got Domain: " << domain.c_str();

    if (GlobalConfig()->DomainRoutes->count(domain) == 0) {
        ClientSocket->write("HTTP/1.0 200 OK\r\n\r\n<html><body><h2>The domain requested is not available on this server.</h2></body></html>");

        ClientSocket->disconnectFromHost();
    } else {
        Destination = GlobalConfig()->DomainRoutes->at(domain);

        DownstreamSocket = new QTcpSocket();

        if (countOccurences(dataRead, " HTTP/") > 1) {
            qDebug() << "Got 2 Requests in 1 connection!!!!!";
        }

        connect(DownstreamSocket, SIGNAL(connected()), this, SLOT(DownstreamConnected()), Qt::QueuedConnection);
        connect(DownstreamSocket, SIGNAL(disconnected()), this, SLOT(DownstreamDisconnected()), Qt::QueuedConnection);

        DownstreamSocket->connectToHost(QString(Destination.DestinationHost.c_str()), Destination.DestinationPort);
    }
}

void HTTPClient::DownstreamConnected() {
    qDebug() << "HTTPClient::DownstreamConnected()";

    DownstreamSocket->write(dataRead.c_str());

    std::string NewHeaders = "Host: " + Destination.HostHeader + "\r\n";
    NewHeaders += "X-Forwarded-For: " + ClientSocket->peerAddress().toString().toStdString() + "\r\n";
    NewHeaders += "X-Forwarded-Host: " + recvHost + "\r\n";
    NewHeaders += "X-Forwarded-Server: " + domain + "\r\n";

    DownstreamSocket->write(NewHeaders.c_str());

    qDebug() << "Wrote to downstream: " << dataRead.c_str() << NewHeaders.c_str();

    int bytesRead = 0;
    QByteArray readByte = ClientSocket->read(1024);

    while (readByte.size() > 0) {
        DownstreamSocket->write(readByte);
        bytesRead += readByte.size();
        readByte = ClientSocket->read(1024);
    }

    qDebug() << "Wrote " << bytesRead << " bytes to downstream.";

    connect(DownstreamSocket, SIGNAL(readyRead()), this, SLOT(DownstreamReadyRead()), Qt::QueuedConnection);

    DownstreamSocket->flush();
}

void HTTPClient::DownstreamReadyRead() {
    QByteArray readByte = DownstreamSocket->read(1024);

    while (readByte.size() > 0) {
        ClientSocket->write(readByte);
        DownstreamBytesRead += readByte.size();
        readByte = DownstreamSocket->read(1024);
    }

    //connect(ClientSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(ClientBytesWritten(qint64)));

    qDebug() << "Wrote " << DownstreamBytesRead << " bytes to client.";
}

void HTTPClient::ClientDisconnected() {
    ClientSocket->deleteLater();
    DownstreamSocket->deleteLater();
}

void HTTPClient::DownstreamDisconnected() {
    // The downstream disconnects once it has written all data.
    // This means we can request a disconnect from the Client as well,
    //     since it will disconnect only once all data has been written.
    //ClientSocket->disconnectFromHost();
}
