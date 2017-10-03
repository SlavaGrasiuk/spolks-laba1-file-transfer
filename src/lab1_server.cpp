#include <QNetworkInterface>
#include <QFileDialog>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "lab1.h"


/*
==================
lab1::OnListenPortChanged
==================
*/
void lab1::OnListenPortChanged() {
	m_ui.startListenBtn->setEnabled(true);
}

/*
==================
lab1::OnListenProtocolChanged
==================
*/
void lab1::OnListenProtocolChanged(int index) {
	if (m_ui.listenProtocol->itemText(m_ui.listenProtocol->currentIndex()) == "TCP") {
		m_ui.udpListenAdress->setEnabled(false);
	} else {
		m_ui.udpListenAdress->setEnabled(true);
	}
	m_ui.startListenBtn->setEnabled(true);
}

/*
==================
lab1::OnStartListen
==================
*/
void lab1::OnStartListen() {
	if (m_tcpServer->isListening()) {
		m_tcpServer->close();
	}
	if (m_ui.listenProtocol->itemText(m_ui.listenProtocol->currentIndex()) == "TCP") {
		if (!m_tcpServer->listen(QHostAddress::SpecialAddress::Any, m_ui.listenPortEdit->text().toShort())) {
			QMessageBox msgBox;
			msgBox.setText(QString::fromLocal8Bit("�� ������� ��������� ������: ") + m_tcpServer->errorString());
			msgBox.setIcon(QMessageBox::Icon::Critical);
			msgBox.exec();
		} else {
			m_ui.startListenBtn->setEnabled(false);
		}
	} else {	//udp

	}
	
}

/*
==================
lab1::OnTCPReadyRead
==================
*/
void lab1::OnTCPReadyRead() {
	if (m_recvState == SendState::SendHeader) {
		const QByteArray header = m_recvSocket->read(g_blockLenTCP);
		const qint64 size = *reinterpret_cast<const qint64*>(header.data());
		const QString incomingFileName(header.mid(sizeof size));


		const auto addr = QHostAddress(m_recvSocket->peerAddress().toIPv4Address());
		const auto reply = QMessageBox::question(this, QString::fromLocal8Bit("�������� ����"), QString::fromLocal8Bit("������� ���� ") + incomingFileName + " (" + QString::number(size) + QString::fromLocal8Bit(" ����) �� ") + addr.toString() + '?', QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
		if (reply == QMessageBox::StandardButton::Yes) {
			const auto saveFileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("��������� ����"), incomingFileName, QString::fromLocal8Bit("��� ����� (*.*)"));
			if (saveFileName.isEmpty()) {
				m_recvSocket->close();
				return;
			}

			m_fileToReceive.open(saveFileName.toStdString(), std::ios::binary);
			InitReceive();
			m_ui.progressBar->setMaximum(size);
			m_doneSize = 0;
			m_totalSize = size;

			m_recvSocket->write("OK");
			m_recvState = SendState::SendData;
		} else {
			m_recvSocket->close();
		}
	} else if (m_recvState == SendState::SendData) {
		static char readBuf[g_blockLenTCP];

		const qint64 readedSize = m_recvSocket->read(readBuf, g_blockLenTCP);

		m_fileToReceive.write(readBuf, readedSize);
		m_doneSize += readedSize;
		m_ui.progressBar->setValue(m_doneSize);

		if (m_doneSize == m_totalSize) {		//File received
			ResetServer();
		}
	}
}

/*
==================
lab1::OnTCPHostLoseClient
==================
*/
void lab1::OnTCPHostLoseClient() {
	ResetServer();
}

/*
==================
lab1::OnTCPIncomingConnection

	Receive starts from here.
==================
*/
void lab1::OnTCPIncomingConnection() {
	m_recvSocket = m_tcpServer->nextPendingConnection();
	m_recvState = SendState::SendHeader;
	connect(m_recvSocket, &QTcpSocket::readyRead, this, &lab1::OnTCPReadyRead);
	connect(m_recvSocket, &QTcpSocket::disconnected, this, &lab1::OnTCPHostLoseClient);
}

/*
==================
lab1::ResetServer
==================
*/
void lab1::ResetServer() {
	m_fileToReceive.close();
	m_recvSocket->close();
	m_doneSize = m_totalSize = 0;
	DisableCancelAndPB();
	m_ui.endpointTypeTab->setEnabled(true);
	m_ui.progressBar->setValue(0);
	m_recvState = SendState::SendHeader;
}

/*
==================
lab1::InitReceive
==================
*/
void lab1::InitReceive() {
	EnableCancelAndPB();
	m_ui.endpointTypeTab->setEnabled(false);
	m_endpRole = EndpointRole::receive;
}
