#include <QtWidgets>
#include <QRegExp>
#include "server.h"
#include <QDebug>

Server::Server(QObject *parent)
    : QTcpServer(parent)
{
}

void Server::incomingConnection(int socketfd)
{
    qDebug() << Q_FUNC_INFO;

    QTcpSocket *client = new QTcpSocket(this);
    client->setSocketDescriptor(socketfd);
    clients.insert(client);

    emit connect_signal();

    connect(client, SIGNAL(readyRead()), this,
                    SLOT(readyRead()));
    connect(client, SIGNAL(disconnected()), this,
                    SLOT(disconnected()));
}

void Server::sendMessage(QString msg) {
    foreach(QTcpSocket *client, clients)
        client->write(msg.toUtf8());
}


void Server::readyRead()
{
    QTcpSocket *client = (QTcpSocket*)sender();
    while(client->canReadLine())
    {
        QString line = QString::fromUtf8(client->readLine()).trimmed();
    }
}

void Server::disconnected()
{
    QTcpSocket *client = (QTcpSocket*)sender();

    clients.remove(client);
}
