var isPageLoaded = false;
var isTranslationFinished = false; // async translation request
var isScheduled = false;

function checkFinished () {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var spans = [].slice.call (document.querySelectorAll ('#OutputText span'));
        var text = spans.reduce (function (res, i) {
            return res + i.innerText;
        }, '');
        st_wtp.translated (text);
    }, 2000); // wait for gui fill
}
function onResourceLoad (url) {
    if (url.indexOf ('/api.microsofttranslator.com/') > -1) {
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
    var url = 'https://bing.com/translator/?text=' + st_wtp.sourceText + '#auto/' +
            st_wtp.resultLanguage;
    window.location = encodeURI (url);
}
