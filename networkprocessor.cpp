#include "networkprocessor.h"

#include <QNetworkDatagram>

constexpr uint8_t dtgrmProtocolVersion = 0x01;

constexpr int dtgrmHeaderSize = 16;
constexpr int dtgrmProtocolVersionOffset = 0;
constexpr int dtgrmDataTypeOffset = 1;
constexpr int dtgrmTransmitIdOffset = 2;
constexpr int dtgrmSizeOffset = 4;
constexpr int dtgrmPartIdOffset = 8;
constexpr int dtgrmPartCountOffset = 12;

QNetworkProcessor::QNetworkProcessor(QObject *parent)
    : QObject(parent), socket_(new QUdpSocket(this))
{
    connect(socket_, &QUdpSocket::readyRead, this, &QNetworkProcessor::socketReadyRead_);
}

void QNetworkProcessor::sendTextMessage(QString text)
{
    if (socket_->state() != QAbstractSocket::BoundState) {
        emit errorOccured(Error::NotConnected);
        return;
    }
    if (socket_->writeDatagram(makeTextDatagram_(text)) == -1) {
        emit errorOccured(Error::MessageTooLarge);
    }
}

void QNetworkProcessor::setupConnection(uint16_t localPort, const QHostAddress& remoteHost, uint16_t remotePort)
{
    remoteHost_ = remoteHost;
    remotePort_ = remotePort;
    if (socket_->state() == QAbstractSocket::BoundState) {
        socket_->disconnectFromHost();
    }
    if (!socket_->bind(localPort)) {
        emit errorOccured(Error::BindError);
    }
}

void QNetworkProcessor::socketReadyRead_()
{
    while (socket_->hasPendingDatagrams()) {
        parseDatagram_(socket_->receiveDatagram());
    }
}

void QNetworkProcessor::parseDatagram_(const QNetworkDatagram& datagram)
{
    if (datagram.data().at(dtgrmProtocolVersionOffset) != dtgrmProtocolVersion) {
        return;
    }
    if (datagram.data().at(dtgrmDataTypeOffset) == static_cast<uint8_t>(DataType::Text)) {
        parseTextDatagram_(datagram);
    } else {
        emit errorOccured(Error::UnknownMessageType);
    }
}

QByteArray QNetworkProcessor::makeDatagramHeader_(DataType dataType, int dataSize)
{
    QByteArray header(dtgrmHeaderSize, 0);
    header[dtgrmProtocolVersionOffset] = dtgrmProtocolVersion;
    header[dtgrmDataTypeOffset] = static_cast<char>(dataType);
    *reinterpret_cast<uint32_t*>(header.data() + dtgrmTransmitIdOffset) = transmitCounter_++;
    *reinterpret_cast<uint32_t*>(header.data() + dtgrmPartIdOffset) = 0;
    *reinterpret_cast<uint32_t*>(header.data() + dtgrmPartCountOffset) = 1;
    *reinterpret_cast<uint32_t*>(header.data() + dtgrmSizeOffset) = dataSize;
    return header;
}

QNetworkDatagram QNetworkProcessor::makeTextDatagram_(const QString& text)
{
    QByteArray data = text.toUtf8();
    QByteArray datagram = makeDatagramHeader_(DataType::Text, data.size());
    datagram += data;
    return QNetworkDatagram(datagram, remoteHost_, remotePort_) ;
}

void QNetworkProcessor::parseTextDatagram_(const QNetworkDatagram& datagram)
{
    QString message = QString::number(datagram.senderPort());
    message += ": ";
    message += datagram.data().mid(dtgrmHeaderSize);
    emit textMesageReceived(message);
}
