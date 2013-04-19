#include <QDebug>
#include "xtaffilesystemmodel.h"



XtafFileSystemModel::XtafFileSystemModel(QObject *parent) :
    QAbstractItemModel(parent)
{
}

QModelIndex XtafFileSystemModel::index(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
    return rootIndex;
}

QModelIndex XtafFileSystemModel::index(int row, int column, const QModelIndex & parent) const
{
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    if (!parent.isValid())
        return createIndex(row, column, fsys->rootDirectory().entries()[row]);

    XtafFile *file = static_cast<XtafFile *>(parent.internalPointer());
    if (!file)
        qWarning() << "Index is NULL";
    XtafDirectory *dir = (XtafDirectory *)file;
    return createIndex(row, column, dir->entries()[row]);
}

QModelIndex XtafFileSystemModel::parent(const QModelIndex & index) const
{
    if (!index.isValid())
        return QModelIndex();

    XtafFile *file = static_cast<XtafFile *>(index.internalPointer());
    if (!file)
        qWarning() << "Parent is NULL";
    XtafDirectory *parentDir = (XtafDirectory *)file->parent();

    if (parentDir == &(fsys->rootDirectory()))
        return QModelIndex();

    int i;
    for (i=0; i<parentDir->entries().size(); i++) {
        if (parentDir->entries().at(i) == file)
            return createIndex(i, 0, parentDir);
    }

    return QModelIndex();
}

int XtafFileSystemModel::rowCount(const QModelIndex & parent) const
{
    XtafFile *file;
    XtafDirectory *dir;
    if (!fsys) {
        qDebug() << "No filesystem object";
        return 0;
    }

    if (parent.column() > 0)
        return 0;

    file = static_cast<XtafFile *>(parent.internalPointer());
    if (!file) {
        return fsys->rootDirectory().entries().size();
    }

    if (!file->isDirectory()) {
        return 0;
    }

	dir = (XtafDirectory *)file;
    return dir->entries().size();
}

int XtafFileSystemModel::columnCount(const QModelIndex & parent) const
{
	Q_UNUSED(parent);
	return 5;
}

QVariant XtafFileSystemModel::data(const QModelIndex & index, int role) const
{
    XtafFile *f = static_cast<XtafFile *>(index.internalPointer());
    if (!f)
        qWarning() << "Data model is NULL";

    if (role == Qt::SizeHintRole) {
		if (index.column() == 1)
            return QVariant(QSize(400,24));
        else
			return QVariant(QSize(50,24));
    }

    else if (role == Qt::DisplayRole) {
        if (index.column() == 0)
            return QVariant(QString(f->name()));
        if (index.column() == 1)
            return QVariant(f->size());
        if (index.column() == 2)
            return QVariant(f->created().toString(Qt::SystemLocaleShortDate));
        if (index.column() == 3)
            return QVariant(f->updated().toString(Qt::SystemLocaleShortDate));
        if (index.column() == 4)
            return QVariant(f->accessed().toString(Qt::SystemLocaleShortDate));
        return QVariant(QString("???"));
    }

    else if (role == Qt::DecorationRole) {
        return QVariant();
    }

    else if (role == Qt::ToolTipRole) {
        return QVariant();
    }

    else if (role == Qt::FontRole)
        return QVariant();

    else if (role == Qt::TextAlignmentRole)
        return QVariant();

    else if (role == Qt::BackgroundColorRole) {
        if (f->isDeleted())
            return QColor::fromRgb(255,0,0);
        return QVariant();
    }

    else if (role == Qt::TextColorRole)
        return QVariant();

    else if (role == Qt::CheckStateRole)
        return QVariant();

	else if (role == Qt::StatusTipRole)
		return QVariant();

    qDebug() << "Unknown item role" << role;
    return QVariant();
}

int XtafFileSystemModel::setXtafFilesystem(XtafFsys *xtafFsys)
{
    fsys = xtafFsys;
	emit reset();
    return 0;
}

int XtafFileSystemModel::setPartitionNumber(int partNum)
{
    fsys->setPartition(partNum);
	emit reset();
    return 0;
}


bool XtafFileSystemModel::hasChildren(const QModelIndex & parent) const
{
    // Special-case for root object
    if (!parent.isValid())
        return true;
    XtafFile *f = static_cast<XtafFile *>(parent.internalPointer());
    return f->isDirectory();
}

Qt::ItemFlags XtafFileSystemModel::flags(const QModelIndex & index) const
{
	Q_UNUSED(index);
	return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

QVariant XtafFileSystemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
	Q_UNUSED(orientation);

	if (role == Qt::SizeHintRole) {
        if (section == 0)
            return QVariant(QSize(400,24));
        else
			return QVariant(QSize(50,24));
    }

    else if (role == Qt::DisplayRole) {
        if (section == 0)
            return QVariant(QString("Name"));
        else if (section == 1)
            return QVariant(QString("Size"));
        else if (section == 2)
            return QVariant(QString("Created"));
        else if (section == 3)
            return QVariant(QString("Modified"));
        else if (section == 4)
            return QVariant(QString("Accessed"));
        else
            return QVariant(QString("????"));
    }

//    qDebug() << "Unknown item role" << role;
    return QVariant();
}

