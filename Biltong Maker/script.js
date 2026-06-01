const bars = document.querySelectorAll('.skill');

const sliders = [
    document.getElementById('slider1'),
    document.getElementById('slider2')
];

const targets = [parseInt(sliders[0].value), parseInt(sliders[1].value)];
const currents = [0, 0];

bars.forEach((bar, index) => {
    const number = bar.querySelector('.number');
    const circle = bar.querySelector('circle');
    const circumference = 2 * Math.PI * 70;

    circle.style.strokeDasharray = circumference;
    circle.style.strokeDashoffset = circumference;

    sliders[index].addEventListener("input", () => {
        targets[index] = parseInt(sliders[index].value);
    });

    function animate() {
        currents[index] += (targets[index] - currents[index]) * 0.05;

        if (Math.abs(targets[index] - currents[index]) < 0.1)
            currents[index] = targets[index];

        number.innerHTML = `${Math.round(currents[index])}%`;

        let offset = circumference - (currents[index] / 100) * circumference;
        circle.style.strokeDashoffset = offset;

        requestAnimationFrame(animate);
    }

    animate();
});