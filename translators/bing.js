var isPageLoaded = false;
var isTranslationFinished = false; // async translation request
var isScheduled = false;

function checkFinished () {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var spans = [].slice.call (document.querySelectorAll ('#destText span'));
        var text = spans.reduce (function (res, i) {
            return res + i.innerText;
        }, '');
        console.log (text);
        st_wtp.translated (text);
        isTranslationFinished = isScheduled = false;
    }, 2000); // wait for gui fill
}
function onResourceLoad (url) {
    if (url.indexOf ('/translator/api/Dictionary/Lookup?') > -1) {
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
    var url = 'https://bing.com/translator/?from=auto&to=' + st_wtp.resultLanguage +
            '&text=' + st_wtp.sourceText;
    window.location = encodeURI (url);
}
