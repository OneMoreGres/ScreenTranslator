var isPageLoaded = false;
var isTranslationFinished = false; // async translation request
var isScheduled = false;

function checkFinished() {
  if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
  isScheduled = true;
  setTimeout(function () {
    var area = document.querySelector('.lmt__target_textarea');
    var text = area ? area.value : '';
    console.log('result text', text);
    st_wtp.translated(text);
    isTranslationFinished = isScheduled = false;
  }, 2000); // wait for gui fill
}

var timeout = null;
function onResourceLoad(url) {
  if (url.indexOf('www2.deepl.com/jsonrpc') > -1) {
    if (timeout !== null) {
      console.log('clear resource timeout');
      clearTimeout(timeout);
    }
    timeout = setTimeout(function () {
      console.log('last resource loaded');
      isTranslationFinished = true;
      if (isPageLoaded) {
        checkFinished();
      }
    }, 500);
  }
}
st_wtp.resourceLoaded.connect(onResourceLoad);

function onPageLoad() {
  console.log('page loaded');
  isPageLoaded = true;
  if (isTranslationFinished) {
    checkFinished();
  }
}
window.onload = onPageLoad();

function translate() {
  var langs = {
    'eng': 'en',
    "rus": 'ru',
    "deu": 'de',
    "spa": 'es',
    "por": 'pt',
    "ita": 'it',
    "pol": 'pl'
  }

  if (langs[st_wtp.sourceLanguage] == undefined) {
    st_wtp.translated('');
    return;
  }

  if (window.location.href.indexOf('www.deepl.com/translator') === -1) {
    var url = 'https://www.deepl.com/translator#' +
      langs[st_wtp.sourceLanguage] + '/' + st_wtp.resultLanguage + '/' +
      st_wtp.sourceText.replace("\n", " ");
    window.location = encodeURI(url);
    return;
  }

  var input = document.querySelector('.lmt__source_textarea');
  input.value = st_wtp.sourceText.replace("\n", " ");
  input.dispatchEvent(new Event('change'));
}
