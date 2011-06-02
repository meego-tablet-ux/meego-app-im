/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include <QApplication>
#include "imchannelapprover.h"
#include <QTimer>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    Tp::registerTypes();
    Tpy::registerTypes();

    IMChannelApprover approver;
    QTimer::singleShot(0, &approver, SLOT(registerApprover()));

    return app.exec();
}
