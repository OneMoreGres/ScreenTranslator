#include <QDir>
#include <QSettings>

#include "LanguageHelper.h"
#include "Settings.h"
#include "StAssert.h"

LanguageHelper::LanguageHelper () {
  init ();
}

QStringList LanguageHelper::availableOcrLanguagesUi () const {
  QStringList uiItems;
  foreach (const QString &item, availableOcrLanguages_) {
    uiItems << ocrCodeToUi (item);
  }
  uiItems.sort ();
  return uiItems;
}

const QStringList &LanguageHelper::availableOcrLanguages () const {
  return availableOcrLanguages_;
}

QStringList LanguageHelper::availableOcrLanguages (const QString &path) const {
  QDir dir (path + "/tessdata/");
  if (!dir.exists ()) {
    return QStringList ();
  }
  QStringList items;
  QStringList files = dir.entryList (QStringList () << "*.traineddata", QDir::Files);
  foreach (const QString &file, files) {
    QString lang = file.left (file.indexOf ("."));
    items << lang;
  }
  return items;
}

QStringList LanguageHelper::availableOcrLanguagesUi (const QString &path) const {
  QStringList uiItems, items;
  items = availableOcrLanguages (path);
  foreach (const QString &item, items) {
    uiItems << ocrCodeToUi (item);
  }
  uiItems.sort ();
  return uiItems;
}

QStringList LanguageHelper::translateLanguagesUi () const {
  QStringList uiItems = translateLanguages_.keys ();
  uiItems.sort ();
  return uiItems;
}

QStringList LanguageHelper::translateLanguages () const {
  return translateLanguages_.values ();
}

QString LanguageHelper::translateCodeToUi (const QString &text) const {
  return translateLanguages_.key (text, text);
}

QString LanguageHelper::translateUiToCode (const QString &text) const {
  return translateLanguages_.value (text, text);
}

QString LanguageHelper::ocrCodeToUi (const QString &text) const {
  return ocrLanguages_.key (text, text);
}

QString LanguageHelper::ocrUiToCode (const QString &text) const {
  return ocrLanguages_.value (text, text);
}

QString LanguageHelper::translateForOcrCode (const QString &text) const {
  QString ocrUi = ocrUiToCode (text);
  QString translate = translateCodeToUi (ocrUi);
  if (translate == ocrUi) {
    translate = "auto";
  }
  return translate;
}

void LanguageHelper::init () {
  initOcrLanguages ();
  initTranslateLanguages ();
  updateAvailableOcrLanguages ();
}

void LanguageHelper::updateAvailableOcrLanguages () {
  availableOcrLanguages_.clear ();
  QSettings settings;
  settings.beginGroup (settings_names::recogntionGroup);
  QString tessDataPlace = settings.value (settings_names::tessDataPlace,
                                          settings_values::tessDataPlace).toString ();
  availableOcrLanguages_ = availableOcrLanguages (tessDataPlace);
}

void LanguageHelper::updateMenu (QMenu *menu, const QStringList &languages, int groupSize) const {
  ST_ASSERT (menu != NULL);
  menu->clear ();
  if (languages.isEmpty ()) {
    return;
  }

  if (languages.size () <= groupSize) {
    foreach (const QString &language, languages) {
      menu->addAction (language);
    }
  }
  else {
    int subIndex = groupSize;
    QMenu *subMenu = NULL;
    QString prevLetter;
    foreach (const QString &language, languages) {
      QString curLetter = language.left (1);
      if (++subIndex >= groupSize && prevLetter != curLetter) {
        if (subMenu != NULL) {
          subMenu->setTitle (subMenu->title () + " - " + prevLetter);
        }
        subMenu = menu->addMenu (curLetter);
        subIndex = 0;
      }
      prevLetter = curLetter;
      subMenu->addAction (language);
    }
    subMenu->setTitle (subMenu->title () + " - " + prevLetter);
  }
}

