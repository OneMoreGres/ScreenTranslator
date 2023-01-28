#include "updates.h"
#include "debug.h"

#include <QApplication>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QScopeGuard>
#include <QSortFilterProxyModel>
#include <QTemporaryFile>
#include <QTimer>
#include <QTreeView>

#include <random>

#define MINIZ_NO_ZLIB_APIS
#define MINIZ_NO_ZLIB_COMPATIBLE_NAMES
#define MINIZ_NO_MALLOC
#define MINIZ_NO_STDIO
#define MINIZ_NO_ARCHIVE_WRITING_APIS
#include <miniz/miniz.h>

static QByteArray unpack(const QByteArray &data)
{
  if (data.size() <= 4 || data.left(2) != "PK") {
    LTRACE() << "Incorrect data to unpack" << LARG(data.size())
             << data.left(10);
    return {};
  }

  mz_zip_archive zip;
  memset(&zip, 0, sizeof(zip));
  if (!mz_zip_reader_init_mem(&zip, data.data(), data.size(), 0)) {
    LTRACE() << "Failed to init zip reader for " << data.left(10)
             << mz_zip_get_error_string(zip.m_last_error);
    return {};
  }

  const auto guard = qScopeGuard([&zip] { mz_zip_reader_end(&zip); });

  const auto fileCount = mz_zip_reader_get_num_files(&zip);
  if (fileCount < 1) {
    LTRACE() << "No files in zip archive";
    return {};
  }

  for (auto i = 0u; i < fileCount; ++i) {
    mz_zip_archive_file_stat file_stat;
    if (!mz_zip_reader_file_stat(&zip, i, &file_stat)) {
      LTRACE() << "Failed to get file info" << LARG(i)
               << mz_zip_get_error_string(zip.m_last_error);
      return {};
    }

    if (file_stat.m_is_directory)
      continue;

    QByteArray result(file_stat.m_uncomp_size, 0);
    mz_zip_reader_extract_to_mem(&zip, 0, result.data(), result.size(), 0);
    return result;
  }

  return {};
}

