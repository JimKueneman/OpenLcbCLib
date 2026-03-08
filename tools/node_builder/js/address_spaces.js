/* ============================================================================
 * address_spaces.js — Address space sub-panel conditional visibility
 * ============================================================================ */

var CONFIG_LEN_USER_NAME = 63;   // LEN_SNIP_USER_NAME_BUFFER (including null)
var CONFIG_LEN_USER_DESC = 64;   // LEN_SNIP_USER_DESCRIPTION_BUFFER (including null)

function onAddrSpaceChange() {
    // 0xFE — disable detail fields when "present" is unchecked
    var fePresent = isChecked('addr-0xfe-present');
    document.getElementById('addr-0xfe-readonly').disabled = !fePresent;
    document.getElementById('addr-0xfe-low-valid').disabled = !fePresent;
    document.getElementById('addr-0xfe-high').disabled = !fePresent;
    document.getElementById('addr-0xfe-desc').disabled = !fePresent;

    // 0xFE low address — enabled only when present AND low_address_valid checked
    var feLowValid = fePresent && isChecked('addr-0xfe-low-valid');
    document.getElementById('addr-0xfe-low').disabled = !feLowValid;
}

/* ── 0xFD Configuration space auto-calculations ── */

function onConfigSpaceChange() {
    var nameAddr = parseIntAuto(getVal('cfg-user-name-addr'));

    var descAddr = nameAddr + CONFIG_LEN_USER_NAME;
    var firstUserAddr = nameAddr + CONFIG_LEN_USER_NAME + CONFIG_LEN_USER_DESC;
    var minHighest = firstUserAddr - 1;  // last byte of description buffer

    document.getElementById('cfg-user-desc-addr').value = formatHexDec(descAddr);
    document.getElementById('cfg-first-user-addr').value = formatHexDec(firstUserAddr);

    var hint = document.getElementById('cfg-min-highest-hint');
    if (hint) hint.textContent = formatHexDec(minHighest);

    // Update 0xFD common display fields
    var lowValid = (nameAddr !== 0);
    document.getElementById('addr-0xfd-low-valid').checked = lowValid;
    document.getElementById('addr-0xfd-low').value = formatHexDec(lowValid ? nameAddr : 0);
}
