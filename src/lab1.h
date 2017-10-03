#ifndef LAB1_H
#define LAB1_H

#include <QMainWindow>
#include <QFileInfo>
#include <fstream>
#include "ui_lab1.h"


class DragFrame;
class QTcpServer;
class QTcpSocket;
class QUdpSocket;
class QTimer;


constexpr qint64 g_blockLenTCP = 32'768;		//'bytes (single brace in this comment are strongly necessary)


class lab1 : public QMainWindow {
	Q_OBJECT

	enum class SendState {
		SendHeader,
		WaitAck,
		SendData,
		SendLastData
	};

	enum EndpointRole {
		receive,
		transmit
	};

public:
	lab1(QWidget *parent = 0);
	~lab1();

private slots:
	void OnCancelClick();
	void OnClientOpenFile(std::string name);
	void OnStartListen();
	void OnTCPIncomingConnection();
	void OnTCPClientConnectedToHost();
	void OnTCPClientWriteBytesToHost(qint64 bytes);
	void OnTCPReadyRead();
	void OnTCPHostLoseClient();
	void OnTCPClientDisconectedFromHost();
	void OnTCPClientReadyRead();
	void OnListenPortChanged();
	void OnListenProtocolChanged(int index);

private:
	Ui::lab1Window m_ui;
	std::ifstream m_fileToTransfer;
	std::ofstream m_fileToReceive;
	QFileInfo m_fileInfo;
	qint64 m_totalSize, m_doneSize;
	DragFrame *m_dragFrame;
	QTcpServer *m_tcpServer;
	QTcpSocket *m_tcpSocket, *m_recvSocket;
	QUdpSocket *m_udpSocket;
	SendState m_sendState, m_recvState;
	EndpointRole m_endpRole;
	QTimer *m_udpTimer;

private:
	qint64 GetFileSize();
	void ResetClient();
	void ResetServer();
	void EnableCancelAndPB();
	void DisableCancelAndPB();
	void InitTransmit();
	void InitReceive();
};

#endif // LAB1_H
