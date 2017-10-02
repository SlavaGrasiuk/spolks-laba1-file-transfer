#pragma once


#include <QFrame>
#include <QObject>


class DragFrame : public QFrame {
	Q_OBJECT

public:
	DragFrame(QWidget *parent = 0);

signals:
	void GetFile(QString name);

protected:
	void dragEnterEvent(QDragEnterEvent *event) override;
	void dropEvent(QDropEvent *event) override;
};
