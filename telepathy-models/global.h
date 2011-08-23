/*
 * Copyright 2011 Intel Corporation.
 *
 * This program is licensed under the terms and conditions of the
 * Apache License, version 2.0.  The full text of the Apache License is at
 * http://www.apache.org/licenses/LICENSE-2.0
 */

#ifndef _Telepathy_Models_global_h_HEADER_GUARD_
#define _Telepathy_Models_global_h_HEADER_GUARD_

#include <QtGlobal>

#  define TELEPATHY_MODELS_EXPORT Q_DECL_EXPORT

#if !defined(Q_OS_WIN) && defined(QT_VISIBILITY_AVAILABLE)
#  define TELEPATHY_MODELS_NO_EXPORT __attribute__((visibility("hidden")))
#endif

#ifndef TELEPATHY_MODELS_NO_EXPORT
#  define TELEPATHY_MODELS_NO_EXPORT
#endif

#endif
