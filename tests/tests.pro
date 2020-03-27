CONFIG += c++17
CONFIG -= app_bundle

INCLUDEPATH += $$PWD/../external $$PWD/../src/service

SOURCES += \
  ../external/gtest/gtest-all.cc \
  ../src/service/geometryutils.cpp \
  geometryutils_test.cpp \
  main.cpp
