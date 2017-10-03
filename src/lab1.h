#ifndef LAB1_H
#define LAB1_H

#include <QMainWindow>
#include <QFileInfo>
#include <fstream>
#include "ui_lab1.h"


class DragFrame;
class QTcpServer;
class QTcpSocket;


constexpr qint64 g_blockLen = 32'768;		//'bytes (single brace in this comment are strongly necessary)


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
	void OnIncomingConnection();
	void OnClientConnectedToHost();
	void OnClientWriteBytesToHost(qint64 bytes);
	void OnReadyRead();
	void OnHostLoseClient();
	void OnClientDisconectedFromHost();
	void OnClientReadyRead();
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
	SendState m_sendState, m_recvState;
	EndpointRole m_endpRole;

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
