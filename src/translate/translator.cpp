#include "translator.h"
#include "debug.h"
#include "languagecodes.h"
#include "manager.h"
#include "settings.h"
#include "task.h"
#include "webpage.h"
#include "widgetstate.h"

#include <QBoxLayout>
#include <QCloseEvent>
#include <QDir>
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

Translator::Translator(Manager &manager, const Settings &settings)
  : manager_(manager)
  , settings_(settings)
  , view_(nullptr)
  , url_(new QLineEdit(this))
  , loadImages_(
        new QAction(QIcon(":/icons/loadImages.png"), tr("Load images"), this))
  , showDebugAction_(new QAction(QIcon(":/icons/debug.png"), tr("Debug"), this))
  , tabs_(new QTabWidget(this))
{
  {
    QTcpSocket socket;
    if (socket.bind()) {
      debugPort_ = socket.localPort();
      qputenv("QTWEBENGINE_REMOTE_DEBUGGING",
              QString::number(debugPort_).toUtf8());
      socket.close();
    }
  }

  setObjectName("Translator");
  setWindowTitle(tr("Translator"));

  view_ = new QWebEngineView(this);

  auto detailsFrame = new QWidget(this);
  {
    auto toolBar = new QToolBar(this);
    toolBar->addWidget(new QLabel(tr("Url:"), this));
    toolBar->addWidget(url_);
    toolBar->addAction(loadImages_);
    toolBar->addAction(showDebugAction_);

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
  connect(showDebugAction_, &QAction::triggered,  //
          this, &Translator::showDebugView);

  connect(tabs_, &QTabWidget::currentChanged,  //
          this, &Translator::udpateCurrentPage);

  view_->setMinimumSize(200, 200);

  new service::WidgetState(this);
}

Translator::~Translator() = default;

void Translator::translate(const TaskPtr &task)
{
  SOFT_ASSERT(task, return );

  if (task->corrected.isEmpty()) {
    LTRACE() << "Corrected text is empty. Skipping translation";
    manager_.translated(task);
    return;
  }

  queue_.push_back(task);
  processQueue();
}

void Translator::updateSettings()
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

  if (settings_.translators.empty()) {
    manager_.fatalError(tr("No translators selected"));
    return;
  }

  const auto loaded =
      loadScripts(settings_.translatorsDir, settings_.translators);
  if (loaded.empty()) {
    manager_.fatalError(
        tr("No translators loaded from\n%1\n(%2)")
            .arg(settings_.translatorsDir, settings_.translators.join(", ")));
    return;
  }

  for (const auto &script : loaded) createPage(script.first, script.second);
}

void Translator::createPage(const QString &scriptName,
                            const QString &scriptText)
{
  pages_.erase(scriptName);
  const auto pageIt = pages_.emplace(
      scriptName, std::make_unique<WebPage>(*this, scriptText, scriptName));
  SOFT_ASSERT(pageIt.second, return );

  const auto &page = pageIt.first->second;
  page->setIgnoreSslErrors(settings_.ignoreSslErrors);
  page->setTimeout(settings_.translationTimeout);
  page->setVisible(true);
  connect(page.get(), &WebPage::visibleChanged,  //
          page.get(), [page = page.get()](bool on) {
            if (!on)
              page->setVisible(true);
          });

  auto log = new QTextEdit(tabs_);
  tabs_->addTab(log, scriptName);

  connect(page.get(), &WebPage::log,  //
          log, &QTextEdit::append);
  connect(page.get(), &WebPage::urlChanged,  //
          this, &Translator::updateUrl);
  connect(page.get(), &WebPage::renderProcessTerminated,  //
          this,
          [this, scriptName,
           scriptText](WebPage::RenderProcessTerminationStatus status) {
            if (status != WebPage::NormalTerminationStatus)
              createPage(scriptName, scriptText);
          });

  SOFT_ASSERT(log->document(), return )
  log->document()->setMaximumBlockCount(1000);
  LTRACE() << "Created page" << LARG(scriptName);
}

void Translator::showDebugView()
{
  if (!debugView_)
    debugView_ = std::make_unique<QWebEngineView>();
  debugView_->load(
      QUrl::fromUserInput("http://localhost:" + QString::number(debugPort_)));
  debugView_->show();
  debugView_->activateWindow();
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

  auto oldPage = view_->page();
  for (auto &i : pages_) {
    if (!i.second->checkBusy()) {
      idlePages.insert(i.first);
    } else {
      busyTasks.insert(i.second->task().get());
      view_->setPage(i.second.get());
      view_->update();
    }
  }

  if (oldPage != view_->page())
    view_->setPage(oldPage);

  if (idlePages.empty())
    return;

  std::vector<TaskPtr> finishedTasks;
  for (const auto &task : queue_) {
    if (idlePages.empty())
      break;

    if (busyTasks.count(task.get()))
      continue;

    if (task->translators.isEmpty()) {
      task->error = tr("All translators failed\n%1")
                        .arg(task->translatorErrors.join("\n"));
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
      LTRACE() << "Started translation at" << translator << task;
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

QStringList Translator::availableTranslators(const QString &path)
{
  if (path.isEmpty())
    return {};

  QDir dir(path);
  if (!dir.exists())
    return {};

  const auto names = dir.entryList({"*.js"}, QDir::Files);
  return names;
}

QStringList Translator::availableLanguageNames()
{
  QStringList names;

  for (const auto &id : LanguageCodes::allIds()) {
    const auto iso = LanguageCodes::iso639_1(id);
    if (!iso.isEmpty())
      names.append(LanguageCodes::name(id));
  }

  return names;
}

void Translator::timerEvent(QTimerEvent * /*event*/)
{
  processQueue();
}
