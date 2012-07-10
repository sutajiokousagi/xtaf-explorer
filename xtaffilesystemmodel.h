#ifndef XTAFFILESYSTEMMODEL_H
#define XTAFFILESYSTEMMODEL_H

#include <QFileSystemModel>

#include "xtaffsys.h"

class XtafFileSystemModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit XtafFileSystemModel(QObject *parent = 0);
    void setReadOnly(bool enable);
    bool isReadOnly() const;

    QModelIndex index(int row, int column, const QModelIndex & parent = QModelIndex()) const;
    QModelIndex index(const QModelIndex & parent = QModelIndex()) const;
    QModelIndex parent(const QModelIndex & index) const;
    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool hasChildren(const QModelIndex & parent = QModelIndex()) const;
    Qt::ItemFlags flags(const QModelIndex & index) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const;

    int setXtafFilesystem(XtafFsys *fsys);
    int setPartitionNumber(int partNum);
    int reloadPartition();

private:
    XtafFsys *fsys;
    QModelIndex rootIndex;

signals:
    
public slots:
    
};

#endif // XTAFFILESYSTEMMODEL_H
