// Add or modify masterspools here
// Format: { id, label, weight (in grams), image (URL or path) }
const masterspools = [
    {
        id: 'bambu_grey',
        label: 'BambuLab Grey',
        weight: 210,
        image: 'img/bambu_grey.png'
    },
    {
        id: 'bambu_transp',
        label: 'BambuLab Transparent',
        weight: 215,
        image: 'img/bambu_transp.png'
    },
    {
        id: 'r3d_grey',
        label: 'R3D Grey',
        weight: 239,
        image: 'img/r3d_grey.png'
    },
    {
        id: 'custom',
        label: 'Custom',
        weight: 0,
        image: 'img/custom.png'
    }
];

// ========== TRANSLATIONS ==========
const translations = {
    fr: {
        waiting: 'Attente du TigerTag...',
        quickActions: 'Actions rapides',
        tare: 'TARE',
        apiKey: 'Clé API',
        user: 'Utilisateur',
        newApiKey: 'Nouvelle clé API',
        update: 'Mettre à jour',
        delete: 'Supprimer',
        calibration: 'Calibration Magique',
        currentFactor: 'Facteur actuel',
        autoCalc: 'Calcul automatique',
        knownWeight: 'Poids connu (g)',
        compute: 'Calculer',
        manual: 'Manuel',
        newFactor: 'Nouveau facteur',
        apply: 'Appliquer',
        advanced: 'Avancé',
        reconfigWifi: 'Reconfigurer Wi‑Fi',
        factoryReset: 'Réinitialisation',
        uptime: 'Durée de fonctionnement',
        // Calibration wizard
        step1Title: 'Step 1',
        step1Instruction: 'Laissez la balance vide puis appuyez sur le bouton',
        step1Button: 'GO →',
        step2Title: 'Step 2',
        step2Instruction: 'Sélectionnez votre Masterspool vide et placez-la sur la balance',
        selectMasterspool: 'Sélectionnez votre Masterspool',
        step2Button: 'Calibrer ✓',
        step3Title: 'Calibré !',
        step3Instruction: 'La balance est maintenant calibrée',
        currentReading: 'Lecture actuelle',
        calibKnownWeight: 'Poids réel (g)',
        newFactor: 'Nouveau facteur',
        back: '← Retour',
        calibAgain: 'Restart',
        manualCalib: 'Calibration manuelle',
        // Status
        cloud: 'Cloud',
        offline: 'Hors ligne',
        validated: 'Validé',
        invalid: 'Invalide',
        notConfigured: 'Non configuré',
        configureApiKey: 'Configurer la clé API',
        apiKeyInvalid: 'Clé API invalide',
        // Alerts
        alertEnterKey: 'Veuillez saisir une clé API',
        alertUpdateError: 'Erreur lors de la mise à jour',
        alertDeleteConfirm: 'Supprimer la clé API ?',
        alertDeleteError: 'Erreur lors de la suppression',
        alertInvalidFactor: 'Facteur invalide',
        alertNegativeFactor: '⚠️ Le coefficient doit être positif',
        alertError: 'Erreur',
        alertInvalidWeight: 'Poids connu invalide',
        alertDataUnavailable: 'Données non disponibles',
        alertWeightTooLight: '⚠️ Poids trop léger (min. 200g)\nVérifiez que le filament est bien sur la balance.',
        errorWeightRequired: '⚠️ Veuillez entrer un poids valide (min. 200g)',
        modalWeightTooLightTitle: 'Poids insuffisant',
        modalWeightTooLightMessage: 'Le poids est trop léger pour une calibration. Utilisez une Masterspool ou un filament d\'au moins 200g.',
        modalWeightInvalidTitle: 'Poids invalide',
        modalWeightInvalidMessage: 'Veuillez entrer un poids valide',
        modalOk: 'OK',
        alertReconfigConfirm: 'Reconfigurer le Wi‑Fi ? L\'appareil redémarrera.',
        alertResetConfirm: '⚠️ ATTENTION : Cette action effacera toutes les données. Continuer ?',
        // Send status
        sending: '⏳ Envoi...',
        sent: '✓ Envoyé',
        sendError: '✗ Erreur',
        sendIn: 'Envoi dans'
    },
    en: {
        waiting: 'Waiting TigerTag...',
        quickActions: 'Quick Actions',
        tare: 'TARE',
        apiKey: 'API Key',
        user: 'User',
        newApiKey: 'New API Key',
        update: 'Update',
        delete: 'Delete',
        calibration: 'Wizard Calibration',
        currentFactor: 'Current factor',
        autoCalc: 'Auto calculation',
        knownWeight: 'Known weight (g)',
        compute: 'Compute',
        manual: 'Manual',
        newFactor: 'New factor',
        apply: 'Apply',
        advanced: 'Advanced',
        reconfigWifi: 'Reconfigure Wi‑Fi',
        factoryReset: 'Factory Reset',
        uptime: 'Uptime',
        // Calibration wizard
        step1Title: 'Step 1',
        step1Instruction: 'Leave the scale empty then press the button',
        step1Button: 'GO →',
        step2Title: 'Step 2',
        step2Instruction: 'Select your empty Masterspool and place it on the scale',
        selectMasterspool: 'Select your Masterspool',
        step2Button: 'Calibrate ✓',
        step3Title: 'Calibrated!',
        step3Instruction: 'The scale is now calibrated',
        currentReading: 'Current reading',
        calibKnownWeight: 'Real weight (g)',
        newFactor: 'New factor',
        back: '← Back',
        calibAgain: 'Restart',
        manualCalib: 'Manual calibration',
        // Status
        cloud: 'Cloud',
        offline: 'Offline',
        validated: 'Validated',
        invalid: 'Invalid',
        notConfigured: 'Not configured',
        configureApiKey: 'Setup API Key',
        apiKeyInvalid: 'Invalid API Key',
        // Alerts
        alertEnterKey: 'Please enter an API key',
        alertUpdateError: 'Update error',
        alertDeleteConfirm: 'Delete API key?',
        alertDeleteError: 'Delete error',
        alertInvalidFactor: 'Invalid factor',
        alertNegativeFactor: '⚠️ Coefficient must be positive',
        alertError: 'Error',
        alertInvalidWeight: 'Invalid known weight',
        alertDataUnavailable: 'Data unavailable',
        alertWeightTooLight: '⚠️ Weight too light (min. 200g)\nCheck that the filament is on the scale.',
        errorWeightRequired: '⚠️ Please enter a valid weight (min. 200g)',
        modalWeightTooLightTitle: 'Insufficient weight',
        modalWeightTooLightMessage: 'The weight is too light for calibration. Use a Masterspool or filament of at least 200g.',
        modalWeightInvalidTitle: 'Invalid weight',
        modalWeightInvalidMessage: 'Please enter a valid weight',
        modalOk: 'OK',
        alertReconfigConfirm: 'Reconfigure Wi‑Fi? Device will restart.',
        alertResetConfirm: '⚠️ WARNING: This will erase all data. Continue?',
        // Send status
        sending: '⏳ Sending...',
        sent: '✓ Sent',
        sendError: '✗ Error',
        sendIn: 'Sending in'
    }
};

