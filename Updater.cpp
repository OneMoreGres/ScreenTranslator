#include <QDir>
#include <QFile>
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QApplication>

#include "Updater.h"
#include "StAssert.h"

namespace {
#define FIELD(NAME) const QString _ ## NAME = #NAME
  FIELD (Application);

  FIELD (name);
  FIELD (version);
  FIELD (compatibleVersion);
  FIELD (built_in);
  FIELD (versionString);
  FIELD (permissions);
  FIELD (url);
  FIELD (path);
#undef FIELD

#if defined(Q_OS_WIN)
  const QString _platform = "_win";
#elif defined(Q_OS_LINUX)
  const QString _platform = "_linux";
#endif

  QString versionField (const QJsonObject &component, const QString &field) {
    return component.contains (field + _platform)
           ? component[field + _platform].toVariant ().toString ()
           : component[field].toVariant ().toString ();
  }

  QFileInfo fileDir (const QString &fileName) {
    return QFileInfo (fileName).absolutePath ();
  }
}

Updater::Updater (QObject *parent)
  : QObject (parent),
  network_ (new QNetworkAccessManager (this)),
  componentsUpdating_ (0) {
  updatesFileName_ = QApplication::applicationDirPath () + QDir::separator () + "updates.json";
  backupSuffix_ = "_backup";
  connect (network_, SIGNAL (finished (QNetworkReply *)),
           SLOT (replyFinished (QNetworkReply *)));

  getCurrentVersion ();
  updateCurrentVersion ();
}

QDateTime Updater::nextCheckTime (const QDateTime &lastCheckTime, int updateType) const {
  QDateTime nextTime;
  switch (updateType) {
    case UpdateTypeDaily:
      nextTime = lastCheckTime.addDays (1);
      break;
    case UpdateTypeWeekly:
      nextTime =  lastCheckTime.addDays (7);
      break;
    case UpdateTypeMonthly:
      nextTime =  lastCheckTime.addDays (30);
      break;
    case UpdateTypeNever:
    default:
      return QDateTime ();
  }
  if (nextTime < QDateTime::currentDateTime ()) {
    return QDateTime::currentDateTime ().addSecs (5);
  }
  return nextTime;
}

void Updater::getCurrentVersion () {
  QFile f (":/version.json");
  if (f.open (QFile::ReadOnly)) {
    currentVersion_ = QJsonDocument::fromJson (f.readAll ()).object ();
    f.close ();
  }
  else {
    emit error (tr ("Ошибка определения текущей версии. Обновление недоступно."));
  }
}

void Updater::updateCurrentVersion () {
  QFile f (updatesFileName_);
  if (!f.open (QFile::ReadOnly)) {
    return;
  }
  QJsonObject updated = QJsonDocument::fromJson (f.readAll ()).object ();
  f.close ();
  foreach (const QString &component, updated.keys ()) {
    QJsonObject current = currentVersion_[component].toObject ();
    int updatedVersion = updated[component].toInt ();
    if (current[_built_in].toBool () || current[_version].toInt () >= updatedVersion) {
      continue;
    }
    current[_version] = updatedVersion;
    currentVersion_[component] = current;
  }
}

QString Updater::currentAppVersion () const {
  return currentVersion_[_Application].toObject ()[_versionString].toString ();
}

void Updater::checkForUpdates () {
  getAvailableVersion ();
}

void Updater::getAvailableVersion () {
  QNetworkRequest request (versionField (currentVersion_, _url));
  request.setAttribute (QNetworkRequest::User, _version);
  network_->get (request);
}

void Updater::replyFinished (QNetworkReply *reply) {
  if (reply->error () != QNetworkReply::NoError) {
    emit tr ("Ошибка загрузки информации для обновления.");
    return;
  }
  QByteArray content = reply->readAll ();
  QString component = reply->request ().attribute (QNetworkRequest::User).toString ();
  if (component == _version) {
    availableVersion_ = QJsonDocument::fromJson (content).object ();
    parseAvailableVersion ();
  }
  else if (availableVersion_.contains (component) && !content.isEmpty ()) {
    installComponent (component, content);
  }
  reply->deleteLater ();
}

