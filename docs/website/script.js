/* =========================================================
   EasyKey Landing Page — tương tác
   1. Nút chuyển dark / light (lưu vào localStorage)
   2. Hiệu ứng gõ chữ demo (telex -> tiếng Việt) khi thấy màn hình
   3. Hiện dần các khối khi cuộn (reveal on scroll)
   ========================================================= */

(function () {
  'use strict';

  /* ---------- 1. Dark / Light mode ---------- */
  var root = document.documentElement;
  var toggle = document.getElementById('themeToggle');

  function applyTheme(theme) {
    root.dataset.theme = theme;
    localStorage.setItem('easykey-theme', theme);
  }

  if (toggle) {
    toggle.addEventListener('click', function () {
      applyTheme(root.dataset.theme === 'dark' ? 'light' : 'dark');
    });
  }

  // Theo dõi thay đổi hệ thống nếu người dùng chưa tự chọn
  window.matchMedia('(prefers-color-scheme: dark)').addEventListener('change', function (e) {
    if (!localStorage.getItem('easykey-theme')) {
      root.dataset.theme = e.matches ? 'dark' : 'light';
    }
  });

  /* ---------- 2. Hiệu ứng gõ chữ demo ---------- */
  // data-typing="chu|qua" : gõ vế trái, sau đó "bỏ dấu" thành vế phải
  var demos = Array.prototype.slice.call(document.querySelectorAll('.typing'));

  function runTyping(el) {
    var parts = (el.getAttribute('data-typing') || '|').split('|');
    var raw = parts[0] || '';
    var done = parts[1] || '';
    var i = 0;

    el.textContent = '';

    function typeNext() {
      if (i < raw.length) {
        el.textContent += raw.charAt(i);
        i++;
        setTimeout(typeNext, 70 + Math.random() * 60);
      } else {
        // Nhấp nháy một chút rồi "Telex hoá" thành kết quả
        setTimeout(function () {
          el.textContent = done;
          // Sau 2.5s chạy lại vòng lặp để trang luôn sống động
          setTimeout(function () { runTyping(el); }, 2500);
        }, 450);
      }
    }
    typeNext();
  }

  /* ---------- 3. Reveal on scroll ---------- */
  var revealables = document.querySelectorAll(
    '.card, .step, .demo-card, .shot, .stat, .faq-item, .privacy-list li'
  );
  revealables.forEach(function (el) { el.classList.add('reveal'); });

  if ('IntersectionObserver' in window) {
    var io = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (!entry.isIntersecting) return;
        entry.target.classList.add('visible');
        io.unobserve(entry.target);
      });
    }, { threshold: 0.12 });
    revealables.forEach(function (el) { io.observe(el); });

    // Chỉ chạy animation gõ chữ khi demo xuất hiện
    var demoObserver = new IntersectionObserver(function (entries) {
      entries.forEach(function (entry) {
        if (entry.isIntersecting) {
          runTyping(entry.target);
          demoObserver.unobserve(entry.target);
        }
      });
    }, { threshold: 0.5 });
    demos.forEach(function (el) { demoObserver.observe(el); });
  } else {
    revealables.forEach(function (el) { el.classList.add('visible'); });
    demos.forEach(runTyping);
  }

  /* ---------- FAQ: đóng các mục đang mở khi mở mục khác ---------- */
  var faqs = document.querySelectorAll('.faq-item');
  faqs.forEach(function (item) {
    item.addEventListener('toggle', function () {
      if (item.open) {
        faqs.forEach(function (other) {
          if (other !== item) other.open = false;
        });
      }
    });
  });
})();
