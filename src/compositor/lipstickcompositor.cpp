#include "lipstickcompositor.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>
#include "switchermodel.h"
#include <QWaylandSurface>
#include "homeapplication.h"
#include <QWaylandInputDevice>

LipstickCompositor *LipstickCompositor::m_instance = 0;

LipstickCompositor::LipstickCompositor()
: m_totalWindowCount(0), m_nextWindowId(1), m_homeActive(true)
{
    if (m_instance) qFatal("LipstickCompositor: Only one compositor instance per process is supported");
    m_instance = this;

    QObject::connect(this, SIGNAL(frameSwapped()), this, SLOT(windowSwapped()));

    emit HomeApplication::instance()->_activeChanged();
}

LipstickCompositor::~LipstickCompositor()
{
}

LipstickCompositor *LipstickCompositor::instance()
{
    return m_instance;
}

void LipstickCompositor::classBegin()
{
}

void LipstickCompositor::componentComplete()
{
    QWaylandCompositor::setOutputGeometry(QRect(0, 0, width(), height()));
}

void LipstickCompositor::surfaceCreated(QWaylandSurface *surface)
{
    Q_UNUSED(surface)
    connect(surface, SIGNAL(destroyed(QObject*)), this, SLOT(surfaceDestroyed()));
    connect(surface, SIGNAL(mapped()), this, SLOT(surfaceMapped()));
    connect(surface, SIGNAL(unmapped()), this, SLOT(surfaceUnmapped()));
    connect(surface, SIGNAL(sizeChanged()), this, SLOT(surfaceSizeChanged()));
    connect(surface, SIGNAL(titleChanged()), this, SLOT(surfaceTitleChanged()));
}

int LipstickCompositor::windowCount() const
{
    return m_mappedSurfaces.count();
}

int LipstickCompositor::ghostWindowCount() const
{
    return m_totalWindowCount - windowCount();
}

bool LipstickCompositor::homeActive() const
{
    return m_homeActive;
}

void LipstickCompositor::setHomeActive(bool a)
{
    if (a == m_homeActive)
        return;

    m_homeActive = a;

    emit homeActiveChanged();
    emit HomeApplication::instance()->_activeChanged();
}

QObject *LipstickCompositor::windowForId(int id) const
{
    if (m_coverSurfaceIds.contains(id))
        return 0;

    for (QHash<QWaylandSurface *, LipstickCompositorWindow *>::ConstIterator iter = m_mappedSurfaces.begin();
         iter != m_mappedSurfaces.end(); ++iter) {

        if ((*iter)->windowId() == id)
            return *iter;
    }

    return 0;
}

QWaylandSurface *LipstickCompositor::surfaceForId(int id) const
{
    if (m_coverSurfaceIds.contains(id))
        return m_coverSurfaceIds[id];

    for (QHash<QWaylandSurface *, LipstickCompositorWindow *>::ConstIterator iter = m_mappedSurfaces.begin();
         iter != m_mappedSurfaces.end(); ++iter) {

        if ((*iter)->windowId() == id)
            return iter.key();
    }

    return 0;
}

int LipstickCompositor::windowIdForLink(QWaylandSurface *s, uint link) const
{
    for (QHash<QWaylandSurface *, LipstickCompositorWindow *>::ConstIterator iter = m_mappedSurfaces.begin();
         iter != m_mappedSurfaces.end(); ++iter) {

        QWaylandSurface *windowSurface = iter.key();

        if (windowSurface->processId() == s->processId() &&
            windowSurface->windowProperties().value("SAILFISH_WINID", uint(0)).toUInt() == link)
            return iter.value()->windowId();
    }

    return 0;
}

void LipstickCompositor::clearKeyboardFocus()
{
    defaultInputDevice()->setKeyboardFocus(0); 
}

void LipstickCompositor::surfaceDestroyed()
{
    surfaceUnmapped(static_cast<QWaylandSurface *>(sender()));
}

