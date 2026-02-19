window.onload = function() {
  "use strict";
  /* The search result container */
  var searchResults = document.getElementById('search-results');
  /* The topic heading <select> element */
  var helpTopics = document.getElementById('matching-topics');
  /* Keyword search <input> */
  var searchBox = document.getElementById('topic-search-box');
  /* Shows the page mapped to the active <select> item */
  var clickToView = function(evnt) {
    evnt.preventDefault(); // Cancel any submit action
    try {
      if (!Boolean(helpTopics.value)) // Nothing selected
        return;

      var searchParam = helpTopics[helpTopics.selectedIndex].dataset.contains
      var targetURI = helpTopics.value + encodeURI('?contains=' + searchParam)
      parent.frames['content'].location.replace(targetURI)
    } catch (e) {
      /* Handle the DOMException that may be thrown by trying to access cross-origin frames on the `file://` protocol
       * See https://developer.mozilla.org/en-US/docs/Web/JavaScript/Reference/Errors/Property_access_denied
       */
      if (e instanceof DOMException) {
          /* Open the requested doc in a "modal" window  */
          window.open(targetURI, '_bank','noopener')
      } else {
          var docIndex = helpTopics.selectedIndex
          var requestedDoc = (docIndex >= 0) ? helpTopics[docIndex].innerHTML : helpTopics.value;
          window.alert('"' + requestedDoc + '" is missing or access is restricted.')
      }
    }
  };

  /* Bind event handlers to the search <input> */
  var findTopics = function(keyEvent) {
    if (keyEvent.isComposing) // Don't react to CTRL, ALT, SHIFT, etc.
      return;

    try {
      /* The <optgroup> to be filled with topic headings */
      var topicHeadings = document.getElementById('topic-headings');

      /* The 'value' property is an empty string when the <input> has been
       * emptied (by DEL, BACKSPACE, CTRL+X, etc.). But the <select> may
       * still be showing, so clear and hide it
       */
      if (!Boolean(searchBox.value)) {
        topicHeadings.replaceChildren(); // Clear the <optgroup>
        searchResults.style.display = 'none'
        return;
      }

      /* The global 'NPPEXEC_HELP_TOPICS' object is defined in 'topics.js', which *must*
       * be loaded before this script
       */
      var topicIndex = JSON.parse(JSON.stringify(NPPEXEC_HELP_TOPICS));
      /* Compile RegExp from the search string that was just typed */
      var query = searchBox.value.trim();
      var queryRE = new RegExp('(?:)(' + query.replace(/[.*+?^${}()|[\]\\]/g, '\\$&') + ')\s?', 'iu');
      /* Collection of objects holding topic metadata */
      var foundTopics = [];

      /* Each key is the title of a help doc */
      Object.keys(topicIndex).forEach(function(k) {
        /* Search all words in the doc's content */
        if (queryRE.test(topicIndex[k]['content'])) {
          foundTopics.push({ 'topic': k, 'contains': query, 'path': topicIndex[k]['path'] })
        }
      });

      if (foundTopics.length === 0) {
        helpTopics.size = 0;
        searchResults.style.display = 'none';
      } else {
        /* Reconstruct the <optgroup> */
        topicHeadings.replaceChildren();
        /* Create an <option> for each topic heading */
        foundTopics.forEach(function(tpc) {
          var item = document.createElement('option')
          /* Store the URI path in the hidden 'value' attribute */
          item.value = tpc.path
          item.dataset.contains = tpc.contains
          /* Display the doc title */
          item.innerText = tpc.topic
          topicHeadings.append(item)
        });
        searchResults.style.display = 'block';
        /* Min visible size is 2 b/c the <optgroup> label is counted */
        helpTopics.size = Math.max(foundTopics.length + 1, 2);
        /* Make sure mobile <select> boxes are never blank */
        helpTopics.selectedIndex = 0;
        helpTopics.addEventListener('change', clickToView, { 'passive': false });
        helpTopics.addEventListener('focus',
          /* Selecting the first <option> should *also* fire a 'change' event */
          function() { if (this.selectedIndex === 0) this.selectedIndex-- }, { 'once': true });
      }
    } catch (e) {
      console.error(e.message)
    }
  };

  searchBox.addEventListener('keyup', findTopics);
  if (window.matchMedia && window.matchMedia('(max-width:350px)')) {
    setInterval(function() {
      if (document.activeElement.tagName.toLowerCase() !== 'input')
        return;
      /* Simulate a hardware keyboard for mobile (Chrome) browsers */
      findTopics(new KeyboardEvent('keyup', { 'key': searchBox.value }));
      }, 1000);
  }

  /* Bind event handler to the doc navigation <button> */
  document.getElementById('goto-doc').addEventListener('click', clickToView, { 'passive': false });

  /* Bind event handlers to the show/hide <button> */
  /* Standalone function to toggle search visibility */
  var toggleSearch = function(forceHide) {
    var toggleBtn = document.getElementById('toggle-search-input');
    var search = document.getElementById('topic-search');
    var display = search.style.display;
    var onClick = function() { this.select() };

    // If forceHide is true OR search is currently visible, we hide it
    if (forceHide === true || display !== 'none') {
      toggleBtn.innerText = 'Search Topics';
      search.style.display = 'none';
      searchResults.style.display = 'none';
      searchBox.removeEventListener('click', onClick);
    } else {
      // Otherwise, we show it
      toggleBtn.innerText = 'Hide Search';
      search.style.display = 'block';
      searchResults.style.display = Boolean(helpTopics.value) ? 'block' : 'none';
      searchBox.addEventListener('click', onClick);
      searchBox.click();
    }
  };

  /* Bind the toggle function to the button click */
  document.getElementById('toggle-search-input').addEventListener('click', function(evnt) {
    evnt.preventDefault();
    toggleSearch();
  }, { 'passive': false });

  /* Bind Escape key to hide the search */
  document.addEventListener('keydown', function(evnt) {
    if (evnt.key === 'Escape' || evnt.keyCode === 27) {
      // Only hide if the search is currently visible
      if (document.getElementById('topic-search').style.display !== 'none') {
        toggleSearch(true); // Force hide
      }
    }
  });
}
