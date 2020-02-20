#pragma once

#include <QLockFile>

class SingleApplication
{
public:
  explicit SingleApplication(const QString &baseName = {});

  bool isValid() const;

private:
  QLockFile lockFile_;
};