namespace update
{
namespace
{
const auto versionKey = "version";
const auto filesKey = "files";

QString sizeString(qint64 bytes, int precision)
{
  if (bytes < 1)
    return {};

  const auto kb = 1024.0;
  const auto mb = 1024 * kb;
  const auto gb = 1024 * mb;
  const auto tb = 1024 * gb;

  if (bytes >= tb) {
    return QString::number(bytes / tb, 'f', precision) + ' ' +
           QApplication::translate("Updates", "Tb");
  }
  if (bytes >= gb) {
    return QString::number(bytes / gb, 'f', precision) + ' ' +
           QApplication::translate("Updates", "Gb");
  }
  if (bytes >= mb) {
    return QString::number(bytes / mb, 'f', precision) + ' ' +
           QApplication::translate("Updates", "Mb");
  }
  if (bytes >= kb) {
    return QString::number(bytes / kb, 'f', precision) + ' ' +
           QApplication::translate("Updates", "Kb");
  }
  return QString::number(bytes) + ' ' +
         QApplication::translate("Updates", "bytes");
}

QString toString(State state)
{
  const QMap<State, QString> names{
      {State::NotAvailable, {}},
      {State::NotInstalled,
       QApplication::translate("Updates", "Not installed")},
      {State::UpdateAvailable,
       QApplication::translate("Updates", "Update available")},
      {State::Actual, QApplication::translate("Updates", "Up to date")},
  };
  return names.value(state);
}

QString toString(Action action)
{
  const QMap<Action, QString> names{
      {Action::Remove, QApplication::translate("Updates", "Remove")},
      {Action::Install, QApplication::translate("Updates", "Install/Update")},
  };
  return names.value(action);
}

QStringList toList(const QJsonValue &value)
{
  if (value.isString())
    return {value.toString()};
  if (!value.isArray())
    return {};

  const auto array = value.toArray();
  QStringList result;
  for (const auto &i : array) {
    if (i.isString())
      result.append(i.toString());
  }
  return result;
}

}  // namespace

//

Model::Model(Updater &updater)
  : updater_(updater)
{
}

QString Model::parse(const QByteArray &data)
{
  QJsonParseError error;
  const auto doc = QJsonDocument::fromJson(data, &error);
  if (doc.isNull()) {
    return tr("Failed to parse: %1 at %2")
        .arg(error.errorString())
        .arg(error.offset);
  }

  const auto json = doc.object();
  const auto version = json[versionKey].toInt();
  if (version != 1) {
    return tr("Wrong updates version: %1").arg(version);
  }

  beginResetModel();

  root_ = parse(json);
  if (root_)
    updateStates();

  endResetModel();

  if (!root_)
    return tr("No data parsed");
  return {};
}

std::unique_ptr<Model::Component> Model::parse(const QJsonObject &json) const
{
  auto result = std::make_unique<Component>();

  if (json[filesKey].isArray()) {  // concrete component
    const auto host = json["host"].toString().toLower();
    if (!host.isEmpty()) {
#if defined(Q_OS_LINUX) && defined(Q_PROCESSOR_X86_64)
      if (host != "linux")
        return {};
#elif defined(Q_OS_WINDOWS) && defined(Q_PROCESSOR_X86_64)
      if (host != "win64")
        return {};
#elif defined(Q_OS_WINDOWS) && defined(Q_PROCESSOR_X86)
      if (host != "win32")
        return {};
#elif defined(Q_OS_MACOS)
      if (host != "macos")
        return {};
#else
      return {};
#endif
    }

    result->version = json["version"].toString();

    const auto files = json[filesKey].toArray();
    result->files.reserve(files.size());

    for (const auto &fileInfo : files) {
      const auto object = fileInfo.toObject();
      File file;
      for (const auto &s : toList(object["url"])) {
        const auto url = QUrl(s);
        if (url.isValid())
          file.urls.append(url);
      }
      if (file.urls.isEmpty()) {
        result->checkOnly = true;
      } else if (file.urls.size() > 1) {
        std::random_device device;
        std::mt19937 generator(device());
        std::shuffle(file.urls.begin(), file.urls.end(), generator);
      }
      file.rawPath = object["path"].toString();
      file.md5 = object["md5"].toString();
      file.versionDate =
          QDateTime::fromString(object["date"].toString(), Qt::ISODate);
      const auto size = object["size"].toInt();
      result->size += size;
      result->files.push_back(file);
    }

    return result;
  }

  result->children.reserve(json.size());
  auto index = -1;
  for (const auto &name : json.keys()) {
    if (name == versionKey)
      continue;

    auto child = parse(json[name].toObject());
    if (!child)
      continue;
    child->name = name;
    child->index = ++index;
    child->parent = result.get();
    result->children.push_back(std::move(child));
  }
  return result;
}

void Model::updateProgress(const QUrl &url, int progress)
{
  if (!root_ || url.isEmpty())
    return;

  auto visitor = [this](Component &component, const QUrl &url, int progress,
                        auto v) -> bool {
    if (!component.files.empty()) {
      for (auto &file : component.files) {
        if (!file.urls.contains(url))
          continue;

        file.progress = progress;
        component.progress = progress;

        for (const auto &f : qAsConst(component.files))
          component.progress = std::max(f.progress, component.progress);

        const auto index = toIndex(component, int(Column::Progress));
        emit dataChanged(index, index, {Qt::DisplayRole});
        return true;
      }

    } else if (!component.children.empty()) {
      for (auto &child : component.children) {
        if (v(*child, url, progress, v))
          return true;
      }
    }
    return false;
  };

  visitor(*root_, url, progress, visitor);
}

void Model::setExpansions(const QHash<QString, QString> &expansions)
{
  expansions_ = expansions;
  updateStates();
}

void Model::updateStates()
{
  if (!root_)
    return;

  auto visitor = [this](Component &component, auto v) -> void {
    if (!component.files.empty()) {
      component.state = State::Actual;
      for (auto &file : component.files) {
        file.expandedPath = expanded(file.rawPath);
        const auto fileState = currentState(file);
        component.state = std::min(component.state, fileState);
      }
      auto index = toIndex(component, int(Column::State));
      emit dataChanged(index, index, {Qt::DisplayRole});

    } else if (!component.children.empty()) {
      for (auto &child : component.children) v(*child, v);
    }
  };

  visitor(*root_, visitor);
}

State Model::currentState(const File &file) const
{
  if (file.expandedPath.isEmpty() ||
      (file.md5.isEmpty() && !file.versionDate.isValid()))
    return State::NotAvailable;

  if (!QFile::exists(file.expandedPath))
    return State::NotInstalled;

  if (file.versionDate.isValid()) {
    QFileInfo info(file.expandedPath);
    const auto date = info.fileTime(QFile::FileModificationTime);
    if (!date.isValid())
      return State::NotInstalled;
    return date >= file.versionDate ? State::Actual : State::UpdateAvailable;
  }

  QFile f(file.expandedPath);
  if (!f.open(QFile::ReadOnly))
    return State::NotInstalled;

  const auto data = f.readAll();
  const auto md5 =
      QCryptographicHash::hash(data, QCryptographicHash::Md5).toHex();
  if (md5 != file.md5)
    return State::UpdateAvailable;
  return State::Actual;
}

QString Model::expanded(const QString &source) const
{
  auto result = source;

  for (auto it = expansions_.cbegin(), end = expansions_.cend(); it != end;
       ++it) {
    if (!result.contains(it.key()))
      continue;
    result.replace(it.key(), it.value());
  }

  return result;
}

bool Model::hasUpdates() const
{
  if (!root_)
    return false;

  const auto visitor = [](const Component &component, auto v) -> bool {
    for (const auto &i : component.children) {
      if (i->state == State::UpdateAvailable || v(*i, v))
        return true;
    }
    return false;
  };

  return visitor(*root_, visitor);
}

void Model::selectAllUpdates()
{
  if (!root_)
    return;

  const auto visitor = [this](Component &component, auto v) -> void {
    if (component.checkOnly)
      return;

    if (component.state == State::UpdateAvailable)
      updater_.applyAction(Action::Install, component.files);

    if (!component.children.empty()) {
      for (auto &child : component.children) v(*child, v);
    }
  };

  visitor(*root_, visitor);
}

void Model::tryAction(Action action, const QModelIndex &index)
{
  const auto visitor = [this, action](Component &component, auto v) -> void {
    if (component.checkOnly)
      return;

    const auto &state = component.state;
    auto ok = (action == Action::Remove &&
               (state == State::UpdateAvailable || state == State::Actual)) ||
              (action == Action::Install && (state == State::UpdateAvailable ||
                                             state == State::NotInstalled));
    if (ok)
      updater_.applyAction(action, component.files);

    if (!component.children.empty()) {
      for (auto &child : component.children) v(*child, v);
    }
  };

  auto component = toComponent(index);
  SOFT_ASSERT(component, return );
  visitor(*component, visitor);
}

Model::Component *Model::toComponent(const QModelIndex &index) const
{
  return static_cast<Component *>(index.internalPointer());
}

QModelIndex Model::toIndex(const Model::Component &component, int column) const
{
  return createIndex(component.index, column,
                     const_cast<Model::Component *>(&component));
}

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
  if (!root_)
    return {};

