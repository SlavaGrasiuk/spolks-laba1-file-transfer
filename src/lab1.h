#ifndef LAB1_H
#define LAB1_H

#include <QMainWindow>
#include <QFileInfo>
#include <fstream>
#include "ui_lab1.h"


class DragFrame;
class QTcpServer;
class QTcpSocket;


class lab1 : public QMainWindow {
	Q_OBJECT

	enum class SendState {
		SendHeader,
		WaitAck,
		SendData,
		SendLastData
	};

public:
	lab1(QWidget *parent = 0);
	~lab1();

private slots:
	void OnOpenFile(std::string name);
	void OnStartListen();
	void OnIncomingConnection();
	void OnConnected();
	void OnBytesWrite(qint64 bytes);
	void OnReadyRead();

private:
	Ui::lab1Window m_ui;
	std::ifstream m_fileToTransfer;
	QFileInfo m_fileInfo;
	DragFrame *m_dragFrame;
	QTcpServer *m_tcpServer;
	QTcpSocket *m_tcpSocket;
	SendState m_sendState;

private:
	qint64 GetFileSize();
};

#endif // LAB1_H
