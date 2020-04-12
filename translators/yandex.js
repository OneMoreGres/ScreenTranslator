var lastText = '';
var active = window.location.href !== "about:blank";

function checkFinished() {
    if (!active) return;

    let spans = [].slice.call(document.querySelectorAll('span.translation-chunk'));
    let text = spans.reduce(function (res, i) {
        return res + ' ' + i.innerText;
    }, '').trim();

    if (text === lastText || text === '')
        return;

    console.log('translated text', text, 'old', lastText, 'size', text.length, lastText.length);
    lastText = text;
    active = false;
    proxy.setTranslated(text);
}

function translate(text, from, to) {
    console.log('start translate', text, from, to)
    active = true;

    let langs = 'lang=' + from + '-' + to;
    let url = 'https://translate.yandex.ru/?' + langs + '&text=' + encodeURIComponent(text);
    console.log("setting url", url);
    window.location = url;
}

function init() {
    proxy.translate.connect(translate);
    setInterval(checkFinished, 300);
}
