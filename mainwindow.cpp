#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    fsModel = NULL;

	ui->setupUi(this);
    connect(&filePicker, SIGNAL(fileSelected(QString)),
			this, SLOT(loadImageFile(QString)));

	openLoadImagePanel();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openLoadImagePanel()
{
    filePicker.setNameFilter(tr("Disk Images (*.img)"));
    if (!filePicker.exec())
		exit(0);
	return;
}

void MainWindow::loadImageFile(const QString &imagePath)
{
	qDebug() << "Loading image file:" << imagePath;
	imageFile.setFileName(imagePath);
	if (!imageFile.open(QIODevice::ReadOnly)) {
		openLoadImagePanel();
		return;
	}

    disk.setFile(imageFile);
    part.setDisk(&disk);
    part.setPartition(6);
    fsys.setPartition(&part);

    if (fsModel)
        delete fsModel;
    fsModel = new XtafFileSystemModel;
    fsModel->setXtafFilesystem(&fsys);
    ui->fsTree->setModel(fsModel);

	return;
}
