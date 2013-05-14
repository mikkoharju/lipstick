#ifndef SWITCHERMODEL_H
#define SWITCHERMODEL_H

#include <QAbstractListModel>
#include "lipstickglobal.h"

class LipstickCompositor;
class LIPSTICK_EXPORT SwitcherModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(int itemCount READ itemCount NOTIFY itemCountChanged)

public:
    SwitcherModel();
    ~SwitcherModel();

    int itemCount() const;

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QHash<int, QByteArray> roleNames() const;

signals:
    void itemCountChanged();
    void itemAdded(int index);

private:
    friend class LipstickCompositor;
    void setCompositor(LipstickCompositor *);

    void addItem(int);
    void remItem(int);
    void titleChanged(int);
    QList<int> m_items;
};

#endif // SWITCHERMODEL_H
