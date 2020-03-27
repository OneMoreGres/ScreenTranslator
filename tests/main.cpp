#include <QCoreApplication>

#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
  QCoreApplication a(argc, argv);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
