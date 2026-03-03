// This file is used by each document in the content frame
window.onload = function() {
  var isCHM = window.location.protocol === 'ms-its:' || window.location.protocol === 'mk:';
  if (isCHM) {
    // Disabling external links in case of a CHM file
    var links = document.getElementsByTagName('a');
    for (var i = 0; i < links.length; i++) {
      var href = links[i].getAttribute('href') || '';
      if (href.substring(0, 4).toLowerCase() === 'http') {
        links[i].onclick = function() {
          alert('This external link is unavailable in the offline manual.');
          return false;
        };
        links[i].style.color = '#888'; // Grey color
        links[i].style.textDecoration = 'none';
        links[i].style.cursor = 'default';
        links[i].title = 'External link disabled in CHM';
      }
    }
  }

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
  if (matches.length) matches[0].scrollIntoView();

  // Notify that the content page has been loaded
  // (Note: an attempt to postMessage to frames['toc-frame']
  // results in "DOMException: Permission denied to access
  // property on cross-origin object")
  window.parent.postMessage('CONTENT_PAGE_LOADED', '*');
}
