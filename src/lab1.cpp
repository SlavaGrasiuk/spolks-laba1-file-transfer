#include <QIntValidator>
#include <QRegExpValidator>
#include <QNetworkProxyFactory>
#include <QFileDialog>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "DragFrame.hpp"
#include "lab1.h"

constexpr qint64 g_maxLen = 32'768;

/*
==================
lab1::lab1
==================
*/
lab1::lab1(QWidget *parent): QMainWindow(parent) {
	m_ui.setupUi(this);

	m_dragFrame = new DragFrame(m_ui.tab);
	m_dragFrame->setGeometry(QRect(200, 10, 181, 91));
	m_dragFrame->setAcceptDrops(true);
	connect(m_dragFrame, &DragFrame::GetFile, this, &lab1::OnOpenFile);

	m_ui.portEdit->setValidator(new QIntValidator(0, 65535, m_ui.portEdit));

	QString ipRange = "(?:[0-1]?[0-9]?[0-9]|2[0-4][0-9]|25[0-5])";
	QRegExp ipRegex("^" + ipRange + "\\." + ipRange + "\\." + ipRange + "\\." + ipRange + "$");
	m_ui.ipAddrEdit->setValidator(new QRegExpValidator(ipRegex, m_ui.ipAddrEdit));

	m_ui.protocolSelect->addItem("TCP");
	m_ui.protocolSelect->addItem("UDP");

	m_ui.listenPortEdit->setValidator(new QIntValidator(0, 65535, m_ui.portEdit));

	m_ui.listenProtocol->addItem("TCP");
	m_ui.listenProtocol->addItem("UDP");

	QNetworkProxyFactory::setUseSystemConfiguration(false);		//Qt 5.8.0 workaround

	m_tcpServer = new QTcpServer(this);
	m_tcpSocket = new QTcpSocket(this);

	connect(m_ui.startListenBtn, &QPushButton::clicked, this, &lab1::OnStartListen);
	connect(m_tcpServer, &QTcpServer::newConnection, this, &lab1::OnIncomingConnection);
	connect(m_tcpSocket, &QTcpSocket::connected, this, &lab1::OnConnected);
	connect(m_tcpSocket, &QTcpSocket::disconnected, this, &lab1::OnDisconected);
}

/*
==================
lab1::~lab1
==================
*/
lab1::~lab1() {
	delete m_tcpSocket;
	delete m_dragFrame;
}

/*
==================
lab1::OnStartListen
==================
*/
void lab1::OnStartListen() {
	if (!m_tcpServer->listen(QHostAddress::SpecialAddress::Any, m_ui.listenPortEdit->text().toShort())) {
		QMessageBox msgBox;
		msgBox.setText("Can't start server! " + m_tcpServer->errorString());
		msgBox.setIcon(QMessageBox::Icon::Critical);
		msgBox.exec();
	}
}

/*
==================
lab1::OnIncomingConnection
==================
*/
void lab1::OnIncomingConnection() {
	m_recvSocket = m_tcpServer->nextPendingConnection();
	m_recvState = SendState::SendHeader;
	connect(m_recvSocket, &QTcpSocket::readyRead, this, &lab1::OnReadyRead);
}

/*
==================
lab1::OnConnected
==================
*/
void lab1::OnConnected() {
	if (m_sendState == SendState::SendHeader) {
		const qint64 size = GetFileSize();

		QByteArray header;
		header.append(reinterpret_cast<const char*>(&size), sizeof size);
		header.append(m_fileInfo.fileName());

		m_tcpSocket->write(header);

		m_sendState = SendState::WaitAck;
	}
}

/*
==================
lab1::OnBytesWrite
==================
*/
void lab1::OnBytesWrite(qint64 bytes) {
	if (m_sendState == SendState::WaitAck) {
		//send part of file here
		m_sendState = SendState::SendData;
	}
}

/*
==================
lab1::OnReadyRead
==================
*/
void lab1::OnReadyRead() {
	if (m_recvState == SendState::SendHeader) {
		const QByteArray header = m_recvSocket->read(g_maxLen);
		const qint64 size = *reinterpret_cast<const qint64*>(header.data());
		const QString filename(header.mid(sizeof size));

		auto reply = QMessageBox::question(this, "Incoming file", "Receive file " + filename + " (" + QString::number(size) + " bytes) from " + m_recvSocket->peerAddress().toString() + '?', QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
		if (reply == QMessageBox::StandardButton::Yes) {
			/*auto fileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Сохранить изображение"), filename, QString::fromLocal8Bit("Изображения Jpeg (*.jpg);;Изображения PNG (*.png);;Изображения BMP (*.bmp)"));
			if (fileName.isEmpty()) {
				return;
			}*/
		} else {
			delete m_recvSocket;
		}
	}
}

/*
==================
lab1::OnDisconected
==================
*/
void lab1::OnDisconected() {
	m_sendState = SendState::SendHeader;
	m_fileToTransfer.close();

	QMessageBox msgBox;
	msgBox.setText("Server disconected!");
	msgBox.setIcon(QMessageBox::Icon::Information);
	msgBox.exec();
}

/*
==================
lab1::GetFileSize
==================
*/
qint64 lab1::GetFileSize() {
	m_fileToTransfer.seekg(0, std::ios::end);
	auto size = m_fileToTransfer.tellg();
	m_fileToTransfer.seekg(0);
	return size;
}

/*
==================
lab1::OnOpenFile
==================
*/
void lab1::OnOpenFile(std::string name) {
	m_fileToTransfer.open(name, std::ios::binary);
	if (m_fileToTransfer) {
		m_fileInfo = QString::fromStdString(name);
		if (m_ui.protocolSelect->itemText(m_ui.protocolSelect->currentIndex()) == "TCP") {
			m_tcpSocket->connectToHost(m_ui.ipAddrEdit->text(), m_ui.portEdit->text().toShort());
			m_sendState = SendState::SendHeader;
		} else {

		}
	}
}
