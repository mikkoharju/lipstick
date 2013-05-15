#include "lipstickapi.h"

#include "homeapplication.h"
#include "compositor/lipstickcompositor.h"

LipstickApi::LipstickApi(QObject *parent)
: QObject(parent)
{
    QObject::connect(HomeApplication::instance(), SIGNAL(homeActiveChanged()), this, SIGNAL(activeChanged()));
}

bool LipstickApi::active() const
{
    return HomeApplication::instance()->homeActive();
}

QObject *LipstickApi::compositor() const
{
    return LipstickCompositor::instance();
}
