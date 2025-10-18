#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#include  <thread>

const double PI = 3.14159265358979323846;
long frames = 0;
const int size = 256;
const int radius = 26;
float resolution = 4;     // Number of pixels per cell or something
int screen_size = size * resolution;
bool p_was_pressed = false;
bool random = true;

float kernel_coeficient = 4.5f;
float growth_coeficient = 0.017f;
float mu = 0.15f;
float dt = 0.1f;








// --- Maths ---
float grid[size][size] = {{0.0f}};
float new_grid[size][size] = {{0.0f}};

float kernel[2*radius+1][2*radius+1] = {{0.0f}};
float distances[2*radius+1][2*radius+1] = {{0.0f}};

void computeDistances(float dists[2*radius+1][2*radius+1]) {
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            if (x == 0 && y == 0) {
                dists[y+radius][x+radius] = 0;
                continue;
            }
            dists[y+radius][x+radius] = hypot(x, y);
        }
    }
}

float kernelFunction(float r) {
    float u = r/radius;
    return exp(pow((1-u*u), 3));
}

void computeKernel(float computed_kernel[2*radius+1][2*radius+1], float dists[2*radius+1][2*radius+1], bool area[2*radius+1][2*radius+1]) { // Use possesion to return a value
    float max = 0;
    float sum = 0;
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            if (area[y+radius][x+radius]) {continue;}
            computed_kernel[y+radius][x+radius] = kernelFunction(dists[y+radius][x+radius]);
            if (max < computed_kernel[y+radius][x+radius]) {
                max = computed_kernel[y+radius][x+radius];
            }
            sum += computed_kernel[y+radius][x+radius];
        }
    }
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            computed_kernel[y+radius][x+radius] /= sum;
        }
    }
}

const int growth_accuracy = 10000;
float growth[growth_accuracy];

float growthFunction(float A) {
    return cos(2*PI*((A - mu) / (2 * growth_coeficient)));
}

void computeGrowth(float grwth[growth_accuracy]) {
    for (int i = 0; i < growth_accuracy; i++) {
        float u = float(i)/growth_accuracy;
        grwth[i] = growthFunction(u);
    }
}

GLFWwindow* initiate_window() {
    GLFWwindow* window = glfwCreateWindow(screen_size, screen_size, "Lenia", NULL, NULL);
    return window;
}

bool area[2*radius+1][2*radius+1];

void computeRadius(bool rad[radius*2+1][radius*2+1]) {
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            rad[y+radius][x+radius] = floor(distances[y+radius][x+radius]) > radius;
        }
    }
}

bool mouse_pressed = false;

void draw(GLFWwindow* window, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
        mouse_pressed = true;
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        mouse_pressed = false;
    }
}








// --- Other & logic ---

void renderGrid(float grid[size][size], int resolution) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBegin(GL_POINTS);
    for (int y = 0; y < size; y += 1) {
        for (int x = 0; x < size; x += 1) {
            float value = grid[y][x];
            //if (value < -0.05f) {
            //    glColor4f(0.0f, 0.0f, 0.0f, 0.1f);
            //} else {
            //    glColor4f(0.3, 0.8-value*0.6, 0.2+value*0.6, 1.0f);
            //}
            glColor4f(value, value, value, 0.2f);
            glVertex2f(x*resolution, y*resolution);
        }
    }
    glEnd();
}