void LanguageHelper::initTranslateLanguages () {
  translateLanguages_.insert (QObject::tr ("Afrikaans"),"af");
  translateLanguages_.insert (QObject::tr ("Albanian"),"sq");
  translateLanguages_.insert (QObject::tr ("Arabic"),"ar");
  translateLanguages_.insert (QObject::tr ("Armenian"),"hy");
  translateLanguages_.insert (QObject::tr ("Azerbaijani"),"az");
  translateLanguages_.insert (QObject::tr ("Basque"),"eu");
  translateLanguages_.insert (QObject::tr ("Belarusian"),"be");
  translateLanguages_.insert (QObject::tr ("Bulgarian"),"bg");
  translateLanguages_.insert (QObject::tr ("Catalan"),"ca");
  translateLanguages_.insert (QObject::tr ("Chinese (Simplified)"),"zh-CN");
  translateLanguages_.insert (QObject::tr ("Chinese (Traditional)"),"zh-TW");
  translateLanguages_.insert (QObject::tr ("Croatian"),"hr");
  translateLanguages_.insert (QObject::tr ("Czech"),"cs");
  translateLanguages_.insert (QObject::tr ("Danish"),"da");
  translateLanguages_.insert (QObject::tr ("Dutch"),"nl");
  translateLanguages_.insert (QObject::tr ("English"),"en");
  translateLanguages_.insert (QObject::tr ("Estonian"),"et");
  translateLanguages_.insert (QObject::tr ("Filipino"),"tl");
  translateLanguages_.insert (QObject::tr ("Finnish"),"fi");
  translateLanguages_.insert (QObject::tr ("French"),"fr");
  translateLanguages_.insert (QObject::tr ("Galician"),"gl");
  translateLanguages_.insert (QObject::tr ("Georgian"),"ka");
  translateLanguages_.insert (QObject::tr ("German"),"de");
  translateLanguages_.insert (QObject::tr ("Greek"),"el");
  translateLanguages_.insert (QObject::tr ("Haitian Creole"),"ht");
  translateLanguages_.insert (QObject::tr ("Hebrew"),"iw");
  translateLanguages_.insert (QObject::tr ("Hindi"),"hi");
  translateLanguages_.insert (QObject::tr ("Hungarian"),"hu");
  translateLanguages_.insert (QObject::tr ("Icelandic"),"is");
  translateLanguages_.insert (QObject::tr ("Indonesian"),"id");
  translateLanguages_.insert (QObject::tr ("Irish"),"ga");
  translateLanguages_.insert (QObject::tr ("Italian"),"it");
  translateLanguages_.insert (QObject::tr ("Japanese"),"ja");
  translateLanguages_.insert (QObject::tr ("Korean"),"ko");
  translateLanguages_.insert (QObject::tr ("Latvian"),"lv");
  translateLanguages_.insert (QObject::tr ("Lithuanian"),"lt");
  translateLanguages_.insert (QObject::tr ("Macedonian"),"mk");
  translateLanguages_.insert (QObject::tr ("Malay"),"ms");
  translateLanguages_.insert (QObject::tr ("Maltese"),"mt");
  translateLanguages_.insert (QObject::tr ("Norwegian"),"no");
  translateLanguages_.insert (QObject::tr ("Persian"),"fa");
  translateLanguages_.insert (QObject::tr ("Polish"),"pl");
  translateLanguages_.insert (QObject::tr ("Portuguese"),"pt");
  translateLanguages_.insert (QObject::tr ("Romanian"),"ro");
  translateLanguages_.insert (QObject::tr ("Russian"),"ru");
  translateLanguages_.insert (QObject::tr ("Serbian"),"sr");
  translateLanguages_.insert (QObject::tr ("Slovak"),"sk");
  translateLanguages_.insert (QObject::tr ("Slovenian"),"sl");
  translateLanguages_.insert (QObject::tr ("Spanish"),"es");
  translateLanguages_.insert (QObject::tr ("Swahili"),"sw");
  translateLanguages_.insert (QObject::tr ("Swedish"),"sv");
  translateLanguages_.insert (QObject::tr ("Thai"),"th");
  translateLanguages_.insert (QObject::tr ("Turkish"),"tr");
  translateLanguages_.insert (QObject::tr ("Ukrainian"),"uk");
  translateLanguages_.insert (QObject::tr ("Urdu"),"ur");
  translateLanguages_.insert (QObject::tr ("Vietnamese"),"vi");
  translateLanguages_.insert (QObject::tr ("Welsh"),"cy");
  translateLanguages_.insert (QObject::tr ("Yiddish"),"yi");
}

