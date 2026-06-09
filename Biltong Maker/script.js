function setupRadialBar({
    skillSelector,
    sliderId,
    numberSuffix,
    safeMin,
    safeMax
})
{
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

    slider.addEventListener('input', () =>
    {
        target = parseFloat(slider.value);
    });

    function animate()
    {
        current += (target - current) * 0.05;

        if (Math.abs(target - current) < 0.1)
            current = target;

        number.textContent = `${Math.round(current)}${numberSuffix}`;

        const mainOffset = mainCircumference - (current / 100) * mainCircumference;
        mainCircle.style.strokeDashoffset = mainOffset;

        const safeStart = safeMin / 100;
        const safeEnd = safeMax / 100;

        const safeLength = (safeEnd - safeStart) * safeCircumference;
        const safeOffset = safeCircumference - (safeStart * safeCircumference);

        safeCircle.style.strokeDasharray = `${safeLength} ${safeCircumference - safeLength}`;
        safeCircle.style.strokeDashoffset = safeOffset;

        requestAnimationFrame(animate);
    }

    animate();
}

setupRadialBar({
    skillSelector: '.skill[data-index="0"]',
    sliderId: 'slider1',
    numberSuffix: '°C',
    safeMin: 45,
    safeMax: 70
});

setupRadialBar({
    skillSelector: '.skill[data-index="1"]',
    sliderId: 'slider2',
    numberSuffix: '%',
    safeMin: 35,
    safeMax: 60
});