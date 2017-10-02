#include <QIntValidator>
#include <QRegExpValidator>
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

	m_socket = new QTcpSocket(this);
}

/*
==================
lab1::~lab1
==================
*/
lab1::~lab1() {
	delete m_socket;
	delete m_dragFrame;
}

/*
==================
lab1::OnOpenFile
==================
*/
void lab1::OnOpenFile(QString name) {
	__asm {nop};
}
