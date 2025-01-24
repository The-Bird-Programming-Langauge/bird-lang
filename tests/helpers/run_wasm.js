const fs = require("fs");

const outputPath = "./output.txt";
fs.writeFileSync(outputPath, "");
/**
 * The memory layout is as follows:
 * 1. The first byte is reserved for the null pointer
 * 2. The next 32 bits are reserved for the pointer to the head of the free list
 * 3. The rest of the memory is used for the blocks
 * 
 * Block layout:
 * 1. The first 32 bits are used to store the size of the block
 * 2. The next 32 bits are used to store the pointer to the next block
 * 3. The next byte is used to store the mark bit
 * 4. The rest of the block is used to store the actual data values
 * 
 * Data value layout:
 * 1. The first byte is used to store the pointer bit
 *   * 0 - the value is a 32/64 bit value
 *   * 1 - the value is a pointer
 * 2. The next 32/64 bits are used to store the actual value
 */


const NULL_PTR = 0;
const HEAD_PTR_OFFSET = 1;
const BLOCK_SIZE_OFFSET = 0;
const BLOCK_PTR_OFFSET = 4;
const BLOCK_MARK_OFFSET = 8;
const BLOCK_HEADER_SIZE = 9;
const FREE_LIST_START = 5;

function get_free_list_head_ptr() {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint32(HEAD_PTR_OFFSET);
}

function get_block_size(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint32(ptr + BLOCK_SIZE_OFFSET);
}

function get_block_next_ptr(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint32(ptr + BLOCK_PTR_OFFSET);
}

function block_is_marked(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint8(ptr + BLOCK_MARK_OFFSET) === 1;
}

function set_block_size(ptr, size) {
    const memory = new DataView(instance.exports.memory.buffer);
    memory.setUint32(ptr + BLOCK_SIZE_OFFSET, size);
}

function set_block_next_ptr(ptr, next_ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    memory.setUint32(ptr + BLOCK_PTR_OFFSET, next_ptr);
}

function set_block_mark(ptr, mark) {
    const memory = new DataView(instance.exports.memory.buffer);
    memory.setUint8(ptr + BLOCK_MARK_OFFSET, mark);
}

function set_free_list_head_ptr(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    memory.setUint32(HEAD_PTR_OFFSET, ptr);
}

function value_is_pointer(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint8(ptr) & 0b01;
}

function value_is_64_bit(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint8(ptr) & 0b10;
}

function value_get_data(value) {
    const memory = new DataView(instance.exports.memory.buffer);
    return memory.getUint32(value + 1);
}