let currentLang = localStorage.getItem('tigertag_lang') || 'en';

function t(key) {
    return translations[currentLang][key] || key;
}

function setLanguage(lang) {
    currentLang = lang;
    localStorage.setItem('tigertag_lang', lang);
    document.documentElement.lang = lang;
    
    // Update lang buttons
    document.querySelectorAll('.lang-btn').forEach(btn => {
        btn.classList.toggle('active', btn.dataset.lang === lang);
    });
    
    // Update all translated elements
    document.querySelectorAll('[data-i18n]').forEach(el => {
        const key = el.dataset.i18n;
        el.textContent = t(key);
    });
    
    // Update placeholders
    document.querySelectorAll('[data-i18n-placeholder]').forEach(el => {
        const key = el.dataset.i18nPlaceholder;
        el.placeholder = t(key);
    });
    
    // Update dynamic content
    updateCloudText();
    updateApiText();
}

// ========== DOM ELEMENTS ==========
const cloudDot = document.getElementById('cloudDot');
const cloudText = document.getElementById('cloudText');
const apiDot = document.getElementById('apiDot');
const apiText = document.getElementById('apiText');
const weightEl = document.getElementById('weight');
const uidEl = document.getElementById('uid');
const calFactorEl = document.getElementById('calFactor');
const userDisplayEl = document.getElementById('userDisplay');
const sendStateEl = document.getElementById('sendState');
const userNameEl = document.getElementById('userName');

