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


/*
==================
lab1::lab1
==================
*/
lab1::lab1(QWidget *parent): QMainWindow(parent) {
	m_ui.setupUi(this);

	m_dragFrame = new DragFrame(m_ui.TransmitTab);
	m_dragFrame->setGeometry(QRect(200, 10, 181, 91));
	m_dragFrame->setAcceptDrops(true);
	connect(m_dragFrame, &DragFrame::GetFile, this, &lab1::OnClientOpenFile);

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
	connect(m_ui.cancelBtn, &QPushButton::clicked, this, &lab1::OnCancelClick);

	connect(m_tcpServer, &QTcpServer::newConnection, this, &lab1::OnIncomingConnection);
	connect(m_ui.listenPortEdit, &QLineEdit::textChanged, this, &lab1::OnListenPortChanged);
	connect(m_ui.listenProtocol, static_cast<void(QComboBox::*)(int)>(&QComboBox::currentIndexChanged), this, &lab1::OnListenProtocolChanged);

	connect(m_tcpSocket, &QTcpSocket::connected, this, &lab1::OnClientConnectedToHost);
	connect(m_tcpSocket, &QTcpSocket::bytesWritten, this, &lab1::OnClientWriteBytesToHost);
	connect(m_tcpSocket, &QTcpSocket::disconnected, this, &lab1::OnClientDisconectedFromHost);
	connect(m_tcpSocket, &QTcpSocket::readyRead, this, &lab1::OnClientReadyRead);

	m_sendState = m_recvState = SendState::SendHeader;
	m_recvSocket = nullptr;
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
lab1::GetFileSize

	Returns opened file size in bytes.
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
lab1::OnCancelClick
==================
*/
void lab1::OnCancelClick() {
	if (m_endpRole == EndpointRole::receive) {
		ResetServer();
	} else {
		ResetClient();
	}
}

/*
==================
lab1::EnableCancelAndPB
==================
*/
void lab1::EnableCancelAndPB() {
	m_ui.progressBar->setEnabled(true);
	m_ui.cancelBtn->setEnabled(true);
}

/*
==================
lab1::DisableCancelAndPB
==================
*/
void lab1::DisableCancelAndPB() {
	m_ui.progressBar->setEnabled(false);
	m_ui.cancelBtn->setEnabled(false);
}
