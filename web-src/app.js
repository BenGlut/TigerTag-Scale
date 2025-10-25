// Variables globales
let latestWeight = null;
let latestCalFactor = null;

// WebSocket
const ws = new WebSocket('ws://' + window.location.hostname + '/ws');

// Gestion du statut cloud
function setCloud(state) {
    const txt = document.getElementById('cloudState');
    const dot = document.getElementById('cloudDot');
    if (!txt || !dot) return;
    
    if (state === 'ok') {
        txt.innerText = 'OK';
        dot.classList.add('cloud-ok');
        dot.classList.remove('cloud-down');
    } else if (state === 'down') {
        txt.innerText = 'DOWN';
        dot.classList.add('cloud-down');
        dot.classList.remove('cloud-ok');
    } else {
        txt.innerText = 'unknown';
        dot.classList.remove('cloud-down');
        dot.classList.remove('cloud-ok');
    }
}

// Initialisation : récupère le statut initial
fetch('/api/status')
    .then(r => r.json())
    .then(s => {
        // Poids
        if (s.weight !== undefined) {
            document.getElementById('weight').innerText = s.weight + ' g';
        }
        
        // UID
        if (s.uid !== undefined && s.uid !== '') {
            const el = document.getElementById('uid');
            el.innerText = s.uid; // decimal
            if (s.uid_hex) el.title = 'HEX: ' + s.uid_hex;
        }
        
        // API Key
        const apiKeyInput = document.getElementById('newApiKey');
        if (apiKeyInput && typeof s.apiKey === 'string') {
            apiKeyInput.value = s.apiKey;
        }
        
        // Calibration factor
        if (typeof s.calibrationFactor !== 'undefined') {
            document.getElementById('calFactor').innerText = Number(s.calibrationFactor).toFixed(4);
            const cfInput = document.getElementById('newCalFactor');
            if (cfInput && !cfInput.value) cfInput.value = s.calibrationFactor;
        }
        
        // Sauvegarde des valeurs
        if (typeof s.weight !== 'undefined') {
            latestWeight = Number(s.weight);
        }
        if (typeof s.calibrationFactor !== 'undefined') {
            latestCalFactor = Number(s.calibrationFactor);
        }
        
        // Statut cloud
        if (typeof s.cloud === 'string') {
            setCloud(s.cloud);
        }
    })
    .catch(() => {});

// WebSocket : messages en temps réel
ws.onmessage = function(event) {
    const data = JSON.parse(event.data);
    document.getElementById('weight').innerText = data.weight + ' g';
    document.getElementById('uid').innerText = data.uid || 'No tag detected';
    latestWeight = Number(data.weight);
};

// Refresh cloud status toutes les 30s
setInterval(() => {
    fetch('/api/status')
        .then(r => r.json())
        .then(s => {
            if (typeof s.cloud === 'string') setCloud(s.cloud);
        })
        .catch(() => {});
}, 30000);

// ============================================
// API : Update API Key
// ============================================
function updateApiKey() {
    const key = document.getElementById('newApiKey').value;
    fetch('/api/config', {
        method: 'POST',
        headers: {'Content-Type': 'application/json'},
        body: JSON.stringify({apiKey: key})
    }).then(() => alert('API key updated!'));
}

// ============================================
// API : Send Weight
// ============================================
function sendWeight() {
    const w = parseInt(document.getElementById('manualWeight').value, 10);
    if (isNaN(w)) { 
        alert('Please enter a valid integer weight.'); 
        return; 
    }
    
    fetch('/api/push-weight', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ weight: w })
    })
    .then(r => r.json())
    .then(j => {
        if (j && j.status === 'ok') {
            alert('Weight sent to TigerTag.');
        } else {
            alert('Failed to send: ' + (j && j.error ? j.error : 'unknown error'));
        }
    })
    .catch(e => alert('Network error'));
}

// ============================================
// API : Tare Scale
// ============================================
function tareScale() {
    fetch('/api/tare', {method: 'POST'})
        .then(r => r.json())
        .then(_ => { 
            document.getElementById('weight').innerText = '0 g'; 
            alert('Tare done!');
        })
        .catch(_ => alert('Tare failed'));
}

// ============================================
// API : Update Calibration Factor
// ============================================
function updateCalibration() {
    const f = parseFloat(document.getElementById('newCalFactor').value);
    if (isNaN(f) || f === 0) { 
        alert('Enter a valid calibration factor'); 
        return; 
    }
    
    fetch('/api/calibration', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ factor: f })
    })
    .then(r => r.json())
    .then(j => {
        if (j && j.status === 'ok') {
            document.getElementById('calFactor').innerText = f.toFixed(4);
            alert('Calibration factor updated');
        } else {
            alert('Failed to update calibration factor');
        }
    })
    .catch(_ => alert('Network error'));
}

// ============================================
// Compute Calibration Factor from Known Weight
// ============================================
function computeFactor() {
    const known = parseFloat(document.getElementById('knownWeight').value);
    if (isNaN(known) || known <= 0) { 
        alert('Enter a valid known weight (g)'); 
        return; 
    }
    
    if (latestWeight === null || isNaN(latestWeight) || latestWeight <= 0) { 
        alert('Waiting for a valid reading...'); 
        return; 
    }
    
    const shownCal = parseFloat(document.getElementById('calFactor').innerText);
    const current = (!isNaN(shownCal) && shownCal > 0) ? shownCal : (latestCalFactor || 1);
    const newF = current * (latestWeight / known);
    
    const out = document.getElementById('newCalFactor');
    if (out) out.value = newF.toFixed(4);
    
    const hint = document.getElementById('calcHint');
    if (hint) {
        hint.innerText = `Displayed=${latestWeight.toFixed(2)} g, Known=${known.toFixed(2)} g → New factor=${newF.toFixed(4)}`;
    }
}

// ============================================
// API : Reset WiFi
// ============================================
function resetWiFi() {
    if (confirm('Reconfigure Wi‑Fi network?')) {
        fetch('/api/reset-wifi', {method: 'POST'})
            .then(() => alert('Rebooting into config mode...'));
    }
}

// ============================================
// API : Factory Reset
// ============================================
function factoryReset() {
    if (confirm('WARNING: All data will be erased!')) {
        fetch('/api/factory-reset', {method: 'POST'})
            .then(() => alert('Reset done. Rebooting...'));
    }
}