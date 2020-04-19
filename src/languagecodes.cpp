#include "languagecodes.h"

#include <QObject>

#define S(XXX) QStringLiteral(XXX)
#define I(XXX) LanguageId(QStringLiteral(XXX))
const std::unordered_map<LanguageId, LanguageCodes::Bundle>
    LanguageCodes::codes_{
        // clang-format off
//  {I("abk"), {I("abk"), S("ab"), S("abk"), QT_TRANSLATE_NOOP("QObject", "Abkhazian")}},
//  {I("aar"), {I("aar"), S("aa"), S("aar"), QT_TRANSLATE_NOOP("QObject", "Afar")}},
  {I("afr"), {I("afr"), S("af"), S("afr"), QT_TRANSLATE_NOOP("QObject", "Afrikaans")}},
//  {I("aka"), {I("aka"), S("ak"), S("aka"), QT_TRANSLATE_NOOP("QObject", "Akan")}},
  {I("sqi"), {I("sqi"), S("sq"), S("sqi"), QT_TRANSLATE_NOOP("QObject", "Albanian")}},
  {I("amh"), {I("amh"), S("am"), S("amh"), QT_TRANSLATE_NOOP("QObject", "Amharic")}},
  {I("ara"), {I("ara"), S("ar"), S("ara"), QT_TRANSLATE_NOOP("QObject", "Arabic")}},
//  {I("arg"), {I("arg"), S("an"), S("arg"), QT_TRANSLATE_NOOP("QObject", "Aragonese")}},
  {I("hye"), {I("hye"), S("hy"), S("hye"), QT_TRANSLATE_NOOP("QObject", "Armenian")}},
//  {I("asm"), {I("asm"), S("as"), S("asm"), QT_TRANSLATE_NOOP("QObject", "Assamese")}},
//  {I("ava"), {I("ava"), S("av"), S("ava"), QT_TRANSLATE_NOOP("QObject", "Avaric")}},
//  {I("ave"), {I("ave"), S("ae"), S("ave"), QT_TRANSLATE_NOOP("QObject", "Avestan")}},
//  {I("aym"), {I("aym"), S("ay"), S("aym"), QT_TRANSLATE_NOOP("QObject", "Aymara")}},
  {I("aze"), {I("aze"), S("az"), S("aze"), QT_TRANSLATE_NOOP("QObject", "Azerbaijani")}},
//  {I("bam"), {I("bam"), S("bm"), S("bam"), QT_TRANSLATE_NOOP("QObject", "Bambara")}},
//  {I("bak"), {I("bak"), S("ba"), S("bak"), QT_TRANSLATE_NOOP("QObject", "Bashkir")}},
  {I("eus"), {I("eus"), S("eu"), S("eus"), QT_TRANSLATE_NOOP("QObject", "Basque")}},
  {I("bel"), {I("bel"), S("be"), S("bel"), QT_TRANSLATE_NOOP("QObject", "Belarusian")}},
  {I("ben"), {I("ben"), S("bn"), S("ben"), QT_TRANSLATE_NOOP("QObject", "Bengali")}},
//  {I("bih"), {I("bih"), S("bh"), S("bih"), QT_TRANSLATE_NOOP("QObject", "Bihari languages")}},
//  {I("bis"), {I("bis"), S("bi"), S("bis"), QT_TRANSLATE_NOOP("QObject", "Bislama")}},
  {I("bos"), {I("bos"), S("bs"), S("bos"), QT_TRANSLATE_NOOP("QObject", "Bosnian")}},
//  {I("bre"), {I("bre"), S("br"), S("bre"), QT_TRANSLATE_NOOP("QObject", "Breton")}},
  {I("bul"), {I("bul"), S("bg"), S("bul"), QT_TRANSLATE_NOOP("QObject", "Bulgarian")}},
  {I("mya"), {I("mya"), S("my"), S("mya"), QT_TRANSLATE_NOOP("QObject", "Burmese")}},
  {I("cat"), {I("cat"), S("ca"), S("cat"), QT_TRANSLATE_NOOP("QObject", "Catalan")}},
//  {I("cha"), {I("cha"), S("ch"), S("cha"), QT_TRANSLATE_NOOP("QObject", "Chamorro")}},
//  {I("che"), {I("che"), S("ce"), S("che"), QT_TRANSLATE_NOOP("QObject", "Chechen")}},
//  {I("nya"), {I("nya"), S("ny"), S("nya"), QT_TRANSLATE_NOOP("QObject", "Chichewa, Chewa, Nyanja")}},
//  {I("chi"), {I("chi"), S("zh"), S("chi"), QT_TRANSLATE_NOOP("QObject", "Chinese")}},
//  {I("chv"), {I("chv"), S("cv"), S("chv"), QT_TRANSLATE_NOOP("QObject", "Chuvash")}},
//  {I("cor"), {I("cor"), S("kw"), S("cor"), QT_TRANSLATE_NOOP("QObject", "Cornish")}},
  {I("cos"), {I("cos"), S("co"), S("cos"), QT_TRANSLATE_NOOP("QObject", "Corsican")}},
//  {I("cre"), {I("cre"), S("cr"), S("cre"), QT_TRANSLATE_NOOP("QObject", "Cree")}},
  {I("hrv"), {I("hrv"), S("hr"), S("hrv"), QT_TRANSLATE_NOOP("QObject", "Croatian")}},
  {I("ces"), {I("ces"), S("cs"), S("ces"), QT_TRANSLATE_NOOP("QObject", "Czech")}},
  {I("dan"), {I("dan"), S("da"), S("dan"), QT_TRANSLATE_NOOP("QObject", "Danish")}},
//  {I("div"), {I("div"), S("dv"), S("div"), QT_TRANSLATE_NOOP("QObject", "Divehi, Dhivehi, Maldivian")}},
  {I("nld"), {I("nld"), S("nl"), S("nld"), QT_TRANSLATE_NOOP("QObject", "Dutch")}},
//  {I("dzo"), {I("dzo"), S("dz"), S("dzo"), QT_TRANSLATE_NOOP("QObject", "Dzongkha")}},
  {I("eng"), {I("eng"), S("en"), S("eng"), QT_TRANSLATE_NOOP("QObject", "English")}},
  {I("epo"), {I("epo"), S("eo"), S("epo"), QT_TRANSLATE_NOOP("QObject", "Esperanto")}},
  {I("est"), {I("est"), S("et"), S("est"), QT_TRANSLATE_NOOP("QObject", "Estonian")}},
//  {I("ewe"), {I("ewe"), S("ee"), S("ewe"), QT_TRANSLATE_NOOP("QObject", "Ewe")}},
//  {I("fao"), {I("fao"), S("fo"), S("fao"), QT_TRANSLATE_NOOP("QObject", "Faroese")}},
//  {I("fij"), {I("fij"), S("fj"), S("fij"), QT_TRANSLATE_NOOP("QObject", "Fijian")}},
  {I("fin"), {I("fin"), S("fi"), S("fin"), QT_TRANSLATE_NOOP("QObject", "Finnish")}},
  {I("fra"), {I("fra"), S("fr"), S("fra"), QT_TRANSLATE_NOOP("QObject", "French")}},
//  {I("ful"), {I("ful"), S("ff"), S("ful"), QT_TRANSLATE_NOOP("QObject", "Fulah")}},
  {I("glg"), {I("glg"), S("gl"), S("glg"), QT_TRANSLATE_NOOP("QObject", "Galician")}},
  {I("kat"), {I("kat"), S("ka"), S("kat"), QT_TRANSLATE_NOOP("QObject", "Georgian")}},
  {I("deu"), {I("deu"), S("de"), S("deu"), QT_TRANSLATE_NOOP("QObject", "German")}},
  {I("ell"), {I("ell"), S("el"), S("ell"), QT_TRANSLATE_NOOP("QObject", "Greek")}},
//  {I("grn"), {I("grn"), S("gn"), S("grn"), QT_TRANSLATE_NOOP("QObject", "Guarani")}},
  {I("guj"), {I("guj"), S("gu"), S("guj"), QT_TRANSLATE_NOOP("QObject", "Gujarati")}},
  {I("hat"), {I("hat"), S("ht"), S("hat"), QT_TRANSLATE_NOOP("QObject", "Haitian")}},
  {I("hau"), {I("hau"), S("ha"), S(""), QT_TRANSLATE_NOOP("QObject", "Hausa")}},
  {I("heb"), {I("heb"), S("he"), S("heb"), QT_TRANSLATE_NOOP("QObject", "Hebrew")}},
//  {I("her"), {I("her"), S("hz"), S("her"), QT_TRANSLATE_NOOP("QObject", "Herero")}},
  {I("hin"), {I("hin"), S("hi"), S("hin"), QT_TRANSLATE_NOOP("QObject", "Hindi")}},
//  {I("hmo"), {I("hmo"), S("ho"), S("hmo"), QT_TRANSLATE_NOOP("QObject", "Hiri Motu")}},
  {I("hun"), {I("hun"), S("hu"), S("hun"), QT_TRANSLATE_NOOP("QObject", "Hungarian")}},
//  {I("ina"), {I("ina"), S("ia"), S("ina"), QT_TRANSLATE_NOOP("QObject", "Interlingua (International Auxiliary Language Association)")}},
  {I("ind"), {I("ind"), S("id"), S("ind"), QT_TRANSLATE_NOOP("QObject", "Indonesian")}},
//  {I("ile"), {I("ile"), S("ie"), S("ile"), QT_TRANSLATE_NOOP("QObject", "Interlingue, Occidental")}},
  {I("gle"), {I("gle"), S("ga"), S("gle"), QT_TRANSLATE_NOOP("QObject", "Irish")}},
  {I("ibo"), {I("ibo"), S("ig"), S(""), QT_TRANSLATE_NOOP("QObject", "Igbo")}},
//  {I("ipk"), {I("ipk"), S("ik"), S("ipk"), QT_TRANSLATE_NOOP("QObject", "Inupiaq")}},
//  {I("ido"), {I("ido"), S("io"), S("ido"), QT_TRANSLATE_NOOP("QObject", "Ido")}},
  {I("isl"), {I("isl"), S("is"), S("isl"), QT_TRANSLATE_NOOP("QObject", "Icelandic")}},
  {I("ita"), {I("ita"), S("it"), S("ita"), QT_TRANSLATE_NOOP("QObject", "Italian")}},
//  {I("iku"), {I("iku"), S("iu"), S("iku"), QT_TRANSLATE_NOOP("QObject", "Inuktitut")}},
  {I("jpn"), {I("jpn"), S("ja"), S("jpn"), QT_TRANSLATE_NOOP("QObject", "Japanese")}},
  {I("jav"), {I("jav"), S("jv"), S("jav"), QT_TRANSLATE_NOOP("QObject", "Javanese")}},
//  {I("kal"), {I("kal"), S("kl"), S("kal"), QT_TRANSLATE_NOOP("QObject", "Kalaallisut, Greenlandic")}},
  {I("kan"), {I("kan"), S("kn"), S("kan"), QT_TRANSLATE_NOOP("QObject", "Kannada")}},
//  {I("kau"), {I("kau"), S("kr"), S("kau"), QT_TRANSLATE_NOOP("QObject", "Kanuri")}},
//  {I("kas"), {I("kas"), S("ks"), S("kas"), QT_TRANSLATE_NOOP("QObject", "Kashmiri")}},
  {I("kaz"), {I("kaz"), S("kk"), S("kaz"), QT_TRANSLATE_NOOP("QObject", "Kazakh")}},
//  {I("khm"), {I("khm"), S("km"), S("khm"), QT_TRANSLATE_NOOP("QObject", "Central Khmer")}},
//  {I("kik"), {I("kik"), S("ki"), S("kik"), QT_TRANSLATE_NOOP("QObject", "Kikuyu, Gikuyu")}},
//  {I("kin"), {I("kin"), S("rw"), S("kin"), QT_TRANSLATE_NOOP("QObject", "Kinyarwanda")}},
  {I("kir"), {I("kir"), S("ky"), S("kir"), QT_TRANSLATE_NOOP("QObject", "Kyrgyz")}},
//  {I("kom"), {I("kom"), S("kv"), S("kom"), QT_TRANSLATE_NOOP("QObject", "Komi")}},
//  {I("kon"), {I("kon"), S("kg"), S("kon"), QT_TRANSLATE_NOOP("QObject", "Kongo")}},
  {I("kor"), {I("kor"), S("ko"), S("kor"), QT_TRANSLATE_NOOP("QObject", "Korean")}},
  {I("kur"), {I("kur"), S("ku"), S(""), QT_TRANSLATE_NOOP("QObject", "Kurdish")}},
//  {I("kua"), {I("kua"), S("kj"), S("kua"), QT_TRANSLATE_NOOP("QObject", "Kuanyama, Kwanyama")}},
  {I("lat"), {I("lat"), S("la"), S("lat"), QT_TRANSLATE_NOOP("QObject", "Latin")}},
  {I("ltz"), {I("ltz"), S("lb"), S("ltz"), QT_TRANSLATE_NOOP("QObject", "Luxembourgish")}},
//  {I("lug"), {I("lug"), S("lg"), S("lug"), QT_TRANSLATE_NOOP("QObject", "Ganda")}},
//  {I("lim"), {I("lim"), S("li"), S("lim"), QT_TRANSLATE_NOOP("QObject", "Limburgan, Limburger, Limburgish")}},
//  {I("lin"), {I("lin"), S("ln"), S("lin"), QT_TRANSLATE_NOOP("QObject", "Lingala")}},
  {I("lao"), {I("lao"), S("lo"), S("lao"), QT_TRANSLATE_NOOP("QObject", "Lao")}},
  {I("lit"), {I("lit"), S("lt"), S("lit"), QT_TRANSLATE_NOOP("QObject", "Lithuanian")}},
//  {I("lub"), {I("lub"), S("lu"), S("lub"), QT_TRANSLATE_NOOP("QObject", "Luba-Katanga")}},
  {I("lav"), {I("lav"), S("lv"), S("lav"), QT_TRANSLATE_NOOP("QObject", "Latvian")}},
//  {I("glv"), {I("glv"), S("gv"), S("glv"), QT_TRANSLATE_NOOP("QObject", "Manx")}},
  {I("mkd"), {I("mkd"), S("mk"), S("mkd"), QT_TRANSLATE_NOOP("QObject", "Macedonian")}},
  {I("mlg"), {I("mlg"), S("mg"), S(""), QT_TRANSLATE_NOOP("QObject", "Malagasy")}},
  {I("msa"), {I("msa"), S("ms"), S("msa"), QT_TRANSLATE_NOOP("QObject", "Malay")}},
  {I("mal"), {I("mal"), S("ml"), S("mal"), QT_TRANSLATE_NOOP("QObject", "Malayalam")}},
  {I("mlt"), {I("mlt"), S("mt"), S("mlt"), QT_TRANSLATE_NOOP("QObject", "Maltese")}},
  {I("mri"), {I("mri"), S("mi"), S("mri"), QT_TRANSLATE_NOOP("QObject", "Maori")}},
  {I("mar"), {I("mar"), S("mr"), S("mar"), QT_TRANSLATE_NOOP("QObject", "Marathi")}},
//  {I("mah"), {I("mah"), S("mh"), S("mah"), QT_TRANSLATE_NOOP("QObject", "Marshallese")}},
  {I("mon"), {I("mon"), S("mn"), S("mon"), QT_TRANSLATE_NOOP("QObject", "Mongolian")}},
//  {I("nau"), {I("nau"), S("na"), S("nau"), QT_TRANSLATE_NOOP("QObject", "Nauru")}},
//  {I("nav"), {I("nav"), S("nv"), S("nav"), QT_TRANSLATE_NOOP("QObject", "Navajo, Navaho")}},
//  {I("nde"), {I("nde"), S("nd"), S("nde"), QT_TRANSLATE_NOOP("QObject", "North Ndebele")}},
  {I("nep"), {I("nep"), S("ne"), S("nep"), QT_TRANSLATE_NOOP("QObject", "Nepali")}},
//  {I("ndo"), {I("ndo"), S("ng"), S("ndo"), QT_TRANSLATE_NOOP("QObject", "Ndonga")}},
//  {I("nob"), {I("nob"), S("nb"), S("nob"), QT_TRANSLATE_NOOP("QObject", "Norwegian Bokmål")}},
//  {I("nno"), {I("nno"), S("nn"), S("nno"), QT_TRANSLATE_NOOP("QObject", "Norwegian Nynorsk")}},
  {I("nor"), {I("nor"), S("no"), S("nor"), QT_TRANSLATE_NOOP("QObject", "Norwegian")}},
//  {I("iii"), {I("iii"), S("ii"), S("iii"), QT_TRANSLATE_NOOP("QObject", "Sichuan Yi, Nuosu")}},
//  {I("nbl"), {I("nbl"), S("nr"), S("nbl"), QT_TRANSLATE_NOOP("QObject", "South Ndebele")}},
//  {I("oci"), {I("oci"), S("oc"), S("oci"), QT_TRANSLATE_NOOP("QObject", "Occitan")}},
//  {I("oji"), {I("oji"), S("oj"), S("oji"), QT_TRANSLATE_NOOP("QObject", "Ojibwa")}},
//  {I("chu"), {I("chu"), S("cu"), S("chu"), QT_TRANSLATE_NOOP("QObject", "Church Slavic, Old Slavonic, Church Slavonic, Old Bulgarian, Old Church Slavonic")}},
//  {I("orm"), {I("orm"), S("om"), S("orm"), QT_TRANSLATE_NOOP("QObject", "Oromo")}},
//  {I("ori"), {I("ori"), S("or"), S("ori"), QT_TRANSLATE_NOOP("QObject", "Oriya")}},
//  {I("oss"), {I("oss"), S("os"), S("oss"), QT_TRANSLATE_NOOP("QObject", "Ossetian, Ossetic")}},
  {I("pan"), {I("pan"), S("pa"), S("pan"), QT_TRANSLATE_NOOP("QObject", "Punjabi")}},
//  {I("pli"), {I("pli"), S("pi"), S("pli"), QT_TRANSLATE_NOOP("QObject", "Pali")}},
  {I("fas"), {I("fas"), S("fa"), S("fas"), QT_TRANSLATE_NOOP("QObject", "Persian")}},
  {I("pol"), {I("pol"), S("pl"), S("pol"), QT_TRANSLATE_NOOP("QObject", "Polish")}},
  {I("pus"), {I("pus"), S("ps"), S("pus"), QT_TRANSLATE_NOOP("QObject", "Pashto")}},
  {I("por"), {I("por"), S("pt"), S("por"), QT_TRANSLATE_NOOP("QObject", "Portuguese")}},
//  {I("que"), {I("que"), S("qu"), S("que"), QT_TRANSLATE_NOOP("QObject", "Quechua")}},
//  {I("roh"), {I("roh"), S("rm"), S("roh"), QT_TRANSLATE_NOOP("QObject", "Romansh")}},
//  {I("run"), {I("run"), S("rn"), S("run"), QT_TRANSLATE_NOOP("QObject", "Rundi")}},
  {I("ron"), {I("ron"), S("ro"), S("ron"), QT_TRANSLATE_NOOP("QObject", "Romanian")}},
  {I("rus"), {I("rus"), S("ru"), S("rus"), QT_TRANSLATE_NOOP("QObject", "Russian")}},
//  {I("san"), {I("san"), S("sa"), S("san"), QT_TRANSLATE_NOOP("QObject", "Sanskrit")}},
//  {I("srd"), {I("srd"), S("sc"), S("srd"), QT_TRANSLATE_NOOP("QObject", "Sardinian")}},
  {I("snd"), {I("snd"), S("sd"), S("snd"), QT_TRANSLATE_NOOP("QObject", "Sindhi")}},
//  {I("sme"), {I("sme"), S("se"), S("sme"), QT_TRANSLATE_NOOP("QObject", "Northern Sami")}},
  {I("smo"), {I("smo"), S("sm"), S(""), QT_TRANSLATE_NOOP("QObject", "Samoan")}},
//  {I("sag"), {I("sag"), S("sg"), S("sag"), QT_TRANSLATE_NOOP("QObject", "Sango")}},
  {I("srp"), {I("srp"), S("sr"), S("srp"), QT_TRANSLATE_NOOP("QObject", "Serbian")}},
  {I("gla"), {I("gla"), S("gd"), S("gla"), QT_TRANSLATE_NOOP("QObject", "Gaelic")}},
  {I("sna"), {I("sna"), S("sn"), S(""), QT_TRANSLATE_NOOP("QObject", "Shona")}},
//  {I("sin"), {I("sin"), S("si"), S("sin"), QT_TRANSLATE_NOOP("QObject", "Sinhala, Sinhalese")}},
  {I("slk"), {I("slk"), S("sk"), S("slk"), QT_TRANSLATE_NOOP("QObject", "Slovak")}},
  {I("slv"), {I("slv"), S("sl"), S("slv"), QT_TRANSLATE_NOOP("QObject", "Slovenian")}},
  {I("som"), {I("som"), S("so"), S(""), QT_TRANSLATE_NOOP("QObject", "Somali")}},
//  {I("sot"), {I("sot"), S("st"), S("sot"), QT_TRANSLATE_NOOP("QObject", "Southern Sotho")}},
  {I("spa"), {I("spa"), S("es"), S("spa"), QT_TRANSLATE_NOOP("QObject", "Spanish")}},
//  {I("sun"), {I("sun"), S("su"), S("sun"), QT_TRANSLATE_NOOP("QObject", "Sundanese")}},
  {I("swa"), {I("swa"), S("sw"), S("swa"), QT_TRANSLATE_NOOP("QObject", "Swahili")}},
//  {I("ssw"), {I("ssw"), S("ss"), S("ssw"), QT_TRANSLATE_NOOP("QObject", "Swati")}},
  {I("swe"), {I("swe"), S("sv"), S("swe"), QT_TRANSLATE_NOOP("QObject", "Swedish")}},
  {I("tam"), {I("tam"), S("ta"), S("tam"), QT_TRANSLATE_NOOP("QObject", "Tamil")}},
  {I("tel"), {I("tel"), S("te"), S("tel"), QT_TRANSLATE_NOOP("QObject", "Telugu")}},
  {I("tgk"), {I("tgk"), S("tg"), S("tgk"), QT_TRANSLATE_NOOP("QObject", "Tajik")}},
  {I("tha"), {I("tha"), S("th"), S("tha"), QT_TRANSLATE_NOOP("QObject", "Thai")}},
//  {I("tir"), {I("tir"), S("ti"), S("tir"), QT_TRANSLATE_NOOP("QObject", "Tigrinya")}},
  {I("bod"), {I("bod"), S("bo"), S("bod"), QT_TRANSLATE_NOOP("QObject", "Tibetan")}},
//  {I("tuk"), {I("tuk"), S("tk"), S("tuk"), QT_TRANSLATE_NOOP("QObject", "Turkmen")}},
  {I("tgl"), {I("tgl"), S("tl"), S(""), QT_TRANSLATE_NOOP("QObject", "Tagalog")}},
//  {I("tsn"), {I("tsn"), S("tn"), S("tsn"), QT_TRANSLATE_NOOP("QObject", "Tswana")}},
//  {I("ton"), {I("ton"), S("to"), S("ton"), QT_TRANSLATE_NOOP("QObject", "Tonga (Tonga Islands)")}},
  {I("tur"), {I("tur"), S("tr"), S("tur"), QT_TRANSLATE_NOOP("QObject", "Turkish")}},
//  {I("tso"), {I("tso"), S("ts"), S("tso"), QT_TRANSLATE_NOOP("QObject", "Tsonga")}},
//  {I("tat"), {I("tat"), S("tt"), S("tat"), QT_TRANSLATE_NOOP("QObject", "Tatar")}},
//  {I("twi"), {I("twi"), S("tw"), S("twi"), QT_TRANSLATE_NOOP("QObject", "Twi")}},
//  {I("tah"), {I("tah"), S("ty"), S("tah"), QT_TRANSLATE_NOOP("QObject", "Tahitian")}},
//  {I("uig"), {I("uig"), S("ug"), S("uig"), QT_TRANSLATE_NOOP("QObject", "Uighur, Uyghur")}},
  {I("ukr"), {I("ukr"), S("uk"), S("ukr"), QT_TRANSLATE_NOOP("QObject", "Ukrainian")}},
  {I("urd"), {I("urd"), S("ur"), S("urd"), QT_TRANSLATE_NOOP("QObject", "Urdu")}},
  {I("uzb"), {I("uzb"), S("uz"), S("uzb"), QT_TRANSLATE_NOOP("QObject", "Uzbek")}},
//  {I("ven"), {I("ven"), S("ve"), S("ven"), QT_TRANSLATE_NOOP("QObject", "Venda")}},
  {I("vie"), {I("vie"), S("vi"), S("vie"), QT_TRANSLATE_NOOP("QObject", "Vietnamese")}},
//  {I("vol"), {I("vol"), S("vo"), S("vol"), QT_TRANSLATE_NOOP("QObject", "Volapük")}},
//  {I("wln"), {I("wln"), S("wa"), S("wln"), QT_TRANSLATE_NOOP("QObject", "Walloon")}},
  {I("cym"), {I("cym"), S("cy"), S("cym"), QT_TRANSLATE_NOOP("QObject", "Welsh")}},
//  {I("wol"), {I("wol"), S("wo"), S("wol"), QT_TRANSLATE_NOOP("QObject", "Wolof")}},
//  {I("fry"), {I("fry"), S("fy"), S("fry"), QT_TRANSLATE_NOOP("QObject", "Western Frisian")}},
//  {I("xho"), {I("xho"), S("xh"), S("xho"), QT_TRANSLATE_NOOP("QObject", "Xhosa")}},
  {I("yid"), {I("yid"), S("yi"), S("yid"), QT_TRANSLATE_NOOP("QObject", "Yiddish")}},
  {I("yor"), {I("yor"), S("yo"), S("yor"), QT_TRANSLATE_NOOP("QObject", "Yoruba")}},
//  {I("zha"), {I("zha"), S("za"), S("zha"), QT_TRANSLATE_NOOP("QObject", "Zhuang, Chuang")}},
  {I("zul"), {I("zul"), S("zu"), S(""), QT_TRANSLATE_NOOP("QObject", "Zulu")}},
  // custom
  {I("chi_sim"), {I("chi_sim"), S("zh-CN"), S("chi_sim"), QT_TRANSLATE_NOOP("QObject", "Chinese (Simplified)")}},
  {I("chi_tra"), {I("chi_tra"), S("zh-TW"), S("chi_tra"), QT_TRANSLATE_NOOP("QObject", "Chinese (Traditional)")}},
  {I("fil"), {I("fil"), S(""), S("fil"), QT_TRANSLATE_NOOP("QObject", "Filipino")}},
  {I("chr"), {I("chr"), S(""), S("chr"), QT_TRANSLATE_NOOP("QObject", "Cherokee")}},
  {I("ceb"), {I("ceb"), S(""), S("ceb"), QT_TRANSLATE_NOOP("QObject", "Cebuano")}},
  {I("syr"), {I("syr"), S(""), S("syr"), QT_TRANSLATE_NOOP("QObject", "Syriac")}},
  {I("enm"), {I("enm"), S(""), S("enm"), QT_TRANSLATE_NOOP("QObject", "English, Middle (1100-1500)")}},
  {I("frm"), {I("frm"), S(""), S("frm"), QT_TRANSLATE_NOOP("QObject", "French, Middle (ca.1400-1600)")}},
  {I("grc"), {I("grc"), S(""), S("grc"), QT_TRANSLATE_NOOP("QObject", "Greek, Ancient (to 1453)")}},
  {I("any"), {I("any"), S(""), S(""), QT_TRANSLATE_NOOP("QObject", "Any")}},
        // clang-format on
    };
