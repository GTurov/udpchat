#ifndef NETWORKPROCESSOR_H
#define NETWORKPROCESSOR_H

#include <QObject>
#include <QUdpSocket>

// Раз уж ТЗ намекает на предстоящее развитие проекта, целесообразно разработать
// протокол под передачу разных типов данных, на первом этапе реализовав передачу
// текстовых данных. Такая совместимость лучше, чем никакой.

//------------------------------Дейтаграмма-------------------------------
//
// Байты:    0       1       2       3       4       5       6       7
//     0 +-------+-------+-------+-------+-------+-------+-------+-------+
//       |   A   |   B   |       С       |       Размер дейтаграммы      |
//     8 +-------+-------+-------+-------+-------+-------+-------+-------+
//       |            ID части           |          Всего частей         |
//    16 +-------+-------+-------+-------+-------+-------+-------+-------+
//       |                             Данные                            |
//       |                              ...                              |
//   ... +-------+-------+-------+-------+-------+-------+-------+-------+
//
//   A - версия протокола
//   B - тип данных (текст, файл и т.п.) (uint8_t)
//   C - ID передачи (uint16_t). Для одновременной передачи нескольких файлов,
//       например, текстовых сообщений и передачи крупного файла на фоне. Каждое
//       сообщение имеет свой ID.
//------------------------------------------------------------------------------


class QNetworkProcessor : public QObject
{
    Q_OBJECT
    enum class DataType {
        ServiceMessage = 0,
        Text = 1
//        File = 2 // на будущее
    };

public:
    enum class Error {
        NotConnected,
        BindError,
        MessageTooLarge,
        UnknownMessageType
    };
    explicit QNetworkProcessor(QObject *parent = nullptr);
    virtual ~QNetworkProcessor() = default;

signals:
    void textMesageReceived(QString);
    void errorOccured(Error);

public slots:
    void sendTextMessage(QString text);
    void setupConnection(uint16_t localPort, const QHostAddress& remoteHost, uint16_t remotePort);
    void disconnectFromHost() { socket_->disconnectFromHost(); }

private slots:
    void socketReadyRead_();
    void parseDatagram_(const QNetworkDatagram& datagram);

private:
    QByteArray makeDatagramHeader_(DataType dataType, int dataSize);
    QNetworkDatagram makeTextDatagram_(const QString& text);
    void parseTextDatagram_(const QNetworkDatagram& datagram);

private:
    QUdpSocket* socket_;
    QHostAddress remoteHost_;
    uint16_t remotePort_ = 0;
    uint32_t transmitCounter_ = 0;
};

#endif // NETWORKPROCESSOR_H
