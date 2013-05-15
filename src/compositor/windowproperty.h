#ifndef WINDOWPROPERTY_H
#define WINDOWPROPERTY_H

#include <QObject>
#include <QPointer>
#include <QVariant>
#include "lipstickglobal.h"

class QWaylandSurface;

class LIPSTICK_EXPORT WindowProperty : public QObject
{
    Q_OBJECT
    Q_PROPERTY(int windowId READ windowId WRITE setWindowId NOTIFY windowIdChanged)
    Q_PROPERTY(QString property READ property WRITE setProperty NOTIFY propertyChanged)
    Q_PROPERTY(QVariant value READ value NOTIFY valueChanged)
public:
    WindowProperty();

    int windowId() const;
    void setWindowId(int);

    QString property() const;
    void setProperty(const QString &);

    QVariant value();

signals:
    void windowIdChanged();
    void propertyChanged();
    void valueChanged();

private slots:
    void availableWinIdsChanged();
    void windowPropertyChanged(const QString &);

private:
    int m_windowId;
    bool m_waitingRefProperty;
    void connectRef();
    void disconnectRef();
    QString m_property;
    QPointer<QWaylandSurface> m_surface;
};

#endif // WINDOWPROPERTY_H
