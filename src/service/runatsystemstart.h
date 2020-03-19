#pragma once

namespace service
{
class RunAtSystemStart
{
public:
  static bool isAvailable();
  static bool isEnabled();
  static void setEnabled(bool isOn);
};

}  // namespace service
