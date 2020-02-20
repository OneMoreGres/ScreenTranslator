#pragma once

#include <QDebug>

#define SOFT_ASSERT(XXX, WORKAROUND)                                         \
  if (!(XXX)) {                                                              \
    qCritical() << "Soft assertion failed at" << __FILE__ << __LINE__ << ":" \
                << #XXX;                                                     \
    WORKAROUND;                                                              \
  }

#define ASSERT(XXX)                                                     \
  if (!(XXX)) {                                                         \
    qCritical() << "Assertion failed at" << __FILE__ << __LINE__ << ":" \
                << #XXX;                                                \
    Q_ASSERT(XXX);                                                      \
  }

#define ASSERT_X(XXX, CONTEXT)                                          \
  if (!(XXX)) {                                                         \
    qCritical() << "Assertion failed at" << __FILE__ << __LINE__ << ":" \
                << #XXX << "Context (" << #CONTEXT << ")" << CONTEXT;   \
    Q_ASSERT(XXX);                                                      \
  }

namespace debug
{
extern std::atomic_bool isTrace;
};

#define LTRACE()      \
  if (debug::isTrace) \
  qDebug()

#define LTRACE_IF(XXX)       \
  if (debug::isTrace && XXX) \
  qDebug()

#define LDEBUG() qDebug()

#define LDEBUG_IF(XXX) \
  if (XXX)             \
  qDebug()

#define LWARNING() qWarning()

#define LWARNING_IF(XXX) \
  if (XXX)               \
  qWarning()

#define LERROR() qCritical()

#define LERROR_IF(XXX) \
  if (XXX)             \
  qCritical()

#define LINFO() qInfo()

#define LINFO_IF(XXX) \
  if (XXX)            \
  qInfo()

#define LFATAL() qFatal

#define LFATAL_IF(XXX) \
  if (XXX)             \
  qFatal

#define LARG(XXX) #XXX "=" << XXX

#define LARG_N(NAME, XXX) NAME << '=' << XXX
