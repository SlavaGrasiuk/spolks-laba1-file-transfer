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

constexpr qint64 g_blockLen = 32'768;		//bytes

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
	connect(m_tcpSocket, &QTcpSocket::bytesWritten, this, &lab1::OnBytesWrite);
	connect(m_tcpSocket, &QTcpSocket::disconnected, this, &lab1::OnDisconected);
	connect(m_tcpSocket, &QTcpSocket::readyRead, this, &lab1::OnClientReadyRead);
}

/*
==================
lab1::~lab1
==================
*/
lab1::~lab1() {
	delete m_tcpServer;
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
		m_totalSize = GetFileSize();
		m_doneSize = 0;
		m_ui.progressBar->setMaximum(m_totalSize);

		QByteArray header;
		header.append(reinterpret_cast<const char*>(&m_totalSize), sizeof m_totalSize);
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
	static char buffer[g_blockLen];

	if (m_sendState == SendState::SendData) {
		m_fileToTransfer.read(buffer, g_blockLen);
		const qint64 readedSize = m_fileToTransfer.gcount();
		m_doneSize += readedSize;
		 
		m_tcpSocket->write(buffer, readedSize);
		m_ui.progressBar->setValue(m_doneSize);

		if (readedSize < g_blockLen) {
			m_sendState = SendState::SendLastData;
		}
	} else if (m_sendState == SendState::SendLastData) {
		//all data transfered successful
	}
}

/*
==================
lab1::OnReadyRead
==================
*/
void lab1::OnReadyRead() {
	if (m_recvState == SendState::SendHeader) {
		const QByteArray header = m_recvSocket->read(g_blockLen);
		const qint64 size = *reinterpret_cast<const qint64*>(header.data());
		const QString incomingFileName(header.mid(sizeof size));

		auto reply = QMessageBox::question(this, QString::fromLocal8Bit("Входящий файл"), QString::fromLocal8Bit("Принять файл ") + incomingFileName + " (" + QString::number(size) + QString::fromLocal8Bit(" байт) от ") + m_recvSocket->peerAddress().toString() + '?', QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
		if (reply == QMessageBox::StandardButton::Yes) {
			auto saveFileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Сохранить файл"), incomingFileName, QString::fromLocal8Bit("Все файлы (*.*)"));
			if (saveFileName.isEmpty()) {
				delete m_recvSocket;
				return;
			}
			
			m_fileToReceive.open(saveFileName.toStdString(), std::ios::binary);
			m_ui.progressBar->setMaximum(size);
			m_ui.progressBar->setEnabled(true);
			m_doneSize = 0;
			m_totalSize = size;

			m_recvSocket->write("OK");
			m_recvState = SendState::SendData;
		} else {
			delete m_recvSocket;
		}
	} else if (m_recvState == SendState::SendData) {
		static char readBuf[g_blockLen];

		const qint64 readedSize = m_recvSocket->read(readBuf, g_blockLen);

		m_fileToReceive.write(readBuf, readedSize);
		m_doneSize += readedSize;
		m_ui.progressBar->setValue(m_doneSize);
		
		if (m_doneSize == m_totalSize) {
			m_fileToReceive.close();
			delete m_recvSocket;
			m_recvState = SendState::SendHeader;
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
lab1::OnClientReadyRead
==================
*/
void lab1::OnClientReadyRead() {
	if (m_sendState == SendState::WaitAck) {
		QString ack = m_tcpSocket->read(g_blockLen);
		if (ack == "OK") {
			m_sendState = SendState::SendData;
			OnBytesWrite(0);
		}
	}
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