#undef I
#undef S

LanguageId LanguageCodes::idForName(const QString &name)
{
  auto it = std::find_if(codes_.cbegin(), codes_.cend(),
                         [name](const std::pair<LanguageId, Bundle> &i) {
                           return name == QObject::tr(i.second.name);
                         });
  if (it != codes_.cend())
    return it->first;
  return name;
}

LanguageId LanguageCodes::idForTesseract(const QString &tesseract)
{
  auto it = std::find_if(codes_.cbegin(), codes_.cend(),
                         [tesseract](const std::pair<LanguageId, Bundle> &i) {
                           return tesseract == i.second.tesseract;
                         });
  if (it != codes_.cend())
    return it->first;
  return tesseract;
}

QString LanguageCodes::iso639_1(const LanguageId &id)
{
  auto it = codes_.find(id);
  return it != codes_.cend() ? it->second.iso639_1 : id;
}

QString LanguageCodes::tesseract(const LanguageId &id)
{
  auto it = codes_.find(id);
  return it != codes_.cend() ? it->second.tesseract : id;
}

QString LanguageCodes::name(const LanguageId &id)
{
  auto it = codes_.find(id);
  return it != codes_.cend() ? QObject::tr(it->second.name) : id;
}

std::vector<LanguageId> LanguageCodes::allIds()
{
  std::vector<LanguageId> result;
  result.reserve(codes_.size());
  for (const auto &code : codes_) result.push_back(code.first);
  return result;
}

LanguageId LanguageCodes::anyLanguageId()
{
  return "any";
}
