#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <math.h>


const float theta_step = 0.03;
const float pi_step    = 0.01;

const int screen_width  = 50;
const int screen_height = 50;

const float R_1 = 1;
const float R_2 = 1.7;
const float K_2 = 180;

const float K_1 = screen_width*K_2*3/(8*(R_1+R_2));

void render_frame(float A, float B) {
    // rotation about the x-axis by A and a rotation about the z-axis by B
    float cos_A = cos(A), sin_A = sin(A);
    float cos_B = cos(B), sin_B = sin(B);

    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    const int terminal_width = w.ws_col;
    const int terminal_height = w.ws_row;

    float zbuffer[screen_width][screen_height];
    char output[screen_width][screen_height];
    for(size_t i=0; i<screen_width; i++) {
        for(size_t j=0; j<screen_height; j++) {
            output[i][j]  = ' ';
            zbuffer[i][j] = 0;
        }
    }

    for (float theta=0; theta < 2*M_PI; theta += theta_step) {
       float cos_theta = cos(theta); float sin_theta = sin(theta);

       for (float pi=0; pi < 2*M_PI; pi += pi_step) {
           float cos_pi = cos(pi); float sin_pi = sin(pi);
           // original 3-d coordinates
           float circle_x = R_2 + R_1*cos_theta;
           float circle_y = R_1*sin_theta;
           // final 3-d coordinates after rotation
           float x = circle_x*(cos_B*cos_pi + sin_A*sin_B*sin_pi) - circle_y*cos_A*sin_B;
           float y = circle_x*(sin_B*cos_pi - sin_A*cos_B*sin_pi) + circle_y*cos_A*cos_B;
           float z = K_2 + cos_A*circle_x*sin_pi + circle_y*sin_A;
           float ooz = 1/z;
           // x and y projection, with rounding to pixels
           int xp = (int) (screen_width/2   + (K_1*x*ooz));
           int yp = (int) (screen_height/2  - (K_1*y*ooz));

           // calculate luminance є (-sqrt(2), sqrt(2))
           float luminance = cos_theta*cos_pi*sin_B - cos_A*sin_pi*cos_theta - sin_A*sin_theta +
                   cos_B*(cos_A*sin_theta - sin_pi*sin_A*cos_theta);
           if (luminance > 0) {     // point is in front of us
               if (ooz > zbuffer[xp][yp]) {     // point is closer
                   zbuffer[xp][yp] = ooz;
                   int luminance_index = luminance*8;     // є (0, 11)
                   output[xp][yp] = ".,-~:;=!*#$@"[luminance_index];
               }
           }
       }
   }
    printf("\x1b[H");
    for (int k = 0; k < abs(terminal_height-screen_height)/2; k++) { putchar('\n'); }

    for (int j = 0; j < screen_height; j++) {
        printf("%*s", abs(terminal_width - screen_width)/2, "");
        for (int i = 0; i < screen_width; i++) {
            putchar(output[i][j]);
        }
        putchar('\n');
    }
}

void close_properly(int signal_num) {
    // show cursor
    printf("\e[?25h");
    exit(0);
}

int main() {
    signal(SIGABRT, close_properly);
    signal(SIGTERM, close_properly);
    signal(SIGILL, close_properly);
    signal(SIGINT, close_properly);
    // hide cursor
    printf("\e[?25l");
    // clear terminal window
    system("clear");
    float A = -1.9;
    float B = -1.4;
    while (1) {
        render_frame(A, B);
        A += 0.007;
        B += 0.002;
    }
}
