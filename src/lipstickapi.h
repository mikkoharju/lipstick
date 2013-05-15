#ifndef LIPSTICKAPI_H
#define LIPSTICKAPI_H

#include <QObject>
#include "lipstickglobal.h"

class LIPSTICK_EXPORT LipstickApi : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool active READ active NOTIFY activeChanged)
    Q_PROPERTY(QObject *compositor READ compositor CONSTANT)

public:
    LipstickApi(QObject *parent = 0);

    bool active() const;
    QObject *compositor() const;

signals:
    void activeChanged();
};

#endif // LIPSTICKAPI_H
