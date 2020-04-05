#pragma once

#include <QDate>
#include <QStyledItemDelegate>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;

namespace update
{
enum class State { NotAvailable, NotInstalled, UpdateAvailable, Actual };
enum class Action { NoAction, Remove, Install };

struct File {
  QVector<QUrl> urls;
  QString rawPath;
  QString expandedPath;
  QString downloadPath;
  QString md5;
  QDateTime versionDate;
  int progress{0};
};

using UserActions = std::multimap<Action, File>;

class UpdateDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit UpdateDelegate(QObject* parent = nullptr);
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
  QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option,
                        const QModelIndex& index) const override;
  void setEditorData(QWidget* editor, const QModelIndex& index) const override;
  void setModelData(QWidget* editor, QAbstractItemModel* model,
                    const QModelIndex& index) const override;
};

class Model : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum class Column {
    Name,
    State,
    Action,
    Size,
    Version,
    Progress,
    Files,
    Count
  };

  explicit Model(QObject* parent = nullptr);

  void parse(const QByteArray& data);
  void setExpansions(const std::map<QString, QString>& expansions);
  UserActions userActions() const;
  void updateStates();
  bool hasUpdates() const;
  void updateProgress(const QUrl& url, int progress);
  void resetProgress();

  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;
  bool setData(const QModelIndex& index, const QVariant& value,
               int role) override;

private:
  struct Component {
    QString name;
    State state{State::NotAvailable};
    Action action{Action::NoAction};
    QString version;
    std::vector<File> files;
    bool checkOnly{false};
    std::vector<std::unique_ptr<Component>> children;
    Component* parent{nullptr};
    int index{-1};
    int progress{0};
    int size{0};
  };

  std::unique_ptr<Component> parse(const QJsonObject& json) const;
  void updateProgress(Component& component, const QUrl& url, int progress);
  void updateState(Component& component);
  bool hasUpdates(const Component& component) const;
  void fillUserActions(UserActions& actions, Component& component) const;
  State currentState(const File& file) const;
  QString expanded(const QString& source) const;
  void emitColumnsChanged(const QModelIndex& parent);
  Component* toComponent(const QModelIndex& index) const;
  QModelIndex toIndex(const Component& component, int column) const;

  std::unique_ptr<Component> root_;
  std::map<QString, QString> expansions_;
};

class Installer
{
public:
  explicit Installer(const UserActions& actions);
  bool commit();
  QString errorString() const;

private:
  void remove(const File& file);
  void install(const File& file);
  void checkRemove(const File& file);
  void checkInstall(const File& file);
  bool checkIsPossible();

  UserActions actions_;
  QStringList errors_;
};

class Loader : public QObject
{
  Q_OBJECT
public:
  explicit Loader(const QUrl& updateUrl, QObject* parent = nullptr);

  void checkForUpdates();
  void applyUserActions();
  Model* model() const;

signals:
  void updatesAvailable();
  void updated();
  void error(const QString& error);

private:
  void handleReply(QNetworkReply* reply);
  bool handleComponentReply(QNetworkReply* reply);
  void handleUpdateReply(QNetworkReply* reply);
  QString toError(QNetworkReply& reply) const;
  void finishUpdate(const QString& error = {});
  void commitUpdate();
  void updateProgress(qint64 bytesSent, qint64 bytesTotal);
  bool startDownload(File& file);

  QNetworkAccessManager* network_;
  Model* model_;
  QUrl updateUrl_;
  QString downloadPath_;
  std::map<QNetworkReply*, File*> downloads_;
  UserActions currentActions_;
};

class AutoChecker : public QObject
{
  Q_OBJECT
public:
  explicit AutoChecker(Loader& loader, QObject* parent = nullptr);
  ~AutoChecker();

  bool isLastCheckDateChanged() const;
  QDateTime lastCheckDate() const;
  void setCheckIntervalDays(int days);
  void setLastCheckDate(const QDateTime& dt);

private:
  void handleModelReset();
  void scheduleNextCheck();

  Loader& loader_;
  bool isLastCheckDateChanged_{false};
  int checkIntervalDays_{0};
  QDateTime lastCheckDate_;
  std::unique_ptr<QTimer> timer_;
};

}  // namespace update
