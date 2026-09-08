/* =========================
   ESP32 WEBSOCKET CONNECTION
   (must be declared before setupRadialBar is called,
   since the message handler references tempBar/humidityBar
   which are only *used*, not *called*, at message time)
========================= */

//HAN COMMENT I have added a comment for every event listener and function

const socket = new WebSocket(`ws://${location.host}/ws`);
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
        const reading = data.values[0];
        console.log("READING:", reading);

        tempBar.setValue(reading.temp);
        humidityBar.setValue(reading.humidity);
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

//HAN COMMENT what does this chunk of code do?
function setupRadialBar({
    skillSelector,
    sliderId,
    minSliderId,
    maxSliderId,
    numberSuffix
}) { //HAN COMMENT is the { needed? it might be, i am just reading the code direct in github
    const skill = document.querySelector(skillSelector);
    const slider = document.getElementById(sliderId);
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
    let target = parseFloat(slider.value);

    // LIVE SAFE RANGE VALUES
    let safeMin = parseFloat(document.getElementById(minSliderId).value);
    let safeMax = parseFloat(document.getElementById(maxSliderId).value);

    const minSlider = document.getElementById(minSliderId);
    const maxSlider = document.getElementById(maxSliderId);
    const mainProgress = skill.closest(".main-progress");
    const controls = mainProgress.querySelector(".progressControls");

    const [upMax, downMax, upMin, downMin] = controls.querySelectorAll("button");
//HAN COMMENT what does this chunk of code do?
    minSlider.addEventListener('input', () => {
        safeMin = parseFloat(minSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    maxSlider.addEventListener('input', () => {
        safeMax = parseFloat(maxSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    slider.addEventListener('input', () => {
        target = parseFloat(slider.value);
    });

    const step = 5; // how much each click changes the value by
//HAN COMMENT what does this chunk of code do?
    upMax.addEventListener('click', () => {
        maxSlider.value = Math.min(parseFloat(maxSlider.max), parseFloat(maxSlider.value) + step);
        safeMax = parseFloat(maxSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    downMax.addEventListener('click', () => {
        maxSlider.value = Math.max(parseFloat(maxSlider.min), parseFloat(maxSlider.value) - step);
        safeMax = parseFloat(maxSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    upMin.addEventListener('click', () => {
        minSlider.value = Math.min(parseFloat(minSlider.max), parseFloat(minSlider.value) + step);
        safeMin = parseFloat(minSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    downMin.addEventListener('click', () => {
        minSlider.value = Math.max(parseFloat(minSlider.min), parseFloat(minSlider.value) - step);
        safeMin = parseFloat(minSlider.value);
    });
//HAN COMMENT what does this chunk of code do?
    function animate() {
        current += (target - current) * 0.05;

        if (Math.abs(target - current) < 0.1)
            current = target;

        number.textContent = `${Math.round(current)}${numberSuffix}`;

        // MAIN progress
        const mainOffset =
            mainCircumference - (current / 100) * mainCircumference;

        mainCircle.style.strokeDashoffset = mainOffset;

        // SAFE range
        // convert 0–100 range into circle space
        const safeStart = safeMin / 100;
        const safeEnd = safeMax / 100;

        // clamp in case sliders cross
        const start = Math.min(safeStart, safeEnd);
        const end = Math.max(safeStart, safeEnd);

        // SAFE ARC LENGTH
        const safeLength = (end - start) * safeCircumference;

        // SAFE ARC POSITION
        const safeOffset = (start * safeCircumference);

        // draw safe zone as fixed segment
        safeCircle.style.strokeDasharray = `${safeLength} ${safeCircumference}`;
        safeCircle.style.strokeDashoffset = -safeOffset;

        requestAnimationFrame(animate);
    }

    animate();

    return {
        setValue(value) {
            slider.value = value;
            target = parseFloat(value);
        }
    };
}

/* =========================
   BAR SETUP
========================= */

const tempBar = setupRadialBar({
    skillSelector: '.skill[data-index="0"]',
    sliderId: 'slider1',
    minSliderId: 'slider1-2',
    maxSliderId: 'slider2-2',
    numberSuffix: '°C'
});

const humidityBar = setupRadialBar({
    skillSelector: '.skill[data-index="1"]',
    sliderId: 'slider2',
    minSliderId: 'slider1-3',
    maxSliderId: 'slider2-3',
    numberSuffix: '%'
});
