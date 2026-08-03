function setupRadialBar({
    skillSelector,
    sliderId,
    minSliderId,
    maxSliderId,
    numberSuffix
}) {
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

    minSlider.addEventListener('input', () => {
        safeMin = parseFloat(minSlider.value);
    });

    maxSlider.addEventListener('input', () => {
        safeMax = parseFloat(maxSlider.value);
    });
    
    maxSlider.addEventListener('input', () => {
    safeMax = parseFloat(maxSlider.value);
    });

    slider.addEventListener('input', () => {
        target = parseFloat(slider.value);
    });
    
    const step = 5; // how much each click changes the value by

    upMax.addEventListener('click', () => {
        maxSlider.value = Math.min(parseFloat(maxSlider.max), parseFloat(maxSlider.value) + step);
        safeMax = parseFloat(maxSlider.value);
    });

    downMax.addEventListener('click', () => {
        maxSlider.value = Math.max(parseFloat(maxSlider.min), parseFloat(maxSlider.value) - step);
        safeMax = parseFloat(maxSlider.value);
    });

    upMin.addEventListener('click', () => {
        minSlider.value = Math.min(parseFloat(minSlider.max), parseFloat(minSlider.value) + step);
        safeMin = parseFloat(minSlider.value);
    });

    downMin.addEventListener('click', () => {
        minSlider.value = Math.max(parseFloat(minSlider.min), parseFloat(minSlider.value) - step);
        safeMin = parseFloat(minSlider.value);
    });

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
}

/* =========================
   BAR SETUP
========================= */

setupRadialBar({
    skillSelector: '.skill[data-index="0"]',
    sliderId: 'slider1',
    minSliderId: 'slider1-2',
    maxSliderId: 'slider2-2',
    numberSuffix: '°C'
});

setupRadialBar({
    skillSelector: '.skill[data-index="1"]',
    sliderId: 'slider2',
    minSliderId: 'slider1-3',
    maxSliderId: 'slider2-3',
    numberSuffix: '%'
});

setupRadialBar({
    skillSelector: '.skill[data-index="2"]',
    sliderId: 'slider1-3',
    minSliderId: 'slider1-3',
    maxSliderId: 'slider2-3',
    numberSuffix: '%'
});