void pause(float* mu, float* growth_coeficient, float* kernel_coeficient, float growth[radius], float kernel[2*radius+1][2*radius+1], float distanes[2*radius+1][2*radius+1], float grid[size][size], GLFWwindow *window) {
    glfwSetMouseButtonCallback(window, draw);
    glfwSwapBuffers(window);
    glfwPollEvents();
    while (!glfwWindowShouldClose(window)) {
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {if (!p_was_pressed) {p_was_pressed = true; break;}} else {p_was_pressed = false;}

        renderGrid(grid, resolution);

        if (mouse_pressed) {
            double xpos;
            double ypos;
            glfwGetCursorPos(window, &xpos, &ypos);
            ypos = screen_size - ypos;
            int cx = int(xpos/resolution);
            int cy = int(ypos/resolution);
            for (int y = -radius; y <= radius; ++y) {
                int ny = (y+cy+size)%size;
                for (int x = -radius; x <= radius; ++x) {
                    int nx = (x+cx+size)%size;
                    if (!area[y+radius][x+radius]) {
                        grid[ny][nx] += sqrt(1.0f-floor(distances[y+radius][x+radius])/radius)*0.001;
                        grid[ny][nx] = std::clamp(grid[ny][nx], 0.0f, 1.0f);
                    }
                }
            }
        }
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
    glfwSetMouseButtonCallback(window, NULL);
    
    computeDistances(distances);
    computeRadius(area);
    computeKernel(kernel, distances, area);
    computeGrowth(growth);
}

void step(float new_grid[size][size], float grid[size][size], int x, int y) {
    float value = 0;
    int tally = 0;
    for (int yi=-radius; yi <= radius; yi++) {
        int yindex = (y+yi+size)%size;
        if (yindex < 0 || yindex >= size) {
            continue;
        } else {
            for (int xi=-radius; xi <= radius; xi++) {
                int xindex = (x+xi+size)%size;
                if (yindex < 0 || yindex >= size || area[yi+radius][xi+radius]) {
                    continue;
                } else {
                    tally += 1;
                    value += kernel[yi+radius][xi+radius] * grid[yindex][xindex];
                }
            }
        }
    }
    value /= tally;
    float G = growth[int(ceil(value*growth_accuracy)-1)];
    new_grid[y][x] = G*dt + grid[y][x];
    new_grid[y][x] = std::clamp(new_grid[y][x], 0.0f, 1.0f);
}

void stepAll(float grid[size][size], float new_grid[size][size]) {
    for (int y = 0; y < size; y++) {
        for (int x = 0; x < size; x++) {
            step(new_grid, grid, x, y);
        }
    }
}

void threadStep(float new_grid[size][size], float grid[size][size], int starty, int endy) {
    for (int y = starty; y < endy; y++) {
        for (int x = 0; x < size; x++) {
            float value = 0;
            int tally = 0;
            for (int yi=-radius; yi <= radius; yi++) {
                int yindex = (y+yi+size)%size;
                if (yindex < 0 || yindex >= size) {
                    continue;
                } else {
                    for (int xi=-radius; xi <= radius; xi++) {
                        int xindex = (x+xi+size)%size;
                        if (yindex < 0 || yindex >= size || area[yi+radius][xi+radius]) {
                            continue;
                        } else {
                            tally += 1;
                            value += kernel[yi+radius][xi+radius] * grid[yindex][xindex];
                        }
                    }
                }
            }
            //value /= tally;
            float G = growth[int(ceil(value*growth_accuracy)-1)];
            new_grid[y][x] = G*dt + grid[y][x];
            new_grid[y][x] = std::clamp(new_grid[y][x], 0.0f, 1.0f);
        }
    }
}

void threadStepAll(float grid[size][size], float new_grid[size][size]) {
    int num_threads = 14;
    int base = size / num_threads;
    int extra = size % num_threads;
    std::thread threads[num_threads];

    int start = 0;
    for (int t = 0; t < num_threads; ++t) {
        int rows = base + (t < extra ? 1 : 0); // distribute remainder
        int end = start + rows;
        threads[t] = std::thread(threadStep, new_grid, grid, start, end);
        start = end;
    }
    for (auto &th : threads) th.join();
}














// --- Presets ---
void splotch(float grid[size][size], int radius, int x, int y) {
    int cx = x % size;
    int cy = y % size;
    for (int y = -radius; y <= radius; ++y) {
        int ny = (y+cy+size)%size;
        for (int x = -radius; x <= radius; ++x) {
            int nx = (x+cx+size)%size;
            if (!area[y+radius][x+radius]) {
                grid[ny][nx] = exp(-pow(distances[y+radius][x+radius], -10));
            }
        }
    }
}

void randomSplotches(float grid[size][size], int radius, int num) {
    for (int i = 0; i < num; ++i) {
        int cx = rand() % size;
        int cy = rand() % size;
        splotch(grid, radius, cx, cy);
    }
}

void fill(float grid[size][size], float value) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid[i][j] = value;
        }
    }
}

void fillRandom(float grid[size][size], float min, float max) {
    for (int i = 0; i < size; i++) {
        for (int j = 0; j < size; j++) {
            grid[i][j] = (rand() % int((max-min)*10000))/10000 + min;
        }
    }
}
















// --- Main ---
int main() {

    if (random) {
        srand(std::time(0));
    }
    
    computeDistances(distances);
    computeRadius(area);
    computeKernel(kernel, distances, area);
    computeGrowth(growth);

    randomSplotches(grid, radius, 5);

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }

    GLFWwindow* window = initiate_window();

    glfwMakeContextCurrent(window);

    glViewport(0, 0, screen_size, screen_size);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, screen_size, 0, screen_size, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glPointSize(resolution);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glfwSetMouseButtonCallback(window, NULL);
    glfwSetWindowPos(window, 100, 75);

    renderGrid(grid, resolution);
    glfwSwapBuffers(window);
    glfwPollEvents();

    while (!glfwWindowShouldClose(window))
    {

        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) {if (!p_was_pressed) {p_was_pressed = true; pause(&mu, &growth_coeficient, &kernel_coeficient, growth, kernel, distances, grid, window);}} else {p_was_pressed = false;}

        frames += 1;

        threadStepAll(grid, new_grid);
        
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                grid[y][x] = new_grid[y][x];
            }
        }

        renderGrid(grid, resolution);

        std::cout << frames << std::endl;

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}



// TODO:
// 1. More mouse drawing stuff  
// 2. Live tweaking
// 3. Save states
// 4. Parameter and kernel presets