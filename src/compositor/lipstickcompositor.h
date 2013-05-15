#ifndef LIPSTICKCOMPOSITOR_H
#define LIPSTICKCOMPOSITOR_H

#include <QQuickWindow>
#include "lipstickglobal.h"
#include <QQmlParserStatus>
#include <QWaylandCompositor>
#include <QWaylandSurfaceItem>

class LipstickCompositorWindow;
class SwitcherModel;
class LIPSTICK_EXPORT LipstickCompositor : public QQuickWindow, public QWaylandCompositor, public QQmlParserStatus
{
    Q_OBJECT
    Q_INTERFACES(QQmlParserStatus)

    Q_PROPERTY(int windowCount READ windowCount NOTIFY windowCountChanged)
    Q_PROPERTY(int ghostWindowCount READ ghostWindowCount NOTIFY ghostWindowCountChanged)
    Q_PROPERTY(bool homeActive READ homeActive WRITE setHomeActive NOTIFY homeActiveChanged)

public:
    LipstickCompositor();
    ~LipstickCompositor();

    static LipstickCompositor *instance();

    virtual void classBegin();
    virtual void componentComplete();
    virtual void surfaceCreated(QWaylandSurface *surface);

    int windowCount() const;
    int ghostWindowCount() const;

    bool homeActive() const;
    void setHomeActive(bool);

    Q_INVOKABLE QObject *windowForId(int) const;
    Q_INVOKABLE void clearKeyboardFocus();

signals:
    void windowAdded(QObject *window);
    void windowRemoved(QObject *window);

    void windowCountChanged();
    void ghostWindowCountChanged();

    void availableWinIdsChanged();

    void homeActiveChanged();

private slots:
    void surfaceDestroyed();
    void surfaceMapped();
    void surfaceUnmapped();
    void surfaceSizeChanged();
    void surfaceTitleChanged();
    void windowSwapped();
    void windowDestroyed();

private:
    friend class LipstickCompositorWindow;
    friend class SwitcherModel;
    friend class SwitcherPixmapItem;
    friend class WindowProperty;

    QWaylandSurface *surfaceForId(int) const;
    int windowIdForLink(QWaylandSurface *, uint) const;

    void surfaceUnmapped(QWaylandSurface *);
    void coverAdded(int);
    void coverRemoved(int);

    static LipstickCompositor *m_instance;

    int m_totalWindowCount;
    QHash<QWaylandSurface *, LipstickCompositorWindow *> m_mappedSurfaces;

    int m_nextWindowId;
    QHash<QWaylandSurface *, int> m_coverSurfaces;
    QHash<int, QWaylandSurface *> m_coverSurfaceIds;
    QList<SwitcherModel *> m_switcherModels;

    bool m_homeActive;
};

class LipstickCompositorWindowReference : public QWaylandSurfaceItem
{
    Q_OBJECT
public:
    LipstickCompositorWindowReference(QWaylandSurface *surface, QQuickItem *parent = 0);

    void addref();
    void release();

private:
    int m_refCount;
};

class LipstickCompositorWindow : public QWaylandSurfaceItem
{
    Q_OBJECT

    Q_PROPERTY(int windowId READ windowId CONSTANT)
    Q_PROPERTY(QVariant userData READ userData WRITE setUserData NOTIFY userDataChanged)

public:
    LipstickCompositorWindow(int windowId, QWaylandSurface *surface, QQuickItem *parent = 0);

    QVariant userData() const;
    void setUserData(QVariant);

    int windowId() const;

public slots:
    void releaseWindow();

signals:
    void userDataChanged();

private:
    int m_windowId;
    QVariant m_data;
};

#endif // LIPSTICKCOMPOSITOR_H
