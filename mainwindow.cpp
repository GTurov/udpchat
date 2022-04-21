#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QBuffer>
#include <QFileDialog>
#include <QMessageBox>

const QHostAddress MainWindow::localHost_ = QHostAddress::LocalHost;

const QString MainWindow::strError_ = "Ошибка";
const QString MainWindow::strNotConnected_ = "Нет соединения";
const QString MainWindow::strBindError_ = "Не удалось открыть порт";
const QString MainWindow::strTooLargeMessage_ = "Слишком большое сообщение";
const QString MainWindow::strUnknownMessageType_ = "Получено сообщение неизвестного формата. Установите обновлённую версию.";
const QString MainWindow::strUnknownError_ = "Неизвестная ошибка";
const QString MainWindow::strInvalidSocketPort_ = "Неверно указан порт";
const QString MainWindow::strOpen_ = "Открыть";
const QString MainWindow::strUserNamePrefix_ = "You: ";

const QString MainWindow::strErrorTemplate_ = "<font color = red>%1: %2</font>";
const QString MainWindow::strImageTemplate_ = "<img src=\"data:image/png;base64,%1\"/>";

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , processor_(new QNetworkProcessor)
    , socketThread_(new QThread)
    , portValidtor_(new QIntValidator(this))
    , sendButtonMenu_(new QMenu(this))
{
    ui->setupUi(this);

    registerMetaTypes_();
    setupUserControls_();
    setupNetworkProcessor_();
}

MainWindow::~MainWindow()
{
    socketThread_->quit();
    socketThread_->wait();
    delete ui;
}

void MainWindow::displayIncomingTextMessage_(QString text)
{
    ui->messageHistory->append(text);
}

void MainWindow::displayError_(QNetworkProcessor::Error error)
{
    QString errorText;
    switch (error) {
    case QNetworkProcessor::Error::NotConnected :
        errorText = strNotConnected_;
        break;
    case QNetworkProcessor::Error::BindError :
        errorText = strBindError_;
        break;
    case QNetworkProcessor::Error::MessageTooLarge :
        errorText = strTooLargeMessage_;
        break;
    case QNetworkProcessor::Error::UnknownMessageType :
        errorText = strUnknownMessageType_;
        break;
    default: errorText = strUnknownError_;
        break;
    }
    ui->messageHistory->append(strErrorTemplate_.arg(strError_).arg(errorText));
}


void MainWindow::prepareTextMessage_()
{
    emit newTextMessage(ui->messageEdit->text());
    displayIncomingTextMessage_(strUserNamePrefix_ + ui->messageEdit->text());
    ui->messageEdit->clear();
}

void MainWindow::prepareImageMessage_()
{
    QString fileName = QFileDialog::getOpenFileName(0, strOpen_, "",  "*.png;; *.jpg;; *.bmp");
    if (!fileName.isEmpty()) {
        QByteArray byteArray;
        QBuffer buffer(&byteArray);
        QPixmap pixmap;
        pixmap.load(fileName);
        pixmap.save(&buffer, "PNG");
        QString imageString = strImageTemplate_.arg(QString(byteArray.toBase64()));
        emit newTextMessage(imageString);
        displayIncomingTextMessage_(strUserNamePrefix_ + imageString);
    }
}

bool MainWindow::checkConnectionSettings_()
{
    if (!ui->localPortEdit->hasAcceptableInput()) {
        QMessageBox::warning(this, strError_, strInvalidSocketPort_ + ": " + ui->localPortEdit->text());
        return false;
    }
    if (!ui->remotePortEdit->hasAcceptableInput()) {
        QMessageBox::warning(this, strError_, strInvalidSocketPort_ + ": " + ui->remotePortEdit->text());
        return false;
    }
    return true;
}

void MainWindow::registerMetaTypes_()
{
    qRegisterMetaType<uint16_t>("uint16_t");
    qRegisterMetaType<QHostAddress>("QHostAddress");
    qRegisterMetaType<QNetworkProcessor::Error>("Error");
}

void MainWindow::setupUserControls_()
{
    portValidtor_->setRange(0, 65535);
    ui->localPortEdit->setValidator(portValidtor_);
    ui->remotePortEdit->setValidator(portValidtor_);

    connect(ui->messageEdit, &QLineEdit::returnPressed, this, &MainWindow::prepareTextMessage_);

    auto* actionSendText = sendButtonMenu_->addAction("Текстовое сообщение");
    connect(actionSendText, &QAction::triggered, this, &MainWindow::prepareTextMessage_);
    auto* actionSendImage = sendButtonMenu_->addAction("Изображение");
    connect(actionSendImage, &QAction::triggered, this, &MainWindow::prepareImageMessage_);
    ui->sendButton->setMenu(sendButtonMenu_);

    connect(ui->connectButton, &QPushButton::clicked, [&](){
        if (checkConnectionSettings_()) {
            emit MainWindow::newConnectionSettings(ui->localPortEdit->text().toUInt(), localHost_, ui->remotePortEdit->text().toUInt());
        }
    });
}

void MainWindow::setupNetworkProcessor_()
{
    connect(processor_, &QNetworkProcessor::textMesageReceived, this, &MainWindow::displayIncomingTextMessage_);
    connect(processor_, &QNetworkProcessor::errorOccured, this, &MainWindow::displayError_);
    connect(this, &MainWindow::newTextMessage, processor_, &QNetworkProcessor::sendTextMessage);
    connect(this, &MainWindow::newConnectionSettings, processor_, &QNetworkProcessor::setupConnection);
    connect(ui->disconnectButton, &QPushButton::clicked, processor_, &QNetworkProcessor::disconnectFromHost);
    connect(socketThread_, &QThread::finished, processor_, &QNetworkProcessor::deleteLater);
    socketThread_->start();
    processor_->moveToThread(socketThread_);
}