void LipstickCompositor::surfaceMapped()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    //Ignore surface if it's not a window surface
    if (!surface->hasShellSurface())
        return;

    QVariantMap properties = surface->windowProperties();
    if (properties.value("SAILFISH_IS_COVER", false) == QVariant(true)) {
        Q_ASSERT(surface->surfaceItem() == 0);

        int id = m_nextWindowId++;
        m_coverSurfaces.insert(surface, id);
        m_coverSurfaceIds.insert(id, surface);

        LipstickCompositorWindowReference *r = new LipstickCompositorWindowReference(surface, contentItem());
        r->setVisible(false);

        coverAdded(id);
    } else if (!m_mappedSurfaces.contains(surface)) {
        Q_ASSERT(surface->surfaceItem() == 0);

        int id = m_nextWindowId++;

        LipstickCompositorWindow *item = new LipstickCompositorWindow(id, surface, contentItem());
        item->setSize(surface->size());
        QObject::connect(item, SIGNAL(destroyed(QObject*)), this, SLOT(windowDestroyed()));
        m_totalWindowCount++;
        m_mappedSurfaces.insert(surface, item);

        item->setTouchEventsEnabled(true);

        emit windowCountChanged();
        emit windowAdded(item);
    }

    emit availableWinIdsChanged();
}

void LipstickCompositor::surfaceUnmapped()
{
    surfaceUnmapped(qobject_cast<QWaylandSurface *>(sender()));
}

void LipstickCompositor::surfaceSizeChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    if (m_mappedSurfaces.contains(surface))
        m_mappedSurfaces[surface]->setSize(surface->size());
}

void LipstickCompositor::surfaceTitleChanged()
{
    QWaylandSurface *surface = qobject_cast<QWaylandSurface *>(sender());
    if (int id = m_coverSurfaces.value(surface, 0)) {
        for (int ii = 0; ii < m_switcherModels.count(); ++ii)
            m_switcherModels.at(ii)->titleChanged(id);
    }
}

void LipstickCompositor::windowSwapped()
{
    frameFinished();
}

void LipstickCompositor::windowDestroyed()
{
    m_totalWindowCount--;
    emit ghostWindowCountChanged();
}

void LipstickCompositor::surfaceUnmapped(QWaylandSurface *surface)
{
    if (m_mappedSurfaces.contains(surface)) {
        QQuickItem *item = surface->surfaceItem();
        int gc = ghostWindowCount();
        m_mappedSurfaces.remove(surface);

        emit windowCountChanged();
        emit windowRemoved(item);
        if (gc != ghostWindowCount())
            emit ghostWindowCountChanged();

        emit availableWinIdsChanged();
    } else if (m_coverSurfaces.contains(surface)) {
        int id = m_coverSurfaces[surface];
        m_coverSurfaces.remove(surface);
        m_coverSurfaceIds.remove(id);

        static_cast<LipstickCompositorWindowReference *>(surface->surfaceItem())->release();

        coverRemoved(id);

        emit availableWinIdsChanged();
    }
}

void LipstickCompositor::coverAdded(int id)
{
    for (int ii = 0; ii < m_switcherModels.count(); ++ii)
        m_switcherModels.at(ii)->addItem(id);
}

void LipstickCompositor::coverRemoved(int id)
{
    for (int ii = 0; ii < m_switcherModels.count(); ++ii)
        m_switcherModels.at(ii)->remItem(id);
}

LipstickCompositorWindow::LipstickCompositorWindow(int windowId, QWaylandSurface *surface, QQuickItem *parent)
: QWaylandSurfaceItem(surface, parent), m_windowId(windowId)
{
}

QVariant LipstickCompositorWindow::userData() const
{
    return m_data;
}

void LipstickCompositorWindow::setUserData(QVariant data)
{
    if (m_data == data)
        return;

    m_data = data;
    emit userDataChanged();
}

int LipstickCompositorWindow::windowId() const
{
    return m_windowId;
}

void LipstickCompositorWindow::releaseWindow()
{
    deleteLater();
}

LipstickCompositorWindowReference::LipstickCompositorWindowReference(QWaylandSurface *surface, QQuickItem *parent)
: QWaylandSurfaceItem(surface, parent), m_refCount(0)
{
}

void LipstickCompositorWindowReference::addref()
{
    ++m_refCount;
}

void LipstickCompositorWindowReference::release()
{
    if (0 == --m_refCount)
        delete this;
}

