#pragma once

#include <QDate>
#include <QStyledItemDelegate>
#include <QUrl>

class QNetworkAccessManager;
class QNetworkReply;
class QTreeView;

namespace update
{
enum class State { NotAvailable, NotInstalled, UpdateAvailable, Actual };
enum class Action { Install, Remove };

class Updater;

struct File {
  QVector<QUrl> urls;
  QString rawPath;
  QString expandedPath;
  QString md5;
  QDateTime versionDate;
  int progress{0};
};

class UpdateDelegate : public QStyledItemDelegate
{
  Q_OBJECT
public:
  explicit UpdateDelegate(QObject* parent = nullptr);
  void paint(QPainter* painter, const QStyleOptionViewItem& option,
             const QModelIndex& index) const override;
};

class Model : public QAbstractItemModel
{
  Q_OBJECT
public:
  enum class Column { Name, State, Size, Version, Progress, Count };

  explicit Model(Updater& updater);

  QString parse(const QByteArray& data);
  void setExpansions(const QHash<QString, QString>& expansions);
  void updateStates();
  bool hasUpdates() const;
  void updateProgress(const QUrl& url, int progress);
  void selectAllUpdates();
  void tryAction(Action action, const QModelIndex& index);

  QModelIndex index(int row, int column,
                    const QModelIndex& parent) const override;
  QModelIndex parent(const QModelIndex& child) const override;
  int rowCount(const QModelIndex& parent) const override;
  int columnCount(const QModelIndex& parent) const override;
  QVariant headerData(int section, Qt::Orientation orientation,
                      int role) const override;
  QVariant data(const QModelIndex& index, int role) const override;
  Qt::ItemFlags flags(const QModelIndex& index) const override;

private:
  struct Component {
    QString name;
    State state{State::NotAvailable};
    QString version;
    QVector<File> files;
    bool checkOnly{false};
    std::vector<std::unique_ptr<Component>> children;
    Component* parent{nullptr};
    int index{-1};
    int progress{0};
    int size{0};
  };

  std::unique_ptr<Component> parse(const QJsonObject& json) const;
  State currentState(const File& file) const;
  QString expanded(const QString& source) const;
  Component* toComponent(const QModelIndex& index) const;
  QModelIndex toIndex(const Component& component, int column) const;

  Updater& updater_;
  std::unique_ptr<Component> root_;
  QHash<QString, QString> expansions_;
};

class Loader : public QObject
{
  Q_OBJECT
public:
  using Urls = QVector<QUrl>;
  explicit Loader(Updater& updater);

  void download(const Urls& urls);

private:
  void start(const Urls& urls, const QUrl& previous, const QString& error);
  void handleReply(QNetworkReply* reply);

  Updater& updater_;
  QNetworkAccessManager* network_;
  QHash<QNetworkReply*, Urls> downloads_;
};

class Installer
{
public:
  void remove(const File& file);
  void install(const File& file, const QByteArray& data);
  void checkInstall(const File& file);
  const QString& error() const;

private:
  QString error_;
};

class AutoChecker : public QObject
{
  Q_OBJECT
public:
  AutoChecker(Updater& updater, int intervalDays, const QDateTime& lastCheck);
  ~AutoChecker();

  const QDateTime& lastCheckDate() const;

private:
  void updateLastCheckDate();
  void scheduleNextCheck();

  Updater& updater_;
  int checkIntervalDays_{0};
  QDateTime lastCheckDate_;
  std::unique_ptr<QTimer> timer_;
};

class Updater : public QObject
{
  Q_OBJECT
public:
  explicit Updater(const QVector<QUrl>& updateUrls);

  void initView(QTreeView* view);
  void setExpansions(const QHash<QString, QString>& expansions);
  void checkForUpdates();

  QDateTime lastUpdateCheck() const;
  void setAutoUpdate(int intervalDays, const QDateTime& lastCheck);

  void applyAction(Action action, const QVector<File>& files);
  void downloaded(const QUrl& url, const QByteArray& data);
  void updateProgress(const QUrl& url, qint64 bytesSent, qint64 bytesTotal);
  void downloadFailed(const QUrl& url, const QString& error);

signals:
  void checkedForUpdates();
  void updatesAvailable();
  void updated();
  void error(const QString& error);

private:
  void handleModelDoubleClick(const QModelIndex& index);
  void showModelContextMenu();
  int findDownload(const QUrl& url) const;
  QModelIndex fromProxy(const QModelIndex& index) const;

  std::unique_ptr<Model> model_;
  std::unique_ptr<Loader> loader_;
  std::unique_ptr<AutoChecker> autoChecker_;
  QVector<QUrl> updateUrls_;
  QVector<File> downloading_;
};

}  // namespace update
