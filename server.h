#ifndef SERVER_H
#define SERVER_H

#include <QtNetwork/QTcpServer>
#include <QtNetwork/QTcpSocket>

class Server : public QTcpServer
{
    Q_OBJECT

public:
    Server(QObject *parent=0);
    void sendMessage(QString msg);

private slots:
    void readyRead();
    void disconnected();

signals:
    void connect_signal();

protected:
    void incomingConnection(int socketfd);

private:
    QSet<QTcpSocket*> clients;
};

#endif // SERVER_H
