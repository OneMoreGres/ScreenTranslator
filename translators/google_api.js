function httpGetAsync(theUrl, callback)
{
  console.log(theUrl);
    var xmlHttp = new XMLHttpRequest();
    xmlHttp.onreadystatechange = function() { 
        if (xmlHttp.readyState == 4 && xmlHttp.status == 200)
            callback(xmlHttp.responseText);
    }
    xmlHttp.open("GET", theUrl, true);
    xmlHttp.send(null);
}

function translate (){
    var url = 'https://translate.googleapis.com/translate_a/single?client=gtx&sl=auto&tl='
    + st_wtp.resultLanguage + '&dt=t&q=' + st_wtp.sourceText;
    
    httpGetAsync(url, function(responce) {
      console.log(responce);
      var object = JSON.parse(responce);
      var result = '';
      object[0].forEach(function(element) {
        result += element[0] + ' ';
      });
      console.log(object);
      st_wtp.translated (result);
    });
}
