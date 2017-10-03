#include <QFileDialog>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "lab1.h"



/*
==================
lab1::OnClientWriteBytesToHost
==================
*/
void lab1::OnClientWriteBytesToHost(qint64 bytes) {
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
		QMessageBox msgBox;
		msgBox.setText(QString::fromLocal8Bit("Файл передан успешно!"));
		msgBox.setIcon(QMessageBox::Icon::Information);
		msgBox.exec();
	}
}

/*
==================
lab1::OnClientDisconectedFromHost
==================
*/
void lab1::OnClientDisconectedFromHost() {
	ResetClient();
}

/*
==================
lab1::OnClientReadyRead

	Server has received header and send to us acknowlege. Start file transfer part by part.
==================
*/
void lab1::OnClientReadyRead() {
	if (m_sendState == SendState::WaitAck) {
		const QString ack = m_tcpSocket->read(g_blockLen);
		if (ack == "OK") {
			m_sendState = SendState::SendData;		//next - OnClientWriteBytesToHost
			OnClientWriteBytesToHost(0);
		}
	}
}

/*
==================
lab1::OnClientConnectedToHost

	Successful connected to server. Now send header - file size and name.
==================
*/
void lab1::OnClientConnectedToHost() {
	if (m_sendState == SendState::SendHeader) {
		InitTransmit();
		m_totalSize = GetFileSize();
		m_doneSize = 0;
		m_ui.progressBar->setMaximum(m_totalSize);

		QByteArray header;
		header.append(reinterpret_cast<const char*>(&m_totalSize), sizeof m_totalSize);
		header.append(m_fileInfo.fileName());

		m_tcpSocket->write(header);

		m_sendState = SendState::WaitAck;	//next - OnClientReadyRead
	}
}

/*
==================
lab1::OnClientOpenFile

	Transmit start from here.
==================
*/
void lab1::OnClientOpenFile(std::string name) {
	m_fileToTransfer.open(name, std::ios::binary);

	if (m_fileToTransfer) {
		m_fileInfo = QString::fromStdString(name);
		if (m_ui.protocolSelect->itemText(m_ui.protocolSelect->currentIndex()) == "TCP") {
			m_tcpSocket->connectToHost(m_ui.ipAddrEdit->text(), m_ui.portEdit->text().toShort());	//next - OnClientConnectedToHost
			if (!m_tcpSocket->waitForConnected()) {
				QMessageBox msgBox;
				msgBox.setText(QString::fromLocal8Bit("Ошибка подключения к серверу: ") + m_tcpSocket->errorString());
				msgBox.setIcon(QMessageBox::Icon::Critical);
				msgBox.exec();
				ResetClient();
			}
		} else {	//udp

		}
	}
}

/*
==================
lab1::ResetClient

	Set all client obects to default state.
==================
*/
void lab1::ResetClient() {
	m_fileToTransfer.close();
	m_totalSize = m_doneSize = 0;
	m_ui.endpointTypeTab->setEnabled(true);
	DisableCancelAndPB();
	m_ui.progressBar->setValue(0);
	if(m_tcpSocket->state() == QAbstractSocket::ConnectedState) {
		m_tcpSocket->close();
	}
	m_sendState = SendState::SendHeader;
}

/*
==================
lab1::InitTransmit
==================
*/
void lab1::InitTransmit() {
	m_ui.endpointTypeTab->setEnabled(false);
	EnableCancelAndPB();
	m_endpRole = EndpointRole::transmit;
}
