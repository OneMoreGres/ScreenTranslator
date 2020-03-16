#include "updates.h"
#include "debug.h"

#include <QComboBox>
#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkReply>
#include <QStandardPaths>

namespace update
{
namespace
{
const auto versionKey = "version";
const auto filesKey = "files";

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

}  // namespace

Loader::Loader(const QUrl &updateUrl, QObject *parent)
  : QObject(parent)
  , network_(new QNetworkAccessManager(this))
  , model_(new Model(this))
  , updateUrl_(updateUrl)
  , downloadPath_(
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) +
        "/updates")
{
  connect(network_, &QNetworkAccessManager::finished,  //
          this, &Loader::handleReply);
}

void Loader::checkForUpdates()
{
  auto reply = network_->get(QNetworkRequest(updateUrl_));
  if (reply->error() == QNetworkReply::NoError)
    return;

  reply->deleteLater();
  emit error(toError(*reply));
}

QString Loader::toError(QNetworkReply &reply) const
{
  return tr("Failed to download file %1. Error %2")
      .arg(reply.url().toString(), reply.errorString());
}

void Loader::applyUserActions()
{
  SOFT_ASSERT(model_, return );
  if (installer_ || !componentReplyToPath_.empty()) {
    emit error(tr("Update already in process"));
    return;
  }

  auto actions = model_->userActions();
  if (actions.empty()) {
    emit error(tr("No actions to apply"));
    return;
  }

  for (auto &action : actions) {
    if (action.first != Action::Install)
      continue;

    auto &file = action.second;

    auto reply = network_->get(QNetworkRequest(file.url));
    if (reply->error() != QNetworkReply::NoError) {
      finishUpdate(toError(*reply));
      break;
    }

    file.downloadPath = downloadPath_ + '/' + file.rawPath;
    componentReplyToPath_.emplace(reply, file.downloadPath);
  }

  installer_ = std::make_unique<Installer>(actions);

  if (componentReplyToPath_.empty())  // no downloads
    commitUpdate();
}

void Loader::finishUpdate(const QString &error)
{
  installer_.reset();
  for (const auto &i : componentReplyToPath_) i.first->deleteLater();
  componentReplyToPath_.clear();
  if (!error.isEmpty())
    emit this->error(error);
  SOFT_ASSERT(model_, return );
  model_->updateStates();
}

void Loader::handleReply(QNetworkReply *reply)
{
  reply->deleteLater();

  const auto isUpdatesReply = reply->url() == updateUrl_;

  if (reply->error() != QNetworkReply::NoError) {
    emit error(toError(*reply));
    if (!isUpdatesReply)
      finishUpdate();
    return;
  }

  const auto replyData = reply->readAll();

  if (isUpdatesReply) {
    SOFT_ASSERT(model_, return );
    model_->parse(replyData);
    if (model_->hasUpdates())
      emit updatesAvailable();
    return;
  }

  SOFT_ASSERT(componentReplyToPath_.count(reply) == 1, return );

  auto replyIt = componentReplyToPath_.find(reply);
  const auto &fileName = replyIt->second;

  auto dir = QFileInfo(fileName).absoluteDir();
  if (!dir.exists())
    dir.mkpath(".");

  QFile f(fileName);
  if (!f.open(QFile::WriteOnly)) {
    const auto error =
        tr("Failed to save downloaded file %1 to %2. Error %3")
            .arg(reply->url().toString(), f.fileName(), f.errorString());
    finishUpdate(error);
    return;
  }
  f.write(replyData);
  f.close();

  componentReplyToPath_.erase(replyIt);

  if (componentReplyToPath_.empty())
    commitUpdate();
}

void Loader::commitUpdate()
{
  SOFT_ASSERT(installer_, return );
  if (installer_->commit()) {
    emit updated();
  } else {
    emit error(tr("Update failed: %1").arg(installer_->errorString()));
  }
  finishUpdate();
}

Model *Loader::model() const
{
  return model_;
}

Model::Model(QObject *parent)
  : QAbstractItemModel(parent)
{
}