  if (auto ptr = toComponent(parent)) {
    SOFT_ASSERT(row >= 0 && row < int(ptr->children.size()), return {});
    return toIndex(*ptr->children[row], column);
  }

  if (row < 0 && row >= int(root_->children.size()))
    return {};

  return toIndex(*root_->children[row], column);
}

QModelIndex Model::parent(const QModelIndex &child) const
{
  auto ptr = toComponent(child);
  if (auto parent = ptr->parent)
    return createIndex(parent->index, 0, parent);
  return {};
}

int Model::rowCount(const QModelIndex &parent) const
{
  if (auto ptr = toComponent(parent)) {
    return int(ptr->children.size());
  }
  return root_ ? int(root_->children.size()) : 0;
}

int Model::columnCount(const QModelIndex & /*parent*/) const
{
  return int(Column::Count);
}

QVariant Model::headerData(int section, Qt::Orientation orientation,
                           int role) const
{
  if (role != Qt::DisplayRole)
    return {};

  if (orientation == Qt::Vertical)
    return section + 1;

  const QMap<Column, QString> names{
      {Column::Name, tr("Name")},         {Column::State, tr("State")},
      {Column::Size, tr("Size")},         {Column::Version, tr("Version")},
      {Column::Progress, tr("Progress")},
  };
  return names.value(Column(section));
}

