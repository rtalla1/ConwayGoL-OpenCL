#include <CL/opencl.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <cstdlib>
#include <unistd.h>
#include <cstring>
#include <string>
#include <cctype>
#include <sys/ioctl.h>
#include <csignal>

// cursor visibility control for pretty output
static void hideCursor()    { std::cout << "\033[?25l"; }
static void showCursor()    { std::cout << "\033[?25h"; }
static void moveCursorTop() { std::cout << "\033[H"; }

// flag and handler for clean shutdown
static volatile sig_atomic_t stopFlag = 0;
static void handle_sigint(int) {
    stopFlag = 1;
}

int main(int argc, char** argv) {
    // load conway source
    std::ifstream in("./conway.cl");
    if (!in.is_open()) {
        std::cerr << "Error: could not open conway.cl\n";
        return 1;
    }
    std::stringstream ss;
    ss << in.rdbuf();
    std::string src_str = ss.str();
    const char* kernel_src = src_str.c_str();

    // setup OpenCL
    cl_int err;
    cl_platform_id platform;
    cl_device_id device;
    cl_context context;
    cl_command_queue queue;
    cl_program program;
    cl_kernel kernel;
    cl_mem buf_in, buf_out;

    // get platform and device
    cl_uint numPlatforms = 0;
    err = clGetPlatformIDs(0, nullptr, &numPlatforms);
    if (err != CL_SUCCESS || numPlatforms == 0) {
        std::cerr << "Error: No OpenCL platforms found\n";
        return 1;
    }
    std::vector<cl_platform_id> platforms(numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms.data(), nullptr);
    platform = platforms[0];

    cl_uint numDevices = 0;
    err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 0, nullptr, &numDevices);
    if (err != CL_SUCCESS || numDevices == 0) {
        std::cerr << "Error: No GPU devices found on platform\n";
        return 1;
    }
    std::vector<cl_device_id> devices(numDevices);
    clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numDevices, devices.data(), nullptr);
    device = devices[0];

    // create context and queue
    context = clCreateContext(nullptr, 1, &device, nullptr, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: failed to create context\n";
        return 1;
    }
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: failed to create queue\n";
        return 1;
    }

    // create program and kernel
    program = clCreateProgramWithSource(context, 1, &kernel_src, nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: clCreateProgramWithSource failed\n";
        return 1;
    }
    err = clBuildProgram(program, 1, &device, nullptr, nullptr, nullptr);
    if (err != CL_SUCCESS) {
        size_t logSize;
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, nullptr, &logSize);
        std::vector<char> log(logSize);
        clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize, log.data(), nullptr);
        std::cerr << "Build log:\n" << log.data() << "\n";
        return 1;
    }
    kernel = clCreateKernel(program, "next_conway", &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: failed to create kernel\n";
        return 1;
    }

    // use args for map, number of generations, and delay
    if (argc < 4) {
        std::cerr << "Usage: " << argv[0] << " <map_file> <num_generations> <delay_microseconds>\n";
        return 1;
    }
    std::ifstream mapIn(argv[1]);
    if (!mapIn) {
        std::cerr << "Error: could not open map file " << argv[1] << "\n";
        return 1;
    }
    int generations = std::atoi(argv[2]);
    int delay = std::atoi(argv[3]);
    if (delay <= 0) {
        std::cerr << "Error: delay_microseconds must be > 0\n";
        return 1;
    }
    if (generations <= 0) {
        std::cerr << "Error: num_generations must be > 0\n";
        return 1;
    }

    // install Ctrl+C handler
    std::signal(SIGINT, handle_sigint);

    // read initial map
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(mapIn, line)) {
        size_t end = line.find_last_not_of(" \t\r\n");
        if (end != std::string::npos) {
            line = line.substr(0, end + 1);
        } else {
            line.clear();
        }
        if (!line.empty()) {
            lines.push_back(line);
        }
    }
    const int height = lines.size();
    const int width  = lines[0].size();
    int cellWidth = width * 2;
    const int numCells = width * height;
    std::vector<unsigned char> inGrid(numCells), outGrid(numCells);

    // precompute horizontal border
    std::string horiz;
    horiz.reserve(cellWidth * 3);
    for (int i = 0; i < cellWidth; ++i) {
        horiz += u8"─";
    }

    for (int y = 0; y < height; ++y) {
        if ((int)lines[y].size() != width) {
            std::cerr << "Error: inconsistent row length in map\n";
            return 1;
        }
        for (int x = 0; x < width; ++x) {
            inGrid[y*width + x] = (lines[y][x] != '.') ? 1 : 0;
        }
    }

    // create buffers
    // avoid clEnqueueWriteBuffer with CL_MEM_COPY_HOST_PTR
    buf_in  = clCreateBuffer(context,
                             CL_MEM_READ_ONLY  | CL_MEM_COPY_HOST_PTR,
                             numCells * sizeof(unsigned char),
                             inGrid.data(), &err);
    buf_out = clCreateBuffer(context,
                             CL_MEM_WRITE_ONLY,
                             numCells * sizeof(unsigned char),
                             nullptr, &err);
    if (err != CL_SUCCESS) {
        std::cerr << "Error: buffer creation failed\n";
        return 1;
    }

    clSetKernelArg(kernel, 0, sizeof(buf_in), &buf_in);
    clSetKernelArg(kernel, 1, sizeof(buf_out), &buf_out);
    clSetKernelArg(kernel, 2, sizeof(int), &width);
    clSetKernelArg(kernel, 3, sizeof(int), &height);

    size_t global[2] = { (size_t)width, (size_t)height };

    // get terminal size for centering
    struct winsize ws;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws);
    int term_cols = ws.ws_col;
    int term_rows = ws.ws_row;

    // compute horizontal and vertical padding
    int content_height = height + 4;
    int topPad = (term_rows > content_height) ? (term_rows - content_height) / 2 : 0;
    int leftPad = (term_cols > cellWidth + 2) ? (term_cols - (cellWidth + 2)) / 2 : 0;

    // clear screen and apply vertical padding
    std::cout << "\033[2J\033[0;0H";
    for (int i = 0; i < topPad; ++i) std::cout << "\n";
    hideCursor();

    // run kernels for evolution
    for (int gen = 1; gen <= generations; ++gen) {
        err = clEnqueueNDRangeKernel(queue, kernel, 2, nullptr, global, nullptr, 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Error: kernel enqueue failed\n";
            break;
        }
        err = clEnqueueReadBuffer(queue, buf_out, CL_TRUE, 0,
                                  numCells * sizeof(unsigned char),
                                  outGrid.data(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Error: read buffer failed\n";
            break;
        }

        if (stopFlag) {
            break;
        }

        // draw UI
        std::cout << "\033[" << (topPad + 1) << ";1H";
        std::string header = "Generation " + std::to_string(gen);
        int header_pad = leftPad + ((cellWidth + 2) - static_cast<int>(header.length())) / 2;
        std::cout << std::string(header_pad, ' ') << header << "\n\n";

        std::cout << std::string(leftPad, ' ') << u8"┌" << horiz << u8"┐\n";
        for (int y = 0; y < height; ++y) {
            std::cout << std::string(leftPad, ' ') << u8"│";
            for (int x = 0; x < width; ++x) {
                if (outGrid[y*width + x]) {
                    std::cout << "\033[37m\u2588\033[0m" << "\033[37m\u2588\033[0m";
                } else {
                    std::cout << "  ";
                }
            }
            std::cout << "│\n";
        }
        std::cout << std::string(leftPad, ' ') << "└" << horiz << "┘\n";

        usleep(delay);

        // swap and write back into buf_in for next iteration
        std::swap(inGrid, outGrid);
        err = clEnqueueWriteBuffer(queue, buf_in, CL_TRUE, 0,
                                   numCells * sizeof(unsigned char),
                                   inGrid.data(), 0, nullptr, nullptr);
        if (err != CL_SUCCESS) {
            std::cerr << "Error: write buffer failed\n";
            break;
        }
    }

    // restore terminal state
    showCursor();

    // cleanup
    clReleaseMemObject(buf_in);
    clReleaseMemObject(buf_out);
    clReleaseKernel(kernel);
    clReleaseProgram(program);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}