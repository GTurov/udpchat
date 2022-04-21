#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "networkprocessor.h"

#include <QHostAddress>
#include <QIntValidator>
#include <QMainWindow>
#include <QMenu>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void newTextMessage(QString);
    void newConnectionSettings(uint16_t, QHostAddress, uint16_t);

private slots:
    void displayIncomingTextMessage_(QString text);
    void displayError_(QNetworkProcessor::Error error);
    void prepareTextMessage_();
    void prepareImageMessage_();

private:
    bool checkConnectionSettings_();
    void registerMetaTypes_();
    void setupUserControls_();
    void setupNetworkProcessor_();

private:
    static const QHostAddress localHost_;
    static const QString strError_;
    static const QString strNotConnected_;
    static const QString strBindError_;
    static const QString strTooLargeMessage_;
    static const QString strUnknownMessageType_;
    static const QString strUnknownError_;
    static const QString strInvalidSocketPort_;
    static const QString strOpen_;
    static const QString strUserNamePrefix_;
    static const QString strErrorTemplate_;
    static const QString strImageTemplate_;

private:
    Ui::MainWindow *ui;
    QNetworkProcessor* processor_;
    QThread* socketThread_;
    QIntValidator* portValidtor_;
    QMenu* sendButtonMenu_;
};
#endif // MAINWINDOW_H
