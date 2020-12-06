var lastText = '';
var active = window.location.href !== "about:blank";

function checkFinished() {
    if (!active) return;

    let spans = [].slice.call(document.querySelectorAll('span.VIiyi > span > span'));
    let text = spans.reduce(function (res, i) {
        return res + ' ' + i.innerText;
    }, '');

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

    if (window.location.href.indexOf('//translate.google') !== -1
        && window.location.href.indexOf('&tl=' + to + '&') !== -1) {
        document.querySelector('textarea.er8xn').value = text;
        document.querySelector('textarea.er8xn').dispatchEvent(
            new Event("input", { bubbles: true, cancelable: true }));
        return;
    }
    let url = 'https://translate.google.com/#view=home&op=translate&sl=auto&tl=' + to + '&text=' + encodeURIComponent(text);
    console.log("setting url", url);
    window.location = url;

}

function init() {
    proxy.translate.connect(translate);
    setInterval(checkFinished, 300);
}
