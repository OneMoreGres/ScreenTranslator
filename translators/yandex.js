var isPageLoaded = false;
var isTranslationFinished = true; // async translation request
var isScheduled = false;

function checkFinished () {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var spans = [].slice.call (document.querySelectorAll ('span.translation-chunk'));
        var text = spans.reduce (function (res, i) {
            return res + i.innerText + ' ';
        }, '');
        st_wtp.translated (text);
        isTranslationFinished = isScheduled = false;
    }, 2000); // wait for gui fill
}
function onResourceLoad (url) {
    if (url.indexOf ('/tr.json/translate?') > -1) {
        isTranslationFinished = true;
        checkFinished ();
    }
}
st_wtp.resourceLoaded.connect (onResourceLoad);
function onPageLoad () {
    isPageLoaded = true;
    checkFinished ();
}
window.onload = onPageLoad();

function translate (){
    var url = 'https://translate.yandex.ru/?lang=' + st_wtp.sourceLanguage + '-' +
            st_wtp.resultLanguage + '&text=' + st_wtp.sourceText ;
    url = url.replace(new RegExp(' ','g') , '%20')
    window.location = (url);
}
