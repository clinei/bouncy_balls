const canvas = document.getElementById('canvas');
const ctx = canvas.getContext('2d');

const step = Module.cwrap('step');
const start_time = Module.cwrap('start_time');
const stop_time = Module.cwrap('stop_time');
const set_screen_size = Module.cwrap('set_screen_size', null, ['number', 'number']);

function get_physics_balls() {
    const ptr = Module.ccall('get_physics_balls', 'number');

    const count       = Module.HEAP32[(ptr+4*0)>>2];
    const ptr_x       = Module.HEAP32[(ptr+4*1)>>2];
    const ptr_y       = Module.HEAP32[(ptr+4*2)>>2];
    const ptr_x_speed = Module.HEAP32[(ptr+4*3)>>2];
    const ptr_y_speed = Module.HEAP32[(ptr+4*4)>>2];
    const ptr_radius  = Module.HEAP32[(ptr+4*5)>>2];

    return {
        count,
        x:         new Float32Array(Module.HEAPF32.buffer, ptr_x,         count),
        y:         new Float32Array(Module.HEAPF32.buffer, ptr_y,         count),
        x_speed:   new Float32Array(Module.HEAPF32.buffer, ptr_x_speed,   count),
        y_speed:   new Float32Array(Module.HEAPF32.buffer, ptr_y_speed,   count),
        radius:    new Float32Array(Module.HEAPF32.buffer, ptr_radius,    count),
        ptr,
    };
}

function main() {

    window.addEventListener('resize', resize);
    resize();

    function resize() {
        canvas.width = window.innerWidth;
        canvas.height = window.innerHeight;
        set_screen_size(window.innerWidth, window.innerHeight);
    }

    Module.ccall('init', null, ['number', 'number'], [canvas.width, canvas.height]);

    requestAnimationFrame(render);

    function render() {
        
        step();

        const physics_balls = get_physics_balls();

        ctx.fillStyle = '#000';
        ctx.fillRect(0, 0, canvas.width, canvas.height);

        for (let i = 0; i < physics_balls.count; i += 1) {
            const x = physics_balls.x[i];
            const y = physics_balls.y[i];
            const radius = physics_balls.radius[i];
            
            ctx.beginPath();
            ctx.arc(x, y, radius, 0, Math.PI*2);
            ctx.fillStyle = '#fff';
            ctx.fill();
        }

        requestAnimationFrame(render);
    }
}

Module.postRun.push(main);

window.addEventListener('blur', onblur);
function onblur(event) {
    stop_time();
}
window.addEventListener('focus', onfocus);
function onfocus(event) {
    start_time();
}