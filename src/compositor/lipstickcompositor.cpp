#include "lipstickcompositor.h"

#include <QDebug>
#include <QQmlEngine>
#include <QQmlContext>
#include "switchermodel.h"
#include <QWaylandSurface>
#include "homeapplication.h"

LipstickCompositor *LipstickCompositor::m_instance = 0;

LipstickCompositor::LipstickCompositor()
: m_totalWindowCount(0), m_nextWindowId(1)
{
    if (m_instance) qFatal("LipstickCompositor: Only one compositor instance per process is supported");
    m_instance = this;

    if (HomeApplication::instance()) {
        HomeApplication::instance()->engine()->rootContext()->setContextProperty("windowCompositor", this);
    } else {
        qWarning("LipstickCompositor: Compositor registered before HomeApplication.  This compositor instance will not be accessible to application.");
    }

    QObject::connect(this, SIGNAL(frameSwapped()), this, SLOT(windowSwapped()));
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

int LipstickCompositor::windowIdForCoverId(int id) const
{
    QHash<int, QWaylandSurface *>::ConstIterator iter = m_coverSurfaceIds.find(id);
    if (iter == m_coverSurfaceIds.end())
        return 0;

    QWaylandSurface *coverSurface = *iter;

    uint localWindowId = coverSurface->windowProperties().value("SAILFISH_COVER_WINDOW", uint(0)).toUInt();

    for (QHash<QWaylandSurface *, LipstickCompositorWindow *>::ConstIterator iter = m_mappedSurfaces.begin();
         iter != m_mappedSurfaces.end(); ++iter) {

        QWaylandSurface *windowSurface = iter.key();

        if (windowSurface->processId() == coverSurface->processId() &&
            windowSurface->windowProperties().value("SAILFISH_WINID", uint(0)).toUInt() == localWindowId)
            return iter.value()->windowId();
    }

    return 0;
}

QObject *LipstickCompositor::windowForCoverId(int id) const
{
    QHash<int, QWaylandSurface *>::ConstIterator iter = m_coverSurfaceIds.find(id);
    if (iter == m_coverSurfaceIds.end())
        return 0;

    QWaylandSurface *coverSurface = *iter;

    uint localWindowId = coverSurface->windowProperties().value("SAILFISH_COVER_WINDOW", uint(0)).toUInt();

    for (QHash<QWaylandSurface *, LipstickCompositorWindow *>::ConstIterator iter = m_mappedSurfaces.begin();
         iter != m_mappedSurfaces.end(); ++iter) {

        QWaylandSurface *windowSurface = iter.key();

        if (windowSurface->processId() == coverSurface->processId() &&
            windowSurface->windowProperties().value("SAILFISH_WINID", uint(0)).toUInt() == localWindowId)
            return iter.value();
    }

    return 0;
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
    } else if (m_coverSurfaces.contains(surface)) {
        int id = m_coverSurfaces[surface];
        m_coverSurfaces.remove(surface);
        m_coverSurfaceIds.remove(id);

        static_cast<LipstickCompositorWindowReference *>(surface->surfaceItem())->release();

        coverRemoved(id);
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

