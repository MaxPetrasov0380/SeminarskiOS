/* ============================================================
 * launcher.c
 *
 * Konzolna aplikacija koja:
 *   - skenira zadati folder i ucitava sve .bat fajlove u
 *     jednostruko spregnutu listu,
 *   - prikazuje dinamicki meni na osnovu te liste,
 *   - pokrece izabrani bat fajl i upisuje rezultat izvrsenja
 *     (vreme + naziv + uspeh/neuspeh) u log fajl,
 *   - omogucava citanje vec napisanog log fajla.
 *
 * Kompajliranje (MinGW / gcc na Windows-u):
 *   gcc launcher.c -o launcher.exe
 *
 * Napomena: FOLDER_BATCH odredjuje folder koji se skenira za
 * .bat fajlove (podrazumevano tekuci folder ".", odnosno folder
 * u kom se nalazi launcher.exe ako se odatle i pokrece).
 * ============================================================ */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <conio.h>   /* _getch() - Windows specific */
#include <windows.h>

#define FOLDER_BATCH   "."
#define LOG_FAJL       "log.txt"
#define MAX_NAZIV      256
#define MAX_PUTANJA    512

/* ---------- Jednostruko spregnuta lista bat fajlova ---------- */

typedef struct Cvor {
    char naziv[MAX_NAZIV];      /* npr. "ocisti_temp.bat" */
    char putanja[MAX_PUTANJA];  /* npr. ".\ocisti_temp.bat" */
    struct Cvor *sledeci;
} Cvor;

Cvor *glava = NULL;

/* Dodaje novi cvor na kraj liste (redosled = redosled skeniranja). */
void dodajCvor(const char *naziv, const char *putanja) {
    Cvor *novi = (Cvor *) malloc(sizeof(Cvor));
    if (!novi) {
        printf("[GRESKA] Neuspesna alokacija memorije.\n");
        exit(1);
    }

    strncpy(novi->naziv, naziv, MAX_NAZIV - 1);
    novi->naziv[MAX_NAZIV - 1] = '\0';

    strncpy(novi->putanja, putanja, MAX_PUTANJA - 1);
    novi->putanja[MAX_PUTANJA - 1] = '\0';

    novi->sledeci = NULL;

    if (glava == NULL) {
        glava = novi;
    } else {
        Cvor *tek = glava;
        while (tek->sledeci != NULL) {
            tek = tek->sledeci;
        }
        tek->sledeci = novi;
    }
}

/* Oslobadja celu listu iz memorije. */
void osloboditiListu(void) {
    Cvor *tek = glava;
    while (tek != NULL) {
        Cvor *sledeci = tek->sledeci;
        free(tek);
        tek = sledeci;
    }
    glava = NULL;
}

/* Skenira folder i puni listu svim pronadjenim .bat fajlovima. */
void ucitajBatFajlove(const char *folder) {
    char maska[MAX_PUTANJA];
    snprintf(maska, sizeof(maska), "%s\\*.bat", folder);

    WIN32_FIND_DATAA podaci;
    HANDLE hFind = FindFirstFileA(maska, &podaci);

    if (hFind == INVALID_HANDLE_VALUE) {
        return; /* nema .bat fajlova - meni ce to prijaviti korisniku */
    }

    do {
        if (!(podaci.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
            char putanja[MAX_PUTANJA];
            snprintf(putanja, sizeof(putanja), "%s\\%s", folder, podaci.cFileName);
            dodajCvor(podaci.cFileName, putanja);
        }
    } while (FindNextFileA(hFind, &podaci));

    FindClose(hFind);
}

/* ---------- Rad sa log fajlom (pisanje i citanje) ---------- */

/* Upisuje jedan red u log fajl: [vreme] naziv -> USPESNO/NEUSPESNO */
void upisiLog(const char *nazivBatFajla, int uspeh) {
    FILE *fp = fopen(LOG_FAJL, "a");
    if (!fp) {
        printf("[GRESKA] Ne mogu da otvorim log fajl za pisanje.\n");
        return;
    }

    time_t sada = time(NULL);
    struct tm *vreme = localtime(&sada);
    char vremenskaOznaka[32];
    strftime(vremenskaOznaka, sizeof(vremenskaOznaka), "%Y-%m-%d %H:%M:%S", vreme);

    fprintf(fp, "[%s] %s -> %s\n", vremenskaOznaka, nazivBatFajla,
            uspeh ? "USPESNO" : "NEUSPESNO");

    fclose(fp);
}

/* Ispisuje ceo sadrzaj log fajla na ekranu. */
void procitajLog(void) {
    system("cls");
    printf("========================================\n");
    printf("              SADRZAJ LOG FAJLA          \n");
    printf("========================================\n\n");

    FILE *fp = fopen(LOG_FAJL, "r");
    if (!fp) {
        printf("Log fajl jos ne postoji (nijedan bat fajl jos nije pokrenut).\n");
    } else {
        char linija[512];
        int ima = 0;
        while (fgets(linija, sizeof(linija), fp)) {
            printf("%s", linija);
            ima = 1;
        }
        if (!ima) {
            printf("Log fajl je prazan.\n");
        }
        fclose(fp);
    }

    printf("\nPritisni bilo koji taster za povratak u meni...");
    _getch();
}

/* ---------- Pokretanje bat fajla ---------- */

void pokreniBatch(const char *naziv, const char *putanja) {
    char komanda[600];
    /* start "" pokrece bat u novom prozoru i odmah vraca kontrolu */
    snprintf(komanda, sizeof(komanda), "start \"\" \"%s\"", putanja);

    int rezultat = system(komanda);
    int uspeh = (rezultat == 0);

    if (uspeh) {
        printf("\n[OK] Pokrenuto: %s\n", putanja);
    } else {
        printf("\n[GRESKA] Nije moguce pokrenuti: %s\n", putanja);
    }

    upisiLog(naziv, uspeh);
}

/* ---------- Meni ---------- */

void ispisiMeni(void) {
    system("cls");
    printf("========================================\n");
    printf("      LAUNCHER - BATCH KONTROLNI PANEL   \n");
    printf("========================================\n\n");

    if (glava == NULL) {
        printf("  (Nema pronadjenih .bat fajlova u folderu \"%s\")\n\n", FOLDER_BATCH);
    } else {
        int redniBroj = 1;
        Cvor *tek = glava;
        while (tek != NULL) {
            printf("  [%d] %s\n", redniBroj, tek->naziv);
            redniBroj++;
            tek = tek->sledeci;
        }
        printf("\n");
    }

    printf("  [L] Prikazi log fajl\n");
    printf("  [R] Ucitaj ponovo listu fajlova\n");
    printf("  [0] Izlaz\n\n");
    printf("Unesi izbor: ");
}

int main(void) {
    int izlaz = 0;

    ucitajBatFajlove(FOLDER_BATCH);

    while (!izlaz) {
        ispisiMeni();

        char unos[16];
        if (!fgets(unos, sizeof(unos), stdin)) {
            break;
        }
        unos[strcspn(unos, "\n")] = '\0';

        if (strlen(unos) == 0) {
            continue;
        }

        if (strcmp(unos, "0") == 0) {
            izlaz = 1;

        } else if (strcmp(unos, "L") == 0 || strcmp(unos, "l") == 0) {
            procitajLog();

        } else if (strcmp(unos, "R") == 0 || strcmp(unos, "r") == 0) {
            osloboditiListu();
            ucitajBatFajlove(FOLDER_BATCH);
            printf("\n[OK] Lista fajlova je ponovo ucitana.\n");
            printf("Pritisni Enter za nastavak...");
            getchar();

        } else {
            char *krajPtr;
            long izbor = strtol(unos, &krajPtr, 10);

            if (*krajPtr != '\0' || izbor <= 0) {
                printf("\nNepoznat izbor. Pritisni Enter za nastavak...");
                getchar();
                continue;
            }

            Cvor *tek = glava;
            long brojac = 1;
            while (tek != NULL && brojac < izbor) {
                tek = tek->sledeci;
                brojac++;
            }

            if (tek == NULL) {
                printf("\nNepostojeci broj fajla. Pritisni Enter za nastavak...");
                getchar();
            } else {
                pokreniBatch(tek->naziv, tek->putanja);
                printf("Pritisni Enter za nastavak...");
                getchar();
            }
        }
    }

    osloboditiListu();
    printf("\nIzlazak iz programa...\n");
    return 0;
}