QVariant Model::data(const QModelIndex &index, int role) const
{
  if ((role != Qt::DisplayRole && role != Qt::EditRole) || !index.isValid())
    return {};

  auto ptr = toComponent(index);
  SOFT_ASSERT(ptr, return {});

  switch (index.column()) {
    case int(Column::Name): return QObject::tr(qPrintable(ptr->name));
    case int(Column::State): return toString(ptr->state);
    case int(Column::Size): return sizeString(ptr->size, 1);
    case int(Column::Version): return ptr->version;
    case int(Column::Progress):
      return ptr->progress > 0 ? ptr->progress : QVariant();
  }

  return {};
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
  auto ptr = toComponent(index);
  SOFT_ASSERT(ptr, return {});
  auto result = Qt::NoItemFlags | Qt::ItemIsSelectable;

  if (ptr->checkOnly)
    return result;

  result |= Qt::ItemIsEnabled;
  return result;
}

//

Loader::Loader(Updater &updater)
  : updater_(updater)
  , network_(new QNetworkAccessManager(this))
{
  network_->setRedirectPolicy(
      QNetworkRequest::RedirectPolicy::NoLessSafeRedirectPolicy);
  connect(network_, &QNetworkAccessManager::finished,  //
          this, &Loader::handleReply);
}

void Loader::download(const Urls &urls)
{
  start(urls, {}, {});
}

void Loader::start(const Urls &urls, const QUrl &previous, const QString &error)
{
  if (!error.isEmpty())
    qCritical() << error;

  if (urls.isEmpty()) {
    if (!previous.isEmpty())
      updater_.downloadFailed(previous, error);
    return;
  }

  auto leftUrls = urls;
  const auto current = leftUrls.takeFirst();
  auto reply = network_->get(QNetworkRequest(current));
  downloads_.insert(reply, leftUrls);

  connect(reply, &QNetworkReply::downloadProgress,  //
          this, [this, current](qint64 bytesSent, qint64 bytesTotal) {
            updater_.updateProgress(current, bytesSent, bytesTotal);
          });

  if (reply->error() == QNetworkReply::NoError) {
    updater_.updateProgress(current, -1, -1);
    return;
  }

  handleReply(reply);
}

void Loader::handleReply(QNetworkReply *reply)
{
  reply->deleteLater();

  SOFT_ASSERT(downloads_.contains(reply), return );
  const auto leftUrls = downloads_.take(reply);
  const auto url = reply->request().url();

  if (reply->error() != QNetworkReply::NoError) {
    const auto error = tr("Failed to download file\n%1. Error %2")
                           .arg(reply->url().toString(), reply->errorString());
    start(leftUrls, url, error);
    return;
  }

  const auto replyData = reply->readAll();
  if (replyData.isEmpty()) {
    const auto error = tr("Empty data downloaded from\n%1").arg(url.toString());
    start(leftUrls, url, error);
    return;
  }

  updater_.downloaded(url, replyData);
}

//

UpdateDelegate::UpdateDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

void UpdateDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
  if (index.column() != int(Model::Column::Progress) || index.data().isNull()) {
    QStyledItemDelegate::paint(painter, option, index);
    return;
  }

  QStyleOptionProgressBar progressBarOption;
  progressBarOption.rect = option.rect;
  progressBarOption.minimum = 0;
  progressBarOption.maximum = 100;
  const auto progress = index.data().toInt();
  progressBarOption.progress = progress;
  progressBarOption.text = QString::number(progress) + "%";
  progressBarOption.textVisible = true;

  QApplication::style()->drawControl(QStyle::CE_ProgressBar, &progressBarOption,
                                     painter);
}

//

void Installer::checkInstall(const File &file)
{
  QFileInfo installDir(QFileInfo(file.expandedPath).absolutePath());
  if (installDir.exists() && !installDir.isWritable()) {
    error_ +=
        QApplication::translate("Updates", "Directory is not writable\n%1")
            .arg(installDir.absolutePath());
  }
}

void Installer::remove(const File &file)
{
  QFile f(file.expandedPath);
  if (!f.exists())
    return;

  if (!f.remove()) {
    error_ += QApplication::translate("Updates",
                                      "Failed to remove file\n%1\nError %2")
                  .arg(f.fileName(), f.errorString());
  }
}

