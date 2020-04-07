#include "updates.h"
#include "debug.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QComboBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMenu>
#include <QNetworkReply>
#include <QSortFilterProxyModel>
#include <QStandardPaths>
#include <QTimer>
#include <QTreeView>

#include <random>

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
           QObject::tr("Tb");
  }
  if (bytes >= gb) {
    return QString::number(bytes / gb, 'f', precision) + ' ' +
           QObject::tr("Gb");
  }
  if (bytes >= mb) {
    return QString::number(bytes / mb, 'f', precision) + ' ' +
           QObject::tr("Mb");
  }
  if (bytes >= kb) {
    return QString::number(bytes / kb, 'f', precision) + ' ' +
           QObject::tr("Kb");
  }
  return QString::number(bytes) + ' ' + QObject::tr("bytes");
}

QString toString(State state)
{
  const QMap<State, QString> names{
      {State::NotAvailable, {}},
      {State::NotInstalled, QObject::tr("Not installed")},
      {State::UpdateAvailable, QObject::tr("Update available")},
      {State::Actual, QObject::tr("Up to date")},
  };
  return names.value(state);
}

QString toString(Action action)
{
  const QMap<Action, QString> names{
      {Action::NoAction, {}},
      {Action::Remove, QObject::tr("Remove")},
      {Action::Install, QObject::tr("Install/Update")},
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

Loader::Loader(const update::Loader::Urls &updateUrls, QObject *parent)
  : QObject(parent)
  , network_(new QNetworkAccessManager(this))
  , model_(new Model(this))
  , updateUrls_(updateUrls)
  , downloadPath_(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        "/updates")
{
  std::random_device device;
  std::mt19937 generator(device());
  std::shuffle(updateUrls_.begin(), updateUrls_.end(), generator);

  connect(network_, &QNetworkAccessManager::finished,  //
          this, &Loader::handleReply);
}

void Loader::handleReply(QNetworkReply *reply)
{
  if (updateUrls_.contains(reply->url())) {
    handleUpdateReply(reply);
  } else {
    handleComponentReply(reply);
  }
}

void Loader::checkForUpdates()
{
  startDownloadUpdates({});
}

void Loader::startDownloadUpdates(const QUrl &previous)
{
  SOFT_ASSERT(!updateUrls_.isEmpty(), return );

  QUrl url;
  if (previous.isEmpty())
    url = updateUrls_.first();
  else {
    const auto index = updateUrls_.indexOf(previous);
    SOFT_ASSERT(index != -1, return );
    if (index == updateUrls_.size() - 1)
      return;
    url = updateUrls_[index + 1];
  }

  if (url.isEmpty())
    return;

  auto reply = network_->get(QNetworkRequest(url));
  if (reply->error() != QNetworkReply::NoError)
    handleUpdateReply(reply);
}

void Loader::handleUpdateReply(QNetworkReply *reply)
{
  reply->deleteLater();

  if (reply->error() != QNetworkReply::NoError) {
    emit error(toError(*reply));
    startDownloadUpdates(reply->url());
    return;
  }

  const auto replyData = reply->readAll();
  if (replyData.isEmpty()) {
    emit error(
        tr("Received empty updates info from %1").arg(reply->url().toString()));
    startDownloadUpdates(reply->url());
    return;
  }


  SOFT_ASSERT(model_, return );
  const auto parseError = model_->parse(replyData);
  if (!parseError.isEmpty()) {
    emit error(tr("Failed to parse updates from %1 (%2)")
                   .arg(reply->url().toString(), parseError));
    startDownloadUpdates(reply->url());
    return;
  }

  if (model_->hasUpdates())
    emit updatesAvailable();
}

QString Loader::toError(QNetworkReply &reply) const
{
  return tr("Failed to download file %1. Error %2")
      .arg(reply.url().toString(), reply.errorString());
}

void Loader::applyUserActions()
{
  SOFT_ASSERT(model_, return );
  if (!currentActions_.empty() || !downloads_.empty()) {
    emit error(tr("Update already in process"));
    return;
  }

  currentActions_ = model_->userActions();
  if (currentActions_.empty()) {
    emit error(tr("No actions to apply"));
    return;
  }

  for (auto &action : currentActions_) {
    if (action.first != Action::Install)
      continue;

    auto &file = action.second;
    file.downloadPath = downloadPath_ + '/' + file.rawPath;

    if (!startDownload(file)) {
      finishUpdate();
      return;
    }
  }

  if (downloads_.empty())  // no downloads
    commitUpdate();
}

bool Loader::startDownload(File &file)
{
  if (file.urls.empty())
    return false;

  auto reply = network_->get(QNetworkRequest(file.urls.takeFirst()));
  downloads_.emplace(reply, &file);

  connect(reply, &QNetworkReply::downloadProgress,  //
          this, &Loader::updateProgress);

  if (reply->error() == QNetworkReply::NoError)
    return true;

  return handleComponentReply(reply);
}

bool Loader::handleComponentReply(QNetworkReply *reply)
{
  reply->deleteLater();

  if (currentActions_.empty()) {  // aborted?
    finishUpdate();
    return false;
  }

  SOFT_ASSERT(downloads_.count(reply) == 1, return false);
  auto *file = downloads_[reply];
  SOFT_ASSERT(file, return false);

  downloads_.erase(reply);

  if (reply->error() != QNetworkReply::NoError) {
    emit error(toError(*reply));

    if (!startDownload(*file))
      finishUpdate();

    return false;
  }

  const auto &fileName = file->downloadPath;
  auto dir = QFileInfo(fileName).absoluteDir();
  if (!dir.exists())
    dir.mkpath(".");

  QFile f(fileName);
  if (!f.open(QFile::WriteOnly)) {
    const auto error =
        tr("Failed to save downloaded file %1 to %2. Error %3")
            .arg(reply->url().toString(), f.fileName(), f.errorString());
    finishUpdate(error);
    return false;
  }

  const auto replyData = reply->readAll();
  f.write(replyData);
  f.close();

  if (downloads_.empty())
    commitUpdate();

  return true;
}

void Loader::finishUpdate(const QString &error)
{
  currentActions_.clear();
  for (const auto &i : downloads_) i.first->deleteLater();
  downloads_.clear();
  if (!error.isEmpty())
    emit this->error(error);
  SOFT_ASSERT(model_, return );
  model_->updateStates();
}

void Loader::commitUpdate()
{
  SOFT_ASSERT(!currentActions_.empty(), return );
  Installer installer(currentActions_);
  if (installer.commit()) {
    model_->resetProgress();
    emit updated();
  } else {
    emit error(tr("Update failed: %1").arg(installer.errorString()));
  }
  finishUpdate();
}

void Loader::updateProgress(qint64 bytesSent, qint64 bytesTotal)
{
  if (bytesTotal < 1)
    return;
  const auto reply = qobject_cast<QNetworkReply *>(sender());
  SOFT_ASSERT(reply, return );
  const auto progress = int(100.0 * bytesSent / bytesTotal);
  model_->updateProgress(reply->url(), progress);
}

Model *Loader::model() const
{
  return model_;
}

Model::Model(QObject *parent)
  : QAbstractItemModel(parent)
{
}

void Model::initView(QTreeView *view)
{
  view->setSelectionMode(QAbstractItemView::ExtendedSelection);
  auto proxy = new QSortFilterProxyModel(view);
  proxy->setSourceModel(this);
  view->setModel(proxy);
  view->setItemDelegate(new update::UpdateDelegate(view));
#ifndef DEVELOP
  view->hideColumn(int(update::Model::Column::Files));
#endif

  auto menu = new QMenu(view);
  menu->addAction(toString(Action::NoAction));
  menu->addAction(toString(Action::Remove));
  menu->addAction(toString(Action::Install));

  view->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(view, &QAbstractItemView::customContextMenuRequested,  //
          menu, [this, menu, view, proxy] {
            const auto selection = view->selectionModel();
            SOFT_ASSERT(selection, return );
            const auto indexes = selection->selectedRows(int(Column::Action));
            if (indexes.isEmpty())
              return;

            const auto menuItem = menu->exec(QCursor::pos());
            if (!menuItem)
              return;

            const auto action = menu->actions().indexOf(menuItem);

            for (const auto &proxyIndex : indexes) {
              auto modelIndex = proxy->mapToSource(proxyIndex);
              if (!modelIndex.isValid() || rowCount(modelIndex) > 0)
                continue;
              setData(modelIndex, action, Qt::EditRole);
            }
          });
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
    return tr("Wrong updates version %1").arg(version);
  }

  beginResetModel();

  root_ = parse(json);
  if (root_)
    updateState(*root_);

  endResetModel();

  if (!root_) {
    return tr("No data parsed");
  }
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

void Model::updateProgress(Model::Component &component, const QUrl &url,
                           int progress)
{
  if (!component.files.empty()) {
    for (auto &file : component.files) {
      if (!url.isEmpty() && !file.urls.contains(url))
        continue;

      file.progress = progress;
      component.progress = progress;

      for (const auto &file : component.files)
        component.progress = std::max(file.progress, component.progress);

      const auto index = toIndex(component, int(Column::Progress));
      emit dataChanged(index, index, {Qt::DisplayRole});

      if (!url.isEmpty())
        break;
    }
    return;
  }

  if (!component.children.empty()) {
    for (auto &child : component.children)
      updateProgress(*child, url, progress);
    return;
  }
}

void Model::setExpansions(const std::map<QString, QString> &expansions)
{
  expansions_ = expansions;
  updateStates();
}

UserActions Model::userActions() const
{
  if (!root_)
    return {};

  UserActions result;
  fillUserActions(result, *root_);
  return result;
}

void Model::fillUserActions(UserActions &actions, Component &component) const
{
  if (!component.files.empty()) {
    if (component.action == Action::NoAction)
      return;

    for (auto &file : component.files) actions.emplace(component.action, file);
    return;
  }

  if (!component.children.empty()) {
    for (auto &child : component.children) fillUserActions(actions, *child);
    return;
  }
}

void Model::updateStates()
{
  if (!root_)
    return;

  updateState(*root_);
  emitColumnsChanged(QModelIndex());
}

bool Model::hasUpdates() const
{
  if (!root_)
    return false;
  return hasUpdates(*root_);
}

void Model::updateProgress(const QUrl &url, int progress)
{
  if (!root_)
    return;

  updateProgress(*root_, url, progress);
}

void Model::resetProgress()
{
  if (!root_)
    return;

  updateProgress(*root_, {}, 0);
}

bool Model::hasUpdates(const Model::Component &component) const
{
  for (const auto &i : component.children) {
    if (i->state == State::UpdateAvailable || hasUpdates(*i))
      return true;
  }
  return false;
}

void Model::updateState(Model::Component &component)
{
  if (!component.files.empty()) {
    std::vector<State> states;
    states.reserve(component.files.size());

    for (auto &file : component.files) {
      file.expandedPath = expanded(file.rawPath);
      states.push_back(currentState(file));
    }

    component.state = *std::min_element(states.cbegin(), states.cend());
    component.action = Action::NoAction;
    return;
  }

  if (!component.children.empty()) {
    for (auto &child : component.children) updateState(*child);
    return;
  }
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

  for (const auto &expansion : expansions_) {
    if (!result.contains(expansion.first))
      continue;
    result.replace(expansion.first, expansion.second);
  }

  return result;
}

void Model::emitColumnsChanged(const QModelIndex &parent)
{
  const auto count = rowCount(parent);
  if (count == 0)
    return;

  emit dataChanged(index(0, int(Column::State), parent),
                   index(count - 1, int(Column::Action), parent),
                   {Qt::DisplayRole, Qt::EditRole});

  for (auto i = 0; i < count; ++i) emitColumnsChanged(index(0, 0, parent));
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
    return ptr->children.size();
  }
  return root_ ? root_->children.size() : 0;
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
      {Column::Name, tr("Name")},       {Column::State, tr("State")},
      {Column::Action, tr("Action")},   {Column::Size, tr("Size")},
      {Column::Version, tr("Version")}, {Column::Progress, tr("Progress")},
      {Column::Files, tr("Files")},
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
    case int(Column::Action): return toString(ptr->action);
    case int(Column::Size): return sizeString(ptr->size, 1);
    case int(Column::Version): return ptr->version;
    case int(Column::Progress):
      return ptr->progress > 0 ? ptr->progress : QVariant();
    case int(Column::Files): {
      QStringList files;
      files.reserve(ptr->files.size());
      for (const auto &f : ptr->files) files.append(f.expandedPath);
      return files.join(',');
    }
  }

  return {};
}

