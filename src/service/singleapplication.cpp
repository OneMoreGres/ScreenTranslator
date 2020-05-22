#include "singleapplication.h"
#include "debug.h"

#include <QCoreApplication>
#include <QDir>
#include <QStandardPaths>

static QString fileName(const QString &baseName)
{
  const auto name = !baseName.isEmpty()
                        ? baseName
                        : QCoreApplication::applicationName().toLower();
  SOFT_ASSERT(!name.isEmpty(), return QStringLiteral("./dummy.lock"));
  return QStandardPaths::writableLocation(QStandardPaths::TempLocation) +
         QDir::separator() + name + QLatin1String(".lock");
}

namespace service
{
SingleApplication::SingleApplication(const QString &baseName)
  : lockFile_(fileName(baseName))
{
  lockFile_.setStaleLockTime(0);

  if (!lockFile_.tryLock()) {
    const auto lockName = fileName(baseName);
    LERROR() << QObject::tr("Another instance is running. Lock file is busy.")
             << LARG(lockName);
  }
}

bool SingleApplication::isValid() const
{
  return lockFile_.isLocked();
}

}  // namespace service
