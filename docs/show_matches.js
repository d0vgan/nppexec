window.onload = function() {
  if (!(document.createRange && location.search))
    return;
  var query = location.search
  var param = 'contains='
  var value = query.slice(query.indexOf(param) + param.length)
  var encodedChars = value.match(/%[0-9A-Fa-f]{2}/g)
  if (encodedChars) {
    encodedChars.forEach(function(ch) {
      value = value.replace(ch, String.fromCharCode(parseInt(ch.slice(1), 16)))
    })
  }
  while (window.find(value)) {
    var sel = window.getSelection()
    var txtRng = sel.getRangeAt(0)
    var selSpan = document.createElement('span')
    selSpan.classList += 'match'
    selSpan.style.backgroundColor = 'rgb(0,127,255)'
    selSpan.style.color = 'white'
    selSpan.innerText = txtRng.extractContents().textContent
    txtRng.insertNode(selSpan)
  }
  window.getSelection().empty()
  var matches = document.querySelectorAll('.match')
  // anchor viewport to first matching string
  if (matches.length) matches[0].scrollIntoView()
}
