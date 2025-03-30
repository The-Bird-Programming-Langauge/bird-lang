const fs = require("fs");

const outputPath = "./output.txt";
fs.writeFileSync(outputPath, "");

/**
 * The memory layout is as follows:
 * 1. The first byte is reserved for the null pointer
 * 2. The next 32 bits are reserved for the pointer to the head of the free list
 * 3. The next 32 bits are reserved for the pointer to the head of the allocated list
 * 4. The rest of the memory is used for the blocks
 * 
 * Block layout:
 * 1. The first 32 bits are used to store the size of the block
 * 2. The next 32 bits are used to store the pointer to the next block
 * 3. The next byte is used to store the mark bit
 * 3. The next 32 bits are used to store how many pointers are in the block 
 * 4. The rest of the block is used to store the actual data values
 * 
 * Data value layout:
 * 1. The first byte is used to store the pointer bit
 *   * 0 - the value is a 32/64 bit value
 *   * 1 - the value is a pointer
 * 2. The next 32/64 bits are used to store the actual value
 */

const NULL_PTR = 0;
const FREE_HEAD_PTR = 1;
const ALLOCATED_HEAD_PTR = 5;
const BLOCK_SIZE_OFFSET = 0;
const BLOCK_PTR_OFFSET = 4;
const BLOCK_MARK_OFFSET = 8;
const BLOCK_NUM_PTRS = 9;
const BLOCK_HEADER_SIZE = 13;
const FREE_LIST_START = 13;

const FLOAT_SIZE = 8;
const INT_SIZE = 4;


function get_free_list_head_ptr() {
    return memory.getUint32(FREE_HEAD_PTR);
}

function get_allocated_list_head_ptr() {
    return memory.getUint32(ALLOCATED_HEAD_PTR);
}

function get_block_size(ptr) {
    return memory.getUint32(ptr + BLOCK_SIZE_OFFSET);
}

function get_block_next_ptr(ptr) {
    return memory.getUint32(ptr + BLOCK_PTR_OFFSET);
}

function block_is_marked(ptr) {
    return memory.getUint8(ptr + BLOCK_MARK_OFFSET) === 1;
}

function set_block_size(ptr, size) {
    memory.setUint32(ptr + BLOCK_SIZE_OFFSET, size);
}

function set_block_next_ptr(ptr, next_ptr) {
    memory.setUint32(ptr + BLOCK_PTR_OFFSET, next_ptr);
}

function get_block_num_ptrs(ptr) {
    return memory.getUint32(ptr + BLOCK_NUM_PTRS);
}

function set_block_num_ptrs(ptr, num_ptrs) {
    memory.setUint32(ptr + BLOCK_NUM_PTRS, num_ptrs);
}

function set_block_mark(ptr, mark) {
    memory.setUint8(ptr + BLOCK_MARK_OFFSET, mark);
}

function set_free_list_head_ptr(ptr) {
    memory.setUint32(FREE_HEAD_PTR, ptr);
}

function set_allocated_list_head_ptr(ptr) {
    memory.setUint32(ALLOCATED_HEAD_PTR, ptr);
}

function value_is_pointer(ptr) {
    return memory.getUint8(ptr) & 0b01;
}

function value_is_64_bit(ptr) {
    return memory.getUint8(ptr) & 0b10;
}


// prints the memory such that each byte is printed as an integer. byte indicies are also printed
function print_memory(memory) {
    const memory_array = new Uint8Array(memory.buffer);
    const row_length = 40; // number of bytes per row
    const total_rows = 3; // total rows

    for (let row = 0; row < total_rows; row += 1) {
        const start_idx = row * row_length;
        const end_idx = start_idx + row_length;

        // print a memory row
        let memory_row = "";
        for (let i = start_idx; i < end_idx; i += 1) {
            memory_row += memory_array[i].toString().padStart(4, ' ');
        }
        console.log(memory_row);

        // print an indicies row
        let index_row = "";
        for (let i = start_idx; i < end_idx; i += 1) {
            index_row += i.toString().padStart(4, ' ');
        }
        console.log(index_row);

        console.log();
    }
}

const moduleOptions = {
    env: {
        push_ptr: (arr_ptr, value) => {
            push_32(arr_ptr, value);
            const length = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE + INT_SIZE);
            const new_block_ptr = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE);
            set_block_num_ptrs(new_block_ptr, length);
        },
        push_32,
        push_64: (arr_ptr, value) => {
            let data_ptr = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE);
            const length = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE + INT_SIZE);
            const capacity = (get_block_size(data_ptr) - BLOCK_HEADER_SIZE) / FLOAT_SIZE;
            if (length + 1 >= capacity) {
                const new_data_ptr = mem_alloc((get_block_size(data_ptr) - BLOCK_HEADER_SIZE) * 2, 0);
                for (let i = 0; i < length; i += 1) {
                    memory.setFloat64(new_data_ptr + BLOCK_HEADER_SIZE + i * FLOAT_SIZE,
                        (memory.getFloat64(data_ptr + BLOCK_HEADER_SIZE + i * FLOAT_SIZE)));
                }
                data_ptr = new_data_ptr;
                memory.setUint32(arr_ptr + BLOCK_HEADER_SIZE, data_ptr);
            }

            memory.setFloat64(data_ptr + BLOCK_HEADER_SIZE + length * FLOAT_SIZE, value);
            memory.setUint32(arr_ptr + BLOCK_HEADER_SIZE + INT_SIZE, length + 1);
        },
        strcat: (left, right) => {
            const left_data = memory.getUint32(left + BLOCK_HEADER_SIZE);
            const left_length = memory.getUint32(left + BLOCK_HEADER_SIZE + INT_SIZE);
            const right_data = memory.getUint32(right + BLOCK_HEADER_SIZE);
            const right_length = memory.getUint32(right + BLOCK_HEADER_SIZE + INT_SIZE);

            const ptr = mem_alloc(8);
            const data = mem_alloc(left_length * INT_SIZE + right_length * INT_SIZE);

            memory.setUint32(ptr + BLOCK_HEADER_SIZE, data);
            memory.setUint32(ptr + BLOCK_HEADER_SIZE + INT_SIZE, left_length + right_length);

            for (let i = 0; i < left_length; i++) {
                const char = memory.getUint32(left_data + BLOCK_HEADER_SIZE + i * INT_SIZE);
                memory.setUint32(data + BLOCK_HEADER_SIZE + i * INT_SIZE, char);
            }

            for (let i = 0; i < right_length; i++) {
                const char = memory.getUint32(right_data + BLOCK_HEADER_SIZE + i * INT_SIZE);
                memory.setUint32(data + BLOCK_HEADER_SIZE + (i + left_length) * INT_SIZE, char);
            }


            return ptr;
        },
        strcmp: (left, right) => {
            const left_data = memory.getUint32(left + BLOCK_HEADER_SIZE);
            const left_length = memory.getUint32(left + BLOCK_HEADER_SIZE + INT_SIZE);
            const right_data = memory.getUint32(right + BLOCK_HEADER_SIZE);
            const right_length = memory.getUint32(right + BLOCK_HEADER_SIZE + INT_SIZE);

            if (left_length != right_length) {
                return false;
            }

            for (let i = 0; i < left_length; i++) {
                if (memory.getUint32(left_data + BLOCK_HEADER_SIZE + i * INT_SIZE) !=
                    memory.getUint32(right_data + BLOCK_HEADER_SIZE + i * INT_SIZE)
                ) {
                    return false;
                }
            }

            return true;
        },
        print_i32: (value) => {
            process.stdout.write(value.toString());
            fs.appendFileSync(outputPath, value.toString());
        },
        print_f64: (value) => {
            process.stdout.write(value.toString());
            fs.appendFileSync(outputPath, value.toString());
        },
        print_bool: (value) => {
            const bool_str = value === 1 ? "true" : "false";
            process.stdout.write(bool_str);
            fs.appendFileSync(outputPath, bool_str);
        },
        print_str: (ptr) => {
            const data = memory.getUint32(ptr + BLOCK_HEADER_SIZE);
            const length = memory.getUint32(ptr + BLOCK_HEADER_SIZE + INT_SIZE);

            let str = "";
            for (let i = 0; i < length; i++) {
                str += String.fromCharCode(memory.getUint32(data + BLOCK_HEADER_SIZE + i * INT_SIZE));
            }

            process.stdout.write(str);
            fs.appendFileSync(outputPath, str);
        },
        print_endline: () => {
            console.log();
            fs.appendFileSync(outputPath, "\n");
        },
        mem_get_32: (ptr, byte_offset) => {
            return memory.getUint32(ptr + BLOCK_HEADER_SIZE + byte_offset);
        },

        mem_get_64: (ptr, byte_offset) => {
            return memory.getFloat64(ptr + BLOCK_HEADER_SIZE + byte_offset);
        },
        mem_set_32: (ptr, offset, value) => {
            memory.setUint32(ptr + BLOCK_HEADER_SIZE + offset, value);
        },
        mem_set_64: (ptr, offset, value) => {
            memory.setFloat64(ptr + BLOCK_HEADER_SIZE + offset, value);
        },
        mem_set_ptr: (ptr, offset, value) => {
            memory.setUint32(ptr + BLOCK_HEADER_SIZE + offset, value);
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
        mem_alloc,

        mark: (ptr) => // the root is any local or global variable that is dynamically allocated
        {
            const stack = [];
            stack.push(ptr);

            while (stack.length > 0) {
                // pointer to the current block
                const block_ptr = stack.pop();

                if (!ptr) {
                    continue;
                }

                // if the block is already marked, skip it. otherwise mark it
                if (block_is_marked(block_ptr)) {
                    continue;
                } else {
                    set_block_mark(block_ptr, 1);
                }

                const num_ptrs = get_block_num_ptrs(block_ptr);
                for (let i = 0; i < num_ptrs; i++) {
                    stack.push(memory.getUint32(block_ptr + BLOCK_HEADER_SIZE + (INT_SIZE * i)));
                }
            }
        },

        sweep: () => {
            let curr_ptr = get_allocated_list_head_ptr();

            // no allocated block exists
            if (curr_ptr === 0) {
                return;
            }

            let prev_ptr = curr_ptr;
            let next_block_is_not_null = true;

            while (curr_ptr < memory.byteLength && next_block_is_not_null) {
                let next_ptr = get_block_next_ptr(curr_ptr); // get the next allocated block to traverse to in the next iteration
                let update_prev_ptr = true;

                // the loop should stop when we reach the end of the allocated list
                if (next_ptr === 0) {
                    next_block_is_not_null = false;
                }

                // if the block is not marked, pop it from the allocated list and push it to the free list
                if (!block_is_marked(curr_ptr)) {
                    // pop the block from the allocated list:
                    // if the block is the head, set the allocated list head pointer to the next allocated block pointer
                    if (curr_ptr === get_allocated_list_head_ptr()) {
                        set_allocated_list_head_ptr(next_ptr);
                        // otherwise, set the previous block's next pointer to the current block's next pointer
                    } else {
                        set_block_next_ptr(prev_ptr, next_ptr);
                        update_prev_ptr = false; // do not update prev_ptr if the block gets popped from the middle of the list
                    }

                    // add the block to the head of the free list
                    set_block_next_ptr(curr_ptr, get_free_list_head_ptr());
                    set_free_list_head_ptr(curr_ptr);
                } else {
                    set_block_mark(curr_ptr, 0); // clear the mark bit
                }

                if (update_prev_ptr) {
                    prev_ptr = curr_ptr;
                }
                curr_ptr = next_ptr;
            }
        },

        initialize_memory: () => {
            // initialize the memory header
            memory.setUint8(NULL_PTR, 0); // set the null pointer to 0
            set_free_list_head_ptr(FREE_LIST_START); // set the free list head pointer to the first free block
            set_allocated_list_head_ptr(0); // set the allocated list head pointer to 0

            // create the first free block
            set_block_size(FREE_LIST_START, memory.byteLength); // set the size to take up the entire memory
            set_block_next_ptr(FREE_LIST_START, 0); // set the next pointer to the null pointer
            set_block_mark(FREE_LIST_START, 0); // set the mark bit to 0
        }
    }
};

