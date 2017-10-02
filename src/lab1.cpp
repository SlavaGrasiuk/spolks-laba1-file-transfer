#include <QIntValidator>
#include <QRegExpValidator>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include "DragFrame.hpp"
#include "lab1.h"


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

	m_tcpServer = new QTcpServer(this);
	m_tcpSocket = new QTcpSocket(this);

	connect(m_ui.startListenBtn, &QPushButton::clicked, this, &lab1::OnStartListen);
	connect(m_tcpServer, &QTcpServer::newConnection, this, &lab1::OnIncomingConnection);
	connect(m_tcpSocket, &QTcpSocket::connected, this, &lab1::OnConnected);
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
		msgBox.setText("Can't start server!");
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
	
}

/*
==================
lab1::OnConnected
==================
*/
void lab1::OnConnected() {
	QMessageBox msgBox;
	msgBox.setText("Connected!");
	msgBox.setIcon(QMessageBox::Icon::Information);
	msgBox.exec();
}

/*
==================
lab1::OnOpenFile
==================
*/
void lab1::OnOpenFile(std::string name) {
	m_fileToTransfer.open(name, std::ios::binary);
	if (m_fileToTransfer) {
		if (m_ui.protocolSelect->itemText(m_ui.protocolSelect->currentIndex()) == "TCP") {
			m_tcpSocket->connectToHost(m_ui.ipAddrEdit->text(), m_ui.portEdit->text().toShort());
		} else {

		}
	}
}