bool Model::setData(const QModelIndex &index, const QVariant &value, int role)
{
  if (!index.isValid() || role != Qt::EditRole)
    return false;

  auto ptr = toComponent(index);
  SOFT_ASSERT(ptr, return false);

  if (index.column() != int(Column::Action))
    return false;

  const auto newAction = Action(
      std::clamp(value.toInt(), int(Action::NoAction), int(Action::Install)));
  if (ptr->action == newAction)
    return false;

  ptr->action = newAction;
  emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

  return true;
}

Qt::ItemFlags Model::flags(const QModelIndex &index) const
{
  auto ptr = toComponent(index);
  SOFT_ASSERT(ptr, return {});
  auto result = Qt::NoItemFlags | Qt::ItemIsSelectable;

  if (ptr->checkOnly)
    return result;

  result |= Qt::ItemIsEnabled;
  if (index.column() != int(Column::Action) ||
      ptr->state == State::NotAvailable)
    return result;

  result |= Qt::ItemIsEditable;
  return result;
}

UpdateDelegate::UpdateDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

void UpdateDelegate::paint(QPainter *painter,
                           const QStyleOptionViewItem &option,
                           const QModelIndex &index) const
{
  if (index.column() == int(Model::Column::Progress) &&
      !index.data().isNull()) {
    QStyleOptionProgressBar progressBarOption;
    progressBarOption.rect = option.rect;
    progressBarOption.minimum = 0;
    progressBarOption.maximum = 100;
    const auto progress = index.data().toInt();
    progressBarOption.progress = progress;
    progressBarOption.text = QString::number(progress) + "%";
    progressBarOption.textVisible = true;

    QApplication::style()->drawControl(QStyle::CE_ProgressBar,
                                       &progressBarOption, painter);
    return;
  }

  QStyledItemDelegate::paint(painter, option, index);
}

