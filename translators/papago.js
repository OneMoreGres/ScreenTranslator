var isPageLoaded = false;
var isTranslationFinished = false; // async translation request
var isScheduled = false;

function checkFinished() {
    if (!isPageLoaded || !isTranslationFinished || isScheduled) return;
    isScheduled = true;
    setTimeout(function () {
        var spans = [].slice.call (document.querySelectorAll ('#txtTarget span'));
        var text = spans.reduce (function (res, i) {
            return res + i.innerText + ' ';
        }, '');
        console.log('result text', text);
        st_wtp.translated(text);
        isTranslationFinished = isScheduled = false;
    }, 2000); // wait for gui fill
}

var timeout = null;
function onResourceLoad(url) {
    console.log(url);
    if (url.indexOf('apis/n2mt/translate') > -1) {
        if (timeout !== null) {
            console.log('clear resource timeout');
            clearTimeout(timeout);
        }
        timeout = setTimeout(function () {
            console.log('last resource loaded');
            isTranslationFinished = true;
            if (isPageLoaded) {
                checkFinished();
            }
        }, 1000);
    }
}
st_wtp.resourceLoaded.connect(onResourceLoad);

function onPageLoad() {
    if (window.location.href.indexOf('about:blank') === 0) {
        translate ();
        return;
    }
    isPageLoaded = true;
    if (isTranslationFinished) {
        checkFinished();
    }
}
window.onload = onPageLoad();

function translate() {
    console.log(st_wtp.resultLanguage)
    var langs = ['ko', 'ru', 'en', 'fr', 'pt', 'th', 'ja',
                 'zb-CN', 'zn-TW', 'de', 'it', 'id', 'es', 'vi', 'hi'];

    if (langs.indexOf(st_wtp.resultLanguage) === -1) {
        st_wtp.translated('');
        console.log('language not supported by this translator ', st_wtp.resultLanguage);
        return;
    }

    if (window.location.href.indexOf('https://papago.naver.com/') === -1) {
        var url = 'https://papago.naver.com/?sk=auto&tk='+st_wtp.resultLanguage+'&st=' +
                st_wtp.sourceText.replace("\n", " ");
        window.location = encodeURI(url);
    }
    else {
        window.location = 'about:blank';
    }
}
