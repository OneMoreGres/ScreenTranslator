#include <gtest/gtest.h>

#include "updates.h"

#include <QDebug>
#include <QFileInfo>
#include <QSignalSpy>

using namespace update;

namespace
{
const auto f1 = "from1.txt";
const auto t1 = "test/to1.txt";
const auto data = "sample";

File toFile(const QString& from, const QString& to)
{
  File result;
  result.downloadPath = from;
  result.expandedPath = to;
  return result;
}

bool writeFile(const QString& name, const QByteArray& data)
{
  QFile f(name);
  if (!f.open(QFile::WriteOnly))
    return false;
  f.write(data);
  return true;
}

QByteArray readFile(const QString& name)
{
  QFile f(name);
  if (!f.open(QFile::ReadOnly))
    return {};
  return f.readAll();
}

bool removeFile(const QString& name)
{
  if (!QFile::exists(name))
    return true;
  return QFile::remove(name);
}
}  // namespace

TEST(UpdateInstaller, Empty)
{
  UserActions actions;
  Installer testee(actions);
  ASSERT_TRUE(testee.commit());
}

TEST(UpdateInstaller, SuccessInstall)
{
  ASSERT_TRUE(removeFile(t1));
  ASSERT_TRUE(writeFile(f1, data));

  UserActions actions{{Action::Install, toFile(f1, t1)}};
  Installer testee(actions);
  ASSERT_TRUE(testee.commit());
  ASSERT_EQ(data, readFile(t1));
}

TEST(UpdateInstaller, SuccessRemove)
{
  ASSERT_TRUE(writeFile(f1, data));

  UserActions actions{{Action::Remove, toFile(f1, f1)}};
  Installer testee(actions);
  ASSERT_TRUE(testee.commit());
  ASSERT_FALSE(QFile::exists(f1));
}

TEST(UpdateInstaller, FailInstallNoSource)
{
  ASSERT_TRUE(removeFile(f1));

  UserActions actions{{Action::Install, toFile(f1, t1)}};
  Installer testee(actions);
  ASSERT_FALSE(testee.commit());
}

TEST(UpdateInstaller, FailInstallNoWritable)
{
  const auto t1 = "/foo.txt";
  QFile f(t1);
  ASSERT_FALSE(f.isWritable());

  UserActions actions{{Action::Install, toFile(f1, t1)}};
  Installer testee(actions);
  ASSERT_FALSE(testee.commit());
}

TEST(UpdateInstaller, FailRemove)
{
  const auto f1 = QString("/var/log/faillog");
  QFile f(f1);
  if (!f.exists())
    return;
  ASSERT_FALSE(QFile::copy(f1, f1 + "1"));  // non writable

  UserActions actions{{Action::Remove, toFile(f1, f1)}};
  Installer testee(actions);
  ASSERT_FALSE(testee.commit());
}

TEST(UpdateModel, ParseFail)
{
  const auto updates = R"({
})";

  Model testee;
  const auto error = testee.parse(updates);

  ASSERT_FALSE(error.isEmpty());
  ASSERT_EQ(0, testee.rowCount({}));
}

TEST(UpdateModel, Parse)
{
  const auto updates = R"({
"version":1
,"app":{
"comp1":{"version":"3.0.0", "files":[{"path":"$appdir$/screen-translator.exe", "md5":""}]}
}
,"recognizers": {
"Afrikaans":{"files":[
{"url":"https://example.com/test1", "path":"$tessdata$/afr.traineddata", "date":"2020-03-09T08:28:45+01:00", "size":12800552}
]}
}
})";

  Model testee;
  const auto error = testee.parse(updates);

  ASSERT_TRUE(error.isEmpty());
  ASSERT_EQ(2, testee.rowCount({}));

  const auto app = testee.index(0, 0, {});
  const auto comp1 = testee.index(0, 0, app);
  const auto comp1Name =
      comp1.sibling(comp1.row(), int(Model::Column::Name)).data().toString();
  ASSERT_EQ("comp1", comp1Name);
}

TEST(UpdateLoader, InstallFile)
{
  const auto uf = "updates.json";
  const auto url = QUrl::fromLocalFile(QFileInfo(f1).absoluteFilePath());
  const auto updates = QString(R"({
"version":1,
"comp1":{"files":[{"url":"%1", "path":"./%2", "md5":"1"}]}
})")
                           .arg(url.toString(), t1);
  ASSERT_TRUE(writeFile(uf, updates.toUtf8()));
  ASSERT_TRUE(writeFile(f1, updates.toUtf8()));
  ASSERT_TRUE(removeFile(t1));

  Loader testee({QUrl::fromLocalFile(QFileInfo(uf).absoluteFilePath())});
  testee.checkForUpdates();
  QCoreApplication::processEvents();

  auto model = testee.model();
  ASSERT_NE(nullptr, model);
  ASSERT_EQ(1, model->rowCount({}));

  const auto comp1 = model->index(0, int(Model::Column::Action), {});
  model->setData(comp1, int(Action::Install), Qt::EditRole);

  QSignalSpy okSpy(&testee, &Loader::updated);
  QSignalSpy errorSpy(&testee, &Loader::error);

  testee.applyUserActions();

  QCoreApplication::processEvents();
  ASSERT_EQ(1, okSpy.count());
  ASSERT_EQ(0, errorSpy.count());

  ASSERT_TRUE(QFile::exists(f1));
  ASSERT_EQ(updates, readFile(t1));
}