// ========== STATE ==========
let currentWeight = null;
let currentUid = null;
let calFactor = null;
let apiKey = '';
let cloudStatus = 'unknown';
let apiStatus = 'none';

// ========== UTILITIES ==========
function setTextIfChanged(el, txt) {
    if (!el || el.textContent === txt) return;
    el.textContent = txt;
}

function toggleSection(el) {
    el.classList.toggle('active');
    const content = el.nextElementSibling;
    content.classList.toggle('active');
}

function updateCloudText() {
    const txt = cloudStatus === 'up' || cloudStatus === 'ok' ? t('cloud') : t('offline');
    setTextIfChanged(cloudText, txt);
}

function updateApiText() {
    let txt = t('user') + ': ';
    if (apiStatus === 'valid') {
        txt = userDisplayEl.textContent;
    } else if (apiStatus === 'invalid') {
        txt += t('invalid');
    } else {
        txt += t('notConfigured');
    }
    setTextIfChanged(userDisplayEl, txt.replace(t('user') + ': ', ''));
}

function setCloudStatus(state) {
    const s = String(state || '').toLowerCase();
    cloudStatus = (s === 'up' || s === 'ok') ? 'up' : 'down';
    updateCloudText();
    
    if (cloudStatus === 'up') {
        cloudDot.className = 'status-dot active';
    } else {
        cloudDot.className = 'status-dot error';
    }
}

function setApiStatus(state, displayName) {
    apiStatus = state;
    const name = displayName ? displayName.trim() : '';
    
    if (state === 'valid') {
        apiDot.className = 'status-dot active';
        setTextIfChanged(userDisplayEl, name || t('validated'));
        // Show username above weight if available
        if (name && userNameEl) {
            setTextIfChanged(userNameEl, name);
            userNameEl.classList.remove('hidden');
            userNameEl.style.color = 'rgba(255,255,255,0.9)';
            userNameEl.style.background = 'transparent';
        }
    } else if (state === 'invalid') {
        apiDot.className = 'status-dot error';
        setTextIfChanged(userDisplayEl, t('invalid'));
        // Show API error alert
        if (userNameEl) {
            setTextIfChanged(userNameEl, '⚠️ ' + t('apiKeyInvalid'));
            userNameEl.classList.remove('hidden');
            userNameEl.style.color = '#fff';
            userNameEl.style.background = 'rgba(245,101,101,0.3)';
        }
    } else {
        apiDot.className = 'status-dot warning';
        setTextIfChanged(userDisplayEl, t('notConfigured'));
        // Show configuration alert
        if (userNameEl) {
            setTextIfChanged(userNameEl, '🚨 ' + t('configureApiKey'));
            userNameEl.classList.remove('hidden');
            userNameEl.style.color = '#fff';
            userNameEl.style.background = 'rgba(237,137,54,0.3)';
        }
    }
}

function setSendState(msg, color) {
    if (!msg) {
        sendStateEl.classList.add('hidden');
        return;
    }
    setTextIfChanged(sendStateEl, msg);
    if (color) sendStateEl.style.background = color;
    sendStateEl.classList.remove('hidden');
}

