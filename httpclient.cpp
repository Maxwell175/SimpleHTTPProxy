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

    while (ClientSocket->canReadLine()) {
        std::string currRead = ClientSocket->readLine().toStdString();

        std::vector<std::string>* readParts = Split(currRead, ": ");

        if (readParts->at(0) == "Host") {
            recvHost = readParts->at(1);
            recvHost.pop_back(); // Remove CRLF from the end.
            recvHost.pop_back();
            domain = Split(recvHost, ":")->at(0);

            break;
        }

        dataRead += currRead;
    }

    if (dataRead == "" || domain == "") {
        ClientSocket->write("HTTP/1.0 400 Bad Request\r\n\r\nInvalid request!");

        ClientSocket->disconnectFromHost();

        return;
    }

    qDebug() << "Got Domain: " << domain.c_str();

    if (GlobalConfig()->DomainRoutes->count(domain) == 0) {
        ClientSocket->write("HTTP/1.0 200 OK\r\n\r\n<html><body><h2>The domain requested is not available on this server.</h2></body></html>");

        ClientSocket->disconnectFromHost();
    } else {
        DestinationStr = GlobalConfig()->DomainRoutes->at(domain);

        destinationIPPort = Split(DestinationStr, ":");

        QString strPort(destinationIPPort->at(1).c_str());

        Port = strPort.toUShort();

        DownstreamSocket = new QTcpSocket();

        connect(DownstreamSocket, SIGNAL(connected()), this, SLOT(DownstreamConnected()), Qt::QueuedConnection);
        connect(DownstreamSocket, SIGNAL(readyRead()), this, SLOT(DownstreamReadyRead()), Qt::QueuedConnection);
        connect(DownstreamSocket, SIGNAL(disconnected()), this, SLOT(DownstreamDisconnected()), Qt::QueuedConnection);

        DownstreamSocket->connectToHost(QString(destinationIPPort->at(0).c_str()), Port);
    }
}

void HTTPClient::DownstreamConnected() {
    qDebug() << "HTTPClient::DownstreamConnected()";

    DownstreamSocket->write(dataRead.c_str());

    std::string NewHeaders = "Host: " + (Port == 80 ? destinationIPPort->at(0).c_str() : DestinationStr) + "\r\n";
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

}

void HTTPClient::DownstreamDisconnected() {
    // The downstream disconnects once it has written all data.
    // This means we can request a disconnect from the Client as well,
    //     since it will disconnect only once all data has been written.
    ClientSocket->disconnectFromHost();
}
