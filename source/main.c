#include <ogcsys.h>
#include <gccore.h>
#include <wiiuse/wpad.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

typedef unsigned char  u8;
typedef unsigned short u16;

// Función que calcula el número de iteraciones para un punto (cre, cim)
// en el conjunto de Mandelbrot, hasta alcanzar max_iter.
int mandelbrot(double cre, double cim, int max_iter) {
    double zr = 0.0, zi = 0.0;
    int iter = 0;
    while ((zr * zr + zi * zi < 4.0) && (iter < max_iter)) {
        double temp = zr * zr - zi * zi + cre;
        zi = 2.0 * zr * zi + cim;
        zr = temp;
        iter++;
    }
    return iter;
}

int main() {
    // Inicialización del sistema de video y del Wii Remote
    VIDEO_Init();
    WPAD_Init();

    // Obtiene el modo de video preferido (normalmente 640x480 para Wii)
    GXRModeObj *rmode = VIDEO_GetPreferredMode(NULL);
    // Reserva el framebuffer y convierte la dirección a K1 (acceso directo a memoria)
    void *frameBuffer = MEM_K0_TO_K1(SYS_AllocateFramebuffer(rmode));

    // Configura y activa el modo de video
    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(frameBuffer);
    VIDEO_SetBlack(FALSE);  // Asegura que la pantalla se encienda
    VIDEO_Flush();
    VIDEO_WaitVSync();
    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();

    // Determina las dimensiones de la pantalla
    int screen_width  = rmode->fbWidth;
    int screen_height = rmode->xfbHeight;
    u16 *fb = (u16*) frameBuffer;

    // Parámetros del fractal
    double center_x = -0.5, center_y = 0.0; // Centro inicial
    double scale = 4.0;                     // Controla el zoom
    int max_iter = 100;                     // Iteraciones máximas

    while (1) {
        // Actualiza el estado del Wii Remote
        WPAD_ScanPads();
        u32 buttons = WPAD_ButtonsDown(0);

        // Si se presiona el botón HOME, regresa al Homebrew Channel
        if (buttons & WPAD_BUTTON_HOME) {
            SYS_ResetSystem(SYS_RETURNTOMENU, 0, 0);
        }

        // Ajuste de iteraciones con A y B
        if (buttons & WPAD_BUTTON_A) {
            max_iter += 10;
        }
        if (buttons & WPAD_BUTTON_B) {
            if (max_iter > 10)
                max_iter -= 10;
        }

        // Control de desplazamiento con las flechas
        if (buttons & WPAD_BUTTON_UP) {
            center_y -= scale * 0.05;
        }
        if (buttons & WPAD_BUTTON_DOWN) {
            center_y += scale * 0.05;
        }
        if (buttons & WPAD_BUTTON_LEFT) {
            center_x -= scale * 0.05;
        }
        if (buttons & WPAD_BUTTON_RIGHT) {
            center_x += scale * 0.05;
        }

        // Control de zoom con PLUS y MINUS
        if (buttons & WPAD_BUTTON_PLUS) {
            scale *= 0.9;  // Acercar
        }
        if (buttons & WPAD_BUTTON_MINUS) {
            scale /= 0.9;  // Alejar
        }

        // Renderiza el conjunto de Mandelbrot
        for (int y = 0; y < screen_height; y++) {
            for (int x = 0; x < screen_width; x++) {
                double cre = (x - screen_width / 2.0) * scale / screen_width + center_x;
                double cim = (y - screen_height / 2.0) * scale / screen_width + center_y;
                int iter = mandelbrot(cre, cim, max_iter);

                u8 color = (u8)(255 * iter / max_iter);
                u16 pixel = ((color >> 3) << 11) | ((color >> 2) << 5) | (color >> 3);
                fb[y * screen_width + x] = pixel;
            }
        }

        // Actualiza el framebuffer y espera el siguiente VSync
        VIDEO_SetNextFramebuffer(frameBuffer);
        VIDEO_Flush();
        VIDEO_WaitVSync();
    }

    return 0;
}
