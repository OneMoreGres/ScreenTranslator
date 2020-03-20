function httpGetAsync(url, callback) {
  let xmlHttp = new XMLHttpRequest();
  xmlHttp.onreadystatechange = function () {
    if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
      callback(xmlHttp.responseText);
  }
  xmlHttp.open("GET", url, true);
  xmlHttp.send(null);
}

function translate(text, from, to) {
  console.log('start translate', text, from, to)

  let url = 'https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl=' + to + '&dt=t&q=' + text;
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