const result = fs.readFileSync("output.wasm");

let instance;
let memory;
let wasmMemory;
WebAssembly.instantiate(result, moduleOptions).then((wasmInstatiatedSource) => {
    instance = wasmInstatiatedSource.instance;
    memory = new DataView(instance.exports.memory.buffer);
    wasmMemory = instance.exports.memory;
    instance.exports.main();
    //print_memory(memory); // debug
});

function mem_alloc(size, num_pointers) {
    let curr_ptr = get_free_list_head_ptr();
    let prev_ptr = curr_ptr;

    // loop until curr_ptr stores a block pointer that is big enough
    while (!(
        get_block_size(curr_ptr) > size + 2 * BLOCK_HEADER_SIZE || // block that is bigger and can be split into 2 blocks, 1 allocated and 1 free, such that the free block has at least 1 byte of data space
        get_block_size(curr_ptr) >= size + BLOCK_HEADER_SIZE // block that is exactly the right size
    )) {
        if (curr_ptr + 1 > memory.byteLength) { // we have reached the end of the memory
            throw new Error("Out of memory");
        }
        if (get_block_next_ptr(curr_ptr) === 0) { // we have reached the end of the list
            wasmMemory.grow(1);
            memory = new DataView(instance.exports.memory.buffer);
            set_block_size(curr_ptr, memory.byteLength - curr_ptr);
            break;
        }

        prev_ptr = curr_ptr;
        curr_ptr = get_block_next_ptr(curr_ptr); // next block
    }

    // we have found a block that is big enough
    if (get_block_size(curr_ptr) - size > BLOCK_HEADER_SIZE + FLOAT_SIZE * 2) { // we can split the block
        const new_block_ptr = curr_ptr + size + BLOCK_HEADER_SIZE;

        set_block_size(new_block_ptr, get_block_size(curr_ptr) - size - BLOCK_HEADER_SIZE); // set the size of the new block
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

        // move the current block to the head of the allocated list
        set_block_next_ptr(curr_ptr, get_allocated_list_head_ptr()); // set the pointer of the current block
        set_allocated_list_head_ptr(curr_ptr); // set the head of the allocated list
    } else {
        if (curr_ptr === get_free_list_head_ptr()) {
            set_free_list_head_ptr(get_block_next_ptr(curr_ptr));
        }

        set_block_next_ptr(curr_ptr, get_allocated_list_head_ptr()); // set the pointer of the current block
        set_allocated_list_head_ptr(curr_ptr); // set the head of the allocated list
    }



    set_block_num_ptrs(curr_ptr, num_pointers);
    return curr_ptr;
}

function push_32(arr_ptr, value) {
    let data_ptr = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE);
    const length = memory.getUint32(arr_ptr + BLOCK_HEADER_SIZE + INT_SIZE);
    const capacity = (get_block_size(data_ptr) - BLOCK_HEADER_SIZE) / INT_SIZE;
    if (length + 1 >= capacity) {
        const new_data_ptr = mem_alloc((get_block_size(data_ptr) - BLOCK_HEADER_SIZE) * 2, 0);
        for (let i = 0; i < length; i += 1) {
            memory.setUint32(new_data_ptr + BLOCK_HEADER_SIZE + i * INT_SIZE,
                (memory.getUint32(data_ptr + BLOCK_HEADER_SIZE + i * INT_SIZE)));
        }
        data_ptr = new_data_ptr;
        memory.setUint32(arr_ptr + BLOCK_HEADER_SIZE, data_ptr);
    }
    memory.setUint32(data_ptr + BLOCK_HEADER_SIZE + (length * INT_SIZE), value);
    memory.setUint32(arr_ptr + BLOCK_HEADER_SIZE + INT_SIZE, length + 1);
}