QWidget *UpdateDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &option,
                                      const QModelIndex &index) const
{
  if (index.column() == int(Model::Column::Action)) {
    auto combo = new QComboBox(parent);
    combo->setEditable(false);
    combo->addItems({toString(Action::NoAction), toString(Action::Remove),
                     toString(Action::Install)});
    return combo;
  }

  return QStyledItemDelegate::createEditor(parent, option, index);
}

void UpdateDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
  if (index.column() == int(Model::Column::Action)) {
    auto combo = qobject_cast<QComboBox *>(editor);
    SOFT_ASSERT(combo, return );
    combo->setCurrentText(index.data(Qt::EditRole).toString());
    return;
  }

  return QStyledItemDelegate::setEditorData(editor, index);
}

void UpdateDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
  if (index.column() == int(Model::Column::Action)) {
    auto combo = qobject_cast<QComboBox *>(editor);
    SOFT_ASSERT(combo, return );
    model->setData(index, combo->currentIndex());
    return;
  }

  return QStyledItemDelegate::setModelData(editor, model, index);
}

Installer::Installer(const UserActions &actions)
  : actions_(actions)
{
}

bool Installer::commit()
{
  if (!checkIsPossible())
    return false;

  for (const auto &action : actions_) {
    const auto &file = action.second;

    if (action.first == Action::Remove)
      remove(file);
    else if (action.first == Action::Install)
      install(file);
  }

  return errors_.isEmpty();
}

