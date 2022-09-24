#pragma once

#include <QtGlobal>

#if defined(WEBSERVER_LIBRARY)
#  define WEBSERVER_EXPORT Q_DECL_EXPORT
#else
#  define WEBSERVER_EXPORT Q_DECL_IMPORT
#endif
