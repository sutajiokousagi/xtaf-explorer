#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QFileDialog>

#include "xtafdisk.h"
#include "xtafpart.h"
#include "xtaffsys.h"
#include "XtafFileSystemModel.h"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void openLoadImagePanel();

private:
	Ui::MainWindow *ui;
	QFileDialog filePicker;
    QFile imageFile;
    XtafFileSystemModel *fsModel;
    XtafDisk disk;
    XtafPart part;
    XtafFsys fsys;


public slots:
		void loadImageFile(const QString &imagePath);
};

#endif // MAINWINDOW_H