void LanguageHelper::initOcrLanguages () {
  ocrLanguages_.insert (QObject::tr ("Ancient Greek"),"grc");
  ocrLanguages_.insert (QObject::tr ("Esperanto alternative"),"epo_alt");
  ocrLanguages_.insert (QObject::tr ("English"),"eng");
  ocrLanguages_.insert (QObject::tr ("Ukrainian"),"ukr");
  ocrLanguages_.insert (QObject::tr ("Turkish"),"tur");
  ocrLanguages_.insert (QObject::tr ("Thai"),"tha");
  ocrLanguages_.insert (QObject::tr ("Tagalog"),"tgl");
  ocrLanguages_.insert (QObject::tr ("Telugu"),"tel");
  ocrLanguages_.insert (QObject::tr ("Tamil"),"tam");
  ocrLanguages_.insert (QObject::tr ("Swedish"),"swe");
  ocrLanguages_.insert (QObject::tr ("Swahili"),"swa");
  ocrLanguages_.insert (QObject::tr ("Serbian"),"srp");
  ocrLanguages_.insert (QObject::tr ("Albanian"),"sqi");
  ocrLanguages_.insert (QObject::tr ("Spanish"),"spa");
  ocrLanguages_.insert (QObject::tr ("Slovenian"),"slv");
  ocrLanguages_.insert (QObject::tr ("Slovakian"),"slk");
  ocrLanguages_.insert (QObject::tr ("Romanian"),"ron");
  ocrLanguages_.insert (QObject::tr ("Portuguese"),"por");
  ocrLanguages_.insert (QObject::tr ("Polish"),"pol");
  ocrLanguages_.insert (QObject::tr ("Norwegian"),"nor");
  ocrLanguages_.insert (QObject::tr ("Dutch"),"nld");
  ocrLanguages_.insert (QObject::tr ("Malay"),"msa");
  ocrLanguages_.insert (QObject::tr ("Maltese"),"mlt");
  ocrLanguages_.insert (QObject::tr ("Macedonian"),"mkd");
  ocrLanguages_.insert (QObject::tr ("Malayalam"),"mal");
  ocrLanguages_.insert (QObject::tr ("Lithuanian"),"lit");
  ocrLanguages_.insert (QObject::tr ("Latvian"),"lav");
  ocrLanguages_.insert (QObject::tr ("Korean"),"kor");
  ocrLanguages_.insert (QObject::tr ("Kannada"),"kan");
  ocrLanguages_.insert (QObject::tr ("Italian"),"ita");
  ocrLanguages_.insert (QObject::tr ("Icelandic"),"isl");
  ocrLanguages_.insert (QObject::tr ("Indonesian"),"ind");
  ocrLanguages_.insert (QObject::tr ("Cherokee"),"chr");
  ocrLanguages_.insert (QObject::tr ("Hungarian"),"hun");
  ocrLanguages_.insert (QObject::tr ("Croatian"),"hrv");
  ocrLanguages_.insert (QObject::tr ("Hindi"),"hin");
  ocrLanguages_.insert (QObject::tr ("Hebrew"),"heb");
  ocrLanguages_.insert (QObject::tr ("Galician"),"glg");
  ocrLanguages_.insert (QObject::tr ("Middle French (ca. 1400-1600)"),"frm");
  ocrLanguages_.insert (QObject::tr ("Frankish"),"frk");
  ocrLanguages_.insert (QObject::tr ("French"),"fra");
  ocrLanguages_.insert (QObject::tr ("Finnish"),"fin");
  ocrLanguages_.insert (QObject::tr ("Basque"),"eus");
  ocrLanguages_.insert (QObject::tr ("Estonian"),"est");
  ocrLanguages_.insert (QObject::tr ("Math / equation"),"equ");
  ocrLanguages_.insert (QObject::tr ("Esperanto"),"epo");
  ocrLanguages_.insert (QObject::tr ("Middle English (1100-1500)"),"enm");
  ocrLanguages_.insert (QObject::tr ("Greek"),"ell");
  ocrLanguages_.insert (QObject::tr ("German"),"deu");
  ocrLanguages_.insert (QObject::tr ("Danish"),"dan");
  ocrLanguages_.insert (QObject::tr ("Czech"),"ces");
  ocrLanguages_.insert (QObject::tr ("Catalan"),"cat");
  ocrLanguages_.insert (QObject::tr ("Bulgarian"),"bul");
  ocrLanguages_.insert (QObject::tr ("Bengali"),"ben");
  ocrLanguages_.insert (QObject::tr ("Belarusian"),"bel");
  ocrLanguages_.insert (QObject::tr ("Azerbaijani"),"aze");
  ocrLanguages_.insert (QObject::tr ("Arabic"),"ara");
  ocrLanguages_.insert (QObject::tr ("Afrikaans"),"afr");
  ocrLanguages_.insert (QObject::tr ("Japanese"),"jpn");
  ocrLanguages_.insert (QObject::tr ("Chinese (Simplified)"),"chi_sim");
  ocrLanguages_.insert (QObject::tr ("Chinese (Traditional)"),"chi_tra");
  ocrLanguages_.insert (QObject::tr ("Russian"),"rus");
  ocrLanguages_.insert (QObject::tr ("Vietnamese"),"vie");
}
