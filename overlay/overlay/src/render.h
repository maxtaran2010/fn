#include "main.h"
#include <iostream>
#include <conio.h>
#include <GLFW/glfw3.h>
#include "globals.h"



// Function to draw an unfilled box by its center, width, and height
void drawBox(float centerX, float centerY, float width, float height) {
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;

    float left = centerX - halfWidth;
    float right = centerX + halfWidth;
    float top = centerY - halfHeight;
    float bottom = centerY + halfHeight;

    glBegin(GL_LINE_LOOP);
    glVertex2f(left, top);    // Top-left corner
    glVertex2f(right, top);   // Top-right corner
    glVertex2f(right, bottom);// Bottom-right corner
    glVertex2f(left, bottom); // Bottom-left corner
    glEnd();
}


// GLOBAL VARIABLES
int app_width = 1920;
int app_height = 1080;

// RUN
void render(void)
{
    if (!glfwInit()) { exit(EXIT_FAILURE); }

    glfwWindowHint(GLFW_DECORATED, 0);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);
    glfwWindowHint(GLFW_AUTO_ICONIFY, GLFW_FALSE);
    glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, GLFW_TRUE);

    GLFWmonitor* monitor = glfwGetPrimaryMonitor();
    GLFWwindow* window = glfwCreateWindow(1920, 1080, "GLFW Test", monitor, NULL);

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwShowWindow(window);
    glfwGetFramebufferSize(window, &app_width, &app_height);
    glViewport(0, 0, app_width, app_height);
    glLoadIdentity();
    glOrtho(0, 1920, 1080, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

    while (!glfwWindowShouldClose(window))
    {   
        
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        

        for (auto player : globals::Players) {
            glLineWidth((GLfloat)1);

            if (player.isVisible) {
                glColor3f(1.0f, 0.0f, 0.0f);
            }
            else {
                    glColor3f(0.0f, 1.0f, 0.0f);
            }
            
            glBegin(GL_LINES);
            glVertex2f(player.neck.x, player.neck.y);
            glVertex2f(player.chest.x, player.chest.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.left_shoulder.x, player.left_shoulder.y);
            glVertex2f(player.chest.x, player.chest.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.right_shoulder.x, player.right_shoulder.y);
            glVertex2f(player.chest.x, player.chest.y);
            glEnd();


            glBegin(GL_LINES);
            glVertex2f(player.left_shoulder.x, player.left_shoulder.y);
            glVertex2f(player.left_elbow.x, player.left_elbow.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.right_shoulder.x, player.right_shoulder.y);
            glVertex2f(player.right_elbow.x, player.right_elbow.y);
            glEnd();


            glBegin(GL_LINES);
            glVertex2f(player.left_hand.x, player.left_hand.y);
            glVertex2f(player.left_elbow.x, player.left_elbow.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.right_hand.x, player.right_hand.y);
            glVertex2f(player.right_elbow.x, player.right_elbow.y);
            glEnd();

            glBegin(GL_LINES);
            glVertex2f(player.pelvis.x, player.pelvis.y);
            glVertex2f(player.chest.x, player.chest.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.pelvis.x, player.pelvis.y);
            glVertex2f(player.right_hip.x, player.right_hip.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.pelvis.x, player.pelvis.y);
            glVertex2f(player.left_hip.x, player.left_hip.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.left_knee.x, player.left_knee.y);
            glVertex2f(player.left_hip.x, player.left_hip.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.right_knee.x, player.right_knee.y);
            glVertex2f(player.right_hip.x, player.right_hip.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.right_knee.x, player.right_knee.y);
            glVertex2f(player.right_foot.x, player.right_foot.y);
            glEnd();
            glBegin(GL_LINES);
            glVertex2f(player.left_knee.x, player.left_knee.y);
            glVertex2f(player.left_foot.x, player.left_foot.y);
            glEnd();
            glLineWidth((GLfloat)1);
            glColor3f(1.0f, player.distance/15000.0f, player.distance / 15000.0f);
            glBegin(GL_LINES);
            glVertex2f(player.neck.x, player.neck.y);
            glVertex2f(1920/2, 1080/4*3);
            glEnd();
            glLineWidth((GLfloat)2);
            glColor3f(0.5f, 1.0f, 0.3f);
            drawBox(player.pelvis.x, player.pelvis.y+ (15000 / player.distance), (15000/player.distance) * 7, 15000 / player.distance * 16);
        }
        // TO DO
        glColor3f(0.87f, 0.0f, 1.0f);
        glLineWidth((GLfloat)3);
        glBegin(GL_LINES);
        glVertex2f(1920/2-15, 1080/2+14);
        glVertex2f(1920 / 2, 1080 / 2);
        glEnd();
        glBegin(GL_LINES);
        glVertex2f(1920 / 2 + 15, 1080 / 2+14);
        glVertex2f(1920 / 2, 1080 / 2);
        glEnd();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    exit(EXIT_SUCCESS);
}