function formatHMS(secs) {
    if (!isFinite(secs)) return '--:--:--';
    const h = Math.floor(secs / 3600);
    const m = Math.floor((secs % 3600) / 60);
    const s = Math.floor(secs % 60);
    return [h,m,s].map(x => String(x).padStart(2,'0')).join(':');
}

// ========== API FUNCTIONS ==========
function updateApiKey() {
    const input = document.getElementById('newApiKey');
    const key = (input.value || '').trim();
    if (!key) { 
        alert(t('alertEnterKey'));
        return;
    }
    // Optional: small UX lock
    const btns = document.querySelectorAll('button[onclick="updateApiKey()"]');
    btns.forEach(b => { b.disabled = true; b.textContent = t('update') + '…'; });

    fetch('/api/apikey', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        // Firmware expects { key: "..." }
        body: JSON.stringify({ key: key })
    })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .then(res => {
        const ok = !!res.success;
        const name = (res.displayName || '').trim();
        if (ok) {
            apiKey = key;
            setApiStatus('valid', name);
        } else {
            setApiStatus('invalid');
            alert(t('alertUpdateError'));
        }
    })
    .catch(() => {
        setApiStatus('invalid');
        alert(t('alertUpdateError'));
    })
    .finally(() => btns.forEach(b => { b.disabled = false; b.textContent = t('update'); }));
}

function deleteApiKey() {
    if (!confirm(t('alertDeleteConfirm'))) return;
    const delBtn = document.querySelector('button.danger[onclick="deleteApiKey()"]');
    if (delBtn) { delBtn.disabled = true; delBtn.textContent = t('delete') + '…'; }

    // Use simple GET endpoint implemented in firmware
    fetch('/apikeydelete', { method: 'GET', cache: 'no-store' })
      .then(async r => {
          if (!r.ok) throw new Error('http ' + r.status);
          const raw = await r.text();
          let ok = raw && raw.trim().toLowerCase() === 'ok';
          if (!ok) { try { const j = JSON.parse(raw); ok = !!j.success; } catch(_) {} }
          return ok;
      })
      .then(ok => {
          if (ok) {
              apiKey = '';
              const input = document.getElementById('newApiKey');
              if (input) input.value = '';
              setApiStatus('none');
          } else {
              alert(t('alertDeleteError'));
          }
      })
      .catch(() => alert(t('alertDeleteError')))
      .finally(() => { if (delBtn) { delBtn.disabled = false; delBtn.textContent = t('delete'); } });
}

// ========== API KEY VISIBILITY TOGGLE ==========
function toggleApiKeyVisibility() {
    const input = document.getElementById('newApiKey');
    const icon = document.getElementById('eyeIcon');
    
    if (!input || !icon) return;
    
    if (input.type === 'password') {
        input.type = 'text';
        icon.src = 'img/visibility.svg'; // Eye visible
    } else {
        input.type = 'password';
        icon.src = 'img/visibility_off.svg'; // Eye hidden
    }
}

function tareScale() {
    fetch('/api/tare', { method: 'POST' })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .catch(() => {});
}

// ========== TARE HOLD BUTTON ==========
let tareTimer = null;
let tareBtn = null;

function startTare() {
    if (!tareBtn) tareBtn = document.getElementById('tareBtn');
    
    // Add holding class to start animation
    tareBtn.classList.add('holding');
    
    // Set timer for 1 second
    tareTimer = setTimeout(() => {
        // Execute tare after 1 second
        tareScale();
        
        // Visual feedback
        tareBtn.classList.remove('holding');
        tareBtn.classList.add('success');
        
        // Reset after short delay
        setTimeout(() => {
            tareBtn.classList.remove('success');
            const progress = tareBtn.querySelector('.tare-progress');
            if (progress) progress.style.width = '0';
        }, 500);
    }, 1000);
}

function cancelTare() {
    if (!tareBtn) tareBtn = document.getElementById('tareBtn');
    
    // Cancel timer
    if (tareTimer) {
        clearTimeout(tareTimer);
        tareTimer = null;
    }
    
    // Remove holding class
    tareBtn.classList.remove('holding');
    
    // Reset progress bar
    const progress = tareBtn.querySelector('.tare-progress');
    if (progress) {
        progress.style.width = '0';
    }
}

