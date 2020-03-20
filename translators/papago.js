var lastText = '';
var active = window.location.href !== "about:blank";

function getText() {
    let spans = [].slice.call(document.querySelectorAll('#txtTarget span'));
    let text = spans.reduce(function (res, i) {
        return res + i.innerText + ' ';
    }, '');

    return text.trim()
}

function checkFinished() {
    if (!active) return;

    let text = getText()
    if (text === lastText || text === lastText + '...' || text === '')
        return;

    active = false;
    // maybe translation will be updated
    setTimeout(function() {
        text = getText();
        console.log('translated text', text, 'old', lastText, 'size', text.length, lastText.length);
        lastText = text;
        active = false;
        proxy.setTranslated(text);
    }, 1000);

}

function translate(text, from, to) {
    console.log('start translate', text, from, to)
    let supported = ['ko', 'ru', 'en', 'fr', 'pt', 'th', 'ja',
        'zh-CN', 'zh-TW', 'de', 'it', 'id', 'es', 'vi', 'hi'];

    if (supported.indexOf(from) == -1) {
        proxy.setFailed('Source language not supported');
        return;
    }
    if (supported.indexOf(to) == -1) {
        proxy.setFailed('Target language not supported');
        return;
    }

    lastText = getText(); // because it can be updated after previous translation
    active = true;
    let langs = '?sk=auto&tk=' + to + '&';
    if (window.location.href.indexOf('//papago.naver.com/') !== -1
        && window.location.href.indexOf(langs) !== -1) {
        document.querySelector('textarea#txtSource').value = text
        document.querySelector('textarea#txtSource').dispatchEvent(
            new Event("input", { bubbles: true, cancelable: true }));
        return;
    }

    let url = 'https://papago.naver.com/?sk=auto&tk=' + to + '&st=' +
        text.replace("\n", " ");
    window.location = encodeURI(url);
}

function init() {
    proxy.translate.connect(translate);
    setInterval(checkFinished, 300);
}
