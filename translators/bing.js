var isPageLoaded = false;
var isTranslationFinished = false; // async translation request
var isScheduled = false;

function checkFinished () {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var text = document.querySelector ('#tta_output_ta').value;
        console.log (text);
        st_wtp.translated (text);
        isTranslationFinished = isScheduled = false;
    }, 2000); // wait for gui fill
}
function onResourceLoad (url) {
    if (url.indexOf ('bing.com/translator/?') > -1) {
        isTranslationFinished = true;
        if (isPageLoaded) {
            checkFinished ();
        }
    }
}
st_wtp.resourceLoaded.connect (onResourceLoad);
function onPageLoad () {
    if (window.location.href.indexOf('about:blank') === 0) {
        translate ();
        return;
    }

    isPageLoaded = true;
    if (isTranslationFinished) {
        checkFinished ();
    }
}
window.onload = onPageLoad();

function translate (){
    var url = 'https://bing.com/translator/?from=auto&to=' + st_wtp.resultLanguage +
            '&text=' + st_wtp.sourceText;
    window.location = encodeURI (url);
}