bool Installer::checkIsPossible()
{
  errors_.clear();

  for (const auto &action : actions_) {
    const auto &file = action.second;

    if (action.first == Action::Remove)
      checkRemove(file);
    else if (action.first == Action::Install)
      checkInstall(file);
  }
  errors_.removeDuplicates();

  return errors_.isEmpty();
}

void Installer::checkRemove(const File &file)
{
  QFileInfo installDir(QFileInfo(file.expandedPath).absolutePath());
  if (!QFile::exists(file.expandedPath))
    return;

  if (installDir.exists() && !installDir.isWritable()) {
    errors_.append(QObject::tr("Directory is not writable %1")
                       .arg(installDir.absolutePath()));
  }
}

void Installer::checkInstall(const File &file)
{
  if (!QFileInfo::exists(file.downloadPath)) {
    errors_.append(
        QObject::tr("Downloaded file not exists %1").arg(file.downloadPath));
    // no return
  }

  QFileInfo installDir(QFileInfo(file.expandedPath).absolutePath());
  if (installDir.exists() && !installDir.isWritable()) {
    errors_.append(QObject::tr("Directory is not writable %1")
                       .arg(installDir.absolutePath()));
  }
}

void Installer::remove(const File &file)
{
  QFile f(file.expandedPath);
  if (!f.exists())
    return;

  if (!f.remove()) {
    errors_.append(QObject::tr("Failed to remove file %1. Error %2")
                       .arg(f.fileName(), f.errorString()));
  }
}