// ========== TARE BUTTON IN CALIBRATION WIZARD ==========
let tareBtnCalib = null;
let tareTimerCalib = null;

function startTareCalib() {
    if (!tareBtnCalib) tareBtnCalib = document.getElementById('tareBtnCalib');
    
    // Add holding class to start animation
    tareBtnCalib.classList.add('holding');
    
    // Set timer for 1 second
    tareTimerCalib = setTimeout(() => {
        // Execute tare after 1 second
        tareScale();
        
        // Visual feedback
        tareBtnCalib.classList.remove('holding');
        tareBtnCalib.classList.add('success');
        
        // Reset after short delay
        setTimeout(() => {
            tareBtnCalib.classList.remove('success');
            const progress = tareBtnCalib.querySelector('.tare-progress');
            if (progress) progress.style.width = '0';
        }, 500);
    }, 1000);
}

function cancelTareCalib() {
    if (!tareBtnCalib) tareBtnCalib = document.getElementById('tareBtnCalib');
    
    // Cancel timer
    if (tareTimerCalib) {
        clearTimeout(tareTimerCalib);
        tareTimerCalib = null;
    }
    
    // Remove holding class
    tareBtnCalib.classList.remove('holding');
    
    // Reset progress bar
    const progress = tareBtnCalib.querySelector('.tare-progress');
    if (progress) {
        progress.style.width = '0';
    }
}

function updateCalibration() {
    const factor = parseFloat(document.getElementById('newCalFactor').value);
    if (isNaN(factor)) { 
        alert(t('alertInvalidFactor'));
        return;
    }
    
    // Validation: factor must be positive
    if (factor <= 0) {
        alert(t('alertNegativeFactor'));
        return;
    }
    
    fetch('/api/calibration', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ value: factor })
    })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .catch(() => alert(t('alertError')));
}

function computeFactor() {
    const known = parseFloat(document.getElementById('knownWeight').value);
    if (isNaN(known) || known <= 0) {
        alert(t('alertInvalidWeight'));
        return;
    }
    if (currentWeight === null || calFactor === null) {
        alert(t('alertDataUnavailable'));
        return;
    }
    
    const newFactor = calFactor * (currentWeight / known);
    document.getElementById('newCalFactor').value = newFactor.toFixed(3);
    
    fetch('/api/calibration', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ value: newFactor })
    })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .catch(() => alert(t('alertError')));
}

function resetWiFi() {
    if (!confirm(t('alertReconfigConfirm'))) return;
    fetch('/api/reset-wifi', { method: 'POST' })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .catch(() => {});
}

function factoryReset() {
    if (!confirm(t('alertResetConfirm'))) return;
    fetch('/api/factory-reset', { method: 'POST' })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .catch(() => {});
}

// ========== CALIBRATION WIZARD ==========
function populateMasterspoolSelect() {
    const select = document.getElementById('masterspoolSelect');
    if (!select) return;
    
    // Clear existing options
    select.innerHTML = '';
    
    // Add masterspools as options
    masterspools.forEach(spool => {
        const option = document.createElement('option');
        option.value = spool.id;
        option.textContent = spool.label;
        select.appendChild(option);
    });
    
    // Trigger change to show first image
    onMasterspoolChange();
}

