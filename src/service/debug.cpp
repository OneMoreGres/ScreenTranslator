#include "debug.h"

#include <QDateTime>
#include <QFileInfo>
#include <QMessageBox>
#include <QMutex>
#include <QThread>

namespace
{
QtMessageHandler original = nullptr;
QMutex mutex;
QFile file;
QTextStream stream;

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

  SOFT_ASSERT(original, return );
  original(type, context, msg);

  QMutexLocker locker(&mutex);
  file.write(message);
}
}  // namespace

namespace debug
{
std::atomic_bool isTrace = false;

QString traceFileName()
{
  QMutexLocker locker(&mutex);
  return file.fileName();
}

bool setTraceFileName(const QString &fileName)
{
  QMutexLocker locker(&mutex);

  original = nullptr;
  qInstallMessageHandler(nullptr);

  if (file.isOpen())
    file.close();

  if (fileName.isEmpty())
    return true;

  file.setFileName(fileName);

  if (!file.open(QFile::WriteOnly | QFile::Unbuffered))
    return false;

  original = qInstallMessageHandler(handler);
  return true;
}

}  // namespace debug