void Installer::install(const File &file)
{
  auto installDir = QFileInfo(file.expandedPath).absoluteDir();
  if (!installDir.exists() && !installDir.mkpath(".")) {
    errors_.append(
        QObject::tr("Failed to create path %1").arg(installDir.absolutePath()));
    return;
  }

  QFile existing(file.expandedPath);
  if (existing.exists() && !existing.remove()) {
    errors_.append(QObject::tr("Failed to remove file %1. Error %2")
                       .arg(existing.fileName(), existing.errorString()));
    return;
  }

  QFile f(file.downloadPath);
  if (!f.rename(file.expandedPath)) {
    errors_.append(QObject::tr("Failed to move file %1 to %2. Error %3")
                       .arg(f.fileName(), file.expandedPath, f.errorString()));
    return;
  }

  if (!file.versionDate.isValid())
    return;

  if (!f.open(QFile::WriteOnly | QFile::Append) ||
      !f.setFileTime(file.versionDate, QFile::FileTime::FileModificationTime)) {
    errors_.append(QObject::tr("Failed to set modification time of "
                               "file %1 to %2. Error %3")
                       .arg(f.fileName(),
                            file.versionDate.toString(Qt::ISODate),
                            f.errorString()));
  }
}

QString Installer::errorString() const
{
  return errors_.join('\n');
}

AutoChecker::AutoChecker(Loader &loader, QObject *parent)
  : QObject(parent)
  , loader_(loader)
{
  SOFT_ASSERT(loader.model(), return );
  connect(loader.model(), &Model::modelReset,  //
          this, &AutoChecker::handleModelReset);
}

AutoChecker::~AutoChecker() = default;

bool AutoChecker::isLastCheckDateChanged() const
{
  return isLastCheckDateChanged_;
}

QDateTime AutoChecker::lastCheckDate() const
{
  return lastCheckDate_;
}

void AutoChecker::setCheckIntervalDays(int days)
{
  checkIntervalDays_ = days;
  scheduleNextCheck();
}

void AutoChecker::setLastCheckDate(const QDateTime &dt)
{
  isLastCheckDateChanged_ = false;

  lastCheckDate_ = dt;
  if (!lastCheckDate_.isValid())
    lastCheckDate_ = QDateTime::currentDateTime();

  scheduleNextCheck();
}

void AutoChecker::scheduleNextCheck()
{
  if (timer_)
    timer_->stop();

  if (checkIntervalDays_ < 1 || !lastCheckDate_.isValid())
    return;

  if (!timer_) {
    timer_ = std::make_unique<QTimer>();
    timer_->setSingleShot(true);
    connect(timer_.get(), &QTimer::timeout,  //
            &loader_, &Loader::checkForUpdates);
  }

  auto nextTime = lastCheckDate_.addDays(checkIntervalDays_);
  const auto now = QDateTime::currentDateTime();
  if (nextTime < now)
    nextTime = now.addSecs(5);

  timer_->start(now.msecsTo(nextTime));
}

void AutoChecker::handleModelReset()
{
  lastCheckDate_ = QDateTime::currentDateTime();
  isLastCheckDateChanged_ = true;
  scheduleNextCheck();
}

}  // namespace update
