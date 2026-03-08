/* ============================================================================
 * protocol_support.js — Auto-derived Protocol Support Indicator display
 * ============================================================================ */

var PSI_BITS = [
    { name: 'Simple Protocol',            flag: null,                  always: false },
    { name: 'Datagram',                   flag: 'flag-datagrams',      always: false },
    { name: 'Memory Configuration',       flag: 'flag-config-mem',     always: false },
    { name: 'Event Exchange',             flag: 'flag-events',         always: false },
    { name: 'Simple Node Information',    flag: null,                  always: true  },
    { name: 'Configuration Description',  flag: 'flag-config-mem',     always: false },
    { name: 'Abbreviated Default CDI',    flag: 'flag-config-mem',     always: false },
    { name: 'Firmware Upgrade',           flag: 'flag-firmware',       always: false },
    { name: 'Train Control',             flag: 'flag-train',          always: false },
    { name: 'Function Description',       flag: 'flag-train',          always: false },
    { name: 'Broadcast Time',            flag: 'flag-broadcast-time', always: false }
];

function updateProtocolSupport() {
    var container = document.getElementById('protocol-list');
    container.innerHTML = '';

    for (var i = 0; i < PSI_BITS.length; i++) {
        var psi = PSI_BITS[i];
        var active = psi.always || isChecked(psi.flag);

        var item = document.createElement('div');
        item.className = 'protocol-item' + (active ? ' active' : '');
        item.innerHTML = '<span class="dot"></span>' + psi.name;
        container.appendChild(item);
    }
}

function buildProtocolSupportParts() {
    var parts = [];
    parts.push('PSI_SIMPLE_NODE_INFORMATION');

    if (isChecked('flag-datagrams'))
        parts.push('PSI_DATAGRAM');
    if (isChecked('flag-config-mem')) {
        parts.push('PSI_MEMORY_CONFIGURATION');
        parts.push('PSI_CONFIGURATION_DESCRIPTION_INFO');
        parts.push('PSI_ABBREVIATED_DEFAULT_CDI');
    }
    if (isChecked('flag-events'))
        parts.push('PSI_EVENT_EXCHANGE');
    if (isChecked('flag-train')) {
        parts.push('PSI_TRAIN_CONTROL');
        parts.push('PSI_FUNCTION_DESCRIPTION');
    }
    if (isChecked('flag-firmware'))
        parts.push('PSI_FIRMWARE_UPGRADE');

    return parts;
}