function onMasterspoolChange() {
    const select = document.getElementById('masterspoolSelect');
    const customInput = document.getElementById('calibKnownWeight');
    const img = document.getElementById('masterspoolImg');
    const errorMsg = document.getElementById('calibWeightError');
    
    if (!select) return;
    
    const selectedId = select.value;
    const selectedSpool = masterspools.find(s => s.id === selectedId);
    
    if (!selectedSpool) return;
    
    // Clear any previous error
    if (customInput) customInput.classList.remove('error');
    if (errorMsg) errorMsg.style.display = 'none';
    
    // Show/hide custom input
    if (selectedId === 'custom') {
        customInput.style.display = 'block';
        customInput.value = '';
        customInput.focus();
        
        // Add input listener to clear error on typing
        customInput.oninput = function() {
            if (this.classList.contains('error')) {
                this.classList.remove('error');
                if (errorMsg) errorMsg.style.display = 'none';
            }
        };
    } else {
        customInput.style.display = 'none';
        customInput.value = selectedSpool.weight;
    }
    
    // Update main image
    if (selectedSpool.image && img) {
        img.src = selectedSpool.image;
        img.style.display = 'block';
        img.onerror = function() {
            // If image fails to load, hide it
            this.style.display = 'none';
        };
    }
}

function updateProgressDots(activeStep) {
    document.querySelectorAll('.progress-dots .dot').forEach((dot, index) => {
        const step = index + 1;
        dot.classList.remove('active', 'completed');
        
        if (step === activeStep) {
            dot.classList.add('active');
        } else if (step < activeStep) {
            dot.classList.add('completed');
        }
    });
}

function calibStep1() {
    // Tare the scale
    fetch('/api/tare', { method: 'POST' })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .then(() => {
        // Move to step 2
        document.getElementById('step1').classList.remove('active');
        document.getElementById('step2').classList.add('active');
        updateProgressDots(2);
        
        // Initialize masterspool selector (set default value)
        populateMasterspoolSelect();
    })
    .catch(() => alert(t('alertError')));
}

function calibStep2() {
    const knownWeight = parseFloat(document.getElementById('calibKnownWeight').value);
    
    // Validation 1: if user entered weight is invalid or <= 0
    if (isNaN(knownWeight) || knownWeight <= 0) {
        showWeightInvalidModal();
        return;
    }
    
    // Validation 2: if user entered weight is between 0 and 200g
    if (knownWeight < 200) {
        showWeightErrorModal();
        return;
    }
    
    // Check data availability
    if (currentWeight === null || calFactor === null) {
        alert(t('alertDataUnavailable'));
        return;
    }
    
    // Calculate new factor
    const newFactor = calFactor * (currentWeight / knownWeight);
    
    // Validation: factor must be positive
    if (newFactor <= 0) {
        alert(t('alertNegativeFactor'));
        return;
    }
    
    // Send to device
    fetch('/api/calibration', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ value: newFactor })
    })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .then(() => {
        // Move to step 3
        document.getElementById('step2').classList.remove('active');
        document.getElementById('step3').classList.add('active');
        updateProgressDots(3);
    })
    .catch(() => alert(t('alertError')));
}

function calibBack(step) {
    // Go back to step
    document.getElementById('step2').classList.remove('active');
    document.getElementById('step' + step).classList.add('active');
    updateProgressDots(step);
    
    // Clear input
    document.getElementById('calibKnownWeight').value = '';
}

function calibReset() {
    // Reset wizard to step 1
    document.getElementById('step3').classList.remove('active');
    document.getElementById('step1').classList.add('active');
    updateProgressDots(1);
    
    // Clear input
    document.getElementById('calibKnownWeight').value = '';
}

// ========== WEIGHT ERROR MODAL ==========
function showWeightErrorModal() {
    const modal = document.getElementById('weightErrorModal');
    if (modal) {
        modal.classList.add('show');
    }
}

function closeWeightErrorModal() {
    const modal = document.getElementById('weightErrorModal');
    if (modal) {
        modal.classList.remove('show');
    }
}

function showWeightInvalidModal() {
    const modal = document.getElementById('weightInvalidModal');
    if (modal) {
        modal.classList.add('show');
    }
}

function closeWeightInvalidModal() {
    const modal = document.getElementById('weightInvalidModal');
    if (modal) {
        modal.classList.remove('show');
    }
}

