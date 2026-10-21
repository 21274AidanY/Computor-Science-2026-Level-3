/* =========================
   ESP32 WEBSOCKET CONNECTION
   (must be declared before setupRadialBar is called,
   since the message handler references tempBar/humidityBar
   which are only *used*, not *called*, at message time)
========================= */

//HAN COMMENT I have added a comment for every event listener and function

const socket = new WebSocket(`ws://${location.host}/ws`);

function sendSettings() {
    if (socket.readyState !== WebSocket.OPEN) return;

    socket.send(JSON.stringify({
        type: "settings",
        TempMin: tempBar.getMin(),
        TempMax: tempBar.getMax(),
        HumidMin: humidityBar.getMin(),
        HumidMax: humidityBar.getMax()
    }));
}

//HAN COMMENT what does this chunk of code do?
function setupRadialBar({
    skillSelector,
    minSliderId,
    maxSliderId,
    numberSuffix
}) {
    const skill = document.querySelector(skillSelector);
    const number = skill.querySelector('.number');
    const circles = skill.querySelectorAll('circle');

    const mainCircle = circles[0];
    const safeCircle = circles[1];

    const mainRadius = mainCircle.r.baseVal.value;
    const safeRadius = safeCircle.r.baseVal.value;

    const mainCircumference = 2 * Math.PI * mainRadius;
    const safeCircumference = 2 * Math.PI * safeRadius;

    mainCircle.style.strokeDasharray = mainCircumference;
    mainCircle.style.strokeDashoffset = mainCircumference;

    safeCircle.style.strokeDasharray = safeCircumference;
    safeCircle.style.strokeDashoffset = safeCircumference;

    let current = 0;
    let target = 0;

    // Safe range values are stored directly instead of using sliders.
    let safeMin = 30;
    let safeMax = 70;

    const mainProgress = skill.closest(".main-progress");
    const controls = mainProgress.querySelector(".progressControls");

    const [upMax, downMax, upMin, downMin] =
        controls.querySelectorAll("button");

    const step = 5;
    
    //Range control buttons, change their max or min and then send the new values using sendSettings();
    upMax.addEventListener('click', () => {
        safeMax = Math.min(100, safeMax + step);
        sendSettings();
    });

    downMax.addEventListener('click', () => {
        safeMax = Math.max(0, safeMax - step);
        sendSettings();
    });

    upMin.addEventListener('click', () => {
        safeMin = Math.min(100, safeMin + step);
        sendSettings();
    });

    downMin.addEventListener('click', () => {
        safeMin = Math.max(0, safeMin - step);
        sendSettings();
    });

    function animate() {
        current += (target - current) * 0.05;

        if (Math.abs(target - current) < 0.1) {
            current = target;
        }

        number.textContent =
            `${Math.round(current)}${numberSuffix}`;

        const mainOffset =
            mainCircumference - (current / 100) * mainCircumference;

        mainCircle.style.strokeDashoffset = mainOffset;

        const start = Math.min(safeMin, safeMax) / 100;
        const end = Math.max(safeMin, safeMax) / 100;

        const safeLength = (end - start) * safeCircumference;
        const safeOffset = start * safeCircumference;

        safeCircle.style.strokeDasharray =
            `${safeLength} ${safeCircumference}`;

        safeCircle.style.strokeDashoffset = -safeOffset;

        requestAnimationFrame(animate);
    }

    animate();
    
    //Expose info
    return {
    setValue(value) {
        target = Math.max(0, Math.min(100, Number(value) || 0));
    },

    getMin() {
        return safeMin;
    },

    getMax() {
        return safeMax;
    }
};
}


/* =========================
   BAR SETUP
========================= */

const tempBar = setupRadialBar({
    skillSelector: '.skill[data-index="0"]',
    numberSuffix: '°C'
});

const humidityBar = setupRadialBar({
    skillSelector: '.skill[data-index="1"]',
    numberSuffix: '%'
});

//HAN COMMENT what does this chunk of code do?
socket.addEventListener('open', () => {
    console.log("Connected to ESP32");
    socket.send("getStates"); // ask for an immediate reading
});

//HAN COMMENT what does this chunk of code do?
socket.addEventListener('message', (event) => {
    console.log("RAW MESSAGE:", event.data);

    try {
        const data = JSON.parse(event.data);
        console.log("PARSED:", data);

        // Handle sensor readings from the ESP32.
        if (data.values) {
            const reading = data.values[0];

            tempBar.setValue(reading.temp);
            humidityBar.setValue(reading.humidity);
        }

        // Settings messages can be handled separately if needed.
        else if (data.type === "settings") {
            console.log("Settings received:", data);
        }

    } catch (err) {
        console.error("Bad WebSocket message:", event.data, err);
    }
});

//HAN COMMENT what does this chunk of code do?
socket.addEventListener('close', () => {
    console.log("Disconnected from ESP32");
});

//HAN COMMENT what does this chunk of code do?
socket.addEventListener('error', (err) => {
    console.error("WebSocket error:", err);
});