void Installer::install(const File &file, const QByteArray &data)
{
  auto installDir = QFileInfo(file.expandedPath).absoluteDir();
  if (!installDir.exists() && !installDir.mkpath(".")) {
    error_ += QApplication::translate("Updates", "Failed to create path\n%1")
                  .arg(installDir.absolutePath());
    return;
  }

  QTemporaryFile tmp;
  if (!tmp.open()) {
    error_ += QApplication::translate(
                  "Updates", "Failed to create temp file\n%1\nError %2")
                  .arg(tmp.fileName(), tmp.errorString());
    return;
  }

  const auto wrote = tmp.write(data);
  if (wrote != data.size()) {
    error_ += QApplication::translate(
                  "Updates", "Failed to write to temp file\n%1\nError %2")
                  .arg(tmp.fileName(), tmp.errorString());
    return;
  }

  tmp.close();

  QFile existing(file.expandedPath);
  if (existing.exists() && !existing.remove()) {
    error_ += QApplication::translate("Updates",
                                      "Failed to remove file\n%1\nError %2")
                  .arg(existing.fileName(), existing.errorString());
    return;
  }

  if (!tmp.copy(file.expandedPath)) {
    error_ += QApplication::translate(
                  "Updates", "Failed to copy file\n%1\nto %2\nError %3")
                  .arg(tmp.fileName(), file.expandedPath, tmp.errorString());
    return;
  }
}

const QString &Installer::error() const
{
  return error_;
}

//

AutoChecker::AutoChecker(Updater &updater, int intervalDays,
                         const QDateTime &lastCheck)
  : updater_(updater)
  , checkIntervalDays_(intervalDays)
  , lastCheckDate_(lastCheck)
{
  connect(&updater_, &Updater::checkedForUpdates,  //
          this, &AutoChecker::updateLastCheckDate);
  scheduleNextCheck();
}

AutoChecker::~AutoChecker() = default;

const QDateTime &AutoChecker::lastCheckDate() const
{
  return lastCheckDate_;
}

void AutoChecker::scheduleNextCheck()
{
  if (timer_)
    timer_->stop();

  if (checkIntervalDays_ < 1)
    return;

  if (!timer_) {
    timer_ = std::make_unique<QTimer>();
    timer_->setSingleShot(true);
    connect(timer_.get(), &QTimer::timeout,  //
            &updater_, &Updater::checkForUpdates);
  }

  const auto now = QDateTime::currentDateTime();
  const auto &last = lastCheckDate_.isValid() ? lastCheckDate_ : now;
  auto nextTime = last.addDays(checkIntervalDays_);
  if (nextTime <= now)
    nextTime = now.addSecs(5);

  timer_->start(now.msecsTo(nextTime));
}

void AutoChecker::updateLastCheckDate()
{
  lastCheckDate_ = QDateTime::currentDateTime();
  scheduleNextCheck();
}

//

Updater::Updater(const QVector<QUrl> &updateUrls)
  : model_(std::make_unique<Model>(*this))
  , loader_(std::make_unique<Loader>(*this))
  , updateUrls_(updateUrls)
{
  std::random_device device;
  std::mt19937 generator(device());
  std::shuffle(updateUrls_.begin(), updateUrls_.end(), generator);
}

void Updater::initView(QTreeView *view)
{
  view->setSelectionMode(QAbstractItemView::ExtendedSelection);

  auto proxy = new QSortFilterProxyModel(view);
  proxy->setSourceModel(model_.get());

  view->setModel(proxy);
  view->setItemDelegate(new update::UpdateDelegate(view));
  view->setSortingEnabled(true);
  view->sortByColumn(int(Model::Column::Name), Qt::AscendingOrder);
  view->setContextMenuPolicy(Qt::CustomContextMenu);

  connect(view, &QAbstractItemView::doubleClicked,  //
          this, &Updater::handleModelDoubleClick);
  connect(view, &QAbstractItemView::customContextMenuRequested,  //
          this, &Updater::showModelContextMenu);
}

void Updater::setExpansions(const QHash<QString, QString> &expansions)
{
  model_->setExpansions(expansions);
}

void Updater::checkForUpdates()
{
  loader_->download(updateUrls_);
}