// Close modal on click outside
document.addEventListener('DOMContentLoaded', function() {
    const errorModal = document.getElementById('weightErrorModal');
    const invalidModal = document.getElementById('weightInvalidModal');
    
    if (errorModal) {
        errorModal.addEventListener('click', function(e) {
            if (e.target === errorModal) {
                closeWeightErrorModal();
            }
        });
    }
    
    if (invalidModal) {
        invalidModal.addEventListener('click', function(e) {
            if (e.target === invalidModal) {
                closeWeightInvalidModal();
            }
        });
    }
});

// ========== STATUS MANAGEMENT ==========
function applyStatusSnapshot(s) {
    if (!s || typeof s !== 'object') return;
    
    // Weight
    if (typeof s.weight !== 'undefined' && currentWeight !== s.weight) {
        currentWeight = s.weight;
        setTextIfChanged(weightEl, String(s.weight));
    }
    
    // UID
    if (typeof s.uid !== 'undefined') {
        const u = s.uid || '';
        if (currentUid !== u) {
            currentUid = u;
            setTextIfChanged(uidEl, u || t('waiting'));
        }
    }
    
    // Cloud
    if (typeof s.cloud !== 'undefined') {
        setCloudStatus(s.cloud);
    }
    
    // API Key
    const apiInput = document.getElementById('newApiKey');
    if (apiInput && typeof s.apiKey === 'string' && apiKey !== s.apiKey) {
        apiKey = s.apiKey;
        if (apiInput.value !== apiKey) apiInput.value = apiKey;
    }
    
    // API Status
    if (typeof s.apiKey === 'string') {
        const hasKey = s.apiKey.trim().length > 0;
        const state = hasKey ? (s.apiValid ? 'valid' : 'invalid') : 'none';
        setApiStatus(state, s.displayName || '');
    }
    
    // Calibration factor
    if (typeof s.calibrationFactor !== 'undefined') {
        const n = Number(s.calibrationFactor);
        const shown = isFinite(n) ? n.toFixed(2) : '—';
        setTextIfChanged(calFactorEl, shown);
        calFactor = n;
    }
    
    // Uptime
    if (typeof s.uptime_s !== 'undefined' || typeof s.uptime_ms !== 'undefined') {
        let secs = (typeof s.uptime_s !== 'undefined') ? Number(s.uptime_s) : Number(s.uptime_ms) / 1000;
        const upEl = document.getElementById('uptime');
        setTextIfChanged(upEl, formatHMS(secs));
    }
    
    // Send to cloud status
    if (typeof s.sendToCloud !== 'undefined') {
        const v = String(s.sendToCloud || '').trim();
        if (v === '' || v === '0') {
            setSendState('');
        } else if (v === 'send') {
            setSendState(t('sending'), 'rgba(255,255,255,0.2)');
        } else if (v === 'success') {
            setSendState(t('sent'), 'rgba(72,187,120,0.3)');
            setTimeout(() => setSendState(''), 1500);
        } else if (v === 'error') {
            setSendState(t('sendError'), 'rgba(245,101,101,0.3)');
            setTimeout(() => setSendState(''), 2000);
        } else if (/^\d+$/.test(v)) {
            setSendState(t('sendIn') + ' ' + v + 's', 'rgba(255,255,255,0.2)');
        }
    }
}

function pollStatus() {
    fetch('/api/status', { cache: 'no-store' })
    .then(r => r.ok ? r.json() : Promise.reject(r.status))
    .then(s => applyStatusSnapshot(s))
    .catch(() => {});
}

// ========== INITIALIZATION ==========
window.onload = () => {
    // Set language
    setLanguage(currentLang);
    
    // Initial weight display
    setTextIfChanged(weightEl, '…');
    
    // Start polling
    pollStatus();
    setInterval(pollStatus, 1000);
    
    // Register Service Worker for PWA
    if ('serviceWorker' in navigator) {
        navigator.serviceWorker.register('/sw.js')
            .then(reg => console.log('Service Worker registered'))
            .catch(err => console.log('Service Worker registration failed'));
    }
};
