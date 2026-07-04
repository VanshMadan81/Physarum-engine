#include <iostream>
#include <cmath>
#include <random>
#include "Engine.hpp" // Include our new graphics engine!

const int WIDTH = 900;
const int HEIGHT = 900;
const int NUM_AGENTS = 10000;
const float PI = 3.14159;
const float MAX_SPEED = 2.0f;
const float DECAY_FACTOR = 0.98f;
const float REPULSION = -10.0f;

bool validCoords(int x, int y) {
    return (x >= 0 && x < WIDTH && y >= 0 && y < HEIGHT);
}

// The Agent Struct
struct Agent
{
    float x, y;
    float angle;
    float speed;

    // Sensing Specifications. You need to modify these in order to change the simulation behaviour.
    float turnAngle = PI / 10.0f;
    float sensorAngleOffset = PI / 3.0f;
    float sensorDistance = 20.0f;

    void senseAndSteer(float *readGrid) {
        float sensorAngleLeft = angle + sensorAngleOffset;
        float sensorAngleFwd = angle;
        float sensorAngleRight = angle - sensorAngleOffset;

        int leftSensorX = static_cast<int>(x + sensorDistance * cos(sensorAngleLeft));
        int leftSensorY = static_cast<int>(y + sensorDistance * sin(sensorAngleLeft));

        int fwdSensorX = static_cast<int>(x + sensorDistance * cos(sensorAngleFwd));
        int fwdSensorY = static_cast<int>(y + sensorDistance * sin(sensorAngleFwd));

        int rightSensorX = static_cast<int>(x + sensorDistance * cos(sensorAngleRight));
        int rightSensorY = static_cast<int>(y + sensorDistance * sin(sensorAngleRight));

        float LeftSensorValue = (validCoords(leftSensorX, leftSensorY) ? readGrid[leftSensorY * WIDTH + leftSensorX] : REPULSION);
        float FwdSensorValue = (validCoords(fwdSensorX, fwdSensorY) ? readGrid[fwdSensorY * WIDTH + fwdSensorX] : REPULSION);
        float RightSensorValue = (validCoords(rightSensorX, rightSensorY) ? readGrid[rightSensorY * WIDTH + rightSensorX] : REPULSION);

        // Discrete Steering Logic
        if (FwdSensorValue > LeftSensorValue && FwdSensorValue > RightSensorValue) angle += 0.0f; 
        else if (FwdSensorValue < LeftSensorValue && FwdSensorValue < RightSensorValue) {
            if (rand() % 2 == 0) angle += turnAngle;
            else angle -= turnAngle;
        } 
        else if (LeftSensorValue > RightSensorValue) angle += turnAngle;
        else if (RightSensorValue > LeftSensorValue) angle -= turnAngle;
    }
};

// Deposit Pheromones at (x, y)
void deposit(float *grid, int x, int y, float value)
{
    if (validCoords(x, y)) grid[y * WIDTH + x] += value;
}

float getRandomFloat(float min, float max)
{
    float random = ((float)rand()) / (float)RAND_MAX;
    float diff = max - min;
    float r = random * diff;
    return min + r;
}

int main()
{
    // Boot up the Graphics Engine
    RenderEngine engine(WIDTH, HEIGHT);
    if (!engine.init())
    {
        std::cerr << "Failed to initialize graphics engine!" << std::endl;
        return -1;
    }

    float *readGrid = new float[WIDTH * HEIGHT]();  // First grid which we read to form the next grid
    float *writeGrid = new float[WIDTH * HEIGHT](); // Second grid which we write to
    Agent *agents = new Agent[NUM_AGENTS]();        // We can use an array of structs for the agents

    // TODO: Write a for-loop here to give every agent a random starting X, Y, and Angle.
    for (int i = 0; i < NUM_AGENTS; i++)
    {
        agents[i].x = getRandomFloat(0.0, (float)WIDTH);
        agents[i].y = getRandomFloat(0.0, (float)HEIGHT);

        agents[i].angle = getRandomFloat(0.0, 2.0 * PI);
        agents[i].speed = getRandomFloat(0.8f * MAX_SPEED, MAX_SPEED);
    }

    // The Main Simulation Loop (Runs 60 times a second)
    while (!engine.shouldClose())
    {
        // Step A: Diffuse (Blur) and Decay the Environment
        for (int y = 0; y < HEIGHT; y++) {
            for (int x = 0; x < WIDTH; x++) {

                // TODO: Write a nested 3x3 loop here to read the 8 neighbors AND the center pixel
                // from the `readGrid`. Add them all to `sum`.
                // Be careful to check for boundary edges so you don't read out of bounds!

                float sum = 0.0f;
                for (int offsetX = -1; offsetX <= 1; offsetX++) {
                    for (int offsetY = -1; offsetY <= 1; offsetY++) {
                        int X = x + offsetX;
                        int Y = y + offsetY;

                        if (validCoords(X, Y)) sum += readGrid[Y * WIDTH + X];
                    }
                }

                float averageBlur = sum / 9.0f;
                float finalValue = averageBlur * DECAY_FACTOR;

                writeGrid[y * WIDTH + x] = finalValue;
            }
        }

        // Step B: Update all agents.
        // Write their new positions to writeGrid
        for (int i = 0; i < NUM_AGENTS; ++i)
        {
            // TODO 1: Move the agent forward using trigonometry
            agents[i].x += cos(agents[i].angle) * agents[i].speed;
            agents[i].y += sin(agents[i].angle) * agents[i].speed;

            agents[i].senseAndSteer(readGrid);

            // TODO 2: Write boundary checks!
            if (!validCoords(agents[i].x, agents[i].y))
                continue;

            // Draw the agent to the 1D grid using your Week 1 function
            int gridX = static_cast<int>(agents[i].x);
            int gridY = static_cast<int>(agents[i].y);

            deposit(writeGrid, gridX, gridY, 1.0f); // 1.0f is White
        }

        // Step C: Send the raw pointer array to the GPU
        engine.renderFrame(writeGrid);

        // Step D: Swap the two grids so that the current frame becomes the previous frame for the next frame.
        float *temp = readGrid;
        readGrid = writeGrid;
        writeGrid = temp;
    }

    // 4. Shutdown gracefully and prevent memory leaks
    delete[] writeGrid;
    delete[] readGrid;
    delete[] agents;
    engine.shutdown();

    return 0;
}