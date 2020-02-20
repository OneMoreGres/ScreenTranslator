#include "languagecodes.h"

#include <QObject>

#define S(XXX) QStringLiteral(XXX)
#define I(XXX) LanguageId(QStringLiteral(XXX))
#define TR(XXX) QT_TR_NOOP(XXX)
const std::unordered_map<LanguageId, LanguageCodes::Bundle>
    LanguageCodes::codes_{
        // clang-format off
//  {I("abk"), {I("abk"), S("ab"), S("abk"), TR("Abkhazian")}},
//  {I("aar"), {I("aar"), S("aa"), S("aar"), TR("Afar")}},
  {I("afr"), {I("afr"), S("af"), S("afr"), TR("Afrikaans")}},
//  {I("aka"), {I("aka"), S("ak"), S("aka"), TR("Akan")}},
  {I("alb"), {I("alb"), S("sq"), S("alb"), TR("Albanian")}},
//  {I("amh"), {I("amh"), S("am"), S("amh"), TR("Amharic")}},
  {I("ara"), {I("ara"), S("ar"), S("ara"), TR("Arabic")}},
//  {I("arg"), {I("arg"), S("an"), S("arg"), TR("Aragonese")}},
  {I("arm"), {I("arm"), S("hy"), S("arm"), TR("Armenian")}},
//  {I("asm"), {I("asm"), S("as"), S("asm"), TR("Assamese")}},
//  {I("ava"), {I("ava"), S("av"), S("ava"), TR("Avaric")}},
//  {I("ave"), {I("ave"), S("ae"), S("ave"), TR("Avestan")}},
//  {I("aym"), {I("aym"), S("ay"), S("aym"), TR("Aymara")}},
  {I("aze"), {I("aze"), S("az"), S("aze"), TR("Azerbaijani")}},
//  {I("bam"), {I("bam"), S("bm"), S("bam"), TR("Bambara")}},
//  {I("bak"), {I("bak"), S("ba"), S("bak"), TR("Bashkir")}},
  {I("baq"), {I("baq"), S("eu"), S("baq"), TR("Basque")}},
  {I("bel"), {I("bel"), S("be"), S("bel"), TR("Belarusian")}},
//  {I("ben"), {I("ben"), S("bn"), S("ben"), TR("Bengali")}},
//  {I("bih"), {I("bih"), S("bh"), S("bih"), TR("Bihari languages")}},
//  {I("bis"), {I("bis"), S("bi"), S("bis"), TR("Bislama")}},
//  {I("bos"), {I("bos"), S("bs"), S("bos"), TR("Bosnian")}},
//  {I("bre"), {I("bre"), S("br"), S("bre"), TR("Breton")}},
  {I("bul"), {I("bul"), S("bg"), S("bul"), TR("Bulgarian")}},
//  {I("bur"), {I("bur"), S("my"), S("bur"), TR("Burmese")}},
  {I("cat"), {I("cat"), S("ca"), S("cat"), TR("Catalan, Valencian")}},
//  {I("cha"), {I("cha"), S("ch"), S("cha"), TR("Chamorro")}},
//  {I("che"), {I("che"), S("ce"), S("che"), TR("Chechen")}},
//  {I("nya"), {I("nya"), S("ny"), S("nya"), TR("Chichewa, Chewa, Nyanja")}},
//  {I("chi"), {I("chi"), S("zh"), S("chi"), TR("Chinese")}},
//  {I("chv"), {I("chv"), S("cv"), S("chv"), TR("Chuvash")}},
//  {I("cor"), {I("cor"), S("kw"), S("cor"), TR("Cornish")}},
//  {I("cos"), {I("cos"), S("co"), S("cos"), TR("Corsican")}},
//  {I("cre"), {I("cre"), S("cr"), S("cre"), TR("Cree")}},
  {I("hrv"), {I("hrv"), S("hr"), S("hrv"), TR("Croatian")}},
  {I("cze"), {I("cze"), S("cs"), S("cze"), TR("Czech")}},
  {I("dan"), {I("dan"), S("da"), S("dan"), TR("Danish")}},
//  {I("div"), {I("div"), S("dv"), S("div"), TR("Divehi, Dhivehi, Maldivian")}},
//  {I("dut"), {I("dut"), S("nl"), S("dut"), TR("Dutch, Flemish")}},
//  {I("dzo"), {I("dzo"), S("dz"), S("dzo"), TR("Dzongkha")}},
  {I("eng"), {I("eng"), S("en"), S("eng"), TR("English")}},
//  {I("epo"), {I("epo"), S("eo"), S("epo"), TR("Esperanto")}},
  {I("est"), {I("est"), S("et"), S("est"), TR("Estonian")}},
//  {I("ewe"), {I("ewe"), S("ee"), S("ewe"), TR("Ewe")}},
//  {I("fao"), {I("fao"), S("fo"), S("fao"), TR("Faroese")}},
//  {I("fij"), {I("fij"), S("fj"), S("fij"), TR("Fijian")}},
  {I("fin"), {I("fin"), S("fi"), S("fin"), TR("Finnish")}},
  {I("fre"), {I("fre"), S("fr"), S("fre"), TR("French")}},
//  {I("ful"), {I("ful"), S("ff"), S("ful"), TR("Fulah")}},
  {I("glg"), {I("glg"), S("gl"), S("glg"), TR("Galician")}},
  {I("geo"), {I("geo"), S("ka"), S("geo"), TR("Georgian")}},
  {I("ger"), {I("ger"), S("de"), S("ger"), TR("German")}},
  {I("gre"), {I("gre"), S("el"), S("gre"), TR("Greek")}},
//  {I("grn"), {I("grn"), S("gn"), S("grn"), TR("Guarani")}},
//  {I("guj"), {I("guj"), S("gu"), S("guj"), TR("Gujarati")}},
  {I("hat"), {I("hat"), S("ht"), S("hat"), TR("Haitian, Haitian Creole")}},
//  {I("hau"), {I("hau"), S("ha"), S("hau"), TR("Hausa")}},
  {I("heb"), {I("heb"), S("he"), S("heb"), TR("Hebrew")}},
//  {I("her"), {I("her"), S("hz"), S("her"), TR("Herero")}},
  {I("hin"), {I("hin"), S("hi"), S("hin"), TR("Hindi")}},
//  {I("hmo"), {I("hmo"), S("ho"), S("hmo"), TR("Hiri Motu")}},
  {I("hun"), {I("hun"), S("hu"), S("hun"), TR("Hungarian")}},
//  {I("ina"), {I("ina"), S("ia"), S("ina"), TR("Interlingua (International Auxiliary Language Association)")}},
  {I("ind"), {I("ind"), S("id"), S("ind"), TR("Indonesian")}},
//  {I("ile"), {I("ile"), S("ie"), S("ile"), TR("Interlingue, Occidental")}},
  {I("gle"), {I("gle"), S("ga"), S("gle"), TR("Irish")}},
//  {I("ibo"), {I("ibo"), S("ig"), S("ibo"), TR("Igbo")}},
//  {I("ipk"), {I("ipk"), S("ik"), S("ipk"), TR("Inupiaq")}},
//  {I("ido"), {I("ido"), S("io"), S("ido"), TR("Ido")}},
  {I("ice"), {I("ice"), S("is"), S("ice"), TR("Icelandic")}},
  {I("ita"), {I("ita"), S("it"), S("ita"), TR("Italian")}},
//  {I("iku"), {I("iku"), S("iu"), S("iku"), TR("Inuktitut")}},
  {I("jpn"), {I("jpn"), S("ja"), S("jpn"), TR("Japanese")}},
//  {I("jav"), {I("jav"), S("jv"), S("jav"), TR("Javanese")}},
//  {I("kal"), {I("kal"), S("kl"), S("kal"), TR("Kalaallisut, Greenlandic")}},
//  {I("kan"), {I("kan"), S("kn"), S("kan"), TR("Kannada")}},
//  {I("kau"), {I("kau"), S("kr"), S("kau"), TR("Kanuri")}},
//  {I("kas"), {I("kas"), S("ks"), S("kas"), TR("Kashmiri")}},
//  {I("kaz"), {I("kaz"), S("kk"), S("kaz"), TR("Kazakh")}},
//  {I("khm"), {I("khm"), S("km"), S("khm"), TR("Central Khmer")}},
//  {I("kik"), {I("kik"), S("ki"), S("kik"), TR("Kikuyu, Gikuyu")}},
//  {I("kin"), {I("kin"), S("rw"), S("kin"), TR("Kinyarwanda")}},
//  {I("kir"), {I("kir"), S("ky"), S("kir"), TR("Kirghiz, Kyrgyz")}},
//  {I("kom"), {I("kom"), S("kv"), S("kom"), TR("Komi")}},
//  {I("kon"), {I("kon"), S("kg"), S("kon"), TR("Kongo")}},
  {I("kor"), {I("kor"), S("ko"), S("kor"), TR("Korean")}},
//  {I("kur"), {I("kur"), S("ku"), S("kur"), TR("Kurdish")}},
//  {I("kua"), {I("kua"), S("kj"), S("kua"), TR("Kuanyama, Kwanyama")}},
//  {I("lat"), {I("lat"), S("la"), S("lat"), TR("Latin")}},
//  {I("ltz"), {I("ltz"), S("lb"), S("ltz"), TR("Luxembourgish, Letzeburgesch")}},
//  {I("lug"), {I("lug"), S("lg"), S("lug"), TR("Ganda")}},
//  {I("lim"), {I("lim"), S("li"), S("lim"), TR("Limburgan, Limburger, Limburgish")}},
//  {I("lin"), {I("lin"), S("ln"), S("lin"), TR("Lingala")}},
//  {I("lao"), {I("lao"), S("lo"), S("lao"), TR("Lao")}},
  {I("lit"), {I("lit"), S("lt"), S("lit"), TR("Lithuanian")}},
//  {I("lub"), {I("lub"), S("lu"), S("lub"), TR("Luba-Katanga")}},
  {I("lav"), {I("lav"), S("lv"), S("lav"), TR("Latvian")}},
//  {I("glv"), {I("glv"), S("gv"), S("glv"), TR("Manx")}},
  {I("mac"), {I("mac"), S("mk"), S("mac"), TR("Macedonian")}},
//  {I("mlg"), {I("mlg"), S("mg"), S("mlg"), TR("Malagasy")}},
  {I("may"), {I("may"), S("ms"), S("may"), TR("Malay")}},
//  {I("mal"), {I("mal"), S("ml"), S("mal"), TR("Malayalam")}},
  {I("mlt"), {I("mlt"), S("mt"), S("mlt"), TR("Maltese")}},
//  {I("mao"), {I("mao"), S("mi"), S("mao"), TR("Maori")}},
//  {I("mar"), {I("mar"), S("mr"), S("mar"), TR("Marathi")}},
//  {I("mah"), {I("mah"), S("mh"), S("mah"), TR("Marshallese")}},
//  {I("mon"), {I("mon"), S("mn"), S("mon"), TR("Mongolian")}},
//  {I("nau"), {I("nau"), S("na"), S("nau"), TR("Nauru")}},
//  {I("nav"), {I("nav"), S("nv"), S("nav"), TR("Navajo, Navaho")}},
//  {I("nde"), {I("nde"), S("nd"), S("nde"), TR("North Ndebele")}},
//  {I("nep"), {I("nep"), S("ne"), S("nep"), TR("Nepali")}},
//  {I("ndo"), {I("ndo"), S("ng"), S("ndo"), TR("Ndonga")}},
//  {I("nob"), {I("nob"), S("nb"), S("nob"), TR("Norwegian Bokmål")}},
//  {I("nno"), {I("nno"), S("nn"), S("nno"), TR("Norwegian Nynorsk")}},
  {I("nor"), {I("nor"), S("no"), S("nor"), TR("Norwegian")}},
//  {I("iii"), {I("iii"), S("ii"), S("iii"), TR("Sichuan Yi, Nuosu")}},
//  {I("nbl"), {I("nbl"), S("nr"), S("nbl"), TR("South Ndebele")}},
//  {I("oci"), {I("oci"), S("oc"), S("oci"), TR("Occitan")}},
//  {I("oji"), {I("oji"), S("oj"), S("oji"), TR("Ojibwa")}},
//  {I("chu"), {I("chu"), S("cu"), S("chu"), TR("Church Slavic, Old Slavonic, Church Slavonic, Old Bulgarian, Old Church Slavonic")}},
//  {I("orm"), {I("orm"), S("om"), S("orm"), TR("Oromo")}},
//  {I("ori"), {I("ori"), S("or"), S("ori"), TR("Oriya")}},
//  {I("oss"), {I("oss"), S("os"), S("oss"), TR("Ossetian, Ossetic")}},
//  {I("pan"), {I("pan"), S("pa"), S("pan"), TR("Punjabi, Panjabi")}},
//  {I("pli"), {I("pli"), S("pi"), S("pli"), TR("Pali")}},
  {I("per"), {I("per"), S("fa"), S("per"), TR("Persian")}},
  {I("pol"), {I("pol"), S("pl"), S("pol"), TR("Polish")}},
//  {I("pus"), {I("pus"), S("ps"), S("pus"), TR("Pashto, Pushto")}},
  {I("por"), {I("por"), S("pt"), S("por"), TR("Portuguese")}},
//  {I("que"), {I("que"), S("qu"), S("que"), TR("Quechua")}},
//  {I("roh"), {I("roh"), S("rm"), S("roh"), TR("Romansh")}},
//  {I("run"), {I("run"), S("rn"), S("run"), TR("Rundi")}},
  {I("rum"), {I("rum"), S("ro"), S("rum"), TR("Romanian, Moldavian, Moldovan")}},
  {I("rus"), {I("rus"), S("ru"), S("rus"), TR("Russian")}},
//  {I("san"), {I("san"), S("sa"), S("san"), TR("Sanskrit")}},
//  {I("srd"), {I("srd"), S("sc"), S("srd"), TR("Sardinian")}},
//  {I("snd"), {I("snd"), S("sd"), S("snd"), TR("Sindhi")}},
//  {I("sme"), {I("sme"), S("se"), S("sme"), TR("Northern Sami")}},
//  {I("smo"), {I("smo"), S("sm"), S("smo"), TR("Samoan")}},
//  {I("sag"), {I("sag"), S("sg"), S("sag"), TR("Sango")}},
  {I("srp"), {I("srp"), S("sr"), S("srp"), TR("Serbian")}},
//  {I("gla"), {I("gla"), S("gd"), S("gla"), TR("Gaelic, Scottish Gaelic")}},
//  {I("sna"), {I("sna"), S("sn"), S("sna"), TR("Shona")}},
//  {I("sin"), {I("sin"), S("si"), S("sin"), TR("Sinhala, Sinhalese")}},
  {I("slo"), {I("slo"), S("sk"), S("slo"), TR("Slovak")}},
  {I("slv"), {I("slv"), S("sl"), S("slv"), TR("Slovenian")}},
//  {I("som"), {I("som"), S("so"), S("som"), TR("Somali")}},
//  {I("sot"), {I("sot"), S("st"), S("sot"), TR("Southern Sotho")}},
  {I("spa"), {I("spa"), S("es"), S("spa"), TR("Spanish, Castilian")}},
//  {I("sun"), {I("sun"), S("su"), S("sun"), TR("Sundanese")}},
  {I("swa"), {I("swa"), S("sw"), S("swa"), TR("Swahili")}},
//  {I("ssw"), {I("ssw"), S("ss"), S("ssw"), TR("Swati")}},
  {I("swe"), {I("swe"), S("sv"), S("swe"), TR("Swedish")}},
//  {I("tam"), {I("tam"), S("ta"), S("tam"), TR("Tamil")}},
//  {I("tel"), {I("tel"), S("te"), S("tel"), TR("Telugu")}},
//  {I("tgk"), {I("tgk"), S("tg"), S("tgk"), TR("Tajik")}},
  {I("tha"), {I("tha"), S("th"), S("tha"), TR("Thai")}},
//  {I("tir"), {I("tir"), S("ti"), S("tir"), TR("Tigrinya")}},
//  {I("tib"), {I("tib"), S("bo"), S("tib"), TR("Tibetan")}},
//  {I("tuk"), {I("tuk"), S("tk"), S("tuk"), TR("Turkmen")}},
//  {I("tgl"), {I("tgl"), S("tl"), S("tgl"), TR("Tagalog")}},
//  {I("tsn"), {I("tsn"), S("tn"), S("tsn"), TR("Tswana")}},
//  {I("ton"), {I("ton"), S("to"), S("ton"), TR("Tonga (Tonga Islands)")}},
  {I("tur"), {I("tur"), S("tr"), S("tur"), TR("Turkish")}},
//  {I("tso"), {I("tso"), S("ts"), S("tso"), TR("Tsonga")}},
//  {I("tat"), {I("tat"), S("tt"), S("tat"), TR("Tatar")}},
//  {I("twi"), {I("twi"), S("tw"), S("twi"), TR("Twi")}},
//  {I("tah"), {I("tah"), S("ty"), S("tah"), TR("Tahitian")}},
//  {I("uig"), {I("uig"), S("ug"), S("uig"), TR("Uighur, Uyghur")}},
  {I("ukr"), {I("ukr"), S("uk"), S("ukr"), TR("Ukrainian")}},
  {I("urd"), {I("urd"), S("ur"), S("urd"), TR("Urdu")}},
//  {I("uzb"), {I("uzb"), S("uz"), S("uzb"), TR("Uzbek")}},
//  {I("ven"), {I("ven"), S("ve"), S("ven"), TR("Venda")}},
  {I("vie"), {I("vie"), S("vi"), S("vie"), TR("Vietnamese")}},
//  {I("vol"), {I("vol"), S("vo"), S("vol"), TR("Volapük")}},
//  {I("wln"), {I("wln"), S("wa"), S("wln"), TR("Walloon")}},
  {I("wel"), {I("wel"), S("cy"), S("wel"), TR("Welsh")}},
//  {I("wol"), {I("wol"), S("wo"), S("wol"), TR("Wolof")}},
//  {I("fry"), {I("fry"), S("fy"), S("fry"), TR("Western Frisian")}},
//  {I("xho"), {I("xho"), S("xh"), S("xho"), TR("Xhosa")}},
  {I("yid"), {I("yid"), S("yi"), S("yid"), TR("Yiddish")}},
//  {I("yor"), {I("yor"), S("yo"), S("yor"), TR("Yoruba")}},
//  {I("zha"), {I("zha"), S("za"), S("zha"), TR("Zhuang, Chuang")}},
//  {I("zul"), {I("zul"), S("zu"), S("zul"), TR("Zulu")}},
  // custom
  {I("chi_sim"), {I("chi_sim"), S("zh-CN"), S("chi_sim"), TR("Chinese (Simplified)")}},
  {I("chi_tra"), {I("chi_tra"), S("zh-TW"), S("chi_tra"), TR("Chinese (Traditional)")}},
        // clang-format on
    };
#undef I
#undef S

std::optional<LanguageCodes::Bundle> LanguageCodes::findById(
    const LanguageId &id) const
{
  auto it = codes_.find(id);
  if (it != codes_.cend())
    return it->second;
  return {};
}

std::optional<LanguageCodes::Bundle> LanguageCodes::findByName(
    const QString &name) const
{
  auto it = std::find_if(codes_.cbegin(), codes_.cend(),
                         [name](const std::pair<LanguageId, Bundle> &i) {
                           return name == i.second.name;
                         });
  if (it != codes_.cend())
    return it->second;
  return {};
}

std::optional<LanguageCodes::Bundle> LanguageCodes::findByTesseract(
    const QString &name) const
{
  auto it = std::find_if(codes_.cbegin(), codes_.cend(),
                         [name](const std::pair<LanguageId, Bundle> &i) {
                           return name == i.second.tesseract;
                         });
  if (it != codes_.cend())
    return it->second;
  return {};
}

const std::unordered_map<LanguageId, LanguageCodes::Bundle>
    &LanguageCodes::all() const
{
  return codes_;
}