void Model::parse(const QByteArray &data)
{
  QJsonParseError error;
  const auto doc = QJsonDocument::fromJson(data, &error);
  if (doc.isNull()) {
    LERROR() << error.errorString();
    return;
  }

  const auto json = doc.object();
  const auto version = json[versionKey].toInt();
  if (version != 1) {
    LERROR() << "Wrong updates.json version" << version;
    return;
  }

  beginResetModel();

  root_ = parse(json);
  if (root_)
    updateState(*root_);

  endResetModel();
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
      file.url = object["url"].toString();
      if (!file.url.isValid())
        result->checkOnly = true;
      file.rawPath = object["path"].toString();
      file.md5 = object["md5"].toString();
      file.versionDate =
          QDateTime::fromString(object["date"].toString(), Qt::ISODate);
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

QModelIndex Model::index(int row, int column, const QModelIndex &parent) const
{
  if (!root_)
    return {};
  if (auto ptr = static_cast<Component *>(parent.internalPointer())) {
    SOFT_ASSERT(row >= 0 && row < int(ptr->children.size()), return {});
    return createIndex(row, column, ptr->children[row].get());
  }
  if (row < 0 && row >= int(root_->children.size()))
    return {};
  return createIndex(row, column, root_->children[row].get());
}

QModelIndex Model::parent(const QModelIndex &child) const
{
  auto ptr = static_cast<Component *>(child.internalPointer());
  if (auto parent = ptr->parent)
    return createIndex(parent->index, 0, parent);
  return {};
}

int Model::rowCount(const QModelIndex &parent) const
{
  if (auto ptr = static_cast<Component *>(parent.internalPointer())) {
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
      {Column::Name, tr("Name")},     {Column::State, tr("State")},
      {Column::Action, tr("Action")}, {Column::Version, tr("Version")},
      {Column::Files, tr("Files")},
  };
  return names.value(Column(section));
}

QVariant Model::data(const QModelIndex &index, int role) const
{
  if ((role != Qt::DisplayRole && role != Qt::EditRole) || !index.isValid())
    return {};

  auto ptr = static_cast<Component *>(index.internalPointer());
  SOFT_ASSERT(ptr, return {});

  switch (index.column()) {
    case int(Column::Name): return ptr->name;
    case int(Column::State): return toString(ptr->state);
    case int(Column::Action): return toString(ptr->action);
    case int(Column::Version): return ptr->version;
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

  auto ptr = static_cast<Component *>(index.internalPointer());
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
  auto ptr = static_cast<Component *>(index.internalPointer());
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

ActionDelegate::ActionDelegate(QObject *parent)
  : QStyledItemDelegate(parent)
{
}

QWidget *ActionDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem & /*option*/,
                                      const QModelIndex & /*index*/) const
{
  auto combo = new QComboBox(parent);
  combo->setEditable(false);
  combo->addItems({toString(Action::NoAction), toString(Action::Remove),
                   toString(Action::Install)});
  return combo;
}

void ActionDelegate::setEditorData(QWidget *editor,
                                   const QModelIndex &index) const
{
  auto combo = qobject_cast<QComboBox *>(editor);
  SOFT_ASSERT(combo, return );
  combo->setCurrentText(index.data(Qt::EditRole).toString());
}

void ActionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model,
                                  const QModelIndex &index) const
{
  auto combo = qobject_cast<QComboBox *>(editor);
  SOFT_ASSERT(combo, return );
  model->setData(index, combo->currentIndex());
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

    if (action.first == Action::Remove) {
      QFile f(file.expandedPath);
      if (!f.exists())
        continue;
      if (!f.remove()) {
        errors_.append(QObject::tr("Failed to remove file %1. Error %2")
                           .arg(f.fileName(), f.errorString()));
      }
      continue;
    }

    if (action.first == Action::Install) {
      auto installDir = QFileInfo(file.expandedPath).absoluteDir();
      if (!installDir.exists() && !installDir.mkpath(".")) {
        errors_.append(QObject::tr("Failed to create path %1")
                           .arg(installDir.absolutePath()));
        continue;
      }
      {
        QFile existing(file.expandedPath);
        if (existing.exists() && !existing.remove()) {
          errors_.append(QObject::tr("Failed to remove file %1. Error %2")
                             .arg(existing.fileName(), existing.errorString()));
          continue;
        }
      }
      QFile f(file.downloadPath);
      if (!f.rename(file.expandedPath)) {
        errors_.append(
            QObject::tr("Failed to move file %1 to %2. Error %3")
                .arg(f.fileName(), file.expandedPath, f.errorString()));
        continue;
      }
    }
  }

  return errors_.isEmpty();
}

bool Installer::checkIsPossible()
{
  errors_.clear();

  for (const auto &action : actions_) {
    const auto &file = action.second;
    QFileInfo installDir(QFileInfo(file.expandedPath).absolutePath());

    if (action.first == Action::Remove) {
      if (!QFile::exists(file.expandedPath))
        continue;
      if (installDir.exists() && !installDir.isWritable()) {
        errors_.append(QObject::tr("Directory is not writable %1")
                           .arg(installDir.absolutePath()));
      }
      continue;
    }

    if (action.first == Action::Install) {
      if (!QFileInfo::exists(file.downloadPath)) {
        errors_.append(QObject::tr("Downloaded file not exists %1")
                           .arg(file.downloadPath));
      }
      if (installDir.exists() && !installDir.isWritable()) {
        errors_.append(QObject::tr("Directory is not writable %1")
                           .arg(installDir.absolutePath()));
      }
    }
  }
  errors_.removeDuplicates();

  return errors_.isEmpty();
}

QString Installer::errorString() const
{
  return errors_.join('\n');
}

}  // namespace update
