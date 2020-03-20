var lastText = '';
var active = window.location.href !== "about:blank";

function checkFinished() {
  if (!active) return;

  let area = document.querySelector('textarea[dl-test=translator-target-input]');
  let text = area ? area.value : '';

  if (text === lastText || text === '')
    return;

  console.log('translated text', text, 'old', lastText, 'size', text.length, lastText.length);
  lastText = text;
  active = false;
  proxy.setTranslated(text);
}

function translate(text, from, to) {
  console.log('start translate', text, from, to)
  from = from == 'zh-CN' ? 'zh' : from;
  to = to == 'zh-CN' ? 'zh' : to;

  let supported = ['ru', 'en', 'de', 'fr', 'es', 'pt', 'it', 'nl', 'pl', 'ja', 'zh']
  if (supported.indexOf(from) == -1) {
    proxy.setFailed('Source language not supported');
    return;
  }
  if (supported.indexOf(to) == -1) {
    proxy.setFailed('Target language not supported');
    return;
  }

  active = true;

  let langs = from + '/' + to + '/';
  if (window.location.href.indexOf('www.deepl.com/translator') !== -1
    && window.location.href.indexOf(langs) !== -1) {
    document.querySelector('textarea[dl-test=translator-source-input]').value = text;
    document.querySelector('textarea[dl-test=translator-source-input]').dispatchEvent(
      new Event("input", { bubbles: true, cancelable: true }));
    return;
  }

  let url = 'https://www.deepl.com/translator#' + langs +
    text.replace('\n', ' ').replace('|', '');
  console.log("setting url", url);
  window.location = encodeURI(url);
}

function init() {
  proxy.translate.connect(translate);
  setInterval(checkFinished, 300);
}
