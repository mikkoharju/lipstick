#include "switcherpixmapitem.h"

#include <QSGGeometryNode>
#include <QSGSimpleMaterial>
#include <QWaylandSurfaceItem>
#include "lipstickcompositor.h"

namespace {

class SurfaceTextureState {
public:
    SurfaceTextureState() : m_texture(0) {}
    void setTexture(QSGTexture *texture) { m_texture = texture; }
    QSGTexture *texture() const { return m_texture; }

private:
    QSGTexture *m_texture;
};

class SurfaceTextureMaterial : public QSGSimpleMaterialShader<SurfaceTextureState>
{
    QSG_DECLARE_SIMPLE_SHADER(SurfaceTextureMaterial, SurfaceTextureState)
public:
    QList<QByteArray> attributes() const;
    void updateState(const SurfaceTextureState *newState, const SurfaceTextureState *oldState);
protected:
    const char *vertexShader() const;
    const char *fragmentShader() const;
};

class SurfaceNode : public QObject, public QSGGeometryNode
{
    Q_OBJECT
public:
    SurfaceNode();
    void setRect(const QRectF &);
    void setTextureProvider(QSGTextureProvider *);

private slots:
    void providerDestroyed();
    void textureChanged();

private:
    void setTexture(QSGTexture *texture);

    QSGSimpleMaterial<SurfaceTextureState> *m_material;
    QRectF m_rect;

    QSGTextureProvider *m_provider;
    QSGTexture *m_texture;
    QSGGeometry m_geometry;
};

QList<QByteArray> SurfaceTextureMaterial::attributes() const
{
    QList<QByteArray> attributeList;
    attributeList << "qt_VertexPosition";
    attributeList << "qt_VertexTexCoord";
    return attributeList;
}

void SurfaceTextureMaterial::updateState(const SurfaceTextureState *newState,
                                         const SurfaceTextureState *)
{
    if (newState->texture())
        newState->texture()->bind();
}

const char *SurfaceTextureMaterial::vertexShader() const
{
    return "uniform highp mat4 qt_Matrix;                      \n"
           "attribute highp vec4 qt_VertexPosition;            \n"
           "attribute highp vec2 qt_VertexTexCoord;            \n"
           "varying highp vec2 qt_TexCoord;                    \n"
           "void main() {                                      \n"
           "    qt_TexCoord = qt_VertexTexCoord;               \n"
           "    gl_Position = qt_Matrix * qt_VertexPosition;   \n"
           "}";
}

const char *SurfaceTextureMaterial::fragmentShader() const
{
    return "varying highp vec2 qt_TexCoord;                    \n"
           "uniform sampler2D qt_Texture;                      \n"
           "uniform lowp float qt_Opacity;                     \n"
           "void main() {                                      \n"
           "    gl_FragColor = texture2D(qt_Texture, qt_TexCoord) * qt_Opacity; \n"
           "}";
}

SurfaceNode::SurfaceNode()
: m_material(0), m_provider(0), m_texture(0), m_geometry(QSGGeometry::defaultAttributes_TexturedPoint2D(), 4)
{
    setGeometry(&m_geometry);
    m_material = SurfaceTextureMaterial::createMaterial();
    setMaterial(m_material);
}

void SurfaceNode::setRect(const QRectF &r)
{
    if (m_rect == r)
        return;

    m_rect = r;

    if (m_texture) {
        QSize ts = m_texture->textureSize();
        QRectF sourceRect(0, 0, ts.width(), ts.height());
        QSGGeometry::updateTexturedRectGeometry(&m_geometry, m_rect,
                                                m_texture->convertToNormalizedSourceRect(sourceRect));
    }
}

void SurfaceNode::setTextureProvider(QSGTextureProvider *p)
{
    if (p == m_provider)
        return;

    if (m_provider) {
        QObject::disconnect(m_provider, SIGNAL(destroyed(QObject *)), this, SLOT(providerDestroyed()));
        QObject::disconnect(m_provider, SIGNAL(textureChanged()), this, SLOT(textureChanged()));
        m_provider = 0;
    }

    m_provider = p;

    QObject::connect(m_provider, SIGNAL(destroyed(QObject *)), this, SLOT(providerDestroyed()));
    QObject::connect(m_provider, SIGNAL(textureChanged()), this, SLOT(textureChanged()));

    setTexture(m_provider->texture());
}

void SurfaceNode::setTexture(QSGTexture *texture)
{
    m_material->state()->setTexture(texture);

    m_texture = texture;

    if (m_texture) {
        QSize ts = m_texture->textureSize();
        QRectF sourceRect(0, 0, ts.width(), ts.height());
        QSGGeometry::updateTexturedRectGeometry(&m_geometry, m_rect,
                                                m_texture->convertToNormalizedSourceRect(sourceRect));
    }

    markDirty(DirtyMaterial);
}

void SurfaceNode::textureChanged()
{
    setTexture(m_provider->texture());
}

void SurfaceNode::providerDestroyed()
{
    m_provider = 0;
    setTexture(0);
}

}

SwitcherPixmapItem::SwitcherPixmapItem()
: m_item(0)
{
    setFlag(ItemHasContents);
}

SwitcherPixmapItem::~SwitcherPixmapItem()
{
    setWindowId(0);
}

int SwitcherPixmapItem::windowId() const
{
    return m_id;
}

void SwitcherPixmapItem::setWindowId(int id)
{
    if (m_id == id)
        return;
    
    if (m_item) {
        m_item->release();
        m_item = 0;
    }

    m_id = id;
    updateItem();

    emit windowIdChanged();
}

QSGNode *SwitcherPixmapItem::updatePaintNode(QSGNode *oldNode, UpdatePaintNodeData *)
{
    SurfaceNode *node = static_cast<SurfaceNode *>(oldNode);

    if (m_item == 0) {
        delete node;
        return 0;
    }

    if (!node) node = new SurfaceNode;

    node->setTextureProvider(m_item->textureProvider());
    node->setRect(QRectF(0, 0, width(), height()));

    return node;
}

void SwitcherPixmapItem::updateItem()
{
    LipstickCompositor *c = LipstickCompositor::instance();
    if (c && m_id) {
        QWaylandSurface *s = c->m_coverSurfaceIds.value(m_id);
        m_item = static_cast<LipstickCompositorWindowReference *>(s->surfaceItem());
        m_item->addref();
        update();
    }
}

#include "switcherpixmapitem.moc"
