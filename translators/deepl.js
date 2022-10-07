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

  let langs = from + '/' + to + '/';
  if (window.location.href.indexOf('www.deepl.com/translator') !== -1
    && window.location.href.indexOf(langs) !== -1) {
    var input = document.querySelector('textarea[dl-test=translator-source-input]');
    if (input.value == text) {
        console.log('using cached result');
        lastText = '';
        return;
    }
    input.value = text;
    input.dispatchEvent(new Event("input", { bubbles: true, cancelable: true }));
    return;
  }

  let url = 'https://www.deepl.com/translator#' + langs + encodeURIComponent(text);
  console.log("setting url", url);
  window.location = url;
}

function init() {
  proxy.translate.connect(translate);
  setInterval(checkFinished, 300);
}
