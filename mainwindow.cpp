#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    fsModel = NULL;

	ui->setupUi(this);

    // Add the "Select Partition" widgets to the toolbar
    partitionPickerLabel.setText(QString("Partition:"));
    ui->toolBar->addWidget(&partitionPickerLabel);
    ui->toolBar->addWidget(&partitionPicker);

    // Add a label to the status bar
    statusLabel.setText(QString(""));
    ui->statusBar->addWidget(&statusLabel);

    connect(&filePicker, SIGNAL(fileSelected(QString)),
			this, SLOT(loadImageFile(QString)));
    connect(&partitionPicker, SIGNAL(currentIndexChanged(int)),
            this, SLOT(selectNewPartition(int)));
    connect(ui->fsTree, SIGNAL(clicked(QModelIndex)),
            this, SLOT(selectFile(QModelIndex)));
	openLoadImagePanel();
}

MainWindow::~MainWindow()
{
	delete ui;
}

void MainWindow::openLoadImagePanel()
{
	filePicker.setNameFilter(tr("Disk Images (*.img; *.bin)"));
    if (!filePicker.exec())
		exit(0);
	return;
}

void MainWindow::loadImageFile(const QString &imagePath)
{
	imageFile.setFileName(imagePath);
	if (!imageFile.open(QIODevice::ReadOnly)) {
		openLoadImagePanel();
		return;
	}

    disk.setFile(imageFile);
    part.setDisk(&disk);
    part.setPartition(part.count()-1);
    fsys.setPartition(&part);

    if (fsModel)
        delete fsModel;
    fsModel = new XtafFileSystemModel;
    fsModel->setXtafFilesystem(&fsys);
    ui->fsTree->setModel(fsModel);

    partitionPicker.clear();
    for (unsigned int i=0; i<part.count(); i++)
        partitionPicker.addItem(part.name(i), QVariant(i));
    partitionPicker.setCurrentIndex(part.count()-1);

	ui->fsTree->header()->setStretchLastSection(false);
	ui->fsTree->header()->setResizeMode(0, QHeaderView::Stretch);

	return;
}

void MainWindow::selectNewPartition(int newPart)
{
    if (part.format(newPart) != XtafPart::XTAF) {
        qWarning() << "Format for partition" << newPart << "not recognized";
        return;
    }
	partitionPicker.setCurrentIndex(newPart);
    fsModel->setXtafFilesystem(&fsys);
    fsModel->setPartitionNumber(newPart);
}

void MainWindow::selectFile(QModelIndex index)
{
    if (!index.isValid()) {
        statusLabel.setText(QString(""));
        return;
    }

    XtafFile *file = static_cast<XtafFile *>(index.internalPointer());
    QString labelText;
    QTextStream(&labelText) << "Start clutser: " << file->startCluster();
    statusLabel.setText(labelText);
}

