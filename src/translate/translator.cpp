#include "translator.h"
#include "debug.h"
#include "manager.h"
#include "settings.h"
#include "task.h"
#include "webpage.h"
#include "widgetstate.h"

#include <QBoxLayout>
#include <QCloseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QSplitter>
#include <QTabWidget>
#include <QTextEdit>
#include <QToolBar>

#include <unordered_set>

static std::map<QString, QString> loadScripts(const QString &dir,
                                              const QStringList &scriptNames)
{
  std::map<QString, QString> result;
  for (const auto &name : scriptNames) {
    QFile f(dir + QLatin1Char('/') + name);
    if (!f.open(QFile::ReadOnly))
      continue;
    const auto script = QString::fromUtf8(f.readAll());
    if (!script.isEmpty())
      result.emplace(name, script);
  }
  return result;
}

Translator::Translator(Manager &manager)
  : manager_(manager)
  , view_(nullptr)
  , url_(new QLineEdit(this))
  , loadImages_(
        new QAction(QIcon(":/icons/loadImages.png"), tr("Load images"), this))
  , tabs_(new QTabWidget(this))
{
#ifdef DEVELOP
  {
    QTcpSocket socket;
    if (socket.bind()) {
      quint16 port = socket.localPort();
      LTRACE() << "debug port" << port;
      qputenv("QTWEBENGINE_REMOTE_DEBUGGING", QString::number(port).toUtf8());
      socket.close();
    }
  }
#endif

  setObjectName("Translator");

  view_ = new QWebEngineView(this);

  auto detailsFrame = new QWidget(this);
  {
    auto toolBar = new QToolBar(this);
    toolBar->addWidget(new QLabel(tr("Url:"), this));
    toolBar->addWidget(url_);
    toolBar->addAction(loadImages_);

    auto layout = new QVBoxLayout(detailsFrame);
    layout->addWidget(toolBar);
    layout->addWidget(tabs_);
  }

  auto splitter = new QSplitter(Qt::Vertical, this);
  splitter->addWidget(view_);
  splitter->addWidget(detailsFrame);

  auto layout = new QVBoxLayout(this);
  layout->addWidget(splitter);

  startTimer(1000);

  url_->setReadOnly(true);

  loadImages_->setCheckable(true);
  connect(loadImages_, &QAction::toggled,  //
          this, &Translator::setPageLoadImages);

  connect(tabs_, &QTabWidget::currentChanged,  //
          this, &Translator::udpateCurrentPage);

  view_->setMinimumSize(200, 200);

  new WidgetState(this);
}

Translator::~Translator() = default;

void Translator::translate(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );
  queue_.push_back(task);
  processQueue();
}

void Translator::updateSettings(const Settings &settings)
{
  view_->setPage(nullptr);
  pages_.clear();
  url_->clear();

  tabs_->blockSignals(true);
  for (auto i = 0, end = tabs_->count(); i < end; ++i) {
    auto tab = tabs_->widget(0);
    tabs_->removeTab(0);
    tab->deleteLater();
  }
  tabs_->blockSignals(false);

  const auto loaded =
      loadScripts(settings.translatorsDir, settings.translators);
  if (loaded.empty()) {
    manager_.fatalError(
        tr("No translators loaded from %1 (named %2)")
            .arg(settings.translatorsDir, settings.translators.join(", ")));
    return;
  }

  for (const auto &script : loaded) {
    const auto &scriptName = script.first;
    const auto &scriptText = script.second;
    const auto pageIt = pages_.emplace(
        scriptName, std::make_unique<WebPage>(*this, scriptText, scriptName));
    SOFT_ASSERT(pageIt.second, continue);

    const auto &page = pageIt.first->second;
    page->setIgnoreSslErrors(settings.ignoreSslErrors);
    page->setTimeout(settings.translationTimeout);

    auto log = new QTextEdit(tabs_);
    tabs_->addTab(log, scriptName);

    connect(page.get(), &WebPage::log,  //
            log, &QTextEdit::append);
    connect(page.get(), &WebPage::urlChanged,  //
            this, &Translator::updateUrl);

    SOFT_ASSERT(log->document(), continue)
    log->document()->setMaximumBlockCount(1000);
  }

  if (settings.debugMode) {
    show();
  } else {
    hide();
  }
}

WebPage *Translator::currentPage() const
{
  const auto index = tabs_->currentIndex();
  if (index == -1)
    return nullptr;

  const auto name = tabs_->tabText(index);
  SOFT_ASSERT(pages_.count(name), return nullptr);

  return pages_.at(name).get();
}

void Translator::udpateCurrentPage()
{
  auto page = currentPage();
  if (!page)
    return;

  view_->setPage(page);
  QSignalBlocker blocker(loadImages_);
  loadImages_->setChecked(page->isLoadImages());
  url_->setText(page->url().toString());
}

void Translator::updateUrl()
{
  auto page = currentPage();
  if (!page)
    return;

  url_->setText(page->url().toString());
}

void Translator::setPageLoadImages(bool isOn)
{
  auto page = currentPage();
  if (!page)
    return;

  page->setLoadImages(isOn);
}

void Translator::processQueue()
{
  if (queue_.empty())
    return;

  std::unordered_set<QString> idlePages;
  std::unordered_set<Task *> busyTasks;

  for (auto &i : pages_) {
    if (i.second->isBusy())
      busyTasks.insert(i.second->task().get());
    else
      idlePages.insert(i.first);
  }

  if (idlePages.empty())
    return;

  std::vector<TaskPtr> finishedTasks;
  for (const auto &task : queue_) {
    if (idlePages.empty())
      break;

    if (busyTasks.count(task.get()))
      continue;

    if (task->translators.isEmpty()) {
      task->error = tr("All translators failed");
      finishedTasks.push_back(task);
      continue;
    }

    for (auto &translator : task->translators) {
      if (!idlePages.count(translator))
        continue;

      SOFT_ASSERT(pages_.count(translator), continue);
      pages_[translator]->start(task);
      task->translators.removeOne(translator);
      idlePages.erase(translator);
      break;
    }
  }

  if (!finishedTasks.empty()) {
    for (const auto &task : finishedTasks) markTranslated(task);
  }
}

void Translator::markTranslated(const TaskPtr &task)
{
  manager_.translated(task);
  queue_.erase(std::remove(queue_.begin(), queue_.end(), task), queue_.end());
}

void Translator::finish(const TaskPtr &task)
{
  markTranslated(task);
  processQueue();
}

void Translator::timerEvent(QTimerEvent * /*event*/)
{
  processQueue();
}

void Translator::closeEvent(QCloseEvent *event)
{
  event->ignore();
}