void Updater::parseAvailableVersion () {
  QStringList inaccessible, incompatible;
  QStringList updateList;
  QDir currentDir;
  foreach (const QString &component, availableVersion_.keys ()) {
    QJsonObject available = availableVersion_[component].toObject ();
    QJsonObject current = currentVersion_[component].toObject ();
    QString path = versionField (available, _path);
    if (path.isEmpty ()) {
      continue;
    }

    QFileInfo installDir = fileDir (path);
    if (!installDir.exists ()) {
      currentDir.mkpath (installDir.absoluteFilePath ());
    }
    if (!installDir.isWritable ()) { // check dir because install = remove + make new
      inaccessible << installDir.absoluteFilePath ();
    }
    if (current[_version].toInt () < available[_compatibleVersion].toInt ()) {
      incompatible << component;
    }
    if (!QFile::exists (path) || current[_version].toInt () < available[_version].toInt ()) {
      updateList << component;
    }
  }
  if (updateList.isEmpty ()) {
    return;
  }

  QFileInfo updateFileDir = fileDir (updatesFileName_);
  if (!updateFileDir.isWritable ()) {
    inaccessible << updateFileDir.absoluteFilePath ();
  }
  inaccessible.removeDuplicates ();

  QString message = tr ("Доступно обновлений: %1.\n").arg (updateList.size ());
  QMessageBox::StandardButtons buttons = QMessageBox::Ok;
  if (!incompatible.isEmpty ()) {
    message += tr ("Несовместимых обновлений: %1.\nВыполните обновление вручную.")
               .arg (incompatible.size ());
  }
  else if (!inaccessible.isEmpty ()) {
    message += tr ("Недоступных для записи директорий: %1.\n%2\nИзмените права доступа и "
                   "повторите попытку или выполните обновление вручную.")
               .arg (inaccessible.size ()).arg (inaccessible.join ("\n"));
  }
  else {
    message += tr ("Обновить?");
    buttons = QMessageBox::Yes | QMessageBox::No;
  }
  int result = QMessageBox::question (NULL, tr ("Обновление"), message, buttons);
  if (result == QMessageBox::Yes) {
    componentsUpdating_ = updateList.size ();
    foreach (const QString &component, updateList) {
      getComponent (component);
    }
  }
}

void Updater::getComponent (const QString &component) {
  QJsonObject available = availableVersion_[component].toObject ();
  QString path = versionField (available, _path);
  if (path.isEmpty ()) {
    --componentsUpdating_;
    return;
  }

  QString url = versionField (available, _url);
  if (url.isEmpty ()) { // just remove component
    installComponent (component, QByteArray ());
  }
  else {
    QNetworkRequest request (url);
    request.setAttribute (QNetworkRequest::User, component);
    network_->get (request);
  }
}

void Updater::installComponent (const QString &component, const QByteArray &newContent) {
  --componentsUpdating_;
  ST_ASSERT (availableVersion_.contains (component));
  QJsonObject available = availableVersion_[component].toObject ();
  QString path = versionField (available, _path);
  ST_ASSERT (!path.isEmpty ());

  QString backup = path + backupSuffix_;
  QFile::remove (backup);
  QFile::rename (path, backup);

  if (!newContent.isEmpty ()) {
    QFile f (path);
    if (!f.open (QFile::WriteOnly)) {
      emit error (tr ("Ошибка обновления файла (%1).").arg (path));
      return;
    }
    f.write (newContent);
    f.close ();
    bool ok;
    QFileDevice::Permissions perm (available[_permissions].toString ().toUInt (&ok, 16));
    if (ok) {
      f.setPermissions (perm);
    }
  }
  updateVersionInfo (component, available[_version].toInt ());

  if (componentsUpdating_ == 0) {
    emit updated ();
    QString message = tr ("Обновление завершено. Для активации некоторых компонентов "
                          "может потребоваться перезапуск.");
    QMessageBox::information (NULL, tr ("Обновление"), message, QMessageBox::Ok);
  }
}

void Updater::updateVersionInfo (const QString &component, int version) {
  QFile f (updatesFileName_);
  if (!f.open (QFile::ReadWrite)) {
    emit error (tr ("Ошибка обновления файла с текущей версией."));
    return;
  }
  QJsonObject updated = QJsonDocument::fromJson (f.readAll ()).object ();
  updated[component] = version;
  f.seek (0);
  f.write (QJsonDocument (updated).toJson ());
  f.close ();
}
