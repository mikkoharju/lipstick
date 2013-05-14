#include "switchermodel.h"

#include "lipstickcompositor.h"

SwitcherModel::SwitcherModel()
{
    LipstickCompositor *c = LipstickCompositor::instance();
    if (!c) {
        qWarning("SwitcherModel: Compositor must be created before SwitcherModel");
    } else {
        c->m_switcherModels.append(this);
        if (c->m_coverSurfaceIds.count())
            m_items = c->m_coverSurfaceIds.keys();
    }
}

SwitcherModel::~SwitcherModel()
{
    LipstickCompositor *c = LipstickCompositor::instance();
    if (c) c->m_switcherModels.removeAll(this);
}

int SwitcherModel::itemCount() const
{
    return m_items.count();
}

int SwitcherModel::rowCount(const QModelIndex &) const
{
    return m_items.count();
}

QVariant SwitcherModel::data(const QModelIndex &index, int role) const
{
    int idx = index.row();
    if (idx < 0 || idx >= m_items.count())
        return QVariant();

    LipstickCompositor *c = LipstickCompositor::instance();
    if (role == Qt::UserRole + 1) {
        return m_items.at(idx);
    } else if (role == Qt::UserRole + 2) {
        return c->m_coverSurfaceIds[m_items.at(idx)]->processId();
    } else if (role == Qt::UserRole + 3) {
        return c->m_coverSurfaceIds[m_items.at(idx)]->title();
    } else {
        return QVariant();
    }
}

QHash<int, QByteArray> SwitcherModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[Qt::UserRole + 1] = "window";
    roles[Qt::UserRole + 2] = "processId";
    roles[Qt::UserRole + 3] = "title";
    return roles;
}

void SwitcherModel::addItem(int id)
{
    beginInsertRows(QModelIndex(), m_items.count(), m_items.count());
    m_items.append(id);
    endInsertRows();
    emit itemAdded(m_items.count() - 1);
    emit itemCountChanged();
}

void SwitcherModel::remItem(int id)
{
    int idx = m_items.indexOf(id);
    if (idx == -1)
        return;

    beginRemoveRows(QModelIndex(), idx, idx);
    m_items.removeAt(idx);
    endRemoveRows();
    emit itemCountChanged();
}

void SwitcherModel::titleChanged(int id)
{
    int idx = m_items.indexOf(id);
    if (idx == -1)
        return;

    emit dataChanged(index(idx, 0), index(idx, 0));
}
