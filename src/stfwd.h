#pragma once

#include <memory>

class QString;
class QStringList;

class Manager;
class Settings;
class Task;
class Translator;
class TrayIcon;
class Capturer;
class Representer;
class Translator;
class Corrector;
class Recognizer;
class CaptureAreaSelector;

namespace update
{
class Loader;
class AutoChecker;
}  // namespace update

using TaskPtr = std::shared_ptr<Task>;
using LanguageId = QString;
using LanguageIds = QStringList;
