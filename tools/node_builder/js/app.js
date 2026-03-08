/* ============================================================================
 * app.js — Initialization and top-level wiring
 * ============================================================================ */

/* --- Sidebar panel navigation --- */
function selectPanel(panelId) {

    // Deactivate all nav items
    var navItems = document.querySelectorAll('.nav-item');
    for (var i = 0; i < navItems.length; i++) {
        navItems[i].classList.remove('active');
    }

    // Deactivate all content panels
    var panels = document.querySelectorAll('.content-panel');
    for (var i = 0; i < panels.length; i++) {
        panels[i].classList.remove('active');
    }

    // Activate selected panel
    var panel = document.getElementById(panelId);
    if (panel) panel.classList.add('active');

    // Activate corresponding nav item (panel-xxx → nav-xxx)
    var navId = panelId.replace('panel-', 'nav-');
    var navItem = document.getElementById(navId);
    if (navItem) navItem.classList.add('active');
}

/* --- Wire up Generate & Download button --- */
function initGenerate() {
    document.getElementById('btn-generate').addEventListener('click', function() {
        downloadTextFile('openlcb_user_config.h', generateUserConfigH());
        downloadTextFile('openlcb_user_config.c', generateUserConfigC());
    });
}

/* --- Preview panel --- */

var PREVIEW_TABS = {
    'user-config-h': 'preview-user-config-h',
    'user-config-c': 'preview-user-config-c'
};

function switchPreviewTab(tabKey) {
    var btns = document.querySelectorAll('.preview-tab-btn');
    var panes = document.querySelectorAll('.preview-code');
    for (var i = 0; i < btns.length; i++) btns[i].classList.remove('active');
    for (var i = 0; i < panes.length; i++) panes[i].classList.remove('active');

    var paneId = PREVIEW_TABS[tabKey];
    if (paneId) document.getElementById(paneId).classList.add('active');

    // Activate corresponding tab button
    for (var i = 0; i < btns.length; i++) {
        if (btns[i].getAttribute('onclick').indexOf(tabKey) !== -1) {
            btns[i].classList.add('active');
        }
    }
}

function refreshPreview() {
    document.getElementById('preview-user-config-h').textContent = generateUserConfigH();
    document.getElementById('preview-user-config-c').textContent = generateUserConfigC();
}

function initPreview() {
    document.getElementById('btn-refresh-preview').addEventListener('click', refreshPreview);
}

/* --- Boot --- */
(function init() {
    initCDI();
    initFDI();
    initRoundtrip();
    initGenerate();
    initPreview();

    // Restore saved state before cascade (if any)
    restoreState();

    // Set initial UI state
    onFeatureFlagChange();
    onAddrSpaceChange();
    onConfigSpaceChange();
    initDragDrop();

    // Auto-save on form changes
    initPersistence();
})();
