#pragma once

#include <QLockFile>

namespace service
{
class SingleApplication
{
public:
  explicit SingleApplication(const QString &baseName = {});

  bool isValid() const;

private:
  QLockFile lockFile_;
};

}  // namespace service