void Updater::applyAction(Action action, const QVector<File> &files)
{
  for (const auto &file : files) {
    LTRACE() << "applyAction" << int(action) << file.rawPath;

    if (action == Action::Remove) {
      Installer installer;
      installer.remove(file);
      if (!installer.error().isEmpty()) {
        emit error(installer.error());
        continue;
      }
      model_->updateStates();
      emit updated();
      continue;
    }

    if (action == Action::Install) {
      if (file.urls.isEmpty() || findDownload(file.urls.first()) != -1)
        continue;

      Installer installer;
      installer.checkInstall(file);

      if (!installer.error().isEmpty()) {
        emit error(installer.error());
        continue;
      }

      downloading_.push_back(file);
      loader_->download(file.urls);
      continue;
    }
  }
}

void Updater::downloaded(const QUrl &url, const QByteArray &data)
{
  LTRACE() << "downloaded" << url << LARG(data.size());

  if (updateUrls_.contains(url)) {
    const auto errors = model_->parse(data);
    emit checkedForUpdates();
    if (!errors.isEmpty()) {
      emit error(errors);
      return;
    }
    if (model_->hasUpdates())
      emit updatesAvailable();
    return;
  }

  model_->updateProgress(url, 0);

  const auto index = findDownload(url);
  if (index == -1)
    return;

  const auto file = downloading_.takeAt(index);
  LTRACE() << "downloaded file" << url << file.expandedPath;

  const auto mustUnpack =
      url.toString().endsWith(".zip") && !file.expandedPath.endsWith(".zip");
  const auto unpacked = mustUnpack ? unpack(data) : data;
  if (unpacked.isEmpty()) {
    emit error(tr("Empty data unpacked from\n%1").arg(url.toString()));
    return;
  }

  Installer installer;
  installer.install(file, unpacked);
  if (!installer.error().isEmpty()) {
    emit error(installer.error());
    return;
  }

  model_->updateStates();
  emit updated();
}

void Updater::updateProgress(const QUrl &url, qint64 bytesSent,
                             qint64 bytesTotal)
{
  auto progress = bytesTotal < 1 ? 1 : int(100.0 * bytesSent / bytesTotal);
  model_->updateProgress(url, progress);
}

void Updater::downloadFailed(const QUrl &url, const QString &error)
{
  if (updateUrls_.contains(url)) {
    emit checkedForUpdates();
    emit this->error(error);
    return;
  }

  model_->updateProgress(url, 0);

  const auto index = findDownload(url);
  if (index == -1)
    return;

  downloading_.removeAt(index);
  emit this->error(error);
}

QDateTime Updater::lastUpdateCheck() const
{
  if (!autoChecker_)
    return {};
  return autoChecker_->lastCheckDate();
}

void Updater::setAutoUpdate(int intervalDays, const QDateTime &lastCheck)
{
  autoChecker_ = std::make_unique<AutoChecker>(*this, intervalDays, lastCheck);
}

void Updater::handleModelDoubleClick(const QModelIndex &index)
{
  if (!index.isValid())
    return;

  model_->tryAction(Action::Install, fromProxy(index));
}

void Updater::showModelContextMenu()
{
  QMenu menu;
  auto install = menu.addAction(toString(Action::Install));
  menu.addAction(toString(Action::Remove));
  menu.addSeparator();
  auto updateAll = menu.addAction(tr("Update all"));

  const auto choice = menu.exec(QCursor::pos());
  if (!choice)
    return;

  if (choice == updateAll) {
    model_->selectAllUpdates();
    return;
  }

  auto view = qobject_cast<QAbstractItemView *>(sender());
  SOFT_ASSERT(view, return );

  const auto selection = view->selectionModel();
  SOFT_ASSERT(selection, return );
  const auto indexes = selection->selectedRows(int(Model::Column::Name));
  if (indexes.isEmpty())
    return;

  const auto action = choice == install ? Action::Install : Action::Remove;
  for (const auto &index : indexes) model_->tryAction(action, fromProxy(index));
}

int Updater::findDownload(const QUrl &url) const
{
  auto it = std::find_if(downloading_.cbegin(), downloading_.cend(),
                         [url](const File &f) { return f.urls.contains(url); });
  if (it == downloading_.end())
    return -1;
  return std::distance(downloading_.cbegin(), it);
}

QModelIndex Updater::fromProxy(const QModelIndex &index) const
{
  if (!index.isValid() || index.model() == model_.get())
    return index;
  auto proxy = qobject_cast<const QSortFilterProxyModel *>(index.model());
  if (!proxy)
    return {};
  return proxy->mapToSource(index);
}

}  // namespace update
