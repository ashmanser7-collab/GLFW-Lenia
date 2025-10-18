#include <GLFW/glfw3.h>
#include <cmath>
#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>

int simulation_size = 64;
const int resolution = 16;
int screen_size = simulation_size*resolution;
const int growth_accuracy = 10000;
const double PI = 3.14159265358979323846;
bool random = true;




GLFWwindow* initiate_window() {
    GLFWwindow* window = glfwCreateWindow(screen_size, screen_size, "Lenia", NULL, NULL);
    return window;
}






std::vector<std::vector<float>> computeDistances(int radius) {
    std::vector<std::vector<float>> distances(2*radius+1, std::vector<float>(2*radius+1));
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            distances[y+radius][x+radius] = hypot(x, y);
        }
    }
    return distances;
}

std::vector<std::vector<bool>> determineArea(const std::vector<std::vector<float>>& distances, int radius) {
    std::vector<std::vector<bool>> rad(2*radius+1, std::vector<bool>(2*radius+1));
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            rad[y+radius][x+radius] = floor(distances[y+radius][x+radius]) > radius;
        }
    }
    return rad;
}

float kernelFunction(float mu, float c, float u) {
    return exp(-c * pow(1-(1-mu)*u*u, 3));
}

std::vector<std::vector<float>> computeKernel(int radius, float mu, float c, const std::vector<std::vector<float>>& distances) {
    std::vector<std::vector<float>> kernel(2*radius+1, std::vector<float>(2*radius+1));
    std::vector<std::vector<bool>> area = determineArea(distances, radius);
    float sum = 0;
    for (int y = -radius; y <= radius; y++) {
        for (int x = -radius; x <= radius; x++) {
            if (area[y+radius][x+radius]) {continue;}
            kernel[y+radius][x+radius] = kernelFunction(mu, c, distances[y+radius][x+radius] / radius);
            sum += kernel[y+radius][x+radius];
        }
    }
    for (int x = -radius; x <= radius; x++) {
        for (int y = -radius; y <= radius; y++) {
            kernel[y+radius][x+radius] /= sum;
        }
    }
    return kernel;
}

float growthFunction(float mu, float c, float A) {
    return cos(2*PI*((A - mu) / (2 * c)));
}

std::vector<float> computeGrowth(int radius, float mu, float c) {
    std::vector<float> growth(growth_accuracy+1);
    for (int i = 0; i <= growth_accuracy; i++) {
        growth[i] = growthFunction(mu, c, float(i)/growth_accuracy);
    }
    return growth;
}






struct Genome {
    float growth_mu;
    float growth_coeficient;
    float kernel_mu;
    float kernel_coeficient;
    int radius;
    float dt = 0.1f;
    Genome(float gm, float gc, float km, float kc, int r) : growth_mu(gm), growth_coeficient(gc), kernel_mu(km), kernel_coeficient(kc), radius(r) {}
};

class Simulation {
public:
    Genome values;
    std::vector<std::vector<bool>> area;
    std::vector<std::vector<float>> distances;
    std::vector<std::vector<float>> kernel;
    std::vector<float> growth;
    std::vector<std::vector<float>> grid;
    std::vector<std::vector<float>> new_grid;
    float fitness = 0;

    Simulation(float gm, float gc, float km, float kc, int r) : values(gm, gc, km, kc, r) {
        distances = computeDistances(values.radius);
        kernel = computeKernel(values.radius, values.kernel_mu, values.kernel_coeficient, distances);
        growth = computeGrowth(values.radius, values.growth_mu, values.growth_coeficient);
        area = determineArea(distances, values.radius);

        grid = std::vector<std::vector<float>>(simulation_size, std::vector<float>(simulation_size, 0.0f));
        new_grid = std::vector<std::vector<float>>(simulation_size, std::vector<float>(simulation_size, 0.0f));
    }


    void step() {
        for (int y = 0; y < simulation_size; y++) {
            for (int x = 0; x < simulation_size; x++) {
                stepCell(x, y);
            }
        }
        grid.swap(new_grid);
    }
    void stepCell(int x, int y) {
        float potential = 0;
        int tally = 0;
        for (int yi = -values.radius; yi <= values.radius; yi++) {
            int yindex = (y+yi+simulation_size)%simulation_size;
            for (int xi = -values.radius; xi <= values.radius; xi++) {
                int xindex = (x+xi+simulation_size)%simulation_size;
                if (area[yi+values.radius][xi+values.radius]) {continue;}
                tally++;
                potential += kernel[yi+values.radius][xi+values.radius] * grid[yindex][xindex];
            }
        }
        //potential /= tally;
        float G = growth[int(floor(potential*growth_accuracy))];
        new_grid[y][x] = G * values.dt + grid[y][x];
        new_grid[y][x] = std::clamp(new_grid[y][x], 0.0f, 1.0f);
    }


    void renderGrid() {
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        glBegin(GL_POINTS);
        for (int y = 0; y < simulation_size; y += 1) {
            for (int x = 0; x < simulation_size; x += 1) {
                float value = grid[y][x];
                //if (value < 0.05f) {
                //    glColor3f(0.0f, 0.0f, 0.0f);
                //} else {
                //    glColor3f(0.3, 0.8-value*0.6, 0.2+value*0.6);
                //}
                glColor4f(value, value, value, 0.2f);
                glVertex2f(x*resolution, y*resolution);
            }
        }
        glEnd();
    }
};






std::vector<std::vector<float>> splotch(std::vector<std::vector<float>> grid, int radius, int x, int y) {
    std::vector<std::vector<float>> distances = computeDistances(radius);
    std::vector<std::vector<bool>> area = determineArea(distances, radius);
    int cx = x % simulation_size;
    int cy = y % simulation_size;
    for (int y = -radius; y <= radius; ++y) {
        int ny = (y+cy+simulation_size)%simulation_size;
        for (int x = -radius; x <= radius; ++x) {
            int nx = (x+cx+simulation_size)%simulation_size;
            if (!area[y+radius][x+radius]) {
                grid[ny][nx] = exp(-pow(distances[y+radius][x+radius]/radius, 2));
            }
        }
    }
    return grid;
}

std::vector<std::vector<float>> fill(std::vector<std::vector<float>> grid, float value) {
    for (int i = 0; i < simulation_size; i++) {
        for (int j = 0; j < simulation_size; j++) {
            grid[i][j] = value;
        }
    }
    return grid;
}

std::vector<std::vector<float>> fillRandom(std::vector<std::vector<float>> grid, float min, float max) {
    for (int i = 0; i < simulation_size; i++) {
        for (int j = 0; j < simulation_size; j++) {
            grid[i][j] = (rand() % int((max-min)*10000))/10000 + min;
        }
    }
    return grid;
}







int main() {

    if (random) {
        srand(std::time(0));
    }

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

    Simulation sim = Simulation(0.15f, 0.017f, 0.35f, 3.0f, 26); // float gm, float gc, float km, float kc, int r
    sim.grid = fillRandom(sim.grid, 0.3f, 0.8f);

    while (!glfwWindowShouldClose(window)) {
        sim.step();
        sim.renderGrid();
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}