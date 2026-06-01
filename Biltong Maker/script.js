// BAR 1
const slider1 = document.getElementById('slider1');
const skill1 = document.querySelectorAll('.skill')[0];
const number1 = skill1.querySelector('.number');
const circle1 = skill1.querySelector('.main-progress circle');

const circumference1 = 2 * Math.PI * 70;

let current1 = 0;
let target1 = parseInt(slider1.value);

circle1.style.strokeDasharray = circumference1;
circle1.style.strokeDashoffset = circumference1;

slider1.addEventListener("input", () => {
    target1 = parseInt(slider1.value);
});

function animateBar1()
{
    current1 += (target1 - current1) * 0.05;

    if (Math.abs(target1 - current1) < 0.1)
        current1 = target1;

    number1.innerHTML = `${Math.round(current1)}°C`;

    let offset = circumference1 - (current1 / 100) * circumference1;
    circle1.style.strokeDashoffset = offset;

    requestAnimationFrame(animateBar1);
}

animateBar1();


// BAR 2
const slider2 = document.getElementById('slider2');
const skill2 = document.querySelectorAll('.skill')[1];
const number2 = skill2.querySelector('.number');
const circle2 = skill2.querySelector('svg circle');

const circumference2 = 2 * Math.PI * 70;

let current2 = 0;
let target2 = parseInt(slider2.value);

circle2.style.strokeDasharray = circumference2;
circle2.style.strokeDashoffset = circumference2;

slider2.addEventListener("input", () => {
    target2 = parseInt(slider2.value);
});

function animateBar2()
{
    current2 += (target2 - current2) * 0.05;

    if (Math.abs(target2 - current2) < 0.1)
        current2 = target2;

    number2.innerHTML = `${Math.round(current2)}%`;

    let offset = circumference2 - (current2 / 100) * circumference2;
    circle2.style.strokeDashoffset = offset;

    requestAnimationFrame(animateBar2);
}

animateBar2();