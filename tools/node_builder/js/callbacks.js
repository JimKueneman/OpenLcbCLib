/* ============================================================================
 * callbacks.js — Callback stub selection checklist
 * ============================================================================ */

var CALLBACK_CATEGORIES = [
    {
        label: 'Core',
        flag: null,
        mandatory: true,
        callbacks: [
            { name: 'Callbacks_initialize(void)',
              help: 'Called once at startup. Set up hardware peripherals, GPIOs, and any application-level state here.',
              mandatory: true },
            { name: 'Callbacks_on_100ms_timer_callback(void)',
              help: 'Called every 100 ms by the library timer. Use for periodic tasks such as polling sensors or toggling LEDs.',
              mandatory: true },
            { name: 'Callbacks_on_can_rx_callback(can_msg_t *)',
              help: 'Called when a CAN frame is received. Use to forward the frame to a CAN driver or log it for debugging.',
              mandatory: true },
            { name: 'Callbacks_on_can_tx_callback(can_msg_t *)',
              help: 'Called when the library needs to transmit a CAN frame. Send it to your CAN driver here.',
              mandatory: true },
            { name: 'Callbacks_alias_change_callback(uint16_t, node_id_t)',
              help: 'Called when an alias is allocated or changed for a node. Useful for logging or updating a display.',
              mandatory: true },
            { name: 'Callbacks_on_login_complete(openlcb_node_t *)',
              help: 'Called when a node finishes CAN alias allocation and is on the bus. Start application logic here (e.g. query clocks, send initial events). Return true if an outgoing message was queued.',
              mandatory: true }
        ]
    },
    {
        label: 'Memory Configuration',
        flag: 'flag-config-mem',
        callbacks: [
            { name: 'Callbacks_operations_request_factory_reset(...)',
              help: 'Called when a remote node requests a factory reset. Erase or reinitialize your configuration data in non-volatile storage.',
              mandatory: false }
        ]
    },
    {
        label: 'Event Exchange',
        flag: 'flag-events',
        callbacks: [
            { name: 'Callbacks_on_consumed_event_identified(...)',
              help: 'Called when a Producer Identified message arrives for an event this node consumes. The index is the position in the consumer list, or 0xFFFF if it falls within a registered consumer range.',
              mandatory: false },
            { name: 'Callbacks_on_consumed_event_pcer(...)',
              help: 'Called when a Producer/Consumer Event Report (PCER) arrives for an event this node consumes. This is the real-time notification that another node produced an event.',
              mandatory: false },
            { name: 'Callbacks_on_event_learn(...)',
              help: 'Called when the node receives an Event Learn message. Use to teach or reassign events on this node.',
              mandatory: false }
        ]
    },
    {
        label: 'Firmware Upgrade',
        flag: 'flag-firmware',
        callbacks: [
            { name: 'Callbacks_freeze(...)',
              help: 'Called when a remote node requests a freeze on address space 0xEF. Prepare for firmware upgrade by initializing the file system and creating the firmware image file.',
              mandatory: false },
            { name: 'Callbacks_write_firmware(...)',
              help: 'Called for each block of firmware data written to address space 0xEF. Write the data to the firmware image file and track progress.',
              mandatory: false },
            { name: 'Callbacks_unfreeze(...)',
              help: 'Called when a remote node requests an unfreeze on address space 0xEF. Finalize the firmware image, validate it, commit the update, and reboot the node.',
              mandatory: false }
        ]
    },
    {
        label: 'Broadcast Time',
        flag: 'flag-broadcast-time',
        callbacks: [
            { name: 'Callbacks_on_broadcast_time_changed(broadcast_clock_t *)',
              help: 'Called when the fast clock time or rate is updated. Use to refresh a time display or trigger time-based actions.',
              mandatory: false }
        ]
    }
];

function updateCallbacksList() {
    var container = document.getElementById('callbacks-list');
    container.innerHTML = '';

    var cbIndex = 0;

    for (var c = 0; c < CALLBACK_CATEGORIES.length; c++) {
        var cat = CALLBACK_CATEGORIES[c];

        // Only show category if its feature flag is enabled (or no flag needed)
        if (cat.flag && !isChecked(cat.flag)) continue;

        // Category heading
        var heading = document.createElement('div');
        heading.className = 'callbacks-category';
        heading.textContent = cat.label;
        if (cat.mandatory) {
            var catTag = document.createElement('span');
            catTag.className = 'tag tag-required';
            catTag.textContent = 'mandatory';
            heading.appendChild(catTag);
        }
        container.appendChild(heading);

        for (var i = 0; i < cat.callbacks.length; i++) {
            var cb = cat.callbacks[i];

            var item = document.createElement('div');
            item.className = 'checkbox-item';

            var row = document.createElement('div');
            row.className = 'checkbox-row';

            var chk = document.createElement('input');
            chk.type = 'checkbox';
            chk.id = 'cb-' + cbIndex;
            chk.checked = true;
            if (cb.mandatory) chk.disabled = true;

            var lbl = document.createElement('label');
            lbl.setAttribute('for', 'cb-' + cbIndex);
            lbl.textContent = cb.name;
            lbl.style.fontFamily = 'var(--font-mono)';
            lbl.style.fontSize = '12px';

            row.appendChild(chk);
            row.appendChild(lbl);

            item.appendChild(row);

            // Help text
            var help = document.createElement('div');
            help.className = 'flag-help';
            help.textContent = cb.help;
            item.appendChild(help);

            container.appendChild(item);
            cbIndex++;
        }
    }
}
