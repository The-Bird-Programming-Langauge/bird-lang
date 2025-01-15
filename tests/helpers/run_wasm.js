const fs = require("fs");
const { off } = require("process");

const outputPath = "./output.txt";
fs.writeFileSync(outputPath, "");

const moduleOptions = {
    env: {
        print_i32: (value) => {
            console.log(value);
            fs.appendFileSync(outputPath, value.toString() + "\n");
        },
        print_f64: (value) => {
            console.log(value);
            fs.appendFileSync(outputPath, value.toString() + "\n");
        },
        print_str: (ptr) => {
            const buffer = new Uint8Array(instance.exports.memory.buffer);
            let str = "";
            for (let i = ptr; buffer[i] !== 0; i++) {
                str += String.fromCharCode(buffer[i]);
            }
            console.log(str);
            fs.appendFileSync(outputPath, str + "\n");
        },
        mem_get: (ptr, index) => {
            const buffer = new Uint8Array(instance.exports.memory.buffer);
            const result = buffer[ptr + index];
            return result;
        },
        mem_set: (ptr, offset, value) => {
            const buffer = new Uint8Array(instance.exports.memory.buffer);
            buffer[ptr + offset] = value;
        },
    }
};

const result = fs.readFileSync("output.wasm");


let instance;
WebAssembly.instantiate(result, moduleOptions).then((wasmInstatiatedSource) => {
    instance = wasmInstatiatedSource.instance;
    instance.exports.main();
});