var lastText = '';
var active = window.location.href !== "about:blank";

function checkFinished() {
  if (!active) return;

  let area = document.querySelector('div#target-dummydiv');
  let text = area ? area.innerHTML.trim() : '';
  if (area == null) {
    area = document.querySelector('d-textarea.lmt__target_textarea p');
    text = area ? area.innerText.trim() : '';
  }
  if (area == null) {
    area = document.querySelector('d-textarea[data-testid=translator-target-input] p');
    text = area ? area.innerText.trim() : '';
  }

  if (text === lastText || text === '')
    return;

  console.log('translated text', text, 'old', lastText, 'size', text.length, lastText.length);
  lastText = text;
  active = false;
  proxy.setTranslated(text);
}

function translate(text, from, to) {
  console.log('start translate', text, from, to)

  if (text.trim().length == 0) {
    proxy.setTranslated('');
    return;
  }

  from = from == 'zh-CN' ? 'zh' : from;
  to = to == 'zh-CN' ? 'zh' : to;

  let supported = ['ru', 'en', 'de', 'fr', 'es', 'pt', 'it', 'nl', 'pl', 'ja', 'zh',
    'uk', 'bg', 'hu', 'el', 'da', 'id', 'lt', 'pt', 'ro', 'sk', 'sk', 'tr', 'fi', 'cs',
    'sv', 'et']
  if (supported.indexOf(from) == -1) {
    proxy.setFailed('Source language not supported');
    return;
  }
  if (supported.indexOf(to) == -1) {
    proxy.setFailed('Target language not supported');
    return;
  }

  active = true;

  var singleLineText = text.replace(/(?:\r\n|\r|\n)/g, ' ');

  let langs = from + '/' + to + '/';
  if (window.location.href.indexOf('www.deepl.com/translator') !== -1
    && window.location.href.indexOf(langs) !== -1) {

    var input = document.querySelector('d-textarea[dl-test=translator-source-input] p');
    if (input == null)
      input = document.querySelector('d-textarea.lmt__source_textarea p');
    if (input == null)
      input = document.querySelector('d-textarea[data-testid=translator-source-input] p');
    if (input.innerText == singleLineText) {
      console.log('using cached result');
      lastText = '';
      return;
    }
    input.innerText = singleLineText;
    if (areaCopy = document.querySelector('div#source-dummydiv'))
      areaCopy.innerHTML = singleLineText;
    setTimeout(function () {
      input.dispatchEvent(new Event("input", { bubbles: true, cancelable: true }));
    }, 300);
    return;
  }

  let url = 'https://www.deepl.com/translator#' + langs + encodeURIComponent(singleLineText);
  console.log("setting url", url);
  window.location = url;
}

function init() {
  proxy.translate.connect(translate);
  setInterval(checkFinished, 300);
}
