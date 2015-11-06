var isPageLoaded = false;
var isTranslationFinished = true; // async translation request
var isScheduled = false;

function checkFinished () {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var spans = [].slice.call (document.querySelectorAll ('#translation > span'));
        var text = spans.reduce (function (res, i) {
            return res + i.innerText;
        }, '');
        st_wtp.translated (text);
    }, 2000); // wait for gui fill
}
function onResourceLoad (url) {
}
st_wtp.resourceLoaded.connect (onResourceLoad);
function onPageLoad () {
    isPageLoaded = true;
    checkFinished ();
}
window.onload = onPageLoad();

function translate (){
    var url = 'https://translate.yandex.ru/?text=' + st_wtp.sourceText + '&lang=auto-' +
            st_wtp.resultLanguage;
    url = url.replace(new RegExp(' ','g') , '%20')
    console.log(encodeURI(url));
    window.location = (url);
}
