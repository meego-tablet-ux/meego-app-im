/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at 
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#include "imchannelapprover.h"
#include <QApplication>
#include <QTimer>
#include <glib-object.h>

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    g_type_init();

    Tp::registerTypes();
    Tpy::registerTypes();

    bool autoApproveCalls = app.arguments().indexOf("--auto-approve-calls") > 0 ||
                            app.arguments().indexOf("-a") > 0;

    IMChannelApprover approver(autoApproveCalls);
    qDebug() << "Auto approve:" << autoApproveCalls;
    QTimer::singleShot(0, &approver, SLOT(registerApprover()));

    return app.exec();
}
