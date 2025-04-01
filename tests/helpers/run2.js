const fs = require("fs");
const outputPath = "./output.txt";
fs.writeFileSync(outputPath, "");

class Printer {
    print_i32(value) {
        process.stdout.write(value.toString());
        fs.appendFileSync(outputPath, value.toString());
    }
    print_f64(value) {
        process.stdout.write(value.toString());
        fs.appendFileSync(outputPath, value.toString());
    }
    print_bool(value) {
        const bool_str = value === 1 ? "true" : "false";
        process.stdout.write(bool_str);
        fs.appendFileSync(outputPath, bool_str);
    }
    print_str(ptr) {
        // TODO: implement
    }
    print_endline() {
        console.log();
        fs.appendFileSync(outputPath, "\n");
    }
};

class Block {
    static SIZE_OFFSET = 4;
    static NUM_PTRS_OFFSET = 8;
    static MARKED_BIT_OFFSET = 12;
    static NEXT_PTR_OFFSET = 16;
    static BLOCK_HEADER_SIZE = 16;
    constructor(ptr, memory) {
        this.ptr = ptr;
        this.memory = memory;
    }

    get_64(offset) {
        return this.memory.getFloat64(this.ptr + offset);
    }

    get_32(offset) {
        return this.memory.getUint32(this.ptr + offset);
    }

    set_64(offset, value) {
        return this.memory.setFloat64(this.ptr + offset, value);
    }

    set_32(offset, value) {
        return this.memory.setUint32(this.ptr + offset, value);
    }

    get_address() {
        return this.ptr;
    }

    get_num_ptrs() {
        return this.memory.getUint32(this.ptr - Block.NUM_PTRS_OFFSET);
    }

    get_size() {
        return this.memory.getUint32(this.ptr - Block.SIZE_OFFSET);
    }

    get_marked() {
        return this.memory.getUint32(this.ptr - Block.MARKED_BIT_OFFSET);
    }

    get_next_address() {
        return this.memory.getUint32(this.ptr - Block.NEXT_PTR_OFFSET);
    }

    set_next_address(ptr) {
        this.memory.setUint32(this.ptr - Block.NEXT_PTR_OFFSET, ptr);
    }

    set_marked(marked) {
        this.memory.setUint32(this.ptr - Block.MARKED_BIT_OFFSET, marked);
    }

    set_num_ptrs(num_ptrs) {
        this.memory.setUint32(this.ptr - Block.NUM_PTRS_OFFSET, num_ptrs);
    }

    set_size(byte_size) {
        this.memory.setUint32(this.ptr - Block.SIZE_OFFSET, byte_size);
    }
}

class Memory {
    static NULL = 0; // 4 bytes
    static FREE_LIST_HEAD_PTR = 4; // 4 bytes
    static ALLOCATED_LIST_HEAD_PTR = 8; // 4 bytes
    constructor(memory) {
        memory.setUint32(Memory.NULL, Memory.NULL);
        const first_initial_free = Memory.ALLOCATED_LIST_HEAD_PTR + 4 + Block.BLOCK_HEADER_SIZE;
        memory.setUint32(Memory.FREE_LIST_HEAD_PTR, first_initial_free);
        memory.setUint32(Memory.ALLOCATED_LIST_HEAD_PTR, Memory.NULL);

        const first_block = new Block(first_initial_free, memory);
        first_block.set_next_address(Memory.NULL);
        first_block.set_marked(0);
        first_block.set_size(memory.byteLength);
        first_block.set_num_ptrs(0);

        this.memory = memory;
    }

    get(ptr) {
        return new Block(ptr, this.memory);
    }

    print_free_list() {
        const free_list_head_ptr = this.get(Memory.FREE_LIST_HEAD_PTR).get_32(0);
        let node = this.get(free_list_head_ptr);
        while (node.get_address() != Memory.NULL) {
            node = this.get(node.get_next_address());
        }
    }

    print_allocated_list() {
        const allocated_list_head_ptr = this.get(Memory.ALLOCATED_LIST_HEAD_PTR).get_32(0);
        let node = this.get(allocated_list_head_ptr);
        while (node.get_address() != Memory.NULL) {
            node = this.get(node.get_next_address());
        }
    }

    alloc(byte_size, num_ptrs) {
        // find the first fitting free node in the free list
        // if its too big, split it
        // then add that ptr to the allocated list
        // finally, return the ptr to the user
        this.print_allocated_list()
        this.print_free_list()

        const free_list_head_ptr = this.get(Memory.FREE_LIST_HEAD_PTR).get_32(0);
        let node = this.get(free_list_head_ptr);
        let prev;
        while (node.get_address() != Memory.NULL) {
            if (node.get_size() < byte_size + Block.BLOCK_HEADER_SIZE) {
                prev = node;
                node = this.get(node.get_next_address());
            } else {
                if (node.get_size() >= byte_size + Block.BLOCK_HEADER_SIZE * 2) {
                    // split block
                    const current_size = node.get_size();
                    const new_block_ptr = node.get_address() + byte_size + Block.BLOCK_HEADER_SIZE;
                    const new_block = this.get(new_block_ptr);

                    new_block.set_size(current_size - byte_size + Block.BLOCK_HEADER_SIZE);
                    new_block.set_next_address(node.get_next_address());
                    new_block.set_marked(0);
                    new_block.set_num_ptrs(0);

                    node.set_size(byte_size + Block.BLOCK_HEADER_SIZE);
                    node.set_next_address(new_block.get_address());
                }

                if (prev) {
                    prev.set_next_address(node.get_next_address());
                } else {
                    this.memory.setUint32(Memory.FREE_LIST_HEAD_PTR, node.get_next_address());
                }

                node.set_next_address(this.get(Memory.ALLOCATED_LIST_HEAD_PTR).get_32(0));
                this.memory.setUint32(Memory.ALLOCATED_LIST_HEAD_PTR, node.get_address());
                node.set_num_ptrs(num_ptrs);

                this.print_allocated_list();
                this.print_free_list();
                return node.get_address();
            }
        }

        throw Error("Could not find block big enough");
    }

