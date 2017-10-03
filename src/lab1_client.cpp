#include <QFileDialog>
#include <QMessageBox>
#include <QTcpServer>
#include <QTcpSocket>
#include <QByteArray>
#include "lab1.h"



/*
==================
lab1::OnTCPClientWriteBytesToHost
==================
*/
void lab1::OnTCPClientWriteBytesToHost(qint64 bytes) {
	static char buffer[g_blockLenTCP];

	if (m_sendState == SendState::SendData) {
		m_fileToTransfer.read(buffer, g_blockLenTCP);
		const qint64 readedSize = m_fileToTransfer.gcount();
		m_doneSize += readedSize;

		m_tcpSocket->write(buffer, readedSize);
		m_ui.progressBar->setValue(m_doneSize);

		if (m_doneSize == m_totalSize) {
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
lab1::OnTCPClientDisconectedFromHost
==================
*/
void lab1::OnTCPClientDisconectedFromHost() {
	ResetClient();
}

/*
==================
lab1::OnTCPClientReadyRead

	Server has received header and send to us acknowlege. Start file transfer part by part.
==================
*/
void lab1::OnTCPClientReadyRead() {
	if (m_sendState == SendState::WaitAck) {
		const QString ack = m_tcpSocket->read(g_blockLenTCP);
		if (ack == "OK") {
			m_sendState = SendState::SendData;		//next - OnTCPClientWriteBytesToHost
			OnTCPClientWriteBytesToHost(0);
		}
	}
}

/*
==================
lab1::OnTCPClientConnectedToHost

	Successful connected to server. Now send header - file size and name.
==================
*/
void lab1::OnTCPClientConnectedToHost() {
	if (m_sendState == SendState::SendHeader) {
		InitTransmit();
		m_totalSize = GetFileSize();
		m_doneSize = 0;
		m_ui.progressBar->setMaximum(m_totalSize);

		QByteArray header;
		header.append(reinterpret_cast<const char*>(&m_totalSize), sizeof m_totalSize);
		header.append(m_fileInfo.fileName());

		m_tcpSocket->write(header);

		m_sendState = SendState::WaitAck;	//next - OnTCPClientReadyRead
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
			m_tcpSocket->connectToHost(m_ui.ipAddrEdit->text(), m_ui.portEdit->text().toShort());	//next - OnTCPClientConnectedToHost
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
