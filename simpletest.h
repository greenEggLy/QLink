#pragma once

#ifndef SIMPLETEST_H
#define SIMPLETEST_H

#include <QObject>
#include <QtTest/QtTest>
#include "gamearea.h"

class SimpleTest : public QObject
{
    Q_OBJECT
public:
    explicit SimpleTest(QObject *parent = nullptr);

private slots:
    void testLink();
};

#endif // SIMPLETEST_H
