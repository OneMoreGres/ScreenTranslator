function httpGetAsync(url, callback) {
  let xmlHttp = new XMLHttpRequest();
  xmlHttp.timeout = 30000; // msecs
  xmlHttp.onreadystatechange = function () {
    if (xmlHttp.readyState != 4)
      return
    if (xmlHttp.status == 200)
      callback(xmlHttp.responseText);
    else
      proxy.setFailed(xmlHttp.statusText)
    xmlHttp.onreadystatechange = null;
    xmlHttp = null;
  }
  xmlHttp.open("GET", url, true);
  xmlHttp.send(null);
}

function translate(text, from, to) {
  console.log('start translate', text, from, to)

  let url = 'https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=' + to + '&dt=t&q=' + encodeURIComponent(text);
  console.log("loading url", url);

  httpGetAsync(url, function (response) {
    console.log('received', response);
    let object = JSON.parse(response);
    let result = '';
    object[0].forEach(function (element) {
      result += element[0] + ' ';
    });
    proxy.setTranslated(result);
  });
}

function init() {
  proxy.translate.connect(translate);
}