function print_block_first_value(ptr) {
    const memory = new DataView(instance.exports.memory.buffer);
    const value = ptr + BLOCK_HEADER_SIZE;
    if (value_is_64_bit(value)) {
        console.log(memory.getFloat64(value + 1));
    } else if (value_is_pointer(value)) {
        console.log("pointer: ", memory.getUint32(value + 1));
    } else {
        console.log(memory.getUint32(value + 1));
    }
}


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
        mem_get: (ptr, byte_offset) => { 
            const buffer = new DataView(instance.exports.memory.buffer);
            return buffer.getUint32(ptr + BLOCK_HEADER_SIZE + 1 + byte_offset);
        },
        /**
         * The first byte of the pointer is used to store the pointer bit 
         * 
         */
        mem_set_32: (ptr, offset, value) => { 
            const buffer = new DataView(instance.exports.memory.buffer);

            buffer.setUint8(ptr + BLOCK_HEADER_SIZE + offset, 0);
            buffer.setUint32(ptr + BLOCK_HEADER_SIZE + offset + 1, value);
        },
        mem_set_64: (ptr, offset, value) => {
            const buffer = new DataView(instance.exports.memory.buffer);

            buffer.setUint8(ptr + BLOCK_HEADER_SIZE + offset, 0b10);
            buffer.setFloat64(ptr + BLOCK_HEADER_SIZE + offset + 1, value);
        },
        mem_set_ptr: (ptr, offset, value) => {
            const buffer = new DataView(instance.exports.memory.buffer);

            buffer.setUint8(ptr + BLOCK_HEADER_SIZE + offset, 0b01);
            buffer.setUint32(ptr + BLOCK_HEADER_SIZE + offset + 1, value);
        },
        /**
         * This is a first-fit free list allocator
         * each block has the following format:
         *  1. The bytes 0-3 are the size of the block
         *  2. The bytes 4-7 are the pointer to the next block
         *  3. The next byte hold the mark bit
         *  4. The rest of the block is the actual data space
         * 
         * index 0 is reserved for the null pointer, so the first block starts at bit 64
         * index 1 is reserved for the head of the free list
         * 
         */
        mem_alloc: (size) => {
            const memory = new DataView(instance.exports.memory.buffer);

            let curr_ptr = get_free_list_head_ptr(); // head of the free list
            let prev_ptr = curr_ptr;
            while (get_block_size(curr_ptr) <= size + BLOCK_HEADER_SIZE) { // block is too small
                if (curr_ptr + 1 > memory.length) { // we have reached the end of the memory
                    throw new Error("Out of memory");
                }
                if (get_block_next_ptr(curr_ptr) == 0) { // we have reached the end of the list
                    throw new Error("Out of memory");
                }
                prev_ptr = curr_ptr;
                curr_ptr = get_block_next_ptr(curr_ptr); // next block
            }

            // we have found a block that is big enough
            if (get_block_size(curr_ptr) - size > BLOCK_HEADER_SIZE + 4) { // we can split the block
                const new_block_ptr = curr_ptr + size + BLOCK_HEADER_SIZE; 

                set_block_size(new_block_ptr, get_block_size(curr_ptr) - size - BLOCK_HEADER_SIZE); // set the size of the current block
                set_block_next_ptr(new_block_ptr, get_block_next_ptr(curr_ptr)); // set the pointer of the new block

                if (prev_ptr !== curr_ptr) { 
                    set_block_next_ptr(prev_ptr, new_block_ptr); // set the pointer of the new block to the current block
                }
                
                if (curr_ptr === get_free_list_head_ptr()) { // we are at the head of the list
                    set_free_list_head_ptr(new_block_ptr);
                }

                // the current block is now allocated
                set_block_size(curr_ptr, size + BLOCK_HEADER_SIZE); // set the size of the current block
                set_block_mark(curr_ptr, 0); // set the mark bit to zero
            }

            return curr_ptr;
        },

        /**
         * NOTE: The mark and sweep functions are not currently used, but they are implemented for future development.
         * I believe there is something wrong with the mark function.
         */
        mark: (ptr) => // the root is any local or global variable that is dynamically allocated
        {
            const memory = new DataView(instance.exports.memory.buffer);
            const stack = [];
            stack.push(ptr);

            while (stack.length > 0) {
                const ptr = stack.pop();
                if (block_is_marked(ptr)) { // check if the block is marked
                    continue;
                }

                set_block_mark(ptr, 1); // mark the block

                const curr_value = ptr + BLOCK_HEADER_SIZE; 
                const size = get_block_size(ptr);
                for (let i = curr_value; i < ptr + size - BLOCK_HEADER_SIZE; i += value_is_64_bit(i) ? 9 : 5) {
                    if (value_is_pointer(i)) { // check if the value is a pointer
                        stack.push(memory[i]);
                    }
                }
            }
        },

        sweep: () => {
            let curr_ptr = FREE_LIST_START;
            const memory = new DataView(instance.exports.memory.buffer);
            while (curr_ptr < memory.byteLength && get_block_size(curr_ptr) !== 0) {
                if (!block_is_marked(curr_ptr)) { // check if the block is marked
                    print_block_first_value(curr_ptr);
                    set_block_next_ptr(curr_ptr, get_free_list_head_ptr()); // set the pointer of the block to the head of the free list
                    set_free_list_head_ptr(curr_ptr); 
                } else {
                    print_block_first_value(curr_ptr);
                    set_block_mark(curr_ptr, 0); // clear the mark bit
                }

                curr_ptr += get_block_size(curr_ptr);
            }
        },

        initialize_memory: () => {
            const memory = new DataView(instance.exports.memory.buffer);

            memory.setUint8(0, 0); // null pointer
            memory.setUint32(1, FREE_LIST_START); // head of the free list
            memory.setUint32(FREE_LIST_START, 3000);
            memory.setUint32(9, 0); // next block
            memory.setUint8(13, 0); // mark bit to 0
        }
    }
};

const result = fs.readFileSync("output.wasm");

let instance;
WebAssembly.instantiate(result, moduleOptions).then((wasmInstatiatedSource) => {
    instance = wasmInstatiatedSource.instance;
    instance.exports.main();
});