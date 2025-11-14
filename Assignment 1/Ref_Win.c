// resize_window.c

#define _XOPEN_SOURCE_EXTENDED
#include <locale.h>
#include <ncurses.h>

static void layout_and_draw(WINDOW *win) {
    int H, W;
    getmaxyx(stdscr, H, W);

    // Window with fixed margin
    int wh = (H > 6) ? H - 6 : H;
    int ww = (W > 10) ? W - 10 : W;
    if (wh < 3) wh = 3;
    if (ww < 3) ww = 3;

    // Resize and recenter window
    wresize(win, wh, ww);
    mvwin(win, (H - wh) / 2, (W - ww) / 2);

    // Clean up and draw again
    werase(stdscr);
    werase(win);
    box(win, 0, 0);
    mvprintw(0, 0, "Ridimensiona la shell; premi 'q' per uscire.");
    mvwprintw(win, 1, 2, "stdscr: %dx%d  win: %dx%d",
              H, W, wh, ww);

    refresh();
    wrefresh(win);
}

int main(void) {
    setlocale(LC_ALL, "");

    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);     // necesseary for KEY_RESIZE
    // (opz.) set_escdelay(25);

    // Create initial window (Dimensions TBM)
    WINDOW *win = newwin(3, 3, 0, 0);
    layout_and_draw(win);

    int ch;
    while ((ch = getch()) != 'q') {
        if (ch == KEY_RESIZE) {
            // Update ncurses internal structures for new dimensions
            resize_term(0, 0);          // o resizeterm(0, 0)
            layout_and_draw(win);       // calcullate layout and draw again
        }
        // ... other buttons ...
    }

    delwin(win);
    endwin();
    return 0;
}