#include <Windows.h>
#include <windowsx.h>
#include <vector>
#include <list>
#include <random>
#include <ctime>

const int WIDTH = 40;
const int HEIGHT = 40;
const int CELL_SIZE = 20;

struct
{
    bool isPaused = false;
    int speed = 500;
    int countUpdates = 0;
} GameState;

std::vector<std::vector<bool>> grid(WIDTH, std::vector<bool>(HEIGHT, false));

const char* CLASS_NAME = "GameOfLifeWindow";

void UpdateGrid()
{
    std::vector<std::vector<bool>> newGrid(WIDTH, std::vector<bool>(HEIGHT, false));

    GameState.countUpdates = 0;

    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            int liveNeighbors = 0;

            for (int dx = -1; dx <= 1; ++dx)
            {
                for (int dy = -1; dy <= 1; ++dy)
                {
                    if (dx == 0 && dy == 0)
                        continue;

                    int nx = (x + dx + WIDTH) % WIDTH;
                    int ny = (y + dy + HEIGHT) % HEIGHT;

                    if (grid[nx][ny])
                        liveNeighbors++;
                }
            }
            
            if (grid[x][y])
            {
                if (liveNeighbors == 2 || liveNeighbors == 3)
                    newGrid[x][y] = true;
                GameState.countUpdates++;
            }
            else
            {
                if (liveNeighbors == 3)
                    newGrid[x][y] = true;
                GameState.countUpdates++;
            }
        }
    }

    grid = std::move(newGrid);
}

void InitField()
{
    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            grid[x][y] = (rand() % 50) % 2 == 0;
        }
    }
}

void DrawGrid(HDC hdc)
{
    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            if (grid[x][y])
            {
                RECT rect{ x * CELL_SIZE, y * CELL_SIZE, (x + 1) * CELL_SIZE, (y + 1) * CELL_SIZE };
                FillRect(hdc, &rect, (HBRUSH)GetStockObject(BLACK_BRUSH));
            }
        }
    }
}

void ClearScreen(HDC hdc, HWND hwnd)
{
    RECT rect;
    GetClientRect(hwnd, &rect);

    HBRUSH hBrush = CreateSolidBrush(RGB(255, 255, 255)); 
    FillRect(hdc, &rect, hBrush);
    DeleteObject(hBrush);
}

bool HasPeriodicConfiguration()
{
    static std::list<std::vector<std::vector<bool>>> previousStates;

    for (const auto& prevState : previousStates)
    {
        bool match = true;
        for (int x = 0; x < WIDTH; ++x)
        {
            for (int y = 0; y < HEIGHT; ++y)
            {
                if (grid[x][y] != prevState[x][y])
                {
                    match = false;
                    break;
                }
            }

            if (!match)
                break;
        }

        if (match)
            return true;
    }

    previousStates.push_back(grid);

    if (previousStates.size() > 5)
        previousStates.pop_front();

    return false;
}


bool HasStaticConfiguration()
{
    if (GameState.countUpdates != 0) {
        return false;
    }
    return true;
}


bool HasOneCellAlive()
{
    for (int x = 0; x < WIDTH; ++x)
    {
        for (int y = 0; y < HEIGHT; ++y)
        {
            if (grid[x][y])
                return true;
        }
    }
    return false;
}


bool IsGameOver()
{
    if (!HasOneCellAlive() || HasPeriodicConfiguration() || HasStaticConfiguration())
        return true;
    return false;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);

            ClearScreen(hdc, hwnd);

            DrawGrid(hdc);

            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_TIMER:
        {
            UpdateGrid();
            if (IsGameOver()) {
                KillTimer(hwnd, 1);
                MessageBox(hwnd, "Game over!", "Game over!", MB_OK);
            }

            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
        case WM_DESTROY:
        {
            PostQuitMessage(0);
            return 0;
        }
        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_SPACE:
                    GameState.isPaused = !GameState.isPaused;
                    if (GameState.isPaused)
                        KillTimer(hwnd, 1);
                    else
                        SetTimer(hwnd, 1, speed, NULL);
                    break;
                case 'R':
                    InitField();
                    break;
                case VK_UP:
                    GameState.speed -= 100;
                    if (GameState.speed < 100)
                        GameState.speed = 100;

                    SetTimer(hwnd, 1, speed, NULL);
                    break;
                case VK_DOWN:
                    GameState.speed += 100;
                    if (GameState.speed > 10000)
                        GameState.speed = 10000;
                    break;
            }
            return 0;
        }
        case WM_LBUTTONDOWN:
        {
            int xPos = GET_X_LPARAM(lParam);
            int yPos = GET_Y_LPARAM(lParam);
            int xCell = xPos / CELL_SIZE;
            int yCell = yPos / CELL_SIZE;

            grid[xCell][yCell] = !grid[xCell][yCell];

            
            InvalidateRect(hwnd, nullptr, FALSE);
            return 0;
        }
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    srand(time(NULL));
    
    WNDCLASS wc{};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
            0,                        
            CLASS_NAME,               
            "Game of Life",           
            WS_OVERLAPPEDWINDOW,      

            CW_USEDEFAULT, CW_USEDEFAULT, WIDTH * CELL_SIZE, HEIGHT * CELL_SIZE,

            NULL,                     
            NULL,                     
            hInstance,                       
            NULL                      
    );

    if (hwnd == NULL)
        return 0;

    ShowWindow(hwnd, nCmdShow);

    InitField();

    SetTimer(hwnd, 1, GameState.speed, NULL);

    MSG msg{};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}