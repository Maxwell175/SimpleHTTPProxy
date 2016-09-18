#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QAbstractSocket>
#include "common.h"
#include "httpclient.h"

class HTTPServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HTTPServer(QObject *parent = 0);
    void startServer();

protected:
    void incomingConnection(qintptr socketDescriptor);

signals:

public slots:

};

#endif // HTTPSERVER_H
