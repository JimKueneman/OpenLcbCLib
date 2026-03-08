// OpenLCB C Library - Documentation Overviews

document.addEventListener('DOMContentLoaded', function() {
    // Initialize Mermaid
    if (typeof mermaid !== 'undefined') {
        mermaid.initialize({
            startOnLoad: true,
            theme: 'default',
            securityLevel: 'loose',
            flowchart: {
                useMaxWidth: true,
                htmlLabels: true,
                curve: 'basis'
            },
            sequence: {
                useMaxWidth: true,
                diagramMarginX: 20,
                diagramMarginY: 20,
                actorMargin: 60,
                width: 150,
                height: 50,
                boxMargin: 10,
                noteMargin: 10,
                messageMargin: 35,
                mirrorActors: true
            },
            themeVariables: {
                primaryColor: '#3498db',
                primaryTextColor: '#fff',
                primaryBorderColor: '#2980b9',
                lineColor: '#666',
                secondaryColor: '#f8f9fa',
                tertiaryColor: '#e8e8e8'
            }
        });
    }

    // Mobile menu toggle
    var menuToggle = document.querySelector('.menu-toggle');
    var sidebar = document.querySelector('.sidebar');
    if (menuToggle && sidebar) {
        menuToggle.addEventListener('click', function() {
            sidebar.classList.toggle('open');
        });

        // Close sidebar when clicking main content on mobile
        var mainContent = document.querySelector('.main-content');
        if (mainContent) {
            mainContent.addEventListener('click', function() {
                sidebar.classList.remove('open');
            });
        }
    }

    // Highlight active sidebar link
    var currentPage = window.location.pathname.split('/').pop() || 'index.html';
    var sidebarLinks = document.querySelectorAll('.sidebar a');
    sidebarLinks.forEach(function(link) {
        var href = link.getAttribute('href');
        if (href === currentPage) {
            link.classList.add('active');
        }
    });

    // Smooth scroll for anchor links
    document.querySelectorAll('a[href^="#"]').forEach(function(anchor) {
        anchor.addEventListener('click', function(e) {
            var target = document.querySelector(this.getAttribute('href'));
            if (target) {
                e.preventDefault();
                target.scrollIntoView({ behavior: 'smooth' });
            }
        });
    });
});
