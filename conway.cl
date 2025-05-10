// avoid 2D array, 1D memory more optimal
// using bits in uchar as cells led to race condition :(
// but I would like to explore bit packing again in the future
__kernel void next_conway(__global const uchar* curr,
                          __global uchar* next,
                          const int width,
                          const int height) {
    
    int x = get_global_id(0);
    int y = get_global_id(1);
    int idx = y * width + x;

    uchar curr_cell = curr[idx];

    int live_neighbors = 0;
    for (int dy = -1; dy <= 1; dy++) {
        for (int dx = -1; dx <= 1; dx++) {
            if (dx == 0 && dy == 0) continue;

            // wrap around
            int nx = (x + dx + width) % width;
            int ny = (y + dy + height) % height;

            int nidx = ny * width + nx;
            live_neighbors += curr[nidx];
        }
    }

    // rules
    uchar next_cell = 0;
    if (curr_cell == 1) {
        if (live_neighbors == 2 || live_neighbors == 3) {
            next_cell = 1;
        }
    } else {
        if (live_neighbors == 3) {
            next_cell = 1;
        }
    }

    // write next state
    next[idx] = next_cell;
}