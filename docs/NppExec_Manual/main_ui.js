document.addEventListener('DOMContentLoaded', function() {
  "use strict";

  // Resizer (Splitter)
  const resizer = document.getElementById('resizer');
  const sidebar = document.getElementById('toc-sidebar');
  const container = document.getElementById('main-container');

  // Content Frame: focusing on mouse events
  const contentArea = document.getElementById('content-area');
  const contentFrame = document.getElementById('content-frame');
  const focusTrigger = document.getElementById('focus-trigger');

  // TOC: focusing on mouse events
  const tocSidebar = document.getElementById('toc-sidebar');
  const tocFrame = document.getElementById('toc-frame');
  const tocFocusTrigger = document.getElementById('toc-focus-trigger');

  (function handleQueryParam() {
    try {
      var query = location.search || '';
      var param = 'q=';
      var index = query.indexOf(param);

      if (index !== -1 && contentFrame) {
        var value = query.slice(index + param.length);
        if (value) {
          // Modified to target the iframe element specifically
          contentFrame.contentWindow.location.replace('NppExec_Manual/' + value + '.html');
        }
      }
    } catch (e) { console.info(e); }
  })();

  // Resizer logic
  if (resizer && sidebar && container) {
    resizer.addEventListener('mousedown', function(e) {
      e.preventDefault();
      container.classList.add('dragging');

      document.addEventListener('mousemove', handleMouseMove);
      document.addEventListener('mouseup', function() {
        container.classList.remove('dragging');
        document.removeEventListener('mousemove', handleMouseMove);
      });
    });

    function handleMouseMove(e) {
      const containerWidth = container.offsetWidth;
      const newWidthPercentage = (e.clientX / containerWidth) * 100;

      // Limits: Prevents the sidebar from disappearing or becoming too huge
      if (newWidthPercentage > 5 && newWidthPercentage < 90) {
        sidebar.style.width = newWidthPercentage + '%';
      }
    }
  }

  // Content Frame focusing logic
  if (contentArea && contentFrame && focusTrigger) {
    contentArea.addEventListener('mouseenter', function() {
      // 1. Setting the focus to the hidden focusTrigger
      focusTrigger.focus();
      // 2. Now setting the focus to the frame
      if (contentFrame.contentWindow) {
        contentFrame.contentWindow.focus();
      }
    });

    // Additionally: ensuring the focus while scrolling
    contentArea.addEventListener('wheel', function() {
      if (document.activeElement !== contentFrame) {
        contentFrame.contentWindow.focus();
      }
    }, { passive: true });
  }

  // TOC Frame focusing logic
  if (tocSidebar && tocFrame && tocFocusTrigger) {
    tocSidebar.addEventListener('mouseenter', function() {
      // 1. Focusing the left pane
      tocFocusTrigger.focus();
      // 2. Setting the focus to the TOC
      if (tocFrame.contentWindow) {
        tocFrame.contentWindow.focus();
      }
    });
  }
});
