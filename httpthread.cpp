#include <string>

#include "httpthread.h"
#include "common.h"
#include "configmanager.h"


HTTPThread::HTTPThread(int socketDescriptor, QObject* parent)
    : QThread(parent), socketDescriptor(socketDescriptor)
{
    qDebug() << "HTTPThread()";
}

void HTTPThread::run()
{
    qDebug() << "HTTPThread::run()";

    if (!ClientSocket.setSocketDescriptor(socketDescriptor)) {
        emit error(ClientSocket.error());
        return;
    }

    connect(&ClientSocket, SIGNAL(connected()), this, SLOT(ClientConnected()), Qt::QueuedConnection);
    connect(&ClientSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(ClientBytesWritten(qint64)), Qt::QueuedConnection);
    connect(&ClientSocket, SIGNAL(readyRead()), this, SLOT(ClientReadyRead()), Qt::QueuedConnection);
}

void HTTPThread::ClientReadyRead() {
    while (ClientSocket.canReadLine()) {
        std::string currRead = ClientSocket.readLine().toStdString();

        std::vector<std::string>* readParts = Split(currRead, ": ");

        if (readParts->at(0) == "Host") {
            recvHost = readParts->at(1);
            domain = Split(recvHost, ":")->at(0);

            break;
        }

        dataRead += currRead;
    }

    if (dataRead == "" || domain == "") {
        ClientSocket.write("HTTP/1.0 400 Bad Request\r\n\r\nInvalid request!");

        ClientSocket.disconnectFromHost();

        return;
    }

    qDebug() << "Got Domain: " << domain.c_str() << "\n";

    if (GlobalConfig()->DomainRoutes->count(domain) == 0) {
        ClientSocket.write("HTTP/1.0 200 OK\r\n\r\n<html><body><h2>The domain requested is not available on this server.</h2></body></html>");

        ClientSocket.disconnectFromHost();
    } else {
        DestinationStr = GlobalConfig()->DomainRoutes->at(domain);

        destinationIPPort = Split(DestinationStr, ":");

        QString strPort(destinationIPPort->at(1).c_str());

        Port = strPort.toUShort();

        connect(&DownstreamSocket, SIGNAL(connected()), this, SLOT(DownstreamConnected()), Qt::QueuedConnection);
        connect(&DownstreamSocket, SIGNAL(readyRead()), this, SLOT(DownstreamReadyRead()), Qt::QueuedConnection);

        DownstreamSocket.connectToHost(QString(destinationIPPort->at(0).c_str()), Port);
    }
}

void HTTPThread::DownstreamConnected() {
    DownstreamSocket.write(dataRead.c_str());

    std::string NewHeaders = "Host: " + (Port == 80 ? destinationIPPort->at(0).c_str() : DestinationStr) + "\r\n";
    NewHeaders += "X-Forwarded-For: " + ClientSocket.peerAddress().toString().toStdString() + "\r\n";
    NewHeaders += "X-Forwarded-Host: " + recvHost + "\r\n";
    NewHeaders += "X-Forwarded-Server: " + domain + "\r\n";

    DownstreamSocket.write(NewHeaders.c_str());

    qDebug() << "Wrote to downstream: " << dataRead.c_str() << NewHeaders.c_str() << "\n";

    int bytesRead = 0;
    QByteArray readByte = ClientSocket.read(1024);

    while (readByte.size() > 0) {
        DownstreamSocket.write(readByte);
        bytesRead += readByte.size();
        readByte = ClientSocket.read(1024);
    }

    qDebug() << "Wrote " << bytesRead << " bytes to downstream.\n";

    DownstreamSocket.flush();
}

void HTTPThread::DownstreamReadyRead() {
    int bytesRead = 0;
    QByteArray readByte = ClientSocket.read(1024);

    while (readByte.size() > 0) {
        ClientSocket.write(readByte);
        bytesRead += readByte.size();
        readByte = DownstreamSocket.read(1024);
    }

    connect(&ClientSocket, SIGNAL(bytesWritten(qint64)), this, SLOT(ClientBytesWritten(qint64)));

    qDebug() << "Wrote " << bytesRead << " bytes to client.\n";
}

void HTTPThread::ClientBytesWritten(qint64) {
    DownstreamSocket.disconnectFromHost();
    ClientSocket.disconnectFromHost();
}
