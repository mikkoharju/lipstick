/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** Copyright (C) 2012 Jolla Ltd.
** Contact: Robin Burchell <robin.burchell@jollamobile.com>
**
** This file is part of lipstick.
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/
#include <QtTest/QtTest>
#include <QQuickView>
#include <QQmlContext>
#include <QScreen>
#include "shutdownscreen.h"
#include "homeapplication.h"
#include "ut_shutdownscreen.h"
#include "notificationmanager_stub.h"
#include "closeeventeater_stub.h"

QList<QQuickView *> qQuickViews;
void QQuickView::setSource(const QUrl &)
{
    qQuickViews.append(this);
}

QHash<QWindow *, bool> qWindowVisible;
void QWindow::show()
{
    qWindowVisible[this] = true;
}
void QWindow::hide()
{
    qWindowVisible[this] = false;
}

HomeApplication::~HomeApplication()
{
}

HomeApplication *HomeApplication::instance()
{
    return 0;
}

bool signalHandlersRestored = false;
void HomeApplication::restoreSignalHandlers()
{
    signalHandlersRestored = true;
}

void HomeApplication::sendStartupNotifications()
{
}

int argc = 1;
char *argv[] = { (char *) "./ut_shutdownscreen", NULL };

void Ut_ShutdownScreen::initTestCase()
{
}

void Ut_ShutdownScreen::cleanupTestCase()
{
}

void Ut_ShutdownScreen::init()
{
    shutdownScreen = new ShutdownScreen;

    gNotificationManagerStub->stubReset();
    gNotificationManagerStub->stubSetReturnValue("Notify", (uint)1);
}

void Ut_ShutdownScreen::cleanup()
{
    delete shutdownScreen;
    qQuickViews.clear();
    qWindowVisible.clear();
    gNotificationManagerStub->stubReset();
}

void Ut_ShutdownScreen::testConnections()
{
    QCOMPARE(disconnect(shutdownScreen->systemState, SIGNAL(systemStateChanged(MeeGo::QmSystemState::StateIndication)), shutdownScreen, SLOT(applySystemState(MeeGo::QmSystemState::StateIndication))), true);
}

void Ut_ShutdownScreen::testSystemState()
{
    QSignalSpy spy(shutdownScreen, SIGNAL(windowVisibleChanged()));
    shutdownScreen->applySystemState(MeeGo::QmSystemState::ThermalStateFatal);
    QCOMPARE(qQuickViews.count(), 0);
    QCOMPARE(gNotificationManagerStub->stubCallCount("Notify"), 1);
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_CATEGORY).toString(), QString("x-nemo.battery.temperature"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_PREVIEW_BODY).toString(), qtTrId("qtn_shut_high_temp"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QString>(2), QString());

    shutdownScreen->applySystemState(MeeGo::QmSystemState::ShutdownDeniedUSB);
    QCOMPARE(qQuickViews.count(), 0);
    QCOMPARE(gNotificationManagerStub->stubCallCount("Notify"), 2);
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_CATEGORY).toString(), QString("device.added"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_PREVIEW_BODY).toString(), qtTrId("qtn_shut_unplug_usb"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QString>(2), QString());

    shutdownScreen->applySystemState(MeeGo::QmSystemState::BatteryStateEmpty);
    QCOMPARE(qQuickViews.count(), 0);
    QCOMPARE(gNotificationManagerStub->stubCallCount("Notify"), 3);
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_CATEGORY).toString(), QString("x-nemo.battery.shutdown"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QVariantHash>(6).value(NotificationManager::HINT_PREVIEW_BODY).toString(), qtTrId("qtn_shut_batt_empty"));
    QCOMPARE(gNotificationManagerStub->stubLastCallTo("Notify").parameter<QString>(2), QString());

    shutdownScreen->applySystemState(MeeGo::QmSystemState::Shutdown);
    QCOMPARE(qQuickViews.count(), 1);

    // Check window properties
    /*
    QCOMPARE(qQuickViews.first()->testAttribute(Qt::WA_TranslucentBackground), true);
    QCOMPARE(qQuickViews.first()->testAttribute(Qt::WA_X11DoNotAcceptFocus), true);
    QCOMPARE(qQuickViews.first()->testAttribute(Qt::WA_X11NetWmWindowTypeNotification), true);
    QCOMPARE(qQuickViews.first()->viewport()->autoFillBackground(), false);
    */
    QCOMPARE(qQuickViews.first()->title(), QString("Shutdown"));
    QCOMPARE(qQuickViews.first()->resizeMode(), QQuickView::SizeRootObjectToView);
    QCOMPARE(qQuickViews.first()->rootContext()->contextProperty("initialSize").toSize(), QGuiApplication::primaryScreen()->size());
    QCOMPARE(qQuickViews.first()->rootContext()->contextProperty("shutdownScreen"), QVariant::fromValue(static_cast<QObject *>(shutdownScreen)));

    // Check that the window was shown
    QCOMPARE(qWindowVisible[static_cast<QWindow *>(qQuickViews.first())], true);
    QCOMPARE(spy.count(), 1);
}

QTEST_MAIN (Ut_ShutdownScreen)
