#ifndef LIB3DS_QT_GLOBAL_H
#define LIB3DS_QT_GLOBAL_H

#include <QtCore/qglobal.h>
#include "gl_check_macro.h"

#if defined(LIB3DS_QT_LIBRARY)
#  define LIB3DS_QTSHARED_EXPORT Q_DECL_EXPORT
#else
#  define LIB3DS_QTSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // LIB3DS_QT_GLOBAL_H