    free(ptr) {
        // search through allocated list to find the ptr
        // remove the ptr from the list and add it to the free list

        const allocated_list_head_ptr = this.get(Memory.ALLOCATED_LIST_HEAD_PTR).get_32(0);
        let node = this.get(allocated_list_head_ptr);
        let prev;
        while (node.get_address() != Memory.NULL) {
            if (ptr == node.get_address()) {
                if (prev) {
                    prev.set_next_address(node.get_next_address());
                } else {
                    this.memory.setUint32(Memory.ALLOCATED_LIST_HEAD_PTR, node.get_next_address());
                }

                const free_list_head_ptr = this.get(Memory.FREE_LIST_HEAD_PTR).get_32(0);
                this.memory.setUint32(Memory.FREE_LIST_HEAD_PTR, node.get_address());
                node.set_next_address(free_list_head_ptr);
                return;
            } else {
                prev = node;
                node = this.get(node.get_next_address());
            }
        }
    }

    mark(ptr) {
        const stack = [];
        stack.push(this.get(ptr));
        while (stack.length > 0) {
            const block = stack.pop();

            if (block.get_address() === 0) {
                continue;
            }

            if (block.get_marked()) {
                continue;
            }

            block.set_marked(1);
            const num_ptrs = block.get_num_ptrs();
            for (let i = 0; i < num_ptrs; i++) {
                const child = this.get(block.get_32(i * 4));
                stack.push(child);
            }
        }
    }

    sweep() {
        // go through allocated list
        // if block isn't marked, free it
        // this.print_free_list();
        // this.print_allocated_list();
        const allocated_list_head_ptr = this.get(Memory.ALLOCATED_LIST_HEAD_PTR).get_32(0);
        let node = this.get(allocated_list_head_ptr);
        let prev;

        while (node.get_address() != Memory.NULL) {
            if (!node.get_marked()) {
                if (prev) {
                    prev.set_next_address(node.get_next_address());
                    node.set_next_address(this.get(Memory.FREE_LIST_HEAD_PTR).get_32(0));
                    this.memory.setUint32(Memory.FREE_LIST_HEAD_PTR, node.get_address());
                    node = this.get(prev.get_next_address());
                } else {
                    const next_ptr = node.get_next_address();
                    node.set_next_address(this.get(Memory.FREE_LIST_HEAD_PTR).get_32(0));
                    this.memory.setUint32(Memory.FREE_LIST_HEAD_PTR, node.get_address());

                    node = this.get(next_ptr);
                    this.memory.setUint32(Memory.ALLOCATED_LIST_HEAD_PTR, node.get_address());
                }
            } else {
                prev = node;
                node.set_marked(0);
                node = this.get(node.get_next_address());
            }

        }

        this.print_free_list();
        this.print_allocated_list();
    }
}


let instance;
let memory;
const printer = new Printer();
let mem;

const moduleOptions = {
    env: {
        push_ptr,
        push_32,
        push_64,
        strcat,
        strcmp,
        print_i32: value => printer.print_i32(value),
        print_f64: value => printer.print_f64(value),
        print_bool: value => printer.print_bool(value),
        print_str: ptr => printer.print_str(ptr),
        print_endline: () => printer.print_endline(),
        mem_get_32: (ptr, offset) => mem.get(ptr).get_32(offset),
        mem_get_64: (ptr, offset) => mem.get(ptr).get_64(offset),
        mem_set_32: (ptr, offset, value) => mem.get(ptr).set_32(offset, value),
        mem_set_64: (ptr, offset, value) => mem.get(ptr).set_64(offset, value),
        mem_alloc: (ptr, num_ptrs) => mem.alloc(ptr, num_ptrs),
        mark: ptr => mem.mark(ptr),
        sweep: () => mem.sweep(),
    }
};

const result = fs.readFileSync("output.wasm");
WebAssembly.instantiate(result, moduleOptions).then((wasmInstatiatedSource) => {
    instance = wasmInstatiatedSource.instance;
    memory = new DataView(instance.exports.memory.buffer);
    wasmMemory = instance.exports.memory;
    mem = new Memory(memory);
    instance.exports.main();

});

function push_ptr(arr_ptr, value) { }
function push_32(arr_ptr, value) { }
function push_64(arr_ptr, value) { }
function strcat(left, right) { }
function strcmp(left, right) { }

