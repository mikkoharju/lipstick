#ifndef SWITCHERPIXMAPITEM_H
#define SWITCHERPIXMAPITEM_H

#include <QQuickItem>
#include "lipstickglobal.h"

class LipstickCompositor;
class LipstickCompositorWindowReference;
class LIPSTICK_EXPORT SwitcherPixmapItem : public QQuickItem
{
    Q_OBJECT
    Q_PROPERTY(int windowId READ windowId WRITE setWindowId NOTIFY windowIdChanged)

public:
    SwitcherPixmapItem();
    ~SwitcherPixmapItem();

    int windowId() const;
    void setWindowId(int);

protected:
    QSGNode *updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *);

signals:
    void windowIdChanged();

private:
    void updateItem();

    LipstickCompositorWindowReference *m_item;
    int m_id;
};

#endif // SWITCHERPIXMAPITEM_H
