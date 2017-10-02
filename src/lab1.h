#ifndef LAB1_H
#define LAB1_H

#include <QMainWindow>
#include <fstream>
#include "ui_lab1.h"


class DragFrame;
class QTcpServer;
class QTcpSocket;


class lab1 : public QMainWindow {
	Q_OBJECT

public:
	lab1(QWidget *parent = 0);
	~lab1();

private slots:
	void OnOpenFile(QString name);

private:
	Ui::lab1Window m_ui;
	std::ifstream m_fileToTransfer;
	DragFrame *m_dragFrame;
	QTcpServer *m_server;
	QTcpSocket *m_socket;
};

#endif // LAB1_H
