var lastText = '';
var active = window.location.href !== "about:blank";

function checkFinished() {
    if (!active) return;

    let spans = [].slice.call(document.querySelectorAll('p.target-output'));
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

    let langs = from + '/' + to;
    if (window.location.href.indexOf('//fanyi.baidu.com/') !== -1
        && window.location.href.indexOf(langs) !== -1) {
        document.querySelector('textarea#baidu_translate_input').value = text;
        document.querySelector('textarea#baidu_translate_input').dispatchEvent(
            new Event("input", { bubbles: true, cancelable: true }));
        return;
    }

    let url = 'https://fanyi.baidu.com/#' + langs + '/' + encodeURIComponent(text);
    console.log("setting url", url);
    window.location = url;
}

function init() {
    proxy.translate.connect(translate);
    setInterval(checkFinished, 300);
}
