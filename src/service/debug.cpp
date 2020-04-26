#include "debug.h"

#include <QDateTime>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutex>
#include <QThread>

#ifdef Q_OS_WIN
#include <io.h>
#else
#include <unistd.h>
#endif

namespace
{
QMutex mutex;
QString fileName;
int realStdout{};
int realStderr{};
FILE *logFile{};

void handler(QtMsgType type, const QMessageLogContext &context,
             const QString &msg)
{
  const auto typeName = QMap<QtMsgType, QByteArray>{{QtDebugMsg, " DEBUG "},
                                                    {QtInfoMsg, " INFO "},
                                                    {QtWarningMsg, " WARN "},
                                                    {QtCriticalMsg, " CRIT "},
                                                    {QtFatalMsg, " FATAL "}}
                            .value(type);

  const auto message =
      QDateTime::currentDateTime().toString(Qt::ISODate).toUtf8() + ' ' +
      QByteArray::number(qintptr(QThread::currentThreadId())) + ' ' +
      QFileInfo(context.file).fileName().toUtf8() + ':' +
      QByteArray::number(context.line) + typeName + msg.toUtf8() + '\n';

  if (logFile)
    write(fileno(logFile), message.data(), message.size());
  if (realStderr > 0)
    write(realStderr, message.data(), message.size());
}

void toDefaults()
{
  qInstallMessageHandler(nullptr);

  if (realStdout > 0) {
    dup2(realStdout, fileno(stdout));
    realStdout = -1;
  }

  if (realStderr > 0) {
    dup2(realStderr, fileno(stderr));
    realStderr = -1;
  }

  if (logFile) {
    fclose(logFile);
    logFile = nullptr;
  }

  fileName.clear();
}
}  // namespace

namespace debug
{
std::atomic_bool isTrace = false;

QString traceFileName()
{
  QMutexLocker locker(&mutex);
  return fileName;
}

bool setTraceFileName(const QString &fileName)
{
  QMutexLocker locker(&mutex);

  toDefaults();

  if (fileName.isEmpty())
    return true;

  logFile = fopen(qPrintable(fileName), "w");
  if (!logFile)
    return false;

  realStdout = dup(fileno(stdout));
  realStderr = dup(fileno(stderr));

  const auto fd = fileno(logFile);
  dup2(fd, fileno(stdout));
  dup2(fd, fileno(stderr));

  ::fileName = fileName;
  qInstallMessageHandler(handler);

  return true;
}

}  // namespace debug
