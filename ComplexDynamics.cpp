#include <SDL2/SDL.h>
#include <complex>
using namespace std;
typedef complex<double> complexlf;

//Complex Dynamic System
double sqrt2 = sqrt(2);
const int MAX_PARAMS = 10;
complexlf params[MAX_PARAMS] = {{0,1}, {0,-1.5}, {-1,0}};
complexlf origin (0,0);
complexlf seed = origin;

const int MAX_ORBIT = 100;
complexlf orbit[MAX_ORBIT];

int maxIter = 50;

double infinity = 100;
double zero = 0.01;
int period = 0;
enum FatouType {DIVERGENT, CONVERGENT, INDETERMINATE} fatouType;

complexlf* ScreenInput = params + 0;
const int NUM_SLIDERS = 1;
complexlf* SliderInput[] = {params + 1, params + 2, &seed};
int SliderRadius = 5;


//camera
const int w = 800; 
const int h = 600;
unsigned char* screen;

double Cx = 0;
double Cy = 4;
double zoom = 10;

complexlf f(complexlf z) {
    return z*z - params[1]*tan(params[0]);
}


double Iterate()
{
    orbit[0] = seed;
    double dist;
    double maxDist = zero;
    double minDist = infinity;

    for (int i = 0; i < maxIter; i++) {
        orbit[i+1] = f(orbit[i]);

        dist = norm(orbit[i+1]);
        maxDist = dist > maxDist ? dist : maxDist;

        if (infinity < dist) {
            double a =   norm(orbit[i+1] - orbit[i]);
            double b = 2*real(orbit[i+1] * conj(orbit[i]));
            double c =   norm(orbit[i]) - infinity;

            double t = (-b + sqrt(b*b-4*a*c)) / (2*a);
            fatouType = DIVERGENT;
            return (t + i) / (double) maxIter;
        }
    }

    
    
    for (int i = 0; i < maxIter; i++)
    {
        dist = norm(orbit[maxIter] - orbit[(maxIter-1)-i]);
        minDist = dist < minDist ? dist : minDist;
        
        if (zero > dist)
        {
            period = i;
            fatouType = CONVERGENT;
            return dist/zero;
        }
    }

    fatouType = INDETERMINATE;
    return maxDist/infinity > zero/minDist ? maxDist/infinity : zero/minDist;
}



void ScreenToPlane(int x, int y, complexlf& z) {
    z = complexlf(
        Cx + zoom * (x - w/2) / (double) h,
        Cy + zoom * (y - h/2) / (double) h
    );
}

void PlaneToScreen(complexlf z, int& x, int& y) {
    x = h*(z.real() - Cx)/(zoom) + w/2.0;
    y = h*(z.imag() - Cy)/(zoom) + h/2.0;
}

void colorPurple(double value, unsigned char* rgb) {
    rgb[0] = 255*value;
    rgb[1] = 0;
    rgb[2] = 255*value;
}
void colorGreen(double value, unsigned char* rgb) {
    rgb[0] = (128 + 127*sin(sqrt2*period))*(1-value);
    rgb[1] = 255*(1-value);
    rgb[2] = (128 + 127*sin(period))*(1-value);
}
void colorConstDark(double value, unsigned char* rgb) {
    rgb[0] = 255*value;
    rgb[1] = 0;
    rgb[2] = 0;
}
void ColorScreen() {
    for (int i = 0; i < w*h; i++)
    {
        ScreenToPlane(i%w, i/w, *ScreenInput);
        double value = Iterate();

        switch (fatouType)
        {
        case DIVERGENT:
            colorPurple(value, screen + 3*i);
            break;
        case CONVERGENT:
            colorGreen(value, screen + 3*i);
            break;
        case INDETERMINATE:
            colorConstDark(value, screen + 3*i);
            break;
        default:
            break;
        }
        
    }
    
}

SDL_Rect GetSliderBox(complexlf z) {
    int centerx, centery;
    PlaneToScreen(z, centerx, centery);
    SDL_Rect SliderBox = {
        centerx - SliderRadius,
        centery - SliderRadius,
        SliderRadius*2,
        SliderRadius*2
    };
    return SliderBox;
}


int main(int argc, char **argv)
{
    SDL_Init(SDL_INIT_VIDEO);

    SDL_Window *window = SDL_CreateWindow(
        "Mandelbrot", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        w, 
        h,
        0
    );
    SDL_Renderer* renderer = SDL_CreateRenderer(
        window,
        -1,
        SDL_RENDERER_ACCELERATED
    );
    SDL_Texture* texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB24, SDL_TEXTUREACCESS_STREAMING, w, h);

    bool quit = SDL_FALSE;
    SDL_Event e;

    int pitch;
    int turbo = 1;
    int mousex, mousey;
    int mouseSelect = -1;

    const Uint8* keyboard = SDL_GetKeyboardState(NULL);

    while (!quit) 
    {
        SDL_GetMouseState(&mousex, &mousey);
        while (SDL_PollEvent(&e)) {
            switch(e.type) {
            case SDL_QUIT:
                quit = SDL_TRUE;    
                break;
            case SDL_KEYDOWN:
                break;
            case SDL_MOUSEBUTTONUP:
                mouseSelect = -1;
                break;
            case SDL_MOUSEBUTTONDOWN:
                for (int i = 0; i < NUM_SLIDERS; i++)
                {
                    const SDL_Point mousePoint = {(int) mousex, (int) mousey};
                    const SDL_Rect zeroRect = GetSliderBox(*SliderInput[i]);
                    if (SDL_PointInRect(&mousePoint, &zeroRect)) {
                        mouseSelect = i;
                    }
                }
                
                break;
            }
        }

        if (mouseSelect != -1)
        {
            ScreenToPlane(mousex, mousey, *SliderInput[mouseSelect]);
        }

        quit = quit || keyboard[SDL_SCANCODE_ESCAPE];
        turbo = 1 + 4*keyboard[SDL_SCANCODE_LSHIFT];
        Cx += 0.03*turbo*zoom*(keyboard[SDL_SCANCODE_D] - keyboard[SDL_SCANCODE_A]);    
        Cy += 0.03*turbo*zoom*(keyboard[SDL_SCANCODE_S] - keyboard[SDL_SCANCODE_W]);
        zoom *= 1 - 0.05*turbo*(keyboard[SDL_SCANCODE_EQUALS] - keyboard[SDL_SCANCODE_MINUS]);
        maxIter += turbo*(keyboard[SDL_SCANCODE_E] - keyboard[SDL_SCANCODE_Q]);
        maxIter = maxIter > 0 ? maxIter : 1;
        maxIter = maxIter < MAX_ORBIT ? maxIter : MAX_ORBIT -1;
        infinity *= 1 - 0.05*turbo*(keyboard[SDL_SCANCODE_Z] - keyboard[SDL_SCANCODE_X]);
        zero *= 1 - 0.05*turbo*(keyboard[SDL_SCANCODE_C] - keyboard[SDL_SCANCODE_V]);
        
        SDL_LockTexture(texture, NULL, (void**) &screen, &pitch);
        ColorScreen();
        SDL_UnlockTexture(texture);
        SDL_RenderCopy(renderer, texture, NULL, NULL);

        SDL_SetRenderDrawColor(renderer, 255,255,255,255);
        for (int i = 0; i < NUM_SLIDERS; i++)
        {
            const SDL_Rect SliderBox = GetSliderBox(*SliderInput[i]);
            SDL_RenderFillRect(renderer, &SliderBox);

        }

        SDL_RenderPresent(renderer);
    }

    SDL_DestroyTexture(texture);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);  //closes window
    SDL_Quit();
    return 0;
}