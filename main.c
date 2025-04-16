#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ncurses.h>
#include <unistd.h>

#define TAILLE 3
#define HAUTEUR 10
#define LARGEUR 20

typedef enum { HUMAIN, IA_FACILE, IA_MOYEN } TypeJoueur;

typedef struct {
    int x;
    int y;
} Coup;

char plateau[TAILLE][TAILLE];
WINDOW *win_plateau, *win_info;
int joueur_actuel = 0;
TypeJoueur types_joueurs[2] = {HUMAIN, HUMAIN};
int curseur_x = 0, curseur_y = 0;

void initialiser_plateau() {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            plateau[i][j] = ' ';
        }
    }
    curseur_x = curseur_y = 0;
}

void afficher_plateau() {
    werase(win_plateau);
    box(win_plateau, 0, 0);
    
    for (int i = 1; i < TAILLE; i++) {
        mvwaddch(win_plateau, i * HAUTEUR/TAILLE, 0, ACS_LTEE);
        mvwhline(win_plateau, i * HAUTEUR/TAILLE, 1, ACS_HLINE, LARGEUR-2);
        mvwaddch(win_plateau, i * HAUTEUR/TAILLE, LARGEUR-1, ACS_RTEE);
        
        mvwaddch(win_plateau, 0, i * LARGEUR/TAILLE, ACS_TTEE);
        mvwvline(win_plateau, 1, i * LARGEUR/TAILLE, ACS_VLINE, HAUTEUR-2);
        mvwaddch(win_plateau, HAUTEUR-1, i * LARGEUR/TAILLE, ACS_BTEE);
    }
    
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            int y = i * HAUTEUR/TAILLE + HAUTEUR/(TAILLE*2);
            int x = j * LARGEUR/TAILLE + LARGEUR/(TAILLE*2) - 1;
            mvwaddch(win_plateau, y, x, plateau[i][j]);
        }
    }
    
    int curseur_win_y = curseur_y * HAUTEUR/TAILLE + HAUTEUR/(TAILLE*2);
    int curseur_win_x = curseur_x * LARGEUR/TAILLE + LARGEUR/(TAILLE*2) - 1;
    wattron(win_plateau, A_REVERSE);
    mvwaddch(win_plateau, curseur_win_y, curseur_win_x, plateau[curseur_y][curseur_x]);
    wattroff(win_plateau, A_REVERSE);
    
    wrefresh(win_plateau);
}

void afficher_message(const char *msg) {
    werase(win_info);
    mvwprintw(win_info, 0, 0, "%s", msg);
    wrefresh(win_info);
}

int est_coup_valide(int ligne, int colonne) {
    return (ligne >= 0 && ligne < TAILLE && colonne >= 0 && colonne < TAILLE && plateau[ligne][colonne] == ' ');
}

int est_plein() {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (plateau[i][j] == ' ') return 0;
        }
    }
    return 1;
}

int verifier_victoire(char symbole) {
    for (int i = 0; i < TAILLE; i++) {
        if ((plateau[i][0] == symbole && plateau[i][1] == symbole && plateau[i][2] == symbole) ||
            (plateau[0][i] == symbole && plateau[1][i] == symbole && plateau[2][i] == symbole)) {
            return 1;
        }
    }
    
    if ((plateau[0][0] == symbole && plateau[1][1] == symbole && plateau[2][2] == symbole) ||
        (plateau[0][2] == symbole && plateau[1][1] == symbole && plateau[2][0] == symbole)) {
        return 1;
    }
    
    return 0;
}

Coup coup_ia_facile() {
    Coup coup;
    do {
        coup.x = rand() % TAILLE;
        coup.y = rand() % TAILLE;
    } while (!est_coup_valide(coup.y, coup.x));
    return coup;
}

Coup coup_ia_moyen(char symbole) {
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (est_coup_valide(i, j)) {
                plateau[i][j] = symbole;
                if (verifier_victoire(symbole)) {
                    plateau[i][j] = ' ';
                    return (Coup){j, i};
                }
                plateau[i][j] = ' ';
            }
        }
    }
    
    char adversaire = (symbole == 'X') ? 'O' : 'X';
    for (int i = 0; i < TAILLE; i++) {
        for (int j = 0; j < TAILLE; j++) {
            if (est_coup_valide(i, j)) {
                plateau[i][j] = adversaire;
                if (verifier_victoire(adversaire)) {
                    plateau[i][j] = ' ';
                    return (Coup){j, i};
                }
                plateau[i][j] = ' ';
            }
        }
    }
    
    if (est_coup_valide(1, 1)) return (Coup){1, 1};
    
    return coup_ia_facile();
}

void jouer_coup(int ligne, int colonne) {
    plateau[ligne][colonne] = (joueur_actuel == 0) ? 'X' : 'O';
}

void changer_joueur() {
    joueur_actuel = 1 - joueur_actuel;
}

void jouer_partie() {
    initialiser_plateau();
    joueur_actuel = 0;
    keypad(win_plateau, TRUE);
    
    while (1) {
        afficher_plateau();
        
        char symbole = (joueur_actuel == 0) ? 'X' : 'O';
        char msg[100];
        snprintf(msg, sizeof(msg), "Joueur %d (%c) - %s", joueur_actuel+1, symbole, 
                types_joueurs[joueur_actuel] == HUMAIN ? "Utilisez les fleches + Entree" : 
                types_joueurs[joueur_actuel] == IA_FACILE ? "IA facile reflechit..." : "IA moyenne reflechit...");
        afficher_message(msg);
        
        Coup coup;
        if (types_joueurs[joueur_actuel] == HUMAIN) {
            int ch;
            while (1) {
                ch = wgetch(win_plateau);
                
                if (ch == KEY_LEFT) {
                    curseur_x = (curseur_x - 1 + TAILLE) % TAILLE;
                } else if (ch == KEY_RIGHT) {
                    curseur_x = (curseur_x + 1) % TAILLE;
                } else if (ch == KEY_UP) {
                    curseur_y = (curseur_y - 1 + TAILLE) % TAILLE;
                } else if (ch == KEY_DOWN) {
                    curseur_y = (curseur_y + 1) % TAILLE;
                } else if (ch == '\n' || ch == KEY_ENTER) {
                    if (est_coup_valide(curseur_y, curseur_x)) {
                        coup.x = curseur_x;
                        coup.y = curseur_y;
                        break;
                    }
                }
                
                afficher_plateau();
            }
        } else {
            if (types_joueurs[joueur_actuel] == IA_FACILE) {
                coup = coup_ia_facile();
            } else {
                coup = coup_ia_moyen(symbole);
            }
            usleep(500000);
            
            curseur_x = coup.x;
            curseur_y = coup.y;
        }
        
        jouer_coup(coup.y, coup.x);
        
        if (verifier_victoire(symbole)) {
            afficher_plateau();
            snprintf(msg, sizeof(msg), "Joueur %d (%c) a gagne!", joueur_actuel+1, symbole);
            afficher_message(msg);
            getch();
            break;
        }
        
        if (est_plein()) {
            afficher_plateau();
            afficher_message("Match nul!");
            getch();
            break;
        }
        
        changer_joueur();
    }
    
    keypad(win_plateau, FALSE);
}

void menu_principal() {
    int choix = 0;
    char *options[] = {
        "Joueur vs Joueur",
        "Joueur vs IA (facile)",
        "Joueur vs IA (moyen)",
        "IA (facile) vs IA (moyen)",
        "IA (moyen) vs IA (facile)",
        "Quitter"
    };
    int nb_options = sizeof(options)/sizeof(options[0]);
    
    while (1) {
        werase(win_info);
        mvwprintw(win_info, 0, 0, "MORPION - Choisissez un mode:");
        
        for (int i = 0; i < nb_options; i++) {
            if (i == choix) wattron(win_info, A_REVERSE);
            mvwprintw(win_info, i+2, 2, "%s", options[i]);
            if (i == choix) wattroff(win_info, A_REVERSE);
        }
        
        wrefresh(win_info);
        
        int ch = getch();
        switch (ch) {
            case KEY_UP: choix = (choix - 1 + nb_options) % nb_options; break;
            case KEY_DOWN: choix = (choix + 1) % nb_options; break;
            case '\n':
                if (choix == nb_options - 1) return;
                
                switch (choix) {
                    case 0: types_joueurs[0] = HUMAIN; types_joueurs[1] = HUMAIN; break;
                    case 1: types_joueurs[0] = HUMAIN; types_joueurs[1] = IA_FACILE; break;
                    case 2: types_joueurs[0] = HUMAIN; types_joueurs[1] = IA_MOYEN; break;
                    case 3: types_joueurs[0] = IA_FACILE; types_joueurs[1] = IA_MOYEN; break;
                    case 4: types_joueurs[0] = IA_MOYEN; types_joueurs[1] = IA_FACILE; break;
                }
                
                jouer_partie();
                break;
        }
    }
}

int main() {
    srand(time(NULL));
    initscr();
    cbreak();
    noecho();
    keypad(stdscr, TRUE);
    curs_set(0);
    
    int hauteur, largeur;
    getmaxyx(stdscr, hauteur, largeur);
    
    win_plateau = newwin(HAUTEUR, LARGEUR, (hauteur-HAUTEUR)/2, (largeur-LARGEUR)/2);
    win_info = newwin(10, LARGEUR, (hauteur-HAUTEUR)/2 + HAUTEUR + 1, (largeur-LARGEUR)/2);
    
    menu_principal();
    
    delwin(win_plateau);
    delwin(win_info);
    endwin();
    
    return 0;
}
