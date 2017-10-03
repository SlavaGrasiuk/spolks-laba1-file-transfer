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
	if (!m_tcpServer->listen(QHostAddress::SpecialAddress::Any, m_ui.listenPortEdit->text().toShort())) {
		QMessageBox msgBox;
		msgBox.setText(QString::fromLocal8Bit("Не удалось запустить сервер: ") + m_tcpServer->errorString());
		msgBox.setIcon(QMessageBox::Icon::Critical);
		msgBox.exec();
	} else {
		m_ui.startListenBtn->setEnabled(false);
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


		const auto addr = QHostAddress(m_recvSocket->peerAddress().toIPv4Address());
		const auto reply = QMessageBox::question(this, QString::fromLocal8Bit("Входящий файл"), QString::fromLocal8Bit("Принять файл ") + incomingFileName + " (" + QString::number(size) + QString::fromLocal8Bit(" байт) от ") + addr.toString() + '?', QMessageBox::StandardButton::Yes | QMessageBox::StandardButton::No);
		if (reply == QMessageBox::StandardButton::Yes) {
			const auto saveFileName = QFileDialog::getSaveFileName(this, QString::fromLocal8Bit("Сохранить файл"), incomingFileName, QString::fromLocal8Bit("Все файлы (*.*)"));
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
		static char readBuf[g_blockLen];

		const qint64 readedSize = m_recvSocket->read(readBuf, g_blockLen);

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
lab1::OnHostLoseClient
==================
*/
void lab1::OnHostLoseClient() {
	ResetServer();
}

/*
==================
lab1::OnIncomingConnection

	Receive starts from here.
==================
*/
void lab1::OnIncomingConnection() {
	m_recvSocket = m_tcpServer->nextPendingConnection();
	m_recvState = SendState::SendHeader;
	connect(m_recvSocket, &QTcpSocket::readyRead, this, &lab1::OnReadyRead);
	connect(m_recvSocket, &QTcpSocket::disconnected, this, &lab1::OnHostLoseClient);